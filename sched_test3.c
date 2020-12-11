#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2;
        pid1 = fork();
        if (pid1 != 0) pid2 = fork();
        if (pid1 != 0 && pid2 != 0) {
                if (prio_set(pid1,1) == -1) {
                        printf(1,"set fail\n");
                        wait();
                        wait();
                        exit();
                }
                if (prio_set(pid2,1) == -1) {
                        printf(1,"set fail\n");
                        wait();
                        wait();
                        exit();
                }
                 wait();
                 wait();
                 exit();
        } else if (pid1 == 0) {

                sleep(10);

                for (int i = 0; i < 10; i++){
                        printf(1,"%d\n",getpid());
                }

                exit();
        } else if (pid2 == 0) {
                sleep(10);

                for (int i = 0; i < 10; i++) {
                        printf(1,"%d\n",getpid());
                }

                exit();
        }
}