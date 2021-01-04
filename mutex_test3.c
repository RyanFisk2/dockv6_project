#include "types.h"
#include "user.h"

/*
 * The purpose of this program is to test if a process can...
 * 1. Not perform any actions on a mutex it does not own.
 * 2. A mutex cannot be created twice. 
 * 3. A mutex cannot be double locked.
 * 4. A mutex cannot be double unlocked.
 * 5. A mutex cannot be deleted twice. 
*/

int main(void){
    int muxid = 0;
    int muxid2 = 0;
    printf(1, "Testing mutex method calls on an invalid mutex...\n");
    printf(1, "Testing Lock\n");
    mutex_lock(muxid);
    printf(1, "Testing Unlock\n");
    mutex_unlock(muxid);
    printf(1, "Testing Delete\n");
    mutex_delete(muxid);
    printf(1, "Testing cv_signal\n");
    cv_signal(muxid);
    printf(1, "Testing cv_wait\n");
    cv_wait(muxid);

    printf(1, "Testing double mutex creation...\n");
    muxid = mutex_create("mutex_test3");
    muxid2 = mutex_create("mutex_test3");
    muxid2 += 0;
    printf(1, "Testing double locking of a mutex...\n");
    mutex_lock(muxid);
    mutex_lock(muxid);

    printf(1, "Testing double unlocking of a mutex...\n");
    mutex_unlock(muxid);
    mutex_unlock(muxid);

    printf(1, "Testing double deletion of a mutex...\n");
    mutex_delete(muxid);
    mutex_delete(muxid);

    exit();
}