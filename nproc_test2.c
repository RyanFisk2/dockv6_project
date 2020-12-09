#include "types.h"
#include "user.h"
#include "cm.h"


/*
 * test that we cannot create more than nproc children in a container
 * DOES NOT TEST EXIT/RE-ALLOCATING CHILDREN
 * MUST BE RUN IN A CONTAINER
 */

int
main()
{
        int pid;
        int done = 0;
        int num_forks = 1;
        
        pid = fork();

        if(pid > 0)
        {
                printf(1, "Created child %d\n", pid);

                num_forks++;

                int pid2 = fork();

                if(pid2 > 0)
                {
                        printf(1, "created child %d\n", pid2);

                        num_forks++;

                        int pid3 = fork();

                        if(pid3 > 0)
                        {
                                printf(1, "created child %d\n", pid3);

                                num_forks ++;

                                int pid4 = fork();

                                if(pid4 > 0)
                                {
                                        printf(1, "created child %d\n", pid4);

                                        int pid5 = fork();

                                        num_forks ++;

                                        if(pid5 > 0)
                                        {
                                                printf(1, "created child %d\n", pid5);

                                                int pid6 = fork();

                                                num_forks++;

                                                if(pid6 > 0)
                                                {
                                                        printf(1, "created child %d\n", pid6);
                                                        printf(1, "nproc_test2 fail, made 6 children, max is 4\n");
                                                        exit();
                                                }else if(pid6 != -1){
                                                        wait();
                                                        exit();
                                                }else{
                                                        printf(1, "ending nproc_test2\n");
                                                        exit();
                                                }

                                        }else if (pid5 != -1){
                                                wait();
                                                //exit();
                                        }else{
                                                printf(1, "ending nproc_test2, num_forks: %d\n", num_forks);
                                                done = 1;
                                                exit();
                                        }

                                }else if(pid4 != -1){
                                        wait();
                                        //exit();
                                }else{
                                        printf(1, "ending nproc_test2, num_forks: %d\n", num_forks);
                                        done = 1;
                                        exit();
                                }

                        }else if(pid3 != -1){
                                wait();
                                //exit();
                        }else{
                                printf(1, "ending nproc_test2, num_forks: %d\n", num_forks);
                                done = 1;
                                exit();
                        }
                }else if (pid2 != -1) {
                        wait();
                        //exit();
                }
        }else if (pid != -1){
                wait();
        }

        while (!(done));

        printf(1, "total forks: &d\n", num_forks);
        exit();
}