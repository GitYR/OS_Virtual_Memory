#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>

extern struct list r_list;

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);

struct process* get_next_proc(void) {
	//simple FIFO
	struct list_elem *e;

	if(cur_process->state == PROC_RUN)
	{
		list_remove(&cur_process->elem_stat);
		list_push_back(&r_list, &cur_process->elem_stat);
	}

	for(e = list_begin (&r_list); e != list_end (&r_list);
		e = list_next (e))
	{
		struct process* p = list_entry(list_front(&r_list), struct process, elem_stat);
		if(p->state == PROC_RUN)
			return p;
	}
	return cur_process;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;

	proc_wake();		

	next = get_next_proc();	
	cur = cur_process;
	cur_process = next;
	cur_process->time_sched++;
	cur_process->time_slice = 0;

	switch_process(cur, next);
}
