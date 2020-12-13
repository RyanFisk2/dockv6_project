#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * set-up program to initialize a directory for the container to operate in
 */
int
main()
{
        //struct proc *p = myproc();
        //struct inode *dir_node;
        //struct inode *ls_node;
        int dir;
        
        if((dir = mkdir("container")) < 0) exit();

        //printf(1, "created directory %d\n", dir);

        //new syscall to copy files to directory
        
        copy_file("container", "ls");
        copy_file("container", "sh");
        copy_file("container", "cat");
        copy_file("container", "echo");
        copy_file("container", "nproc_test2");
        copy_file("container", "nproc_test3");
        copy_file("container", "shmem_testp1");
        copy_file("container", "shmem_testp2");
        copy_file("container", "shmem2_p1");
        copy_file("container", "shmem2_p2");




        
        exit();
}