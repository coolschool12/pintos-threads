/* Comparison functions. */
#include "threads/compare.h"

/* Returns true if a has higher priority than b for threads. */
bool priority_more (
    const struct list_elem *a,
    const struct list_elem *b,
    void *aux UNUSED
) {
    struct thread *a_thread = list_entry (a, struct thread, elem);
    struct thread *b_thread = list_entry (b, struct thread, elem);

    return thread_get_priority_t(a_thread) > thread_get_priority_t(b_thread);
}

/* Returns true if a has lower priority than b for threads. */
bool priority_less (
    const struct list_elem *a,
    const struct list_elem *b,
    void *aux UNUSED
) {
    struct thread *a_thread = list_entry (a, struct thread, elem);
    struct thread *b_thread = list_entry (b, struct thread, elem);
  
    return thread_get_priority_t(a_thread) < thread_get_priority_t(b_thread);
}

/* Returns true if a has lower priority than b for locks. */
bool priority_less_lock (
    const struct list_elem *a,
    const struct list_elem *b,
    void *aux UNUSED
) {
    struct lock *a_lock = list_entry (a, struct lock, elem);
    struct lock *b_lock = list_entry (b, struct lock, elem);
  
    return sema_calculate_priority(&a_lock->semaphore) < sema_calculate_priority(&b_lock->semaphore);
}
