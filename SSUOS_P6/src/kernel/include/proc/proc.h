#ifndef		__PROC_H__
#define		__PROC_H__

#include <list.h>
typedef int pid_t;

enum page_location
  {
    IN_MEMORY,
    IN_SWAP,
  };

// supplementary page table entry
struct spte
  {
    void *vaddr;                  // virtual address
    bool writable;                // if false, this page is read only
    enum page_location location;  // page location
    struct list_elem elem;        // for hash table
    size_t swap_idx;              // swap space index
    //struct lock swap_lock;        // lock for swap
  };



enum p_state {
	PROC_UNUSED,
	PROC_RUN,	
	PROC_STOP,
	PROC_ZOMBIE,
	PROC_BLOCK,
};

#pragma pack(push, 1)

struct proc_option
{
	unsigned priority;
};

struct process
{
	pid_t pid;						//process id
	void *stack;					//stack for esp
	void *pd;						//page directory
	struct spte pagetable;
	enum p_state state;				// process state
	unsigned long long time_sleep;	//프로세스를 재울 시간

	unsigned char priority;			//프로세스의 스케쥴링 우선순위 0~256
	unsigned time_slice;			//프로세스의 수행 tick. 스케줄 시 초기화

	struct list_elem elem_all;		//프로세스 전체 리스트 노드
	struct list_elem elem_stat;		//프로세스 상태 별 리스트 노드

	unsigned long long time_used;		// 총 실행 시간
	unsigned long long time_sched;		// 스케줄 횟수
	struct process* parent;				// 부모 프로세스
	int simple_lock;					// 부모 락
	int child_pid;						// 종료된 자식
	int exit_status;					// 종료 상태
};

#pragma pack(pop)

extern struct process *cur_process;	//현재 프로세스

typedef void proc_func(void* aux);	//process 동작 함수 원형
proc_func print_pid;				//디버그용, pid 출력
proc_func idle;						//idle 함수

void schedule(void);				//스케줄링

void init_proc(void);				//부팅시 프로세스 초기 설정
void proc_free(void);				//프로세스 자원 해제

pid_t proc_create(proc_func func, struct proc_option *priority, void* aux);	//프로세스 생성
void proc_wake(void);				//자는 프로세스 깨움
void proc_sleep(unsigned ticks);	//프로세스 재움
void proc_block(void);				//프로세스 블럭
void proc_unblock(struct process* proc);	//프로세스 언블럭

void proc_print_data(void);			//디버그용 데이터 출력







#endif

