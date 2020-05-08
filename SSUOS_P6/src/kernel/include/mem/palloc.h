#ifndef __PALLOC_H__
#define __PALLOC_H__

#include <ssulib.h>
#include <synch.h>

enum palloc_flags
{
	kernel_area= 0,	// kernel page
	user_area = 1,  // user page
};

void init_palloc (void);

void *palloc_get_one_page(enum palloc_flags flags); // modified

void *palloc_get_multiple_page(enum palloc_flags flags,size_t page_cnt); // modified

void palloc_free_one_page(void * pages); // modified

void palloc_free_multiple_page(void * pages, size_t page_cnt); // modified

void palloc_pf_test(void);

void swap_test(void);

#endif /* palloc.h */

