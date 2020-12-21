#ifndef THREADS_COMPARE_H
#define THREADS_COMPARE_H

#include "threads/thread.h"
#include "threads/synch.h"

bool priority_more (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
bool priority_less (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
bool priority_less_lock (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);

#endif /* threads/compare.h */
