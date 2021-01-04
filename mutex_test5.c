#include "types.h"
#include "user.h"

/*
 * This program tests to make sure a lock is still usable, even if a process crashes with the lock held. 
*/

int main(void){
    int muxid, pid;

    pid = fork();
    if (pid == 0) {
        muxid = mutex_create("lockcrashtest");
        printf(1,"Child taking lock\n");
        mutex_lock(muxid);
        printf(1,"Child dereferencing null ptr to cause crash\n");
        char *ptr = (char*)0;
        printf(1,"%d\n",*ptr);
        exit();
    }else{
        wait();
        muxid = mutex_create("lockcrashtest");
        printf(1, "\nParent trying to take lock after child crash...\n");
        mutex_lock(muxid);
        printf(1,"parent got lock\n");
        mutex_delete(muxid);
        exit();
    }
}