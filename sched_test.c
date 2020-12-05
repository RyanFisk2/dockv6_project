#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2;
        pid1 = fork();
        if (pid1 != 0) {
                prio_set(pid1,2);
//                prio_set(3,3);
                pid2 = fork();
                if (pid2 != 0) {
                        prio_set(pid2,1);
                        sleep(1);
                        printf(1,"ch2 prio set\n");
                        wait();
                        wait();
                        printf(1,"back to parent\n");
                        exit();
                } else{
                        printf(1,"prio=2\n");
                        exit();
                }
        } else{
                printf(1,"prio=1\n");
                exit();
        }
}