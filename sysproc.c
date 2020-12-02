#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0; // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if (argint(0, &pid) < 0) return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if (argint(0, &n) < 0) return -1;
	addr = myproc()->sz;
	if (growproc(n) < 0) return -1;
	return addr;
}

int
sys_sleep(void)
{
	int  n;
	uint ticks0;

	if (argint(0, &n) < 0) return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

int
sys_m_get(void)
{
	char *name;

	if (argptr(0,(char**)&name,sizeof(name)) < 0) return -1;

	if (shm_get(name) < 0) return -1;

	return 0;
}

int
sys_m_rem(void)
{
	char *name;

	if (argptr(0,(char**)&name,sizeof(name)) < 0) return -1;

	return shm_rem(name);
}

int
sys_cm_create_and_enter(void)
{
	/* TODO: might need params here */
	return cm_create_and_enter();
}

int
sys_cm_setroot(void)
{
	char* path;
	int path_len;

	if((argstr(0, &path) < 0) || (argint(1, &path_len) < 0))
	{
		return -1;
	}

	return cm_setroot(path, path_len);
}

int
sys_cm_maxproc(void)
{
	int nproc;

	if(argint(0, &nproc) < 0){
		return -1;
	}

	return cm_maxproc(nproc);
}


