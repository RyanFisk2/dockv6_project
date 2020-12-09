#include "types.h"
#include "user.h"

int
main(void)
{
        int pid;
        char *shmem_addr = shm_get("test2");
        printf(1,"pre-fork, in parent: shmem_addr=%s\n",shmem_addr);
        pid = fork();
        if (pid != 0) {
                printf(1,"parent setting shmem_addr to abcdefg\n");
                strcpy(shmem_addr,"abcdefg");
                wait();
                exit();
        } else{
                sleep(10);
                printf(1,"in child: shmem_addr=%s\n",shmem_addr);
                exit();
        }
}