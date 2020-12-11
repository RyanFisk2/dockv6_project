#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * test that zombie procs are counted against the max num procs
 * this loop should generate a whole lot of zombies
 */
int
main()
{
        int pid;

        while((pid = fork()) != -1)
        {
                printf(1, "brrraaaaaaaaiiiiiiiinnnnssssss\n");
        }

        if(pid == -1)
        {
                printf(1, "no more zombies... huzzah!");
        }

        exit();
}