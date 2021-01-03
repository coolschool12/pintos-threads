#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>

typedef int pid_t;

void syscall_init (void);

void halt_syscall(void);
void exit_syscall(int status);
pid_t exec_syscall(const char *cmd_line);
int wait_syscall(pid_t pid);

#endif /* userprog/syscall.h */
