#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr;

        shmem_addr = (char*)1;
        *shmem_addr = 1;

        int pid = fork();
        if (pid != 0 ) {
                shmem_addr = shm_get("hi");
                *shmem_addr = 2;
                wait();
                exit();
        } else{
                printf(1,"pre get: shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                shmem_addr = shm_get("hi");
                printf(1,"post get: shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                exit();
        }
}