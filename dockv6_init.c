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
        copy_file("container", "cat");
        copy_file("container", "echo");




        
        exit();
}