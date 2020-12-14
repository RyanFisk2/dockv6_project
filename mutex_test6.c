#include "types.h"
#include "user.h"

/*
 * This program tests to make sure a lock's condition variable is still usable, even if a process crashes while sleeping in cv_wait(). 
*/

int main(void){
    int muxid, pid, pid2 = -1;

    pid = fork();
    if (pid != 0) pid2 = fork();
    if (pid != 0 && pid2 != 0) { /* parent */
            muxid = mutex_create("lockcrashtest");
            sleep(40);
            kill(pid);
            printf(1,"killed child1\n");
            cv_signal(muxid);
            wait();
            wait();
            exit();
    } else if (pid == 0) { /* child 1 */
        printf(1,"child1 calling cv_wait, will be killed by parent\n");
        muxid = mutex_create("lockcrashtest");
        mutex_lock(muxid);
        cv_wait(muxid);

        exit();
    }else if (pid2 == 0){ /* child 2 */
        muxid = mutex_create("lockcrashtest");
        sleep(20);
        printf(1, "Child2 calling cv_wait\n");
        mutex_lock(muxid);
        cv_wait(muxid);
        mutex_unlock(muxid);
        printf(1, "Child 2 got signaled\n");
        exit();
    }
}