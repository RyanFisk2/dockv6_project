#include "types.h"
#include "user.h"

int
main(void)
{
        int pid, muxid, last_int, i;
        char *shm_addr;

        shm_addr = shm_get("muxtest");
        pid = fork();
        if (pid == 0) {
                muxid = mutex_create("muxtest");
                mutex_lock(muxid);
                *shm_addr = 0;
                last_int = *shm_addr;
                for (i = 0; i < 1000; i++) {
                        if (*shm_addr != last_int) {
                                printf(1,"RACE!\n");
                                exit();
                        }
                        *shm_addr = i;
                        last_int = *shm_addr;
                }
                mutex_unlock(muxid);
                mutex_delete(muxid);
                exit();
        } else{
                muxid = mutex_create("muxtest");
                mutex_lock(muxid);
                strcpy(shm_addr,"hello");
                for (i = 0; i < 1000; i++) {
                        if (strcmp(shm_addr,"hello") != 0) {
                                printf(1,"RACE!\n");
                                exit();
                        }
                        strcpy(shm_addr,"hello");
                }
                mutex_unlock(muxid);
                mutex_delete(muxid);
                wait();
                exit();
        }
}