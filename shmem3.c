#include "types.h"
#include "user.h"

int
main(void)
{
        char *shm_addr;
        shm_addr = shm_get("test3");
        *shm_addr = 12;
        printf(1,"pre-rem: *shm_addr=%d\n",*shm_addr);
        shm_rem("test3");
        printf(1,"post-rem: *shm_addr=%d\n",*shm_addr);
        exit();
}