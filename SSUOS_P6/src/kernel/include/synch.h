#ifndef __SYNCH_H__
#define __SYNCH_H__

#include <list.h>
#include <type.h>
//#include <proc/proc.h>

/* A counting semaphore. */
struct semaphore 
  {
      unsigned value;             /* Current value. */
      struct list waiters;        /* List of waiting threads. */
  };

void sema_init (struct semaphore *, unsigned value);
void sema_down (struct semaphore *);
bool sema_try_down (struct semaphore *);
void sema_up (struct semaphore *);
void sema_self_test (void);


/* Lock. */
struct lock 
  {
      struct process *holder;      /* Process holding lock (for debugging). */
      struct semaphore semaphore; /* Binary semaphore controlling access. */
  };

void lock_init (struct lock *);
void lock_acquire (struct lock *);
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *);
bool lock_held_by_current_process (const struct lock *);

/* Condition variable. */
struct condition 
  {
      struct list waiters;        /* List of waiting threads. */
  };

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);

/* Optimization barrier.

     The compiler will not reorder operations across an
     optimization barrier.  See "Optimization Barriers" in the
     reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* synch.h */

