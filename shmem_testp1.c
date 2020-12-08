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
                char *init = "sh";
                char *fs = "/temp";
                int nproc = 6;
                strcpy(shmem_addr,init);
                shmem_addr += strlen(init) + sizeof(char);
                strcpy(shmem_addr,fs);
                shmem_addr += strlen(fs) + sizeof(char);
                *shmem_addr = nproc;
                wait();
                exit();
        } else{
                char *init, *fs;
                int nproc;
                init = fs = "";

                printf(1,"pre get: shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                shmem_addr = shm_get("hi");
                strcpy(init,shmem_addr);
                printf(1,"init: %s\n",init);
                shmem_addr += strlen(init)+sizeof(char);
                strcpy(fs,shmem_addr);
                printf(1,"fs: %s\n",fs);
                shmem_addr += strlen(fs) + sizeof(char);
                nproc = *shmem_addr;
                if (nproc != 6) printf(1,"[Err]\n");
                while(1);
                exit();
        }
}