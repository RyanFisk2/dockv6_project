#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2;

        printf(1,"TESTING PRIO_SET CALLED BY PROC NOT IN ANCESTRY\n");
        pid1 = fork();
        if (pid1 != 0) pid2= fork();
        if (pid1 != 0 && pid2 != 0) {
                wait();
                wait();
                exit();
        } else if (pid1 == 0) {
                sleep(100);
                exit();
        } else{
                printf(1,"child2 trying to set child1's priority\n");
                if (prio_set(pid1,1) != -1) {
                        printf(1,"[Err]: prio_set did not return error when child2 tried to set child1 priority\n");
                } else{
                        printf(1,"prio_set correctly returned error\n");
                }
                exit();
        }
}