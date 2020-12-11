#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(m_get)
SYSCALL(m_rem)
SYSCALL(cm_create_and_enter)
SYSCALL(cm_setroot)
SYSCALL(cm_maxproc)
SYSCALL(copy_file)
SYSCALL(mutex_create)
SYSCALL(mutex_delete)
SYSCALL(mutex_lock)
SYSCALL(mutex_unlock)
SYSCALL(cv_wait)
SYSCALL(cv_signal)