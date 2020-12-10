#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr;

        shmem_addr = (char*)1;
        *shmem_addr = 1;
        printf(1,"TESTING PASSING BETWEEN CHILD PROC:\n");
        int pid = fork();
        if (pid != 0 ) {
                shmem_addr = shm_get("test1");
                printf(1,"      parent forked\nparent called shm_get('test1')\n");
                char *init = "sh";
                char *fs = "/temp";
                int nproc = 6;
                strcpy(shmem_addr,init);
                printf(1,"      parent put 'sh' at start of shared page\n");
                shmem_addr += strlen(init) + sizeof(char);
                strcpy(shmem_addr,fs);
                printf(1,"      parent put '/temp' after 'sh' in shared page\n");
                shmem_addr += strlen(fs) + sizeof(char);
                *shmem_addr = nproc;
                printf(1,"      parent put '6' after '/temp' in shared page\n");
                wait();
                exit();
        } else{
                sleep(10);
                char *init, *fs;
                int nproc;
                init = fs = "";

                printf(1,"      child proc, pre get: shmem_addr=%d *shmem_addr=%d\n",shmem_addr,*shmem_addr);
                shmem_addr = shm_get("test1");
                printf(1,"      child called shm_get('test1')\n");
                strcpy(init,shmem_addr);
                printf(1,"      child got:%s from shared page\n",init);
                shmem_addr += strlen(init)+sizeof(char);
                strcpy(fs,shmem_addr);
                printf(1,"      child got: %s from shared page\n",fs);
                shmem_addr += strlen(fs) + sizeof(char);
                nproc = *shmem_addr;
                printf(1,"      child got %d from shared page\n",nproc);
                if (nproc != 6) printf(1,"[Err]\n");
                while(1);
                exit();
        }
}