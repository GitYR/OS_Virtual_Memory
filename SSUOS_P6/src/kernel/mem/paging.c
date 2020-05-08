#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>
#include <device/ide.h>

uint32_t *PID0_PAGE_DIR;

intr_handler_func pf_handler;

//해당 코드를 사용하지 않고 구현해도 무관함
void pagememcpy(void* from, void* to, uint32_t len)
{
	uint32_t *p1 = (uint32_t*)from;
 	uint32_t *p2 = (uint32_t*)to;
 	int i, e;

 	e = len/sizeof(uint32_t);
 
	for(i = 0; i<e; i++)
 		p2[i] = p1[i];

 	e = len%sizeof(uint32_t);

	if( e != 0)
 	{
 		uint8_t *p3 = (uint8_t*)p1;
 		uint8_t *p4 = (uint8_t*)p2;

 		for(i = 0; i<e; i++)
 			p4[i] = p3[i];
 	}
}

/*
 scale함수는 size에 따라 뽑아내는 비트들이 달라진다.
 size가 page directory에 해당하는 경우 pd에 해당하는 앞 10비트만 뽑아낸다(base & mask)
 size가 page table에 해당하는 경우 pd,pt에 해당하는 앞 10+10비트를 뽑아낸다.(base & mask)
 즉, pd의 경우 해당 디렉토리가 몇 번 디렉토리인지
 pt의 경우 해당 테이블이 몇 번 디렉토리의 몇 번째 테이블인지 알 수 있게 해준다.
 scale_up()의 경우는 그렇게 뽑아낸 인덱스에 +1,
 scale_down()의 경우는 그렇게 뽑아낸 인덱스에 -1한 결과이다.
*/
uint32_t scale_up(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0)
		base = base & mask + size;
	return base;
}

uint32_t scale_down(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0)
		base = base & mask - size;
	return base;
}

void init_paging()
{
	uint32_t *page_dir = palloc_get_one_page(kernel_area); 
	uint32_t *page_tbl = palloc_get_one_page(kernel_area); 

	PID0_PAGE_DIR = page_dir; // page_dir 대신 가져다 쓰면 될듯.

	int NUM_PT, NUM_PE;
	uint32_t cr0_paging_set;
	int i;

	// mem_size() : 134217728 -> 0x08000000
	// NUM_PE : 32768
	// PAGE_SIZE : 4KB
	// 즉, 전체 메모리에 32768개의 PAGE ENTRY가 들어갈 수 있다.

	NUM_PE = mem_size() / PAGE_SIZE;
	NUM_PT = NUM_PE / 1024;

	//페이지 디렉토리 구성
	page_dir[0] = (uint32_t)page_tbl | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

	NUM_PE = RKERNEL_HEAP_START / PAGE_SIZE;

	//페이지 테이블 구성
	for(i = 0; i < NUM_PE; i++ ) // reapeats 8192 times
	{
		page_tbl[i] = (PAGE_SIZE * i) // 4096 * i
			| PAGE_FLAG_RW // 0x02
			| PAGE_FLAG_PRESENT; // 0x01
		//writable & present
	}

	//CR0레지스터 설정
	cr0_paging_set = read_cr0() | CR0_FLAG_PG;		// PG bit set

	//컨트롤 레지스터 저장
	write_cr3((unsigned)page_dir);	// cr3 레지스터에 PDE 시작주소 저장
	write_cr0(cr0_paging_set);        // PG bit를 설정한 값을 cr0 레지스터에 저장

	reg_handler(14, pf_handler); // INT 14
}

// 뭔지 제대로 이해는 못하겠다.
// memory copy hardware의 의미로 보임
// from에서 to로 len만큼의 크기를 복사하는 것으로 보임
void memcpy_hard(void* from, void* to, uint32_t len)
{
	write_cr0(read_cr0() & ~CR0_FLAG_PG);
	memcpy(from, to, len); // len만큼 from -> to로 copy
	write_cr0(read_cr0() | CR0_FLAG_PG);
}

// 현재 쓰고 있는 page directory를 가져온다.
uint32_t* get_cur_pd()
{
	return (uint32_t*)read_cr3();
}

// page directory entry의 인덱스를 리턴
uint32_t pde_idx_addr(uint32_t* addr)
{
	// PAGE_MASK_PDE : 0xFFC00000
	// PAGE_OFFSET_PDE : 22 
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PDE) >> PAGE_OFFSET_PDE;
	return ret;
}

// page table entry의 인덱스를 리턴
uint32_t pte_idx_addr(uint32_t* addr)
{
	// PAGE_MASK_PTE : 0x003FF000
	// PAGE_OFFSET_PTE : 12
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PTE) >> PAGE_OFFSET_PTE;
	return ret;
}

//page directory에서 index 위치의 page table 얻기
uint32_t* pt_pde(uint32_t pde)
{
	uint32_t * ret = (uint32_t*)(pde & PAGE_MASK_BASE);
	return ret;
}

