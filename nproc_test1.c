#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * test that we cannot call nproc when nproc is already set
 * nproc is set in cm_create_and_enter()
 */

int
main()
{
        if(cm_maxproc(8) != -1)
        {
                printf(1, "ERR: set nproc from inside the container\n");
                exit();
        }

        exit();

}