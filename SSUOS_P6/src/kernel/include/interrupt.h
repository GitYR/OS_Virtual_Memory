#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#define HLT()  __asm__ __volatile("hlt");

#include <type.h>


enum intr_level
{
	INTR_OFF,             /* Interrupts disabled. */
	INTR_ON               /* Interrupts enabled. */
};

enum intr_level intr_get_level (void);
enum intr_level intr_set_level (enum intr_level);
enum intr_level intr_enable (void);
enum intr_level intr_disable (void);

#pragma pack(push, 1)

/* Interrupt stack frame. */
struct intr_frame
{
	/* Pushed by intr_entry in intr-stubs.S.
	   These are the interrupted task's saved registers. */
	uint32_t edi;               /* Saved EDI. */
	uint32_t esi;               /* Saved ESI. */
	uint32_t ebp;               /* Saved EBP. */
	uint32_t esp_dummy;         /* Not used. */
	uint32_t ebx;               /* Saved EBX. */
	uint32_t edx;               /* Saved EDX. */
	uint32_t ecx;               /* Saved ECX. */
	uint32_t eax;               /* Saved EAX. */
	uint16_t gs, :16;           /* Saved GS segment register. */
	uint16_t fs, :16;           /* Saved FS segment register. */
	uint16_t es, :16;           /* Saved ES segment register. */
	uint16_t ds, :16;           /* Saved DS segment register. */

	/* Pushed by intrNN_stub in intr-stubs.S. */
	uint32_t vec_no;            /* Interrupt vector number. */

	/* Sometimes pushed by the CPU,
	   otherwise for consistency pushed as 0 by intrNN_stub.
	   The CPU puts it just under `eip', but we move it here. */
	uint32_t error_code;        /* Error code. */

	/* Pushed by intrNN_stub in intr-stubs.S.
	   This frame pointer eases interpretation of backtraces. */
	void *frame_pointer;        /* Saved EBP (frame pointer). */

	/* Pushed by the CPU.
	   These are the interrupted task's saved registers. */
	void (*eip) (void);         /* Next instruction to execute. */
	uint16_t cs, :16;           /* Code segment for eip. */
	uint32_t eflags;            /* Saved CPU flags. */
	void *esp;                  /* Saved stack pointer. */
	uint16_t ss, :16;           /* Data segment for esp. */
};

#pragma pack(pop)

typedef void intr_handler_func (struct intr_frame *);
bool intr_context (void);
void do_sched_on_return(void);

int init_intr(void);
void reg_handler(unsigned short idx, intr_handler_func *handler);
unsigned long get_ticks(void);

#define reg_push() __asm__ __volatile(\
		"pushl %eax\n\tpushl %ebx\n"\
		"pushl %ecx\n\tpushl %edx\n"\
		"pushl %ebp\n\tpushl %esi\n"\
		"pushl %edi"\
		)

#ifndef __ASSEMBLER__
#endif

#endif
