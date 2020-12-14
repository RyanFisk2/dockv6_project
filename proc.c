#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "cm.h"
#include "fs.h"
#include "file.h"
#include "shmem.h"
#include "cas.h"

#ifndef NBIN
#define NBIN 16
#endif

struct {
	struct spinlock lock;
	struct proc     proc[NPROC];
} ptable;

struct {
	struct spinlock lock;
	struct container Arr[MAX_NUM_CONTAINERS];
} containers;

struct {
	struct spinlock lock;
	struct mutex 	mux[MUX_MAXNUM];
} mtable;

struct {
	struct spinlock lock;
	struct list Arr[NBIN];
} pqueue;

static struct proc *initproc;

int         nextpid = 1;
int	    nextcid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pqueue_enqueue(struct proc *p, uint priority)
{
	p->priority = priority;
	acquire(&pqueue.lock);
	struct list *bin = &pqueue.Arr[priority];


	if (bin->head == (struct proc *)0) {
		bin->head = bin->tail = p;

	} else{
		bin->tail->next = p;
		bin->tail = p;
	}
	bin->size++;

	p->next = (struct proc*)0;
	release(&pqueue.lock);

}

void
queueinit(void)
{
	initlock(&pqueue.lock, "pqueue");
	for (int i = 0; i < NBIN; i++) {
		pqueue.Arr[i].head = (struct proc*)0;
		pqueue.Arr[i].tail = (struct proc*)0;
		pqueue.Arr[i].size = 0;
	}
}

void
pinit(void)
{
	struct proc *p;
	initlock(&ptable.lock, "ptable");
	
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		p->container_id = 0;
	}
}

void
cinit(void)
{
	initlock(&containers.lock, "containers");
  	initlock(&mtable.lock, "mtable");
	struct container *c;
	for(c = containers.Arr; c < &containers.Arr[MAX_NUM_CONTAINERS]; c++){
		c->container_id = -1;
		c->nproc = -1;

	}
}

// Must be called with interrupts disabled
int
cpuid()
{
	return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being rescheduled
// between reading lapicid and running through the loop.
struct cpu *
mycpu(void)
{
	int apicid, i;

	if (readeflags() & FL_IF) panic("mycpu called with interrupts enabled\n");

	apicid = lapicid();
	// APIC IDs are not guaranteed to be contiguous. Maybe we should have
	// a reverse map, or reserve a register to store &cpus[i].
	for (i = 0; i < ncpu; ++i) {
		if (cpus[i].apicid == apicid) return &cpus[i];
	}
	panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc(void)
{
	struct cpu * c;
	struct proc *p;
	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();
	return p;
}

// PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(uint priority)
{
	struct proc *p;
	char *       sp;

	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED) goto found;

	release(&ptable.lock);
	return 0;

found:
	p->state = EMBRYO;
	p->pid   = nextpid++;
	pqueue_enqueue(p,priority);

	p->container_id = 0;

	for (int i = 0; i < SHM_MAXNUM; i++) {
		p->shared_mem[i].in_use = 0;
	}
	for(int i = 0; i < MUX_MAXNUM; i++){
		p->mutex[i] = (struct mutex*)0;
	}

	release(&ptable.lock);

	// Initialize proc's local mutex array
	memset((void*)p->mutex,0,sizeof(p->mutex));

	// Allocate kernel stack.
	if ((p->kstack = kalloc()) == 0) {
		p->state = UNUSED;
		return 0;
	}
	sp = p->kstack + KSTACKSIZE;

	// Leave room for trap frame.
	sp -= sizeof *p->tf;
	p->tf = (struct trapframe *)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(uint *)sp = (uint)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context *)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (uint)forkret;
	return p;
}

// PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
	struct proc *p;
	extern char  _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc(0);

	initproc = p;
	if ((p->pgdir = setupkvm()) == 0) panic("userinit: out of memory?");
	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
	p->sz = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));
	p->tf->cs     = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ds     = (SEG_UDATA << 3) | DPL_USER;
	p->tf->es     = p->tf->ds;
	p->tf->ss     = p->tf->ds;
	p->tf->eflags = FL_IF;
	p->tf->esp    = PGSIZE;
	p->tf->eip    = 0; // beginning of initcode.S

	safestrcpy(p->name, "initcode", sizeof(p->name));
	p->cwd = namei("/");
	p->container_id = 0;

	// this assignment to p->state lets other cores
	// run this process. the acquire forces the above
	// writes to be visible, and the lock is also needed
	// because the assignment might not be atomic.
	acquire(&ptable.lock);

	p->state = RUNNABLE;


	release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
	uint         sz;
	struct proc *curproc = myproc();

	sz = curproc->sz;
	if (n > 0) {
		if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0) return -1;
	} else if (n < 0) {
		if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0) return -1;
	}
	curproc->sz = sz;
	switchuvm(curproc);
	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
	int          i, pid;
	struct proc *np;
	struct shmem *newproc_shmem, *parent_shmem;
	struct proc *curproc = myproc();

	// Allocate process.
	if (curproc -> container_id)
	{

		//in a container, need to limit number of forks
		if(curproc->container->nproc == 0)
		{
			//reached limit on procs, return ERR
			return -1;
		}else{
			curproc->container->nproc --;
		}
	}

	if ((np = allocproc(curproc->priority)) == 0) {
		return -1;
	}

	// Copy process state from proc.
	if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
		kfree(np->kstack);
		np->kstack = 0;
		np->state  = UNUSED;
		return -1;
	}
	np->sz     = curproc->sz;
	np->parent = curproc;
	*np->tf    = *curproc->tf;
	np -> container = curproc -> container;
	np -> container_id = curproc -> container_id;

	for (int i = 0; i < SHM_MAXNUM; i++) {
		if (strncmp(curproc->name,"cm",strlen(curproc->name)) != 0) {
			newproc_shmem = &np->shared_mem[i];
			parent_shmem = &curproc->shared_mem[i];
			newproc_shmem->in_use = parent_shmem->in_use;
			strncpy(newproc_shmem->name,parent_shmem->name,strlen(parent_shmem->name));
			newproc_shmem->va = parent_shmem->va;
			newproc_shmem->global_ptr = parent_shmem->global_ptr;
			newproc_shmem->global_ptr->refcount++;
		}
	}

	// Clear %eax so that fork returns 0 in the child.
	np->tf->eax = 0;

	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i]) np->ofile[i] = filedup(curproc->ofile[i]);
	np->cwd = idup(curproc->cwd);

	if (strncmp(curproc->name,"cm",strlen(curproc->name)) != 0) {
		safestrcpy(np->name, curproc->name, sizeof(curproc->name));
	} else{
		safestrcpy(np->name,np->container->init,strlen(np->container->init));
	}
	pid = np->pid;

	/* if not global(not in container) shell or container manager, set priority=1 so priority is lower than container manager */

	if ( (curproc->pid > 1) && strncmp(np->name,"cm",strlen(np->name)) != 0) {
		prio_set(pid,1);
	}

	prio_set(np->pid,curproc->priority);

	acquire(&ptable.lock);

	np->state = RUNNABLE;

	release(&ptable.lock);

	return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
	struct proc *curproc = myproc();
	struct proc *p;
	int          fd;


	if (curproc == initproc) panic("init exiting");


	// Close all open files.
	for (fd = 0; fd < NOFILE; fd++) {
		if (curproc->ofile[fd]) {
			fileclose(curproc->ofile[fd]);
			curproc->ofile[fd] = 0;
		}
	}

	// remove shared mem

	for (int i = 0; i < SHM_MAXNUM; i++) {
		if (curproc->shared_mem[i].in_use) {
			shm_rem(curproc->shared_mem[i].name);
		}
	}
	// Remove access to all mutexes held in the proc struct
	for(int i = 0; i < MUX_MAXNUM; i++){
		if(curproc->mutex[i]){
			mutex_delete(i);
		}
	}

	begin_op();
	iput(curproc->cwd);
	end_op();
	curproc->cwd = 0;

	acquire(&ptable.lock);

	// Parent might be sleeping in wait().
	wakeup1(curproc->parent);

	// Pass abandoned children to init.
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->parent == curproc) {
			p->parent = initproc;
			if (p->state == ZOMBIE) wakeup1(initproc);
		}
	}

	curproc->state = ZOMBIE;
	sched();
	panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
	struct proc *p;
	int          havekids, pid;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (;;) {
		// Scan through table looking for exited children.
		havekids = 0;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->parent != curproc) continue;
			havekids = 1;
			if (p->state == ZOMBIE) {
				// Found one.
				pid = p->pid;
				kfree(p->kstack);
				p->kstack = 0;
				freevm(p->pgdir);
				p->pid     = 0;
				p->parent  = 0;
				p->name[0] = 0;
				p->killed  = 0;
				p->state   = UNUSED;
				release(&ptable.lock);

				if(curproc -> container_id)
				{
					//freed proc, so we can allocate another in this container
					curproc -> container -> nproc ++;
				}
				return pid;
			}
		}

		// No point waiting if we don't have any children.
		if (!havekids || curproc->killed) {
			release(&ptable.lock);
			return -1;
		}

		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
		sleep(curproc, &ptable.lock); // DOC: wait-sleep
	}
}

// PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

void
scheduler(void)
{
	struct proc *p;
	struct cpu * c = mycpu();
	c->proc        = 0;

	for (;;) {
		// Enable interrupts on this processor.
		sti();

		// Loop over process table looking for process to run.
		acquire(&ptable.lock);
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->state != RUNNABLE) continue;

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;

			swtch(&(c->scheduler), p->context);
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			c->proc = 0;
		}
		release(&ptable.lock);
	}
}
/*
void
scheduler(void)
{
	struct proc *p;
	struct cpu * c = mycpu();
	c->proc        = 0;
	struct list *bin;

	for (;;) {
start:
		// Enable interrupts on this processor.
		sti();

		// Loop over process table looking for process to run.
		acquire(&ptable.lock);

		for (int i = 0; i < NBIN; i++) {
			bin = &pqueue.Arr[i];
			if (bin->head != (struct proc*)0) {			
				
				p = bin->head;
				while (p != (struct proc*)0) {

					if (p->state == RUNNABLE) {
				
						for (int j = 0; j < NCPU; j++) {
							if(cpus[j].proc != 0 && (&cpus[j] != c)) {
								if ((cpus[j].proc->priority < p->priority) && (cpus[j].proc->state == RUNNING)) {
									release(&ptable.lock);
									goto start;
								}						
							}
						}

						c->proc = p;
						i = 0;
						switchuvm(p);
						p->state = RUNNING;

						swtch(&(c->scheduler),p->context);
						
						switchkvm();
						c->proc = 0;				
					}
					
					p = p->next;
				}
			}
		}
		release(&ptable.lock);
	}
}
*/
// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
	int          intena;
	struct proc *p = myproc();
	if (!holding(&ptable.lock)) panic("sched ptable.lock");
	if (mycpu()->ncli != 1) panic("sched locks");
	if (p->state == RUNNING) panic("sched running");
	if (readeflags() & FL_IF) panic("sched interruptible");
	intena = mycpu()->intena;
	swtch(&p->context, mycpu()->scheduler);
	mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
	acquire(&ptable.lock); // DOC: yieldlock
	myproc()->state = RUNNABLE;
	sched();
	release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
	static int first = 1;
	// Still holding ptable.lock from scheduler.
	release(&ptable.lock);

	if (first) {
		// Some initialization functions must be run in the context
		// of a regular process (e.g., they call sleep), and thus cannot
		// be run from main().
		first = 0;
		iinit(ROOTDEV);
		initlog(ROOTDEV);
	}

	// Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
	struct proc *p = myproc();

	if (p == 0) panic("sleep");

	if (lk == 0) panic("sleep without lk");

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	if (lk != &ptable.lock) {      // DOC: sleeplock0
		acquire(&ptable.lock); // DOC: sleeplock1
		release(lk);
	}
	// Go to sleep.
	p->chan  = chan;
	p->state = SLEEPING;

	sched();

	// Tidy up.
	p->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) { // DOC: sleeplock2
		release(&ptable.lock);
		acquire(lk);
	}
}

// PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
	struct proc *p;
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == SLEEPING && p->chan == chan) p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
	struct proc *p;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if ((p->pid == pid) && (p->container_id == curproc -> container_id)) {
			p->killed = 1;
			/*
			 * If kill() has been called on a process, we need to check to make sure that it no longer has access
			 * to any of its mutexes, along with checking for status of that process. If it is being blocked in cv_wait, we must make it pass through 
			 * by setting the condition variable to 1, then setting the state to runnable. We also delete all access to any other mutexes. 
			*/
			acquire(&mtable.lock);
			struct mutex *mux;
			for(int i = 0; i < MUX_MAXNUM; i++){
				mux = p->mutex[i];
				if(mux){
					if (p->state == SLEEPING && p->chan == mux) {
						cas((unsigned long*)&(mux->cv),0, 1);
					}
					if(mux->refcount == 1){
						mux->isAlloc = 0;
						mux->refcount = 0;
						mux->locked = 0;
						mux->cv = 0;
						strncpy(mux->name, "", strlen(""));
						mux->container_id = 0;
					}else{
						mux->locked = 0;
						mux->refcount--;
					}
				}	
			}
			release(&mtable.lock);

			// Wake process from sleep if necessary.
			if (p->state == SLEEPING) p->state = RUNNABLE;
			release(&ptable.lock);
			return 0;
		}
	}
	release(&ptable.lock);
	return -1;
}

// PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
	static char *states[] = {[UNUSED] "unused",   [EMBRYO] "embryo",  [SLEEPING] "sleep ",
	                         [RUNNABLE] "runble", [RUNNING] "run   ", [ZOMBIE] "zombie"};
	int          i;
	struct proc *p;
	char *       state;
	uint         pc[10];

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state == UNUSED) continue;
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
			state = states[p->state];
		else
			state = "???";
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			getcallerpcs((uint *)p->context->ebp + 2, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++) cprintf(" %p", pc[i]);
		}
		cprintf("\n");
	}
}


/* BELOW ARE THE CONTAINER MANAGER SYS CALLS */

/*
 * create and enter checks for an open container to use (up to MAX_NUM_CONTAINERS)
 * and initializes the initial process, file system root, and maximum child procs for the new container.
 * Then forks a child to run the initial process while the parent (Container Manager) waits for it to finish.
 * 
 * Since fork returns to user space, this function returns a single integer to use as a boolean in cm.c
 * returns 1 for the child process so we exec the init process, 0 for the parent so that we do not exec the init process twice
 */
int
cm_create_and_enter(char *init, char *fs, int nproc)
{

	struct container *c;
	struct proc *curproc = myproc();

	strncpy(curproc->name,"cm",strlen("cm"));

	acquire(&containers.lock);
		for (c = containers.Arr; c < &containers.Arr[MAX_NUM_CONTAINERS]; c++) {
			if (c->container_id == -1) goto found;
		}
	release(&containers.lock);

	return -1;

found:
	c->container_id = nextcid++;
	strncpy(c->init,init,strlen(init));
	release(&containers.lock);
	


	curproc -> container = c;

	//set root for container
	begin_op();
	cm_setroot(fs,strlen(fs),c);
	curproc->cwd = idup(c->root);
	end_op();

	curproc -> container_id = c -> container_id;
	cm_maxproc(nproc);

	int child = fork();
	if (child != 0) {

		curproc->container_id = 0;
		return 1;
	}else{
		return 0;
	}

}

/*
 * cm_maxproc sets the maximum number of procs that can be forked in the container
 * 
 * returns an error if we try to assign more than NPROC to any one container
 * or if nproc has already been set for this container 
 * or if we are not in a container
 */
 
int
cm_maxproc(int nproc)
{
	if(nproc >= NPROC){
		return -1;
	}

	struct proc *p = myproc();
	struct container *c = p->container;

	if ((c->nproc != -1) || (p->container_id == 0))
	{
		return -1;
	}


	c -> nproc = nproc;

	return 1;
}

/*
 * setroot gets an inode for the given path, and sets the 
 * root for the container to that inode
 */
int
cm_setroot(char* path, int path_len, struct container *container)
{
	struct inode *root_node;
	if (path_len <= 0) return -1;
	root_node = namei(path);
	
	memmove(&container->root,&root_node, sizeof(root_node));
	safestrcpy(container->fs, path, strlen(path));
	
	return 1;
}

