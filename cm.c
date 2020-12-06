#include "types.h"
#include "user.h"
#include "cm.h"


/*
 * will get the container specs from dockv6 and create the new container
 */
int
main ()
{
        printf(1, "Starting CM....\n");

        char init[16], fs[16];
        int nproc;


        char *shmem_addr = shm_get("dockv6");
        strcpy(init,shmem_addr);
        printf(1,"init: %s\n",init);
        shmem_addr += strlen(init)+sizeof(char);
        strcpy(fs,shmem_addr);
        printf(1,"fs: %s\n",fs);
        shmem_addr += strlen(fs)+sizeof(char);
        nproc = *shmem_addr;
        printf(1,"nproc: %d\n",nproc);
        exit();
}