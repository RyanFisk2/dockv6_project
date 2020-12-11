#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * cm_create_and_enter forks a new process to run 
 * the specified init process for a container.
 * The child returns here with child = 1, and exec's the init process.
 * Parent returns here after child exits with child = 0, and exits CM
 */
void
create_and_enter(char *init, char* fs, int nproc)
{
        char *argv[] = {"", 0};
        int child = cm_create_and_enter(init, fs, nproc);

        if(child) exec(init, argv);
        
        exit();
}

/*
 * Retrieve container specification from shared memory set by dockv6
 * 
 * TODO: this should run as infinite loop, sleeping until dockv6 sets
 *       shared memory, then wakeup to handle request and go back to sleep.
 *       Need further implementation of CV module for this.
 */
int
main ()
{
        printf(1, "Starting CM....\n");

        char init[16], fs[16];
        int nproc;


        char *shmem_addr = shm_get("dockv6");
        strcpy(init,shmem_addr);

        shmem_addr += strlen(init)+sizeof(char);
        strcpy(fs,shmem_addr);

        shmem_addr += strlen(fs)+sizeof(char);
        nproc = *shmem_addr;

        create_and_enter(init, fs, nproc);

        while(1);
        exit();
}