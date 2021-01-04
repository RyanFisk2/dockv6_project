#include "types.h"
#include "user.h"

/*
 * This program is meant to be called in the container. This is to test that mutexes are separated by container. 
 * On success, program mux_container2.c when called in the container should print out "Mutexes are separated."
 * assuming that the mutexes are separated by container. If they were not, mux_container2.c would not be able to take the lock. 
 * On fail, mux_container1.c should print an error message. 
 * This also tests the condition variables, as we call cv_wait in mux_container1 and cv_signal in mux_container2. If they
 * are not separated by container, mux_container2 will wakeup mux_container1.
*/

int main(void){
    int muxid = mutex_create("MuxContainerTest");
    mutex_lock(muxid);
    mutex_unlock(muxid);
    cv_signal(muxid);
    printf(1, "Mutexes are separated.\n");
    exit();
}