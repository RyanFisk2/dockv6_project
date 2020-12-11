#include "types.h"
#include "user.h"

int
main(void)
{
        int child, grandchild;
        printf(1,"TEST CANT LOWER PRIORITY\n");
        child = fork();
        if (child == 0) {
                prio_set(getpid(),2);
                grandchild = fork();
                if (grandchild == 0) {
                        if (prio_set(getpid(),1) == -1) {
                                printf(1,"set fail- this is good\n");
                                exit();
                        }

                        exit();
                } else{

                        wait();
                        exit();
                }
        } else{       
                wait();
                exit();
        }
}