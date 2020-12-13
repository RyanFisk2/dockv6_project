#include "types.h"
#include "user.h"

int
main(void)
{
        char *shm_addr;
        int child1, child2, muxid;

        printf(1,"TEST SHARED MEM ACCESS WHEN OTHER PROCS REMOVE IT\n");

        shm_addr = shm_get("test4");
        strcpy(shm_addr,"hello");
        printf(1,"parent called shm_get('test4'), set value @ first address to 'hello'\n");

        child1 = fork();
        if (child1 != 0) {
                printf(1,"parent forked first child\n");
                child2 = fork();
        }
        if (child1 != 0 && child2 != 0) {
                printf(1,"parent forked second child\n");
                muxid = mutex_create("test4");
                cv_signal(muxid);
                wait();
                wait();
                printf(1,"back to parent: value @ first address of shared page = %s\n",shm_addr);
                exit();
        } else if(child1 == 0) {
                muxid = mutex_create("test4");

                /* wait for parent to finish printing before executing */
                mutex_lock(muxid);
                cv_wait(muxid);
                mutex_unlock(muxid);

                printf(1,"child1 calling shm_rem('test4')\n");
                if (shm_rem("test4") == -1) {
                        printf(1,"Remove Err!\n");
                        cv_signal(muxid);
                        exit();
                }
                cv_signal(muxid);
                exit();
        } else{
                muxid = mutex_create("test4");
                /* wait for 1st child to call remove */
                mutex_lock(muxid);
                cv_wait(muxid);
                mutex_unlock(muxid);
                printf(1,"child2 accessing shared page: value @ first address = %s\n",shm_addr);
                printf(1,"child2 calling shm_rem('test4')\n");
                if (shm_rem("test4") == -1) {
                        printf(1,"Remove Err!\n");
                        exit();
                }
                exit();
        }
}