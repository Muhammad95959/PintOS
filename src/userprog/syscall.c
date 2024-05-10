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

      break;
    case SYS_EXEC:
      //strat
      break;

  }
  
}
