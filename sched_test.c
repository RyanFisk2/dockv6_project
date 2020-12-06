#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2;
        pid1 = fork();
        if (pid1 != 0) pid2 = fork();
        if (pid1 != 0 && pid2 != 0) {
 //               printf(1,"parent\n");
                prio_set(pid1,2);
                prio_set(pid2,1);
                 wait();
 //                printf(1,"back from first wait\n");
                 wait();
//                 printf(1,"back from 2nd wait\n");
                 exit();
        }
        if (pid1 == 0) {
                sleep(4);
                for (int i = 0; i < 10; i++){
                        printf(1,"prio=2\n");
                }
                exit();
        }
        if (pid2 == 0) {
                sleep(4);
                for (int i = 0; i < 10; i++) {
                        printf(1,"prio = 1\n");
                }
                if (prio_set(pid1,1) != -1) printf(1,"[Err]: set prio - not in ancestry\n");
                exit();
        }
}