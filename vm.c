#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "spinlock.h"
#include "shmem.h"

#ifndef SHM_MAXNUM
#define SHM_MAXNUM 4
#endif

extern char data[]; // defined by kernel.ld
pde_t *     kpgdir; // for use in scheduler()

struct {
	struct spinlock 	lock;
	struct shared_mem	list[SHM_MAXNUM];
}shm_list;

void
init_shm_list(void)
{
	//struct shared_mem *ptr;
	initlock(&shm_list.lock, "shm_list");

	acquire(&shm_list.lock);
	for (int i = 0; i < SHM_MAXNUM; i++) {
		shm_list.list[i].refcount = 0;
		shm_list.list[i].in_use = 0;
		shm_list.list[i].container_id = 0;
	}
//	memset(&shm_list.list->refcount,0,SHM_MAXNUM);
	release(&shm_list.lock);
}


// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
	struct cpu *c;

	// Map "logical" addresses to virtual addresses using identity map.
	// Cannot share a CODE descriptor for both kernel and user
	// because it would have to have DPL_USR, but the CPU forbids
	// an interrupt from CPL=0 to DPL=3.
	c                 = &cpus[cpuid()];
	c->gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
	c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
	c->gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
	c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
	lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
	pde_t *pde;
	pte_t *pgtab;

	pde = &pgdir[PDX(va)];
	if (*pde & PTE_P) {
		pgtab = (pte_t *)P2V(PTE_ADDR(*pde));
	} else {
		if (!alloc || (pgtab = (pte_t *)kalloc()) == 0) return 0;
		// Make sure all those PTE_P bits are zero.
		memset(pgtab, 0, PGSIZE);
		// The permissions here are overly generous, but they can
		// be further restricted by the permissions in the page table
		// entries, if necessary.
		*pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
	char * a, *last;
	pte_t *pte;

	a    = (char *)PGROUNDDOWN((uint)va);
	last = (char *)PGROUNDDOWN(((uint)va) + size - 1);
	for (;;) {
		if ((pte = walkpgdir(pgdir, a, 1)) == 0) return -1;
		if (*pte & PTE_P) panic("remap");
		*pte = pa | perm | PTE_P;
		if (a == last) break;
		a += PGSIZE;
		pa += PGSIZE;
	}
	return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap {
	void *virt;
	uint  phys_start;
	uint  phys_end;
	int   perm;
} kmap[] = {
  {(void *)KERNBASE, 0, EXTMEM, PTE_W},            // I/O space
  {(void *)KERNLINK, V2P(KERNLINK), V2P(data), 0}, // kern text+rodata
  {(void *)data, V2P(data), PHYSTOP, PTE_W},       // kern data+memory
  {(void *)DEVSPACE, DEVSPACE, 0, PTE_W},          // more devices
};

// Set up kernel part of a page table.
pde_t *
setupkvm(void)
{
	pde_t *      pgdir;
	struct kmap *k;

	if ((pgdir = (pde_t *)kalloc()) == 0) return 0;
	memset(pgdir, 0, PGSIZE);
	if (P2V(PHYSTOP) > (void *)DEVSPACE) panic("PHYSTOP too high");
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
		if (mappages(pgdir, k->virt, k->phys_end - k->phys_start, (uint)k->phys_start, k->perm) < 0) {
			freevm(pgdir);
			return 0;
		}
	return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
	kpgdir = setupkvm();
	switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
	lcr3(V2P(kpgdir)); // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void
switchuvm(struct proc *p)
{
	if (p == 0) panic("switchuvm: no process");
	if (p->kstack == 0) panic("switchuvm: no kstack");
	if (p->pgdir == 0) panic("switchuvm: no pgdir");

	pushcli();
	mycpu()->gdt[SEG_TSS]   = SEG16(STS_T32A, &mycpu()->ts, sizeof(mycpu()->ts) - 1, 0);
	mycpu()->gdt[SEG_TSS].s = 0;
	mycpu()->ts.ss0         = SEG_KDATA << 3;
	mycpu()->ts.esp0        = (uint)p->kstack + KSTACKSIZE;
	// setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
	// forbids I/O instructions (e.g., inb and outb) from user space
	mycpu()->ts.iomb = (ushort)0xFFFF;
	ltr(SEG_TSS << 3);
	lcr3(V2P(p->pgdir)); // switch to process's address space
	popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
	char *mem;

	if (sz >= PGSIZE) panic("inituvm: more than a page");
	mem = kalloc();
	memset(mem, 0, PGSIZE);
	mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
	memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
	uint   i, pa, n;
	pte_t *pte;

	if ((uint)addr % PGSIZE != 0) panic("loaduvm: addr must be page aligned");
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0) panic("loaduvm: address should exist");
		pa = PTE_ADDR(*pte);
		if (sz - i < PGSIZE)
			n = sz - i;
		else
			n = PGSIZE;
		if (readi(ip, P2V(pa), offset + i, n) != n) return -1;
	}
	return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	char *mem;
	uint  a;

	if (newsz >= KERNBASE) return 0;
	if (newsz < oldsz) return oldsz;

	a = PGROUNDUP(oldsz);
	for (; a < newsz; a += PGSIZE) {
		mem = kalloc();
		if (mem == 0) {
			cprintf("allocuvm out of memory\n");
			deallocuvm(pgdir, newsz, oldsz);
			return 0;
		}
		memset(mem, 0, PGSIZE);
		if (mappages(pgdir, (char *)a, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
			cprintf("allocuvm out of memory (2)\n");
			deallocuvm(pgdir, newsz, oldsz);
			kfree(mem);
			return 0;
		}
	}
	return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	pte_t *pte;
	uint   a, pa;

	if (newsz >= oldsz) return oldsz;

	a = PGROUNDUP(newsz);
	for (; a < oldsz; a += PGSIZE) {
		pte = walkpgdir(pgdir, (char *)a, 0);
		if (!pte)
			a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
		else if ((*pte & PTE_P) != 0) {
			pa = PTE_ADDR(*pte);
			if (pa == 0) panic("kfree");
			char *v = P2V(pa);
			kfree(v);
			*pte = 0;
		}
	}
	return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
	struct proc *curproc = myproc();
	uint i, pa, is_shmem;

	if (pgdir == 0) panic("freevm: no pgdir");
	deallocuvm(pgdir, KERNBASE, 0);
	for (i = 0; i < NPDENTRIES; i++) {
		if (pgdir[i] & PTE_P) {
			is_shmem = 0;
			pa = PTE_ADDR(pgdir[i]);
			for (int j = 0; j < SHM_MAXNUM; j++) {
				if (curproc->shared_mem[j].in_use == 1) {
					if (pa == curproc->shared_mem[j].global_ptr->pa) {
					//	va = curproc->shared_mem[j].va;
						is_shmem = 1;
						break;
					}
				}
			}
			if (!is_shmem) {
				char *v = P2V(pa);
				kfree(v);
			}
		}
	}
	kfree((char *)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
	pte_t *pte;

	pte = walkpgdir(pgdir, uva, 0);
	if (pte == 0) panic("clearpteu");
	*pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
pde_t *
copyuvm(pde_t *pgdir, uint sz)
{
	pde_t *d;
	pte_t *pte;
	uint   pa, i, flags;
	char * mem, *va;
	struct proc *curproc = myproc();
	uint is_shmem = 0;

	if ((d = setupkvm()) == 0) return 0;
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, (void *)i, 0)) == 0) panic("copyuvm: pte should exist");
		if (!(*pte & PTE_P)) panic("copyuvm: page not present");
		pa    = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);

		for (int j = 0; j < SHM_MAXNUM; j++) {
			if (curproc->shared_mem[j].in_use == 1) {
				if (pa == curproc->shared_mem[j].global_ptr->pa) {
					va = curproc->shared_mem[j].va;
					is_shmem = 1;
					break;
				}
			}
		}

		if (!is_shmem) {
			if ((mem = kalloc()) == 0) goto bad;
			memmove(mem, (char *)P2V(pa), PGSIZE);
			if (mappages(d, (void *)i, PGSIZE, V2P(mem), flags) < 0) goto bad;
		} else if (strncmp(curproc->name,"cm",strlen(curproc->name)) != 0){
			mappages(d,(void*)va,PGSIZE,pa,flags);
			is_shmem = 0;
		}
	}