//address에서 page table 얻기
uint32_t* pt_addr(uint32_t* addr)
{
	uint32_t idx = pde_idx_addr(addr);
	uint32_t* pd = get_cur_pd();
	return pt_pde(pd[idx]);
}

//address에서 page 주소 얻기
uint32_t* pg_addr(uint32_t* addr)
{
	uint32_t *pt = pt_addr(addr);
	uint32_t idx = pte_idx_addr(addr);
	uint32_t *ret = (uint32_t*)(pt[idx] & PAGE_MASK_BASE);
	return ret;
}

//page table 복사
void pt_copy(uint32_t *pd, uint32_t *dest_pd, uint32_t idx, uint32_t* start, uint32_t* end, bool share)
{
	uint32_t *pt = pt_pde(pd[idx]); // page_dir의 idx번째 디렉토리로부터 page table 주소값 가져오기
	uint32_t *new_pt;
	uint32_t i;

	new_pt = palloc_get_one_page(kernel_area); // 새로운 pt를 위한 page frame할당.
	dest_pd[idx] = (uint32_t)new_pt | (pd[idx] & ~PAGE_MASK_BASE & ~PAGE_FLAG_ACCESS); // 

	for(i = 0; i < 1024; i++)
	{
		if(pt[i] & PAGE_FLAG_PRESENT)
		{
		
			if(share) // true
				new_pt[i] = pt[i]; // share할 때는 해당 page frame을 공유함.
			else // false
			{
				uint32_t* new_page = palloc_get_one_page(kernel_area);
				new_pt[i] = (uint32_t)new_page | (pt[i] & ~PAGE_MASK_BASE & ~PAGE_FLAG_ACCESS); // 새로운 페이지 테이블을 생성&기존 테이블 엔트리를 복사
				pagememcpy((void*)(pt[i] & PAGE_MASK_BASE), (void*)new_page, PAGE_SIZE); // 기존 테이블 엔트리를 새로운 페이지에 복사
			}
		}
	}
}

void pd_copy(uint32_t* pd, uint32_t* dest_pd,uint32_t idx,uint32_t* start, uint32_t* end, bool share)
{
	uint32_t from,to,i;

	from = pde_idx_addr(start); // start위치에 있는 주소값이 몇 번째 pd인덱스인지 뽑기
	to = pde_idx_addr(end); // start와 같은 이유

	for(i = from; i < to; i++)
	{
		if(pd[i] & PAGE_FLAG_PRESENT) // 해당 디렉토리가 현재 메모리에 올라와 있다?
			pt_copy(pd,dest_pd,i,start,end,share); // 해당 디렉토리 인덱스를 i로 줘서 pt_copy호출
		// pt_copy를 통해 해당 디렉토리의 테이블을 share값에 따라 복사/공유할지 결정
	}
}

uint32_t* pd_create(pid_t pid)
{
	uint32_t *pd = PID0_PAGE_DIR; // init_paging에서 page dir을 저장해둠
	uint32_t *dest_pd = palloc_get_one_page(kernel_area); // 새로운 pd를 위한 페이지 프레임 할당

	//KERNEL_SOURCE ~ RKERNEL_HEAP_START전까지의 페이지를 관리
	pd_copy(pd,dest_pd,0,(uint32_t *)0,(uint32_t *)RKERNEL_HEAP_START,true);

	return dest_pd;
}

void pf_handler(struct intr_frame *iframe)
{
	void *fault_addr;

	asm ("movl %%cr2, %0" : "=r" (fault_addr));

	printk("Page fault : %X\n", fault_addr);

#ifdef SCREEN_SCROLL
	refreshScreen();
#endif
	uint32_t pdi, pti;
    uint32_t *pta;
    uint32_t *pda = (uint32_t*)read_cr3();

    pdi = pde_idx_addr(fault_addr);
    pti = pte_idx_addr(fault_addr);

    if(pda == PID0_PAGE_DIR)
	{
        write_cr0( read_cr0() & ~CR0_FLAG_PG);
        pta = pt_pde(pda[pdi]);
        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else
	{
        //pda = RH_TO_VH(pda);
        pta = pt_pde(pda[pdi]);
    }

    if(pta == NULL)
	{
        write_cr0(read_cr0() & ~CR0_FLAG_PG);

        pta = palloc_get_one_page(kernel_area); // modified
        //pta = VH_TO_RH(pta);
        memset(pta,0,PAGE_SIZE);
        
        pda[pdi] = (uint32_t)pta | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

        //fault_addr = VH_TO_RH(fault_addr);
        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;

        //pta = RH_TO_VH(pta);
        pdi = pde_idx_addr(pta);
        pti = pte_idx_addr(pta);

        uint32_t *tmp_pta = pt_pde(pda[pdi]);
        tmp_pta[pti] = (uint32_t)pta | PAGE_FLAG_RW | PAGE_FLAG_PRESENT; // VH_TO_RH(pta)

        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else
	{
        //pta = RH_TO_VH(pta);
        //fault_addr = VH_TO_RH(fault_addr);
        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;
    }
}
