#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr = (char*)1;
        *shmem_addr = 1;
        char *init, *fs;
        int nproc;
        init = fs = "";
        printf(1,"      TESTING PROC IN SEPERATE ANCESTRY GETTING PAGE:\n");
        printf(1,"      pre get: shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
        shmem_addr = shm_get("test1");
        printf(1,"      got shared mem page 'test1'\n");
        strcpy(init,shmem_addr);
        printf(1,"      got:%s from shared page\n",init);
        shmem_addr += strlen(init)+sizeof(char);
        strcpy(fs,shmem_addr);
        printf(1,"      got: %s from shared page\n",fs);
        shmem_addr += strlen(fs) + sizeof(char);
        nproc = *shmem_addr;
        printf(1,"      got %d from shared page\n",nproc);
        exit();
}