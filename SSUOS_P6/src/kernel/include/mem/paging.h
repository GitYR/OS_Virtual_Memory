#ifndef __PAGING_H__
#define __PAGING_H__

#include <type.h>
#include <proc/proc.h>

#define CR0_FLAG_PG			0x80000000		// Paging Flag
#define CR0_FLAG_PE			0x00000001		// Protect Mode Flag

#define PAGE_FLAG_PRESENT	0x01			// Present
#define PAGE_FLAG_RW		0x02			// Read & Write
#define PAGE_FLAG_USER		0x04			// User level
#define PAGE_FLAG_WRITE     0x08            // Write-through
#define PAGE_FLAG_CACHE     0x10            // Cache disabled
#define PAGE_FLAG_ACCESS    0x20            // Accessed
#define PAGE_FLAG_AVAIL     0x40            // Available
#define PAGE_FLAG_DIRTY     0x40            // Dirty
#define PAGE_FLAG_SIZE      0x80            // Page size (0 indicates 4 KBytes)
#define PAGE_FLAG_PTA       0x80            // Page Table Attribute Index
#define PAGE_FLAG_GLOB      0x100           // Global page
#define PAGE_ADDR_AVAIL     0x200           // Available for system progammer's use
#define PAGE_ADDR_BASE      0x1000          //

#define PAGE_MASK_BASE      0xFFFFF000      // Base address from PD/PT entry
#define PAGE_MASK_PDE       0xFFC00000      // PDE index from address
#define PAGE_MASK_PTE       0x003FF000      // PTE index from address
#define PAGE_OFFSET_PDE	    22				// offset to PDE index from address
#define PAGE_OFFSET_PTE	    12				// offset to PTE index from address

#define PAGE_SIZE			4096			// 4kb Page
#define PAGE_TBL_SIZE		4096			// 4kb Page Table
#define PAGE_DIR_SIZE		4096			// 4kb Page Directory

#define KERNEL_ADDR			0x00100000      // 커널 소스코드

#define RKERNEL_HEAP_START	0x02000000	// 실제 메모리( pool 구성시 수정할 것 )
#define VKERNEL_HEAP_START	0xc0000000  // 가상 메모리 페이지 할당 시작 주소
#define USER_POOL_START		0x3e0000

#define VH_TO_RH(addr) (uint32_t *)((uint32_t)addr - VKERNEL_HEAP_START + RKERNEL_HEAP_START)
#define RH_TO_VH(addr) (uint32_t *)((uint32_t)addr - RKERNEL_HEAP_START + VKERNEL_HEAP_START)

//uint32_t *pd_create(pid_t pid);

static inline uint32_t pg_no(const uint32_t *page)
{ 
	return (uint32_t)page >> PAGE_OFFSET_PTE; 
}

void init_paging();

uint32_t *pd_create(pid_t pid);

uint32_t pde_idx_addr(uint32_t *addr);

uint32_t pte_idx_addr(uint32_t *addr);

//uint32_t * pt_addr(uint32_t * addr); -> address에서 page table 얻기

//uint32_t * pg_addr(uint32_t * addr); -> address에서 page 주소 얻기

#endif
