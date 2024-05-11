#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

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
  //thread_exit ();
  //if (f == NULL || ){

  //}
  int syscall_num = *(int *)f->esp;
  switch(syscall_num){
    case SYS_HALT:
      //printf ("system call Haltttttttttt!\n");
      shutdown_power_off();
      break;
    case SYS_EXIT:
      int status = *((int*)f->esp + 1); 
      printf("%s: exit(%d)\n", thread_current()->name, status);
      break;
    case SYS_EXEC:
      //strat
      break;
    case SYS_WAIT:
    //wait bta3 7oda
    break;

  case SYS_CREATE:
    //create_wrapper(f);
    break;

  case SYS_REMOVE:
    //remove_wrapper(f);
    break;

  case SYS_OPEN:
    //open_wrapper(f);
    break;

  case SYS_FILESIZE:
    //filesize_wrapper(f);
    break;

  case SYS_READ:
    //read_wrapper(f);
    break;

  case SYS_WRITE:
    //write_wrapper(f);
    break;

  case SYS_SEEK:
    //seek_wrapper(f);
    break;

  case SYS_TELL:
    //tell_wrapper(f);
    break;

  case SYS_CLOSE:
    //close_wrapper(f);
    break;

  default:
    // negative area
    break;

  }
  
}
