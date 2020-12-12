#include "types.h"
#include "user.h"

int
main(void)
{
        int i;
        char *shm_addr;
        char *ptr;

        shm_addr = shm_get("shmem4");
        *shm_addr = 12;
        if ( (ptr = malloc(2048)) == (char*)0) {
                printf(1,"couldn't alloc 2048 bytes\n");
        }
        for( i = 0; i < 10; i++) {
                if ( (ptr = malloc(2048)) == (char*)0) goto end;
        }
        ptr++; /* unused variable warnings annoy me */
end:
        printf(1,"%d\n",*shm_addr);
        exit();
}