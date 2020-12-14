#include "types.h"
#include "user.h"

int
main(void)
{
        int child, grandchild, muxid, muxid2;
        char *shm_addr;

        shm_addr = shm_get("priotest");

        child = fork();
        if (child != 0) {
                muxid = mutex_create("priotest");
                muxid2 = mutex_create("for_grandchild");
                printf(1,"parent trying to set child's priority\n");
                if (prio_set(child,1) == -1) {
                        printf(1,"prio_set returned error!\n");
                        wait();
                        exit();
                }
                printf(1,"Success!\n");

                // wait for child to put grandchild's pid in shared mem
                mutex_lock(muxid);
                cv_wait(muxid);
                mutex_unlock(muxid);

                grandchild = *shm_addr; /* get grandchild's pid from shared mem */                
                printf(1,"parent trying to set grandchild's priority\n");
                if (prio_set(grandchild,1) == -1) {
                        printf(1,"prio_set returned error!\n");
                        cv_signal(muxid2);
                        wait();
                        exit();
                }
                printf(1,"Success!\n");
                cv_signal(muxid2);
                wait();
                exit();
        } else{
                grandchild = fork();
                if (grandchild != 0) {
                        muxid = mutex_create("priotest");
                        *shm_addr = grandchild;
                        cv_signal(muxid); /* tell parent grandchild pid is available */
                        wait();
                        exit();
                } else{
                        muxid = mutex_create("for_grandchild");
                        mutex_lock(muxid);
                        cv_wait(muxid);
                        mutex_unlock(muxid);
                        exit();
                }
        }
}