/*
	for (i = 0; i < SHM_MAXNUM; i++) {
		struct shmem *curshmem = &curproc->shared_mem[i];
		char* va = curshmem->va;
		pte = walkpgdir(pgdir,va,0);
		if (*pte & PTE_P) *pte = 0;
		pa = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);
		mappages(d,(void*)va,PGSIZE,V2P((char*)pa),flags);
	}
*/
	return d;

bad:
	freevm(d);
	return 0;
}

// PAGEBREAK!
// Map user virtual address to kernel address.
char *
uva2ka(pde_t *pgdir, char *uva)
{
	pte_t *pte;

	pte = walkpgdir(pgdir, uva, 0);
	if ((*pte & PTE_P) == 0) return 0;
	if ((*pte & PTE_U) == 0) return 0;
	return (char *)P2V(PTE_ADDR(*pte));
}

/* 
 * Copy len bytes from p to user address va in page table pgdir.
 * Most useful when pgdir is not the current page table.
 * uva2ka ensures this only works for PTE_U pages.
 */

int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
	char *buf, *pa0;
	uint  n, va0;

	buf = (char *)p;
	while (len > 0) {
		va0 = (uint)PGROUNDDOWN(va);
		pa0 = uva2ka(pgdir, (char *)va0);
		if (pa0 == 0) return -1;
		n = PGSIZE - (va - va0);
		if (n > len) n= len;
		memmove(pa0 + (va - va0), buf, n);
		len -= n;
		buf += n;
		va = va0 + PGSIZE;
	}
	return 0;
}

