#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_start;

        shmem_start = shm_get("hi");
        printf(1,"%d\n",*shmem_start);
        exit();
}