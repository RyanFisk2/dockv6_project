#include "types.h"
#include "user.h"
#include "cm.h"
#include "jsmn/jsmn.h"


/*
 * check for correct usage of the dockv6 command
 * should be "dockv6 create config.json"
 * then pass specs to conatiner manager to create
 */
int
main(int argc, char* argv[])
{

        if(argc != 3){
                printf(1, "Usage: dockv6 create <config json file>\n");
                exit();
        }

        //jsmn_parser p;
        //jsmntok_t tokens[128];
        //char* cm_root;
        //int r, nproc;

        //jsmn_init(&p);

        /* TODO: check for correct JSON file */
        //char* filename = argv[2];
     

        cm_create_and_enter();
        cm_setroot("/root", 5);
        cm_maxproc(75);
        cm_maxproc(42);

        exit();   
}