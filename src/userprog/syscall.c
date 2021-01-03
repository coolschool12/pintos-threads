#include "userprog/syscall.h"
#include <stdio.h>
#include <stdbool.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "process.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);


void sys_halt(void);
void sys_exit(int status);
pid_t sys_exec(const char *cmd_line);
int sys_wait(pid_t pid);
bool sys_create(const char * file,unsigned initial_size);
bool sys_remove (const char * file);
int  sys_open(const char *file);
int  sys_filesize(int fd);
int  sys_read(int fd, void * buffer, unsigned size);
int  sys_write(int fd,const void * buffer,unsigned size);
void sys_seek (int fd, unsigned postion);
unsigned sys_tell(int fd);
void sys_close(int fd);

static void valid_pointer(void * ptr);
int get_fd_f(struct file * file);
void remove(int fd);
struct file * get_file(int fd);

static struct lock lock;

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
}

void sys_halt(void)
{
    shutdown_power_off();
}

void sys_exit(int status)
{
    struct thread *current = thread_current();
    struct list des = current->file_descriptors;
    for (struct list_elem *e = list_begin(&des); e != list_end(&des); e = list_next(e)) {
        struct file_descriptor *d = list_entry (e, struct file_descriptor, elem);
        sys_close(d->fd);
    }

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

pid_t sys_exec(const char *cmd_line)
{
    // verify cmd_line.
    return (pid_t) process_execute(cmd_line);
}

int sys_wait(pid_t pid)
{
    return process_wait((tid_t) pid);
}

static void valid_pointer(void * ptr){
    if  (!is_user_vaddr (ptr) || pagedir_get_page(thread_current()->pagedir,ptr) == NULL){
        if(lock_held_by_current_thread(&lock))
            lock_release (&lock);
        sys_exit(-1);
    }
}

bool sys_create(const char * file, unsigned initial_size) {
    ASSERT(file != NULL);
    void * v = (void *)file;
    valid_pointer(v);
    lock_acquire(&lock);
    bool sucess = filesys_create (file,initial_size);
    lock_release(&lock);
    return sucess;
}

bool sys_remove(const char * file) {
    ASSERT(file != NULL);
    void * v = (void *)file;
    valid_pointer(v);
    lock_acquire(&lock);
    bool sucess = filesys_remove (file);
    lock_release(&lock);
    return sucess;
}

int  sys_open(const char *file) {
    ASSERT(file != NULL);
    void * v = (void *)file;
    valid_pointer(v);
    lock_acquire(&lock);
    struct file * file_ptr = filesys_open(file);
    lock_release(&lock);
    if(file_ptr == NULL) return -1;
    return get_fd_f(file_ptr);
}

int get_fd_f(struct file * file) {
    struct thread * crt = thread_current();
    struct file_descriptor * des = malloc(sizeof(struct file_descriptor));
    des->file_itself = file;
    des->fd = crt->number_of_files++;
    list_push_back(crt->thread_files, &des->elem);
    return des->fd;
}

int sys_read(int fd, void *buffer, unsigned size){
    ASSERT(buffer != NULL);
    void * v = (void *)buffer;
    valid_pointer(v);
    if(fd == 0){
        char * str = (char *)buffer;
        int i=0;
        lock_acquire(&lock);
        for(i=0;i<size;i++)
            str[i]=input_getc();
        lock_release(&lock);
        return i;
    }
    struct file * file = get_file(fd);
    if(file == NULL) sys_exit(-1);
    lock_acquire(&lock);
    int sz = file_read(file,buffer,size);
    lock_release(&lock);
    return sz;
}

int sys_write(int fd,const void * buffer,unsigned size){
    ASSERT(buffer != NULL);
    void * v = (void *)buffer;
    valid_pointer(v);
    if(fd == 1){
        lock_acquire(&lock);
        int tmpsz = size;
        while(size >100){
            putbuf(buffer,100);
            size-=100;
            buffer+=100;
        }
        putbuf(buffer,size);
        lock_release(&lock);
        return tmpsz;
    }
    else if(fd == 0)
        return 0;
    struct file * file = get_file(fd);
    if(file == NULL) sys_exit(-1);
    lock_acquire(&lock);
    int sz = file_write(file,buffer,size);
    lock_release(&lock);
    return sz;
}


int sys_filesize(int fd) {
    struct file * file = get_file(fd);
    ASSERT(file != NULL);
    lock_acquire(&lock);
    int op = (int32_t) file_length(file);  // u can find it in file.h in filesys directory
    lock_release(&lock);
    return op;
}

void sys_seek(int fd, unsigned position) {
    struct file * file = get_file(fd);
    ASSERT(file != NULL);
    lock_acquire(&lock);
    file_seek(file,position);  // u can find it in file.h in filesys directory
    lock_release(&lock);
}


unsigned sys_tell(int fd) {
    struct file * file = get_file(fd);
    ASSERT(file != NULL);
    lock_acquire(&lock);
    uint32_t position = file_tell(file);  // u can find it in file.h in filesys directory
    lock_release(&lock);
    return position;
}

//finished
void sys_close(int fd) {
    struct file * file = get_file(fd);
    ASSERT(file != NULL);
    lock_acquire(&lock);
    file_close(file);
    remove(fd);
    lock_release(&lock);
}

void remove(int fd) {
    struct list *files = thread_current()->thread_files;
    for (struct list_elem *elem_x = list_begin(files); elem_x != list_end(files); elem_x = list_next(files)) {
        struct file_descriptor *des = list_entry(elem_x,
        struct file_descriptor, elem);
        if (des->fd == fd) {
            list_remove(elem_x);
            free(des);
            return;
        }
    }
}

struct file * get_file(int fd) {
    struct list * files = thread_current()->thread_files;
    for (struct list_elem * elem_x = list_begin(files); elem_x != list_end(files); elem_x = list_next(files)) {
        struct file_descriptor * des = list_entry(elem_x, struct file_descriptor, elem) ;
        if(des->fd == fd)
            return des->file_itself;
    }
    return NULL;
}

