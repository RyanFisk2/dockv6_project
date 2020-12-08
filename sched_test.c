#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2;
        pid1 = fork();
        if (pid1 != 0) pid2 = fork();
        if (pid1 != 0 && pid2 != 0) {

                 wait();
                 wait();
                 exit();
        } else if (pid1 == 0) {
                sleep(10);
                prio_set(getpid(),2);
                if (prio_set(getpid(),1) != -1) printf(1,"[Err]: set prio higher than current\n");
                if (prio_set(getpid() -1, 3) != -1) printf(1,"[Err]: set prio when not in ancestry\n");
 
                for (int i = 0; i < 10; i++){
                        printf(1,"2\n");
                }

                exit();
        } else if (pid2 == 0) {
                prio_set(getpid(),1);

                for (int i = 0; i < 10; i++) {
                        printf(1,"1\n");
                }

                exit();
        }
}