#include "types.h"
#include "user.h"

int
main(void)
{
        int i, numpage = 0;
        char *shm_addr;
        char *ptr;
        printf(1,"TESTING SBRK DOESNT AFFECT SHARED MEM PAGE\n");
        shm_addr = shm_get("shmem4");
        *shm_addr = 12;
        printf(1,"got shared page - set value @ first address to 12\nallocating page with malloc\n");
        if ( (ptr = malloc(1024)) == (char*)0) {
                printf(1,"couldn't alloc 2048 bytes\n");
        }
        printf(1,"page malloced: *shm_addr=%d\n",*shm_addr);
        for( i = 0; i < 10; i++) {
                if ( (ptr = malloc(1024)) == (char*)0) goto end;
                numpage++;
        }
        ptr++; /* unused variable warnings annoy me */
end:
        printf(1,"successfully allocated %d pages\n value @ first addr of shared page:%d\n",numpage,*shm_addr);
        exit();
}