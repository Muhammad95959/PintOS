#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

static struct lock files_sys_lock;               /* lock for syschronization between files */

static void syscall_handler (struct intr_frame *);

struct open_file* get_file(int fd);


void exit_sys_call(struct intr_frame *f);
void create_sys_call(struct intr_frame *f);
void remove_sys_call(struct intr_frame *f);
int open_sys_call(struct intr_frame *f);
int filesize_sys_call(struct intr_frame *f);
int read_sys_call(struct intr_frame *f);
int write_sys_call(struct intr_frame *f);
void seek_sys_call(struct intr_frame *f);
unsigned tell_sys_call(struct intr_frame *f);
void close_sys_call(struct intr_frame *f);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  lock_init(&files_sys_lock);
}

void 
validate_ptr(const void* f)
{
  if (f == NULL|| !is_user_vaddr(f) || pagedir_get_page(thread_current()->pagedir, f) == NULL) 
  {
    system_exit(-1);
  }
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  validate_ptr(f->esp);

  switch (*(int*)f->esp)
  {
  case SYS_HALT:
    shutdown_power_off();
    break;
  
  case SYS_EXIT:
    exit_sys_call(f);
    break;

  case SYS_EXEC:
    //exec_wrapper(f);
    break;

  case SYS_WAIT:
    //wait_wrapper(f);
    break;

  case SYS_CREATE:
    create_sys_call(f);
    break;

  case SYS_REMOVE:
    remove_sys_call(f);
    break;

  case SYS_OPEN:
    f->eax = open_sys_call(f);
    break;

  case SYS_FILESIZE:
    f->eax = filesize_sys_call(f);
    break;

  case SYS_READ:
    f->eax = read_sys_call(f);
    break;

  case SYS_WRITE:
    f->eax = write_sys_call(f);
    break;

  case SYS_SEEK:
    seek_sys_call(f);
    break;

  case SYS_TELL:
    f->eax = tell_sys_call(f);
    break;

  case SYS_CLOSE:
    close_sys_call(f);
    break;

  default:
    // negative area
    break;
  }

}

void
system_exit(int status)
{
  struct thread* parent = thread_current()->parent_thread;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  if(parent) parent->child_status = status;
  thread_exit();
}

void
exit_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp+4);
  int status = *((int*)f->esp + 1);
  system_exit(status);
}


void
create_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  validate_ptr(f->esp + 8);

  char* name = (char*)(*((int*)f->esp + 1));
  size_t size = *((int*)f->esp + 2);

  if (name == NULL) system_exit(-1);

  bool res;
  lock_acquire(&files_sys_lock);
  res = filesys_create(name,size);
  lock_release(&files_sys_lock);
  f->eax = res;
}

void
remove_sys_call(struct intr_frame *f)
{

  validate_ptr(f->esp + 4);

  char* name = (char*)(*((int*)f->esp + 1));

  if (name == NULL) system_exit(-1);
  bool res;
  lock_acquire(&files_sys_lock);
  res = filesys_remove(name);
  lock_release(&files_sys_lock);

  f->eax = res;
}

int
open_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  char* name = (char*)(*((int*)f->esp + 1));
  if (name == NULL) system_exit(-1);

  struct open_file* open = palloc_get_page(0);

  if (open == NULL) 
  {
    palloc_free_page(open);
    return -1;
  }
  lock_acquire(&files_sys_lock);
  open->ptr = filesys_open(name);
  lock_release(&files_sys_lock);
  if (open->ptr == NULL)
  {
    return -1;
  }
  open->file_desc = ++thread_current()->fd_last;
  list_push_back(&thread_current()->opened_files_list,&open->elem);
  return open->file_desc;

}

int
filesize_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  int fd = *((int*)f->esp + 1);
  struct thread* t = thread_current();
  struct file* my_file = get_file(fd)->ptr;

  if (my_file == NULL)
  {
    return -1;
  }
  int res;
  lock_acquire(&files_sys_lock);
  res = file_length(my_file);
  lock_release(&files_sys_lock);
  return res;
}

int
read_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  validate_ptr(f->esp + 8);
  validate_ptr(f->esp + 12);

  int fd, size;
  void* buffer;
  fd = *((int*)f->esp + 1);
  buffer = (void*)(*((int*)f->esp + 2));
  size = *((int*)f->esp + 3);
  validate_ptr(buffer + size);

  if (fd == 0)
  {
    for (size_t i = 0; i < size; i++)
    {
      lock_acquire(&files_sys_lock);
      ((char*)buffer)[i] = input_getc();
      lock_release(&files_sys_lock);
    }
    return size;
  } else {
    struct thread* t = thread_current();
    struct file* my_file = get_file(fd)->ptr;
    if (my_file == NULL)
    {
      return -1;
    }
    int res;
    lock_acquire(&files_sys_lock);
    res = file_read(my_file,buffer,size);
    lock_release(&files_sys_lock);
    return res;
  }
}

int
write_sys_call(struct intr_frame *f)
{

  validate_ptr(f->esp + 4);
  validate_ptr(f->esp + 8);
  validate_ptr(f->esp + 12);

  int fd, size;
  void* buffer;
  fd = *((int*)f->esp + 1);
  buffer = (void*)(*((int*)f->esp + 2));
  size = *((int*)f->esp + 3);

  if (buffer == NULL) system_exit(-1);
  
  if (fd == 1)
  {
    lock_acquire(&files_sys_lock);
    putbuf(buffer,size);
    lock_release(&files_sys_lock);
    return size;
  } else {
    struct thread* t = thread_current();
    struct file* my_file = get_file(fd)->ptr;
    if (my_file == NULL)
    {
      return -1;
    }
    int res;
    lock_acquire(&files_sys_lock);
    res = file_write(my_file,buffer,size);
    lock_release(&files_sys_lock);
    return res;
  }
}

void
seek_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  validate_ptr(f->esp + 8);

  int fd;
  unsigned pos;
  fd = *((int*)f->esp + 1);
  pos = *((unsigned*)f->esp + 2);

  struct thread* t = thread_current();
  struct file* my_file = get_file(fd)->ptr;
  if (my_file == NULL)
  {
    return;
  }
  lock_acquire(&files_sys_lock);
  file_seek(my_file,pos);
  lock_release(&files_sys_lock);
}

unsigned
tell_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  int fd = *((int*)f->esp + 1);

  struct thread* t = thread_current();
  struct file* my_file = get_file(fd)->ptr;

  if (my_file == NULL)
  {
    return -1;
  }
  unsigned res;
  lock_acquire(&files_sys_lock);
  res = file_tell(my_file);
  lock_release(&files_sys_lock);
  return res;
}

void
close_sys_call(struct intr_frame *f)
{
  validate_ptr(f->esp + 4);
  int fd = *((int*)f->esp + 1);
  struct thread* t = thread_current();
  struct open_file* my_file = get_file(fd);

  if (my_file == NULL)
  {
    return;
  }
  lock_acquire(&files_sys_lock);
  file_close(my_file->ptr);
  lock_release(&files_sys_lock);
  list_remove(&my_file->elem);
  palloc_free_page(my_file);
}

struct open_file* get_file(int fd){
    struct thread* t = thread_current();
    struct file* my_file = NULL;
    for (struct list_elem* e = list_begin (&t->opened_files_list); e != list_end (&t->opened_files_list);
    e = list_next (e))
    {
      struct open_file* opened_file = list_entry (e, struct open_file, elem);
      if (opened_file->file_desc == fd)
      {
        return opened_file;
      }
    }
    return NULL;
}