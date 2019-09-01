// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;
	extern volatile pte_t uvpt[];     // VA of "virtual page table"
	extern volatile pde_t uvpd[];     // VA of current page directory

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if(!( (FEC_WR & err) && ( uvpd[PDX(addr)] & PTE_P ) &&  (uvpt[PGNUM(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_COW))){
		panic("not COW!");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	//alloc
	if(sys_page_alloc(sys_getenvid(), (void *)PFTEMP, PTE_P | PTE_W | PTE_U)){
		panic("user page fault allocation failed!");
	}
	else{
		addr = ROUNDDOWN(addr, PGSIZE);
		//copy
		memcpy((void *)PFTEMP, (void *)addr, PGSIZE);
		//map
		if(sys_page_unmap(sys_getenvid(), addr)){
			panic("unmap old page, failed!");
		}
		if(sys_page_map(sys_getenvid(), PFTEMP, sys_getenvid(), addr, PTE_W|PTE_U|PTE_P)){
			panic("copy mapping failed");
		}
		if(sys_page_unmap(sys_getenvid(), PFTEMP)){
			panic("unmap new temp page, failed!");
		}
	}

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	// panic("duppage not implemented");
	void *addr = (void*) (pn*PGSIZE);
	if ((uvpt[pn] & PTE_COW) || (!(uvpt[pn] & PTE_SHARE) && (uvpt[pn] & PTE_W))) {
		if (sys_page_map(thisenv->env_id, addr, envid, addr,  ((uvpt[pn] & PTE_SYSCALL) & (~PTE_W)) | PTE_COW)){
			panic("child copy-on-write failed");
		}
		if (sys_page_map(thisenv->env_id, addr, thisenv->env_id, addr,  ((uvpt[pn] & PTE_SYSCALL) & (~PTE_W)) | PTE_COW)){
			panic("father copy-on-write failed");
		}
	} 
	else {
		if( sys_page_map(thisenv->env_id, addr, envid, addr, (uvpt[pn] & PTE_SYSCALL))){
			panic("default mapping failed");
		}
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);

  	envid_t ch_envid;
  	uint32_t addr;
	ch_envid = sys_exofork();
	if (ch_envid == 0) {
		//in child process
		//fix thisenv
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	if (ch_envid < 0){
		panic("sys_exofork: %e", ch_envid);
	}

	
	for (addr = 0; addr < USTACKTOP; addr += PGSIZE)
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P)
			&& (uvpt[PGNUM(addr)] & PTE_U)) {
			duppage(ch_envid, PGNUM(addr));
		}

	if (sys_page_alloc(ch_envid, (void *)(UXSTACKTOP-PGSIZE), PTE_U|PTE_W|PTE_P) < 0)
		panic("exception stack allocation failed!");

	void _pgfault_upcall();
	sys_env_set_pgfault_upcall(ch_envid, _pgfault_upcall);

	if (sys_env_set_status(ch_envid, ENV_RUNNABLE)){
		panic("child sys_env_set_status failed");
	}
	return ch_envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