int
shm_get(char *name)
{
	struct shared_mem *ptr = (struct shared_mem*)0;
	struct shmem *new_shmem = (struct shmem*)0;
	char *cur_name;
	pde_t *pgdir;
	struct proc *p = myproc();
	uint a;
	char *mem;
	int ret_val;
	uint found = 0;
	uint pa;


	ret_val = p->sz; /* shmem starting addr will b last address in VAS before page is mapped */
	
	// Tracks the first available spot in shm_list for a new page of shared memory 
	struct{
		uint set;
		struct shared_mem *ptr;
	}free_ptr;
	free_ptr.set = 0;

	if (strncmp(name,(char*)0,strlen(name)) == 0) return -1; /* can't have null name */

	for (int i = 0;  i < SHM_MAXNUM; i++) {
		cur_name = p->shared_mem[i].name;
		if (!p->shared_mem[i].in_use && new_shmem == (struct shmem*)0) {
			new_shmem = &p->shared_mem[i];
		}
//		if (strncmp(cur_name,(char*)0,sizeof((char*)0)) == 0) new_shmem = &p->shared_mem[i];
		if (strncmp(cur_name,name,strlen(name)) == 0) return -1; /* proc already holds shmem */
	}

//	if (new_shmem == (struct shmem*)0) return -1; /* proc at SHM_MAXNUM shmem pages */

	/*
	 * Acquire shm_list lock, iterate through array to find our shared memory with that name. 
	 * If that memory is found, set found = 1 and break out of the loop. 
	 * During this loop, we look for any un-allocated shared memory pages, 
	 * and save it to be used if this is the first instance of this page name
	 */
	acquire(&shm_list.lock);
	free_ptr.ptr = shm_list.list;
	for (ptr = shm_list.list; ptr < &shm_list.list[SHM_MAXNUM]; ptr++) {
		if (!(free_ptr.set || ptr->in_use)) {
			free_ptr.ptr = ptr;
			free_ptr.set = 1;		
		}
		
		if (strncmp(ptr->name, name, strlen(name)) == 0 && ptr->container_id == p->container_id) {

			found = 1;
			break;
		}
	}
	release(&shm_list.lock);

	/* 
	 * If a shared memory page is found with the same name 
	 * map that page's physical address to our current process
	 */
	if(found) {
		a = PGROUNDUP(p->sz);
		pgdir = p->pgdir;

		
		if (mappages(pgdir,(char*)a,PGSIZE,ptr->pa,PTE_W | PTE_U) < 0) {
			cprintf("shmem out of memory\n");
			return -1;
		}

	}else{
		/* 
		 * This is creating a new page of shared memory, 
		 * as this was the first instance of this name
		 */
		if (!free_ptr.set) {
			return -1; /* already at SHM_MAXNUM pages */
		}

		ptr = free_ptr.ptr;
		ptr->in_use = 1;
		
		a = PGROUNDUP(p->sz);
		pgdir = p->pgdir;
		mem = kalloc();
		
		if (mem == 0) {
			cprintf("shmem out of memory\n");
			return -1;
		}

		memset(mem, 0, PGSIZE);


		if (mappages(pgdir, (char *)a, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
			panic("mem");
			cprintf("shmem out of memory (2)\n");
			kfree(mem);
			return -1;
		}

		pa = V2P(mem);

		acquire(&shm_list.lock);

		strncpy(ptr->name,name,strlen(name));
		ptr->pa = pa;
		release(&shm_list.lock);
	}
	p->sz = p->sz + PGSIZE;
	
	strncpy(new_shmem->name,name,strlen(name));
	
	new_shmem->va = (char*)ret_val;
	new_shmem->global_ptr = ptr;
	new_shmem->in_use = 1;
	acquire(&shm_list.lock);
	ptr->refcount++;
	
		
	ptr->container_id = p->container_id;
	release(&shm_list.lock);

	return ret_val;
}

int
shm_rem(char *name)
{
	struct shared_mem *ptr = (struct shared_mem*)0;
	struct shmem *proc_ptr = (struct shmem*)0;
	struct proc *cur_proc = myproc();
	char *cur_name;

	acquire(&shm_list.lock);
	for (ptr = shm_list.list; ptr < &shm_list.list[SHM_MAXNUM]; ptr++) {
		if (strncmp(ptr->name, name, strlen(name)) == 0 && ptr->container_id == cur_proc->container_id) {
			goto found_page;
		}
	}
	release(&shm_list.lock);

	return -1; /* no shared page w/ given name */
found_page:
	release(&shm_list.lock);

	/* releasing existing shared mem */
	for (int i = 0; i < SHM_MAXNUM; i++) {
		cur_name = cur_proc->shared_mem[i].name;
		if (strncmp(cur_name,name,strlen(name)) == 0) {
			proc_ptr = &cur_proc->shared_mem[i];
			break;
		}
	}
	
	if (proc_ptr == (struct shmem*)0) return -1; /* curproc does not hold shmem w/ this name */
	proc_ptr->in_use = 0;

	pde_t *pte = walkpgdir(cur_proc->pgdir,proc_ptr->va,0);

	if (ptr->refcount == 1) { // curproc is only proc holding shmem -- deallocate mem 
		uint pa = PTE_ADDR(*pte);
		char *v = P2V(pa);
		kfree(v);
		ptr->pa = -1;
		ptr->in_use = 0;
		ptr->refcount--;

		if ((*pte & PTE_P) != 0) *pte = 0; /* unmap page */
		strncpy(proc_ptr->name,"",strlen(proc_ptr->name));
		proc_ptr->va = (char*)-1;
		cur_proc->sz -= PGSIZE;


		strncpy(ptr->name,"",strlen(ptr->name));
		return 0;
	}

	if ((*pte & PTE_P) != 0) *pte = 0; /* unmap page */
	
	strncpy(proc_ptr->name,"",strlen(proc_ptr->name));
	proc_ptr->va = (char*)-1;
	ptr->refcount--;

	cur_proc->sz -= PGSIZE;
	return 0;
}


// PAGEBREAK!
// Blank page.
