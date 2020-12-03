#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr;//, *shmem_addr2 = (char*)2000;

        shmem_addr = shm_get("hi");
        *shmem_addr = 1;
        exit();
/*
        int pid = fork();
        if (pid != 0 ) {
                shmem_addr = shm_get("hi");
                *shmem_addr = 2;
                printf(1,"in 1st proc, shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                wait();
                exit();
        } else{
                shmem_addr2 = shm_get("aa");
                *shmem_addr2 = 1;
                shmem_addr = shm_get("hi");
                printf(1,"in child, shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                exit();
        }
        */
}