#include "types.h"
#include "user.h"

int
main(void)
{
        int child, grandchild;
        printf(1,"TEST CANT LOWER PRIORITY & PRIORITY INHERITED ON FORK\n");
        child = fork();
        if (child == 0) {
                prio_set(getpid(),2);
                printf(1,"child proc called prio_set(getpid(),2)\n");
                grandchild = fork();
                if (grandchild == 0) {
                        printf(1,"grandchild (forked from child) calling prio_set(getpid(),1)\n");
                        if (prio_set(getpid(),1) != -1) {
                                printf(1,"[ERR]: prio_set did not return error\nEither priority wasn't inherited on fork or error not returned for raising priority\n");
                                exit();
                        }
                        printf(1,"prio_set correctly returned error\n");

                        exit();
                } else{

                        wait();
                        exit();
                }
        } else{       
                wait();
                printf(1,"TEST PRIO_SET ON NON-EXISTENT PROCESS\n");
                printf(1,"parent calling prio_set(childpid,3) after child exits\n");
                if (prio_set(child,3) != -1) {
                        printf(1,"[ERR]: prio_set did not return error\n");
                        exit();
                }
                printf(1," prio_set correctly returned error\n");
                exit();
        }
}