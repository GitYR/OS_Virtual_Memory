#include <synch.h>
#include <interrupt.h>
#include <proc/proc.h>
#include <device/console.h>

#ifndef ASSERT
#define ASSERT(COND) ;
#endif
/* semaphore */
/* Initializes semaphore SEMA to VALUE.  A semaphore is a
   nonnegative integer along with two atomic operators for
   manipulating it:

   - down or "P": wait for the value to become positive, then
   decrement it.

   - up or "V": increment the value (and wake up one waiting
   thread, if any). */

	void
sema_init(struct semaphore *sema, unsigned value)
{
	ASSERT (sema != NULL);

	sema->value = value;
	list_init(&sema->waiters);
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
   to become positive and then atomically decrements it.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it sleeps then the next scheduled
   thread will probably turn interrupts back on. */

	void
sema_down (struct semaphore *sema) 
{
	enum intr_level old_level;

	ASSERT (sema != NULL);
	ASSERT (!intr_context ());

	old_level = intr_disable ();
	while (sema->value == 0) 
	{
		//list_push_back (&sema->waiters, &cur_process->elem_wait);
		list_push_back (&sema->waiters, &cur_process->elem_stat);
		proc_block ();
	}
	sema->value--;
	intr_set_level (old_level);
}

/* Down or "P" operation on a semaphore, but only if the
   semaphore is not already 0.  Returns true if the semaphore is
   decremented, false otherwise.

   This function may be called from an interrupt handler. */
	bool
sema_try_down (struct semaphore *sema) 
{
	enum intr_level old_level;

	bool success;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (sema->value > 0) 
	{
		sema->value--;
		success = true; 
	}
	else
		success = false;
	intr_set_level (old_level);

	return success;
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */
	void
sema_up (struct semaphore *sema) 
{
	enum intr_level old_level;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (!list_empty (&sema->waiters)) 
		proc_unblock (list_entry (list_pop_front (&sema->waiters),
					struct process, elem_stat));
	sema->value++;
	intr_set_level (old_level);
}
static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
   between a pair of threads.  Insert calls to printf() to see
   what's going on. */
	void
sema_self_test (void) 
{
	static struct semaphore sema[2];
	int i;

	printk ("Testing semaphores...");
	sema_init (&sema[0], 0);
	sema_init (&sema[1], 0);
	proc_create (sema_test_helper, NULL, &sema);
	for (i = 0; i < 10; i++) 
	{
		sema_up (&sema[0]);
		sema_down (&sema[1]);
	}
	printk ("done.\n");
}

/* Thread function used by sema_self_test(). */
	static void
sema_test_helper (void *sema_) 
{
	struct semaphore *sema = sema_;
	int i;
	cur_process -> priority = 100;

	for (i = 0; i < 10; i++) 
	{
		sema_down (&sema[0]);
		sema_up (&sema[1]);
	}
}

/* lock */
/* Initializes LOCK.  A lock can be held by at most a single
   thread at any given time.  Our locks are not "recursive", that
   is, it is an error for the thread currently holding a lock to
   try to acquire that lock.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
	void
lock_init (struct lock *lock)
{
	ASSERT (lock != NULL);

	lock->holder = NULL;
	sema_init (&lock->semaphore, 1);
}

/* Acquires LOCK, sleeping until it becomes available if
   necessary.  The lock must not already be held by the current
   thread.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
	void
lock_acquire (struct lock *lock)
{
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));

	sema_down (&lock->semaphore);
	lock->holder = cur_process;
}

/* Tries to acquires LOCK and returns true if successful or false
   on failure.  The lock must not already be held by the current
   thread.

   This function will not sleep, so it may be called within an
   interrupt handler. */
	bool
lock_try_acquire (struct lock *lock)
{
	bool success;

	ASSERT (lock != NULL);
	ASSERT (!lock_held_by_current_thread (lock));

	success = sema_try_down (&lock->semaphore);
	if (success)
		lock->holder = cur_process;
	return success;
}

/* Releases LOCK, which must be owned by the current thread.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to release a lock within an interrupt
   handler. */
	void
lock_release (struct lock *lock) 
{
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));

	lock->holder = NULL;
	sema_up (&lock->semaphore);
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
	bool
lock_held_by_current_thread (const struct lock *lock) 
{
	ASSERT (lock != NULL);

	return lock->holder == cur_process;
}
/* cond */
/* ... */
