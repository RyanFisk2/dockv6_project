#include "types.h"
#include "user.h"

int
main(void)
{
        int pid1,pid2, num_preemptions, i;
        char *shm_addr;

        shm_addr = shm_get("test4");
        pid1 = fork();
        if (pid1 != 0) pid2 = fork();
        if (pid1 != 0 && pid2 != 0) {
                if (prio_set(pid1,1) == -1) {
                        printf(1,"set fail\n");
                        wait();
                        wait();
                        exit();
                }
                if (prio_set(pid2,2) == -1) {
                        printf(1,"set fail\n");
                        wait();
                        wait();
                        exit();
                }
                wait();
                wait();
                exit();
        } else if(pid1 == 0){
                sleep(10);
                num_preemptions = 0;
                *shm_addr = 1;
                for (i = 0; i < 100; i++) {
                        printf(1,"prio=1\n");
                        if (*shm_addr != 1) num_preemptions++;
                        *shm_addr = 1;
                }
                printf(1,"prio=1 proc preempted by prio=2 proc %d times\n",num_preemptions);
                exit();
        } else{
                sleep(10);
                
                for (i = 0; i < 100; i++) {
                        (*shm_addr)++;
                }
                exit();
        }
}