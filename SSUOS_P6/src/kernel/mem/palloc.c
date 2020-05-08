#include <mem/palloc.h>
#include <bitmap.h>
#include <type.h>
#include <round.h>
#include <mem/mm.h>
#include <synch.h>
#include <device/console.h>
#include <mem/paging.h>
#include <mem/swap.h>

#define KERNEL_BITS 736 // bitmap이 쓰는 page 크기 제외
#define USER_BITS 7200  // bitmap이 쓰는 page 크기 제외

/* 
   Page allocator.  Hands out memory in page-size (or
   page-multiple) chunks.  
*/

/* pool for memory */
struct memory_pool
{
	struct lock lock;                   
	struct bitmap *bitmap; 
	uint32_t *addr;                    
};

/* kernel heap page struct */
struct khpage
{
	uint16_t page_type;
	uint16_t nalloc;
	uint32_t used_bit[4];
	struct khpage *next;
};

/* free list */
struct freelist
{
	struct khpage *list;
	int nfree;
};

// values for virtual memory
static struct khpage *khpage_list;
static struct freelist freelist;
static uint32_t page_alloc_index;

struct memory_pool kernel_pool,user_pool;

/* Initializes the page allocator. */
void init_palloc (void) 
{
	// bitmap address initializing
	kernel_pool.addr = (uint32_t *)KERNEL_ADDR;
	user_pool.addr = (uint32_t *)USER_POOL_START;

	// bitmap creating
	kernel_pool.bitmap = create_bitmap(KERNEL_BITS,(void *)KERNEL_ADDR,PAGE_SIZE);
	user_pool.bitmap = create_bitmap(USER_BITS,(void *)USER_POOL_START,PAGE_SIZE);

	// the other initializing - lock struct
	memset((struct lock *)&kernel_pool.lock,0,sizeof(struct lock));
	memset((struct lock *)&user_pool.lock,0,sizeof(struct lock));

	// 첫 번째 page는 bitmap이 사용하므로
	or_bitmap(kernel_pool.bitmap,0);
	or_bitmap(user_pool.bitmap,0);
}

/* Obtains and returns a group of PAGE_CNT contiguous free pages. */
void *palloc_get_multiple_page(enum palloc_flags flags,size_t page_cnt)
{
	int i;
	struct memory_pool *pool;
	void * pages = NULL;
	size_t from,page_idx,bit_count;

	if(page_cnt <= 0) // error handling
		return (void *)NULL;

	if(flags == kernel_area) // kernel?
		pool = &kernel_pool;
	else // user?
		pool = &user_pool;

	bit_count = bitmap_size(pool->bitmap);
	for(i = 0; i < bit_count; i++)
	{
		if(test_bitmap(pool->bitmap,i) == 0) // find the starting index
		{
			from = i;
			break;
		}
	}
	page_idx = find_set_bitmap(pool->bitmap,from,page_cnt,false); // get a page index

	if(flags == kernel_area)
		pages = (void *)(KERNEL_ADDR + PAGE_SIZE * page_idx);
	else
		pages = (void *)(USER_POOL_START + PAGE_SIZE * page_idx);

	if(pages != NULL)
		memset((void *)pages,0,page_cnt*PAGE_SIZE);

	return (void *)pages;
}

/* Obtains a single free page and returns its address. */
void *palloc_get_one_page(enum palloc_flags flags) 
{
	return palloc_get_multiple_page(flags,1);
}

/* Frees the PAGE_CNT pages starting at PAGES. */
void palloc_free_multiple_page(void * pages,size_t page_cnt) 
{
	int i;
	void * addr;
	size_t from,bit_count;
	struct memory_pool * pool;
	int test;

	if(pages == NULL || page_cnt <= 0) // error handling
		return (void)NULL;

	if((void *)KERNEL_ADDR <= pages && pages < (void *)USER_POOL_START) // kernel
	{
		pool = &kernel_pool;
		from = (size_t)((uint32_t)(pages - KERNEL_ADDR) / PAGE_SIZE);
		test = 0;
	}
	if((void *)USER_POOL_START <= pages && pages < (void *)RKERNEL_HEAP_START) // user
	{
		pool = &user_pool;
		from = (size_t)((uint32_t)(pages - USER_POOL_START) / PAGE_SIZE);
		test = 1;
	}

	for(i = from; i < from+page_cnt; i++)
		set_bitmap(pool->bitmap,i,false); // bitmap의 bit변경
}

/* Frees the page at PAGE. */
void palloc_free_one_page(void * pages) 
{
	palloc_free_multiple_page(pages,1);
}

void palloc_pf_test(void)
{
	uint32_t *one_page1 = palloc_get_one_page(user_area);
	uint32_t *one_page2 = palloc_get_one_page(user_area);
	uint32_t *two_page1 = palloc_get_multiple_page(user_area,2);
	uint32_t *three_page;

	printk("one_page1 = %x\n", one_page1); 
	printk("one_page2 = %x\n", one_page2); 
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_one_page(one_page1);
	palloc_free_one_page(one_page2);
	palloc_free_multiple_page(two_page1,2);

	one_page1 = palloc_get_one_page(user_area);
	one_page2 = palloc_get_one_page(user_area);
	two_page1 = palloc_get_multiple_page(user_area,2);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_multiple_page(one_page2, 3);
	one_page2 = palloc_get_one_page(user_area);
	three_page = palloc_get_multiple_page(user_area,3);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("three_page = %x\n", three_page);

	palloc_free_one_page(one_page1);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	palloc_free_one_page(one_page2);
}
