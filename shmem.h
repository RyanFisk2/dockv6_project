#include "types.h"

struct shared_mem{
        char *name;
        uint pa;
        uint refcount;
};