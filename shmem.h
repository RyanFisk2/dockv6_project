#include "types.h"

struct shared_mem{
        char name[16];
        uint pa;
        uint refcount;
        uint in_use;
        int container_id;
};