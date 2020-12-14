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
        printf(1, "TEST THAT WE CANNOT SET NPROC IF ALREADY SET\n");
        if(cm_maxproc(8) != -1)
        {
                printf(1, "ERR: set nproc from inside the container\n");
                exit();
        }

        printf(1, "nproc_test1 CORRECTLY RETURNED ERROR\n");

        exit();

}