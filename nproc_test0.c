#include "types.h"
#include "user.h"
#include "cm.h"

/*
 * test that we cannot call nproc from outside a container
 */

int
main()
{
        printf(1, "TEST THAT WE CANNOT SET MAX PROC FROM OUTSIDE A CONTAINER\n");
        if(cm_maxproc(4) != -1)
        {
                printf(1, "nproc_test0 FAIL, SHOULD RETURN ERROR\n");
                exit();
        }

        printf(1, "nproc_test0 CORRECTLY RETURNED ERROR\n");

        exit();
}