#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * test that we cannot call nproc from outside a container
 */

int
main()
{
        if(cm_maxproc(4) != -1)
        {
                printf(1, "nproc_test0 fail\n");
                exit();
        }

        exit();
}