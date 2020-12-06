#ifndef MAX_NUM_CONTAINERS

#define MAX_NUM_CONTAINERS 4

#endif

struct container
{
        uint container_id;
        uint nproc;
        char *init;
        struct inode *root;

};

int cm_create_and_enter(char *init, char *fs, int nproc);
int cm_setroot(char* path, int path_len, struct container *c);
int cm_maxproc(int nproc);