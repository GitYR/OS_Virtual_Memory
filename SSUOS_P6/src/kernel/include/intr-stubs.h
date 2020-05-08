#ifndef _INTR_STUBS_H_
#define _INTR_STUBS_H_

#define INTR_MAX   256

/* Interrupt stubs.

   These are little snippets of code in intr-stubs.S, one for
   each of the 256 possible x86 interrupts.  Each one does a
   little bit of stack manipulation, then jumps to intr_entry().
   See intr-stubs.s for more information.

   This array points to each of the interrupt stub entry points
   so that intr_init() can easily find them. */
typedef void intr_stub_func (void);
extern intr_stub_func *intr_stubs[INTR_MAX];

/* Interrupt return path. */
void intr_exit (void);

#endif 
