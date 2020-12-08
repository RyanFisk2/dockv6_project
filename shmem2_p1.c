#include "types.h"
#include "user.h"

int
main(void)
{
        char *shmem_addr = shm_get("test2");
        char *var = "Success";
        strcpy(shmem_addr,var);
        while(1);
        exit();
}