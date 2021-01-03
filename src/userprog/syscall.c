#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
    printf ("system call!\n");
    thread_exit ();
    //halt_syscall();
}

void halt_syscall(void)
{
    shutdown_power_off();
}

void exit_syscall(int status)
{
    //
    // Release resources
    //
    struct thread *current = thread_current();
    if (current->parent->waiting_on == current->tid) {
        current->parent->child_status = status;
        current->parent->waiting_on = -1;
        
        sema_up(&current->parent->wait_child);
    } else {
        remove_child(&current->parent->children, current->tid);
    }

    struct list *children = &current->children;
    for (struct list_elem *e = list_begin(children); e != list_end(children); e = list_next(e)) {
        struct child_process *c = list_entry (e, struct child_process, elem);
        sema_up(&c->t->wait_on_parent);
    }

    thread_exit();
}

pid_t exec_syscall(const char *cmd_line)
{
    // verify cmd_line.
    return (pid_t) process_execute(cmd_line);
}

int wait_syscall(pid_t pid)
{
    return process_wait((tid_t) pid);
}
