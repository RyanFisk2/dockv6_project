
#include "types.h"
#include "user.h"

// cvsignal before cv wait, crash handling (wait and signal can be used normally after a trap), container segregation, calling methods on a mutex you dont own, double calls of lock/unlock from the same process
void cvtest(void){
    int lockid = mutex_create("test2");
    
    mutex_lock(lockid);
    printf(1, "Calling cv wait in child\n");
    cv_wait(lockid);
    printf(1, "cv wait was signaled in child\n");
    mutex_unlock(lockid);
    
    mutex_delete(lockid);
    exit();
}
int main(void){
    int lockid, i;

    printf(1, "Testing cv_signal call after cv_wait call\n");
    lockid = mutex_create("test2");
    if(fork() == 0){
        cvtest();
    }else{
        printf(1, "Locking and unlocking locks 1000 times\n");
        for(i = 0; i < 1000; i++){
            mutex_lock(lockid);
            mutex_unlock(lockid);
        }
        printf(1, "Locks done, now calling cv signal in parent\n");
        cv_signal(lockid);
        wait();
    }

    printf(1, "\nTesting cv_wait call after cv_signal call\n");
    printf(1,"Calling cv_signal in parent\n");

    cv_signal(lockid);
    if(fork() == 0){
        cvtest();
    }else{
        wait();
    }

    exit();
}
