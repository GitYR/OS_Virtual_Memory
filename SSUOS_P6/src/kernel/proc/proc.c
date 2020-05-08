#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <ssulib.h>
#include <interrupt.h>
#include <proc/sched.h>
#include <device/console.h>
#include <device/io.h>
#include <syscall.h>
#include <mem/paging.h>
#include <mem/palloc.h>
#include <string.h>

#define STACK_SIZE 512
#define PROC_NUM_MAX 16

struct list p_list;		// All Porcess List
struct list r_list;		// Run Porcess List
struct list s_list;		// Sleep Process List
struct list d_list;		// Deleted Process List

struct process procs[PROC_NUM_MAX];
struct process *cur_process;
int pid_num_max;

uint32_t process_stack_ofs;

//values for pid
static int lock_pid_simple; 
static int lately_pid;		

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux);
pid_t getValidPid(int *idx);

void proc_start(void);			
void proc_end(void);			

static void login_prompt(void *);
static bool check_user(char *, char *);
void shell_proc(void* aux);

typedef struct{
	char id[BUFSIZ];
	char password[BUFSIZ];
}user_list;

user_list temp_list = {"ssuos\n","oslab\n"};

void loop_proc(void *aux)
{
	printk("loop start...\n");
	while(1);
}

void login_prompt(void * aux)
{
	char id[BUFSIZ];
	char password[BUFSIZ];

	while(1)	
	{
		printk("\nid : ");
		while(getkbd(id,BUFSIZ) == TRUE);

		printk("password : ");
		while(getkbd(password,BUFSIZ) == TRUE);

		if(check_user(id,password))
			shell_proc(NULL);
		else
			printk("\nincorrect id or password.\n");
	}
}

bool check_user(char *id, char *password)
{
	if (strcmp(temp_list.id,id) || strcmp(temp_list.password,password)) return false;
	return true;
}

void init_proc()
{
	process_stack_ofs = offsetof (struct process, stack);

	lock_pid_simple = 0;
	lately_pid = -1;
	
	list_init(&p_list);
	list_init(&r_list);
	list_init(&s_list);
	list_init(&d_list);

	int i;
	for (i = 0; i < PROC_NUM_MAX; i++)
	{
		procs[i].pid = i;
		procs[i].state = PROC_UNUSED;
		procs[i].parent = NULL;
	}

	pid_t pid = getValidPid(&i);
    cur_process = &procs[i];

    cur_process->pid = pid;
    cur_process->parent = NULL;
    cur_process->state = PROC_RUN;
	cur_process->priority = 0;
	cur_process->stack = 0;
	cur_process->pd = (void*)read_cr3();

	cur_process->elem_all.prev = NULL;
	cur_process->elem_all.next = NULL;
	cur_process->elem_stat.prev = NULL;
	cur_process->elem_stat.next = NULL;

	list_push_back(&p_list, &cur_process->elem_all);
	list_push_back(&r_list, &cur_process->elem_stat);
}

pid_t getValidPid(int *idx) {

	pid_t pid = -1;
	int i;

	while(lock_pid_simple)
		;

	lock_pid_simple++;

	for(i = 0; i < PROC_NUM_MAX; i++)
	{
		int tmp = i + lately_pid + 1;
		if(procs[tmp % PROC_NUM_MAX].state == PROC_UNUSED) { 
			pid = lately_pid + 1;
			*idx = tmp % PROC_NUM_MAX;
			break;
		}
	}

	if(pid != -1)
	{
		lately_pid = pid;	
	}

	lock_pid_simple = 0;

	return pid;
}
/* Stack frame for switch_entry(). */
struct switch_entry_frame
  {
    void (*eip) (void);
  };
/* Stack frame for kernel_thread(). */

/* switch_thread()'s stack frame. */
struct switch_threads_frame 
  {
    uint32_t edi;               /*  0: Saved %edi. */
    uint32_t esi;               /*  4: Saved %esi. */
    uint32_t ebp;               /*  8: Saved %ebp. */
    uint32_t ebx;               /* 12: Saved %ebx. */
    void (*eip) (void);         /* 16: Return address. */
    struct thread *cur;         /* 20: switch_threads()'s CUR argument. */
    struct thread *next;        /* 24: switch_threads()'s NEXT argument. */
  };

