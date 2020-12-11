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

	if (argptr(0,(char**)&name,sizeof(name)) < 0 ) return -1;


	return shm_get(name);
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
	char *init, *fs;
	int nproc;

	if ((argstr(0, &init) < 0) || (argstr(1, &fs) < 0) || (argint(2, &nproc) < 0)) return -1;
	return cm_create_and_enter(init, fs, nproc);
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

int sys_mutex_create(void){
	char *name;
	if(argstr(0, &name) < 0) return -1;
	return mutex_create(name);
}

void sys_mutex_delete(void){
	int muxid;

	if (argint(0, &muxid) < 0) return;
	mutex_delete(muxid);
	return;
}

void sys_mutex_lock(void){
	int muxid;

	if (argint(0, &muxid) < 0) return;
	mutex_lock(muxid);	
	return;
}

void sys_mutex_unlock(void){
	int muxid;

	if (argint(0, &muxid) < 0) return;
	mutex_unlock(muxid);
	return;
}

void sys_cv_wait(void){
	int muxid;

	if (argint(0, &muxid) < 0) return;
	cv_wait(muxid);
	return;
}

void sys_cv_signal(void){
	int muxid;

	if (argint(0, &muxid) < 0) return;
	cv_signal(muxid);
	return;
}

int
sys_prio_set(void)
{
	int pid, priority;

	if (argint(0,&pid) < 0 || argint(1,&priority) < 0) return -1;
	return prio_set(pid,priority);
}