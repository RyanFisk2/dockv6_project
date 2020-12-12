#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr;
        shmem_addr = (char*)1;
        *shmem_addr = 1;
        printf(1,"TESTING PASSING BETWEEN CHILD PROC:\n");
        if ( (shmem_addr = shm_get("test1")) == (char*)0) {
                printf(1,"get ERR\n");
                exit();
        }
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

        while(1);
        exit();
}