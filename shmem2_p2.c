#include "types.h"
#include "user.h"

int
main(void)
{
        int pid;
        char *shmem_addr = shm_get("test2");

        printf(1,"      2nd proc called shm_get('test2')\n");
        printf(1,"      shmem_addr=%s\n",shmem_addr);
        printf(1,"TESTING CHILD PROC ACCESS TO SHARED PAGE:\n");
        printf(1,"      forking child proc\n");

        pid = fork();
        if (pid != 0) {
                printf(1,"      parent setting shmem_addr to from_parent\n");
                strcpy(shmem_addr,"from_parent");
                wait();
                printf(1,"      back to parent - shmem_addr=%s\n",shmem_addr);
                exit();
        } else{
                sleep(10);
                printf(1,"              in child: shmem_addr=%s\n",shmem_addr);
                printf(1,"              child setting shmem_addr=from_child\n");
                strcpy(shmem_addr,"from_child");
                exit();
        }
}