pid_t proc_create(proc_func func, struct proc_option *opt, void* aux)
{
	struct process *p;
	int idx;

	enum intr_level old_level = intr_disable();

	pid_t pid = getValidPid(&idx);
	p = &procs[idx];

	p->pid = pid;
	p->state = PROC_RUN;

	if(opt != NULL)
		p->priority = opt->priority;   
	else
		p->priority = (unsigned char)0;

	p->time_used = 0;
	p->time_sched= 0;
	p->parent = cur_process;
	p->simple_lock = 0;
	p->child_pid = -1;

    int *top = (int*)palloc_get_one_page(kernel_area); // modified
	int stack = (int)top;
	top = (int*)stack + STACK_SIZE - 1;

	*(--top) = (int)aux;		//argument for func
	*(--top) = (int)proc_end;	//return address from func
	*(--top) = (int)func;		//return address from proc_start
	*(--top) = (int)proc_start; //return address from switch_process

	*(--top) = (int)((int*)stack + STACK_SIZE - 1); //ebp
	*(--top) = 1; //eax
	*(--top) = 2; //ebx
	*(--top) = 3; //ecx
	*(--top) = 4; //edx
	*(--top) = 5; //esi
	*(--top) = 6; //edi

	p->stack = top;
	p->pd = pd_create(pid); // create a page directory 
	p->pagetable.vaddr=0;
	p->pagetable.writable=0;
	p->pagetable.swap_idx=0;
	p->elem_all.prev = NULL;
	p->elem_all.next = NULL;
	p->elem_stat.prev = NULL;
	p->elem_stat.next = NULL;

	list_push_back(&p_list, &p->elem_all);
	list_push_back(&r_list, &p->elem_stat);

	intr_set_level (old_level);
	return p->pid;
}

void* getEIP()
{
    return __builtin_return_address(0);
}

void  proc_start(void)
{
	intr_enable ();
	return;
}

void proc_free(void)
{
	uint32_t *pd = (uint32_t*)cur_process->pd;
	uint32_t *pt = (uint32_t*)(pd[0] & PAGE_MASK_BASE);
	uint32_t *pt2 = (uint32_t*)(pd[768] & PAGE_MASK_BASE);

	cur_process->parent->child_pid = cur_process->pid;
	cur_process->parent->simple_lock = 0;

	list_remove(&cur_process->elem_stat);

	cur_process->state = PROC_ZOMBIE;	
	list_push_back(&d_list, &cur_process->elem_stat);

	palloc_free_one_page(cur_process->stack);
	palloc_free_one_page(cur_process->pd);
	palloc_free_one_page(pt);
	palloc_free_one_page(pt2);
}

void proc_end(void)
{
	proc_free();
	schedule();
	printk("never reach\n");
	return;	//never reach
}

void proc_wake(void)
{
	struct process* p;
	unsigned long long t = get_ticks();

    while(!list_empty(&s_list))
	{
		p = list_entry(list_front(&s_list), struct process, elem_stat);
		if(p->time_sleep > t)
			break;
		p->state = PROC_RUN;
		list_remove(&p->elem_stat);
	}
}

void proc_sleep(unsigned ticks)
{
	unsigned long cur_ticks = get_ticks();
	cur_process->time_sleep =  ticks + cur_ticks;
	cur_process->state = PROC_STOP;
	list_insert_ordered(&s_list, &cur_process->elem_stat,
			less_time_sleep, NULL);
	schedule();
}

void proc_block(void)
{
	cur_process->state = PROC_BLOCK;
	schedule();	
}

void proc_unblock(struct process* proc)
{
	enum intr_level old_level;

	old_level = intr_disable();

	list_push_back(&r_list, &proc->elem_stat);
	proc->state = PROC_RUN;

	intr_set_level(old_level);
}     

bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, elem_stat);
	struct process *p2 = list_entry(b, struct process, elem_stat);

    return p1->time_sleep < p2->time_sleep;
}

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, elem_stat);
	struct process *p2 = list_entry(b, struct process, elem_stat);
    return p1->priority > p2->priority;
}


void kernel1_proc(void* aux)
{
	cur_process -> priority = 200;
	while(1)
	{
		schedule();
	}
}

void kernel2_proc(void* aux)
{
	cur_process -> priority = 200;
	while(1)
	{
		schedule();
	}
}

void ps_proc(void* aux)
{
	int i;
	for(i = 0; i<PROC_NUM_MAX; i++)
	{
		struct process *p = &procs[i];

		if(p->state == PROC_UNUSED)
			continue;

		printk("pid %d ppid ", p->pid);

		if(p->parent != NULL)
			printk("%d", p->parent->pid);
		else
			printk("non");

		printk(" state %d prio %d using time %d sched time %d",
				p->state, p->priority, p->time_used, p->time_sched);
		printk(", pd = %x\n", p->pd);

	}
	exit(1);
}

extern const char* VERSION;
extern const char* AUTHOR;
extern const char* MODIFIER;
void uname_proc(void* aux)
{
	printk("SSUOS %s\nmade by %s\nmodefied by %s\n", VERSION, AUTHOR, MODIFIER);	

}

void print_pid(void* aux) {

	while(1) {
		printk("pid = %d ", cur_process->pid);
		printk("prio = %d ", cur_process->priority);
		printk("time = %d ", cur_process->time_slice);
		printk("ticks = %d ", get_ticks());
		printk("in %s\n", aux);

#define SLEEP_FREQ 100
		proc_sleep(cur_process->pid * cur_process->pid * SLEEP_FREQ);
	}
}

typedef struct
{
	char* cmd;
	unsigned char type;	
	void* func;
} CmdList;

void shell_proc(void* aux)
{
	CmdList cmdlist[] = {
		{"shutdown", 0, shutdown},
		{"ps", 1, ps_proc},
		{"uname", 1, uname_proc},
	};
#define CMDNUM 3
#define TOKNUM 10
	char buf[BUFSIZ];
	char token[TOKNUM][BUFSIZ];
	int token_num;

	cur_process -> priority = 100;

	while(1)
	{
		proc_func *func;
		int i, len;

		printk("> ");

		while(getkbd(buf,BUFSIZ))
		{
			;
		}
		
		for(i=0;buf[i] != '\n'; i++); 
		for(i--; buf[i] == ' '; i--)
			buf[i] = 0;

		token_num = getToken(buf,token,TOKNUM);

		if( strcmp(token[0], "exit") == 0)
			break;

		if( strncmp(token[0], "list", BUFSIZ) == 0)
		{
			for(i = 0; i < CMDNUM; i++)
				printk("%s\n", cmdlist[i].cmd);
			continue;
		}
		
		for(i = 0; i < CMDNUM; i++)
		{
			if( strncmp(cmdlist[i].cmd, token[0], BUFSIZ) == 0)
				break;
		}

		if(i == CMDNUM)
		{
			printk("Unknown command %s\n", buf);
			continue;
		}

		if(cmdlist[i].type == 0)
		{
			void (*func)(void);
			func = cmdlist[i].func;
			func();
		}
		else if(cmdlist[i].type == 1)
		{
			cur_process->simple_lock = 1;
			int pid = fork(cmdlist[i].func, (void*)0x999);

			while(cur_process->simple_lock)
				;
		}
		else
		{
			printk("Unknown type\n");
			continue;
		}
	}
}

void idle(void* aux)
{
	
	proc_create(kernel1_proc, NULL, NULL);
	proc_create(kernel2_proc, NULL, NULL);
	proc_create(login_prompt,NULL,NULL);

	while(1) {  
		if(cur_process->pid != 0) {
			printk("error : idle process's pid != 0\n", cur_process->pid);
			while(1);
		}

		while( !list_empty(&d_list) )
		{
			struct list_elem *e = list_pop_front(&d_list);
			struct process *p = list_entry(e, struct process, elem_stat);
			p->state = PROC_UNUSED;
			list_remove( &p->elem_all);
		}

		schedule();     
	}
}

void proc_print_data()
{
	int a, b, c, d, bp, si, di, sp;

	//eax ebx ecx edx
	__asm__ __volatile("mov %%eax ,%0": "=m"(a));

	__asm__ __volatile("mov %ebx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(b));
	
	__asm__ __volatile("mov %ecx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(c));
	
	__asm__ __volatile("mov %edx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(d));
	
	//ebp esi edi esp
	__asm__ __volatile("mov %ebp ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(bp));

	__asm__ __volatile("mov %esi ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(si));

	__asm__ __volatile("mov %edi ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(di));

	__asm__ __volatile("mov %esp ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(sp));

	printk(	"\neax %o ebx %o ecx %o edx %o"\
			"\nebp %o esi %o edi %o esp %o\n"\
			, a, b, c, d, bp, si, di, sp);
}

void hexDump (void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (len == 0) {
        printk("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printk("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    for (i = 0; i < len; i++) {

        if ((i % 16) == 0) {
            if (i != 0)
                printk ("  %s\n", buff);

            printk ("  %04x ", i);
        }

        printk (" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printk ("   ");
        i++;
    }

    printk ("  %s\n", buff);
}


