#include "types.h"
#include "user.h"

/*
 * This program is meant to be called in the shell, prior to creation of a container. This is to test that mutexes are separated by container. 
 * On success, program mux_container2.c when called in the container should print out "Mutexes are separated."
*/

int main(void){
    int muxid = mutex_create("MuxContainerTest");
    mutex_lock(muxid);
    cv_wait(muxid);
    printf(1,"mux_container1.c was woken up when it was not supposed to.\n");
    exit();
}