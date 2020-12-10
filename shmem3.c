#include "types.h"
#include "user.h"

int
main(void)
{
        char *shm_addr;
        printf(1,"TESTING PROC CANT ACCESS PAGE AFTER REMOVE:\n");
        shm_addr = shm_get("test3");
        printf(1,"      got shared mem page 'test3'\n");
        *shm_addr = 12;
        printf(1,"      set value @ first addr of page to %d\n",*shm_addr);
        if (shm_rem("test3") == -1) {
                printf(1,"ERROR REMOVING\n");
                exit();
        }
        printf(1,"      called shm_rem('test3')\n");
        printf(1,"      trying to get value from shared page\n");
        printf(1,"      value @ first addr of page:%d\n",*shm_addr);
        exit();
}