#include "types.h"
#include "user.h"

/*
 * The purpose of this code is to make sure that the maximum number a mutexes is enforced. Run this in shell to make sure this affects the global mutex struct.
 * Run mutex_test4b in a container to make sure that the number of mutexes isn't only limited by the proc struct, but also by the global struct
*/

int main(void){
    char arr[20] = "abcdefghijklmnopqrst";
    for(int i = 0; i < 10; i++){
        mutex_create(&arr[i]);
    }

    exit();
}