/*
 * Go through mtable. Save the first unallocated mutex, so long as we do not
 * find one with the same name as *name from the same container. If we find one with the same name&container,
 * goto addtoProc, as all we need to do is give our current process access, and return its muxid.
 * Otherwise, we need to set up a new mutex with the 'unallocated' variable
 */
int mutex_create(char *name){
	struct proc *p = myproc();
	struct mutex *m;
	int muxid = -1;
	struct mutex *unallocated = (struct mutex*)0;

	 

	// Go through mtable, find which path we need to take
	acquire(&mtable.lock);
	for(m = mtable.mux; m < &mtable.mux[MUX_MAXNUM]; m++){

		if(m->isAlloc == 0 && unallocated == (struct mutex*)0 ){
			unallocated = m;
		}else if(strncmp(name, m->name, strlen(name)) == 0 && m->container_id == p->container_id){
			goto addtoProc;
		}
	}

	// That lock name was not found, so we need to create it.
	if(unallocated != (struct mutex*)0){
		goto setupMutex;
	}


	// We didn't have space to make another lock, so return -1.
	release(&mtable.lock);
	return -1;

addtoProc:
	// Save the first possible index we can place our mutex in our process struct. Check to make sure this process doesnt already have access
	for(int i = 0; i < MUX_MAXNUM; i++){
		if(p->mutex[i] == (struct mutex*)0 && muxid == -1){
			muxid = i;
		}else if(p->mutex[i] == m){

			release(&mtable.lock);
			return -1;
		}
	}
	// If we are able to allocate the process, do so. Otherwise, return -1.
	if(muxid != -1){
		p->mutex[muxid] = m;
		m->refcount++;
		release(&mtable.lock);
		return muxid;
	}else{

		release(&mtable.lock);
		return -1;
	}
	
setupMutex:
	// General setup code for a new mutex
	unallocated->isAlloc = 1;
	strncpy(unallocated->name,name,strlen(name));
	unallocated->refcount = 1;
	unallocated->container_id = p->container_id;
	unallocated->cv = 0;

	// Find the first available location in the proc struct
	for(int i = 0; i < MUX_MAXNUM; i++){
		if(p->mutex[i] == (struct mutex*)0){
			muxid = i;
			break;
		}
	}

	// If we have space in the process to save the lock, save it, otherwise return -1.
	if(muxid != -1){
		p->mutex[muxid] = unallocated;
		release(&mtable.lock);
		return muxid;
	}else{

		release(&mtable.lock);
		return -1;
	}

	return muxid;
}

/*
 * Returns 1 if the mutex is locked by the current process, otherwise 0. Takes a mutex pointer.
*/
int
mutex_holding(struct mutex *mux)
{
	return mux->locked && mux->pid == myproc()->pid;
}

/*
 * First, check if that mutex id corresponds to a valid mutex. If it does, remove that process' access to the mutex.
 * Then, if that process was the only reference to that mutex, delete it so that space may be used later for a new mutex.
 * Otherwise, unlock the mutex if it was holding it and decrease the refcount. 
*/
void mutex_delete(int muxid){
	struct proc *curr_proc = myproc();
	struct mutex *m = curr_proc->mutex[muxid];

	acquire(&mtable.lock);
	//If our current process has access to a mutex at this id
	if (m){
		curr_proc->mutex[muxid] = 0; /* remove proc's access to lock from the proc struct */
		if (m->refcount == 1) { /* deallocate lock */
			m->isAlloc = 0; 
			m->refcount = 0;
			strncpy(m->name,"",strlen(""));
			m->container_id = 0;
			m->locked = 0;
			release(&mtable.lock);
			return;
		}
	}else{
		release(&mtable.lock);

		return; /* curproc doesn't have access to lock */
	}
	if(mutex_holding(m)){
		m->locked = 0;
	}
	m->refcount--;
	release(&mtable.lock);
	return;
}

/*
 * If the passed in mutex is valid and held by the current process, unlock it and wakeup all other processes sleeping on chan "mux"
 * Otherwise, return nothing.
*/
void mutex_unlock(int muxid){
	struct proc *curr_proc = myproc();
	struct mutex *mux = curr_proc->mutex[muxid];

	acquire(&mtable.lock);
	if(mux && mutex_holding(mux)){
		mux->locked = 0;
		release(&mtable.lock);
		wakeup(mux);
		return;
	}else{

		release(&mtable.lock);
		return;
	}
}

/*
 * If the process passes in a valid mutex and isn't already holding it, try to take it. If unable, sleep on chan "mux".
 * Once able, take the lock and set the pid of the lock to the current process, then return.
*/

void mutex_lock(int muxid){
	struct proc *curr_proc = myproc();
	struct mutex *mux = curr_proc->mutex[muxid];

	
	/* if process doesn't have access to mutex (i.e. curr_proc->mutex[muxid]==0) or already owns the lock, return */
	if(!mux || mutex_holding(mux)) {

		return;
	}


	acquire(&mtable.lock);
	while(mux->locked == 1){
		sleep(mux, &mtable.lock);
	}
	mux->locked = 1;
	mux->pid = curr_proc->pid;
	release(&mtable.lock);
	return;
}

/*
 * If the process passes in a valid mutex that it is holding, unlock the mutex and try to change the condition variable from 1 to 0. 
 * If it fails, sleep the process on chan "mux" to try again later.
 * On success, relock the mutex, and return. 
*/

void cv_wait(int muxid){
	struct proc *curr_proc = myproc();
	struct mutex *mux = curr_proc->mutex[muxid];
	if(mux && mutex_holding(mux)){

		mutex_unlock(muxid);
		//Maybe add a condition variable that gets changed on wakeup(muxid)? Maybe just continue with code. 
		// I don't know how the container calls EXACTLY work, but im assumming it similar to pipes
		while(1){ /*we have not recieved reply from container manager*/
			if (cas((unsigned long*)&(mux->cv),1, 0) != 0) break;
			acquire(&ptable.lock);
			curr_proc->chan = mux;
			curr_proc->state = SLEEPING;
			sched();
			release(&ptable.lock);
		}
		
		mutex_lock(muxid);
	} else{

	}
	return;
}

/*
 * If the process passes in a valid mutex, try to set the condition variable to 1 if it is 0. 
 * On fail, wakeup all processes sleeping on "mux", as a process is waiting to be signaled.
 * On sucess, change the condition variable to 1, wakeup all processes sleeping on "mux", and return. 
*/

void cv_signal(int muxid){
	struct mutex *mux = myproc()->mutex[muxid];
	int done = 0;
	if(mux){
		//Keep looping until we successfully signal
		while(done == 0){
			acquire(&mtable.lock);
			if(mux->cv == 0){
				mux->cv = 1;
				done = 1;
			}
			release(&mtable.lock);
			wakeup(mux);
		}
	}

	return;
}

int
prio_set(int pid, int priority)
{
	struct proc *p = (struct proc*)0;
	struct list *bin;
	struct proc *proc = (struct proc*)0;
	struct proc *temp, *temp_parent;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->pid == pid) break;
	}
	release(&ptable.lock);

	if (p->pid != pid) return -1; /* no proc w/ given pid */

	
	if (priority < p->priority) {
		return -1; /* cant set priority higher than curr priority */
	}

	if (p == (struct proc*)0) return -1; /* no proc w/ given pid exists */
	temp = p;
	if (pid != curproc->pid) {
		while(1) {
			temp_parent = temp->parent;
			if (temp_parent == curproc) break;
			if (temp_parent == initproc) {
				return -1; /* calling proc not in ancestry */
			}
			temp = temp_parent;
		}
	}

	if (priority == p->priority) return 0; /* no point in setting prio to current prio */

	acquire(&pqueue.lock);
	for (int i = 0; i < NBIN; i++) {
		bin = &pqueue.Arr[i];
		if ( (p = bin->head) == (struct proc*)0) continue;
		if (p->pid == pid) {
			proc = p;
			break;
		}
		while (p->next != (struct proc*)0) {
			if (p->next->pid == pid) {
				proc = p->next;
				p->next = p->next->next; /* remove proc from old bin */
				break;
			}
			p = p->next;
		}
	}
	release(&pqueue.lock);
	pqueue_enqueue(proc,priority); /* add proc to new bin */
	return 0;
}
