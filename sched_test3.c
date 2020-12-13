#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1, pid2, num_preemptions;
        char *shm_addr;

        printf(1,"TESTING 2 PROCS with same priority\n");
        sleep(200); /* give time to read above print */
        shm_addr = shm_get("sched_test");
        
        pid1 = fork();
        if (pid1 != 0) pid2 = fork();
        if (pid1 != 0 && pid2 != 0) {
                 wait();
                 wait();
                 exit();
        } else if (pid1 == 0) {
                *shm_addr = 1;
                num_preemptions = 0;
                for (int i = 0; i < 500; i++){
                        printf(1,"1\n");
                        if (*shm_addr != 1) num_preemptions++;
                        *shm_addr = 1;
                }
                printf(1,"child1 preempted by child2 %d times\n",num_preemptions);
                exit();
        } else if (pid2 == 0) {
                *shm_addr = 0;
                num_preemptions = 0;
                for (int i = 0; i < 500; i++) {
                        printf(1,"2\n");
                        if (*shm_addr != 0) num_preemptions++;
                        *shm_addr = 0;
                }
                printf(1,"child2 preempted by child1 %d times\n",num_preemptions);
                exit();
        }
}