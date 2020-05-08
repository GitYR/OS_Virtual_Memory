#include <bitmap.h>
#include <limits.h>
#include <round.h>

/* 						<< unit_type >> 								*/

// 적어도 int형과 같은 크기를 가진 부호 없는 정수형이다.
// 각 비트는 bitmap에서의 한 비트를 나타낸다.
// 만약, unit에 있는 bit가 0이라는 의미가 비트맵에서의 k번째 비트를 말한다면
// unit에 있는 bit가 1이라는 의미는 비트맵에서의 k+1번째 비트를 말한다.

typedef unsigned long unit_type; // 4 bytes

/* Number of bits in an unit. */
// CHAR_BIT : 8
// BIT_NUM : 32

#define BIT_NUM (sizeof (unit_type) * CHAR_BIT)

// 외적으로 볼 때, bitmap은 비트들에 대한 배열이다.
// 내적으로 볼 때, bitmap은 (위에 정의되어 있는)unit_type에 대한 배열이다.
// unit_type은 비트들의 배열인 것처럼 사용된다.


/*
bitmap은 비트연산을 이용하여 한 자료형내에 0과 1을 기록함.
unsigned long(=unit_type)형에 기록한다면 32개의 비트가 있으니 32개의 원소를 기록할 수 있음.
물론 32개만 저장할 수는 없으니 비트맵은 unsigned long의 배열의 포인터를 갖는다. 
32개 이상을 기록하기 위해서는 여러 개의 unsigned long 변수 중에 어디에 기록해야 할지 정해야 함.
이 때 사용되는것이 unit_index. 몇번째 unsigned long 자료형에 값을 저장할지 대입하는 것
반대로 자료형 내에서 몇번째 비트가 해당 인덱스를 표시하는지 알아보는 함수는 bit_mask().
*/

struct bitmap
{
	size_t bit_count;     // 비트의 개수 - page frame을 할당할 수 있는 개수
	unit_type *bits;      // 비트를 나타내는 단위
};

/******************************************************************************/

/* Returns the index of the unit that contains the bit numbered BIT_INDEX. */
static inline size_t unit_index(size_t bit_index) 
{
	return bit_index / BIT_NUM; 
}

/* Returns an unit_type where only the bit corresponding to BIT_INDEX is turned on. */
static inline unit_type bit_mask(size_t bit_index) 
{
	return (unit_type) 1 << (bit_index % BIT_NUM);
}

/* Returns the number of units required for BIT_COUNT bits. */
static inline size_t unit_count(size_t bit_count)
{
	return DIV_ROUND_UP(bit_count, BIT_NUM);
}

/* Returns the number of bytes required for BIT_COUNT bits. */
static inline size_t byte_count(size_t bit_count)
{
	return sizeof (unit_type) * unit_count(bit_count);
}

/* 
   Returns a bit mask in which the bits actually used in the last
   unit of B's bits are set to 1 and the rest are set to 0. 
*/
static inline unit_type end_mask(const struct bitmap *b) 
{
	int last_bits = b->bit_count % BIT_NUM;

	return last_bits ? ((unit_type) 1 << last_bits) - 1 : (unit_type) -1;
}


/******************************************************************************/

// 생성 및 삭제

/* 
   Creates and returns a bitmap with BIT_COUNT bits in the
   BLOCK_SIZE bytes of storage pre-allocated at BLOCK.
   BLOCK_SIZE must be at least bitmap_needed_bytes(BIT_COUNT). 
*/

// BLOCK에 미리 할당된 저장공간의 'BLOCK_SIZE'바이트안에 있는 'BIT_COUNT' 비트들을 가지고 있는
// bitmap을 생성하고 리턴한다.
// 'BLOCK_SIZE'는 적어도 'BIT_COUNT'만큼의 바이트이어야 한다.

struct bitmap * create_bitmap(size_t bit_count, void *block, size_t block_size)
{
	struct bitmap *b = block;

	b->bit_count = bit_count;
	b->bits = (unit_type *)(b + 1);
	set_all_bitmap(b, false);
	return b;
}

/* 
   Returns the number of bytes required to accomodate a bitmap
   with BIT_COUNT bits (for use with create_bitmap()). 
*/

size_t bitmap_struct_size(size_t bit_count) 
{
	return sizeof (struct bitmap) + byte_count(bit_count);
}

/******************************************************************************/

/* Bitmap size. */

/* Returns the number of bits in B. */
size_t bitmap_size(const struct bitmap *b)
{
	return b->bit_count;
}

/******************************************************************************/

/* Setting and testing single bits. */

/* Atomically sets the bit numbered INDEX in B to VALUE. */
// B에서 비트 번호 INDEX를 원자적(분리할 수 없음)으로 VALUE로 설정합니다.
void set_bitmap(struct bitmap *b, size_t index, bool value) 
{
	if(value)
		or_bitmap (b, index);
	else
		and_bitmap (b, index);
}

/* Atomically sets the bit numbered BIT_INDEX in B to true. */
// B에서 비트 번호가 지정된 BIT_INDEX를 원자적으로 1값으로 설정합니다.
void or_bitmap(struct bitmap *b, size_t bit_index) 
{
	size_t index = unit_index(bit_index);
	unit_type mask = bit_mask(bit_index);

	/* 
	   This is equivalent to `b->bits[index] |= mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the OR instruction in [IA32-v2b]. 
	*/
	b->bits[index] |= mask;
}

/* Atomically sets the bit numbered BIT_INDEX in B to false. */
// B에서 비트 번호가 지정된 BIT_INDEX를 원자적으로 0값으로 설정합니다.
void and_bitmap(struct bitmap *b, size_t bit_index) 
{
	size_t index = unit_index(bit_index);
	unit_type mask = bit_mask(bit_index);

	/*
	   This is equivalent to `b->bits[index] &= ~mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the AND instruction in [IA32-v2a]. 
	*/
	b->bits[index] &= ~mask;
}

/* 
   Atomically toggles the bit numbered INDEX in B;
   that is, if it is true, makes it false,
   and if it is false, makes it true. 
*/
// 말그대로 XOR 연산
void xor_bitmap(struct bitmap *b, size_t bit_index) 
{
	size_t index = unit_index (bit_index);
	unit_type mask = bit_mask (bit_index);

	/* This is equivalent to `b->bits[index] ^= mask' except that it
	   is guaranteed to be atomic on a uniprocessor machine.  See
	   the description of the XOR instruction in [IA32-v2b]. */
	b->bits[index] ^= mask;
}

/* Returns the value of the bit numbered INDEX in B. */
bool test_bitmap(const struct bitmap *b, size_t index) 
{
	return (b->bits[unit_index(index)] & bit_mask(index)) != 0;
	// bits의 unit_index번째 32비트 중 index번 비트가 1이면 1을 리턴
	// bits의 unit_index번째 32비트 중 index번 비트가 0이면 0을 리턴
}

/******************************************************************************/

/* Setting and testing multiple bits. */

/* Sets all bits in B to VALUE. */
void set_all_bitmap(struct bitmap *b, bool value) 
{
	// bitmap_size() -> returns bit_count which is the member of 'b'
	set_multi_bitmap(b, 0, bitmap_size(b), value);
}

/* Sets the COUNT bits starting at FROM in B to VALUE. */
void set_multi_bitmap(struct bitmap *b, size_t from, size_t count, bool value) 
{
	size_t i;

	for(i = 0; i < count; i++)
		set_bitmap(b, from + i, value);
}

/*
   Returns the number of bits in B between FROM and FROM + COUNT,
   exclusive, that are set to VALUE. 
*/
// FROM부터 FROM+count까지의 비트들 중 비트가 value와 같은 개수를 리턴함.
size_t bitmap_count(const struct bitmap *b, size_t from, size_t count, bool value) 
{
	size_t i, value_count;

	value_count = 0;
	for(i = 0; i < count; i++)
		if(test_bitmap(b, from + i) == value)
			value_count++;
	return value_count;
}

/*
   Returns true if any bits in B between FROM and FROM + COUNT,
   exclusive, are set to VALUE, and false otherwise. 
*/
// FROM부터 FROM+count까지의 비트들 중 하나의 비트라도 value와 같으면 true를 리턴함.
bool bitmap_contains(const struct bitmap *b, size_t from, size_t count, bool value) 
{
	size_t i;

	for (i = 0; i < count; i++)
		if(test_bitmap(b, from + i) == value)
			return true;
	return false;
}

/* 
   Returns true if any bits in B between FROM and FROM + COUNT,
   exclusive, are set to true, and false otherwise.
*/
// FROM부터 FROM+count까지의 비트들 중 어느 한 비트라도 1이면 true를 리턴함.
bool bitmap_any(const struct bitmap *b, size_t from, size_t count) 
{
	return bitmap_contains(b, from, count, true);
}

/*
   Returns true if no bits in B between FROM and FROM + COUNT,
   exclusive, are set to true, and false otherwise.
*/

// FROM부터 FROM+count까지의 비트들 중 어느 한 비트라도 1이면 false를 리턴함.
bool bitmap_none(const struct bitmap *b, size_t from, size_t count) 
{
	return !bitmap_contains(b, from, count, true);
}

/*
   Returns true if every bit in B between FROM and FROM + COUNT,
   exclusive, is set to true, and false otherwise. 
*/

// FROM부터 FROM+count까지의 비트들 중 어느 한 비트라도 0이면 false를 리턴함.
bool bitmap_all(const struct bitmap *b, size_t from, size_t count) 
{
	return !bitmap_contains(b, from, count, false);
}

/******************************************************************************/

/* Finding set or unset bits. */

/* 
   Finds and returns the starting index of the first group of COUNT
   consecutive bits in B at or after FROM that are all set to
   VALUE.
   If there is no such group, returns BITMAP_ERROR. 
*/

size_t find_bitmap(const struct bitmap *b, size_t from, size_t count, bool value) 
{
	if(count <= b->bit_count) 
	{
		size_t last = b->bit_count - count; // last = b의 총 비트 수 - count
		size_t i;

		for (i = from; i <= last; i++) // from부터 last까지 loop
			// from부터 from+count까지 비트들 중 어느 하나라도 !value와 같으면 false
			// if문이 true가 되기 위해선 비트들 중 어느 한 비트가 value와 같아야함.
			if(!bitmap_contains(b,i, count, !value)) 
				return i; 
	}
	return BITMAP_ERROR;
}

/* 
   Finds the first group of COUNT consecutive bits in B at or after
   FROM that are all set to VALUE, flips them all to !VALUE,
   and returns the index of the first bit in the group.
   If there is no such group, returns BITMAP_ERROR.
   If COUNT is zero, returns 0.
   Bits are set atomically, but testing bits is not atomic with
   setting them. 
*/

size_t find_set_bitmap(struct bitmap *b, size_t from, size_t count, bool value)
{
	size_t index = find_bitmap(b, from, count, value);

	if (index != BITMAP_ERROR) 
		set_multi_bitmap(b, index, count, !value);
	return index;
}
