#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"


static void syscall_handler (struct intr_frame *);

static struct lock files_sys_lock;               /* lock for syschronization between files */

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void system_exit(int status){
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}
void validate_ptr(const void* f){
  if (f == NULL || pagedir_get_page(thread_current()->pagedir, f) == NULL){
      system_exit(-1);
    }
}
struct open_file* get_file(int fd){
    struct thread* t = thread_current();
    struct file* my_file = NULL;
    for (struct list_elem* e = list_begin (&t->open_file_list); e != list_end (&t->open_file_list);
    e = list_next (e))
    {
      struct open_file* opened_file = list_entry(e, struct open_file, elem);
      if (opened_file->fd == fd)
      {
        return opened_file;
      }
    }
    return NULL;
}
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  char* name;
  bool response;
  struct thread* t = thread_current();
  struct file* my_file;
  int fd, size;
  void* buffer;
  //printf ("system call!\n");
  //thread_exit ();
  validate_ptr(f->esp);
  int syscall_num = *(int *)f->esp;
  switch(syscall_num){
    case SYS_HALT:
      //printf ("system call Haltttttttttt!\n");
      shutdown_power_off();
      break;
    case SYS_EXIT:
      validate_ptr(f->esp+4);
      int status = *((int*)f->esp + 1);
      system_exit(status);
      break;
    case SYS_EXEC:
      printf ("system call Exec\n");
      //strat
      break;
    case SYS_WAIT:
    //wait bta3 7oda
    break;

  case SYS_CREATE:
    //printf("Create system call engadged\n");
    validate_ptr(f->esp + 4);
    validate_ptr(f->esp + 8);
    name = (char*)(*((int*)f->esp + 1));
    size_t size = *((int*)f->esp + 2);
    if (name == NULL){
      system_exit(-1);
    }
    lock_acquire(&files_sys_lock);
    response = filesys_create(name,size);
    lock_release(&files_sys_lock);
    f->eax = response;
    break;

  case SYS_REMOVE:
    validate_ptr(f->esp + 4);
    name = (char*)(*((int*)f->esp + 1));
    if (name == NULL) {
      system_exit(-1);
    }
    lock_acquire(&files_sys_lock);
    response = filesys_remove(name);
    lock_release(&files_sys_lock);
    f->eax = response;
    break;

  case SYS_OPEN:
    validate_ptr(f->esp + 4);
    name = (char*)(*((int*)f->esp + 1));
    if (name == NULL) {
      system_exit(-1);
    }
    struct open_file* open = palloc_get_page(0);
    if (open == NULL) 
    {
      palloc_free_page(open);
      f->eax = -1;
      break;
    }
    lock_acquire(&files_sys_lock);
    open->ptr = filesys_open(name);
    lock_release(&files_sys_lock);
    if (open->ptr == NULL)
    {
      f->eax = -1;
      break;
    }
    open->fd = ++thread_current()->fd_last;
    list_push_back(&thread_current()->open_file_list,&open->elem);
    f->eax = open->fd;
    break;

  case SYS_FILESIZE:
    validate_ptr(f->esp + 4);
    fd = *((int*)f->esp + 1);
    my_file = get_file(fd)->ptr;
    if (my_file == NULL)
    {
      f->eax = -1;
      break;
    }
    int response;
    lock_acquire(&files_sys_lock);
    response = file_length(my_file);
    lock_release(&files_sys_lock);
    f->eax = response;
    break;

  case SYS_READ:
    validate_ptr(f->esp + 4);
    validate_ptr(f->esp + 8);
    validate_ptr(f->esp + 12);
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
      f->eax = size;
      break;
    } else {
      my_file = get_file(fd)->ptr;
      if (my_file == NULL)
      {
        f->eax = -1;
        break;
      }
      int response;
      lock_acquire(&files_sys_lock);
      response = file_read(my_file,buffer,size);
      lock_release(&files_sys_lock);
      f->eax = response;
    }
    break;

  case SYS_WRITE:
    validate_ptr(f->esp + 4);
    validate_ptr(f->esp + 8);
    validate_ptr(f->esp + 12);
    fd = *((int*)f->esp + 1);
    buffer = (void*)(*((int*)f->esp + 2));
    size = *((int*)f->esp + 3);
    if (buffer == NULL){
      system_exit(-1);
    }

    if (fd == 1)
    {
      lock_acquire(&files_sys_lock);
      putbuf(buffer,size);
      lock_release(&files_sys_lock);
      f->eax = size;
      break;
    } else {
      my_file = get_file(fd)->ptr;
      if (my_file == NULL)
      {
        f->eax = -1;
        break;
      }
      int response;
      lock_acquire(&files_sys_lock);
      response = file_write(my_file,buffer,size);
      lock_release(&files_sys_lock);
      f->eax = response;
    }
    break;

  case SYS_SEEK:
    validate_ptr(f->esp + 4);
    validate_ptr(f->esp + 8);
    unsigned pos;
    fd = *((int*)f->esp + 1);
    pos = *((unsigned*)f->esp + 2);

    my_file = get_file(fd)->ptr;
    if (my_file == NULL)
    {
      break;
    }
    lock_acquire(&files_sys_lock);
    file_seek(my_file,pos);
    lock_release(&files_sys_lock);
    break;

  case SYS_TELL:
    validate_ptr(f->esp + 4);
    fd = *((int*)f->esp + 1);
    my_file = get_file(fd)->ptr;
    if (my_file == NULL)
    {
      f->eax = -1;
      break;
    }
    unsigned response2;
    lock_acquire(&files_sys_lock);
    response2 = file_tell(my_file);
    lock_release(&files_sys_lock);
    f->eax = response2;
    break;

  case SYS_CLOSE:
    validate_ptr(f->esp + 4);
    fd = *((int*)f->esp + 1);

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
    break;

  default:
    // negative area
    break;

  }
  
}
