#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <type.h>

/* Bitmap abstract data type. */

/* Creation and destruction. */
struct bitmap *create_bitmap (size_t bit_count, void *, size_t byte_count);
size_t bitmap_struct_size (size_t bit_count);

/* Bitmap size. */
size_t bitmap_size (const struct bitmap *);

/* Setting and testing single bits. */
void set_bitmap (struct bitmap *, size_t index, bool);
void or_bitmap (struct bitmap *, size_t index);
void and_bitmap (struct bitmap *, size_t index);
void xor_bitmap (struct bitmap *, size_t index);
bool test_bitmap (const struct bitmap *, size_t index);

/* Setting and testing multiple bits. */
void set_all_bitmap (struct bitmap *, bool);
void set_multi_bitmap (struct bitmap *, size_t from, size_t count, bool);
size_t bitmap_count (const struct bitmap *, size_t from, size_t count, bool);
bool bitmap_contains (const struct bitmap *, size_t from, size_t count, bool);
bool bitmap_any (const struct bitmap *, size_t from, size_t count);
bool bitmap_none (const struct bitmap *, size_t from, size_t count);
bool bitmap_all (const struct bitmap *, size_t from, size_t count);

/* Finding set or unset bits. */
#define BITMAP_ERROR SIZE_MAX
size_t find_bitmap (const struct bitmap *, size_t from, size_t count, bool);
size_t find_set_bitmap (struct bitmap *, size_t from, size_t count, bool);

#endif /* bitmap.h */
