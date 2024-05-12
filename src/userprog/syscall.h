#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void system_exit(int status);//exit thread
void validate_ptr(const void* f); //validate ptr and exit if not valid

struct open_file* get_file(int fd);


#endif /* userprog/syscall.h */
