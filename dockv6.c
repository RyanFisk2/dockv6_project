#include "types.h"
#include "user.h"
#include "cm.h"
#include "jsmn/jsmn.h"


static int jsoneq(const char *json, jsmntok_t *tok, char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}


/*
 * check for correct usage of the dockv6 command
 * should be "dockv6 create config.json"
 * then pass specs to conatiner manager to create
 */
int
main(int argc, char* argv[])
{
        int jfd, n, r, nproc = 0;
        char buf[512];
        char init[32], fs[32], nproc_array[32];
        jsmn_parser p;
        jsmntok_t t[128];
        

        if(argc != 3){
                printf(1, "Usage: dockv6 create <config json file>\n");
                exit();
        }

        //read container_config.json

        printf(1, "getting container specs from %s\n", argv[2]);

        if((jfd = open(argv[2], 0)) < 0)
        {
                printf(1, "ERROR opening container_config.json\n");
                exit();
        }else{
                while((n = read(jfd, buf, sizeof(buf))) > 0)
                {
                        /*if(write(1, buf, n) != n)
                        {
                                printf(1, "Error writing to cm_pipe\n");
                                exit();
                        }*/
                }

                if(n < 0) 
                {
                        printf(1, "Error reading container_config.json\n");
                        exit();
                }

                //printf(1, "json string = %s\n", buf);
        }

        //set up jsmn parser
        /* TODO: MOVE THIS TO CM.C, SEND BUF TO CM BY SHARED MEM */

        jsmn_init(&p);

        r = jsmn_parse(&p, buf, strlen(buf), t, 128);

        for (int i = 1; i < r; i++)
        {
                //print out json tokens
                if(jsoneq(buf, &t[i], "init") == 0){
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                init[j] = *(buf + t[i + 1].start + j);
                        }
                        printf(1, "init: %s\n", init);
                        i++;
                }else if(jsoneq(buf, &t[i], "fs") == 0){
                        //printf(1, "fs: %s\n", buf + t[i + 1].start);
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                fs[j] = *(buf + t[i + 1].start + j);
                        }

                        printf(1, "fs: %s\n", fs);
                        i++;         
                }else if(jsoneq(buf, &t[i], "nproc") == 0){
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                nproc_array[j] = *(buf + t[i + 1].start + j);
                        }
                        nproc = atoi(nproc_array);
                        printf(1, "nproc: %d\n", nproc);
                        i++;
                }

        }


        cm_create_and_enter();
        cm_setroot("container", 5);
        cm_maxproc(75);
        cm_maxproc(42);

        exit();   
}