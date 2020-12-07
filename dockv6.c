#include "types.h"
#include "user.h"
#include "cm.h"
#include "jsmn/jsmn.h"

/*
 * jsoneq function copied from jsmn.c
 * needed this to be visible for json parsing
 */
static int 
jsoneq(const char *json, jsmntok_t *tok, char *s) {
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
        char *init2, *fs2;
        init2 = fs2 = "";

        if(argc != 3){
                printf(1, "Usage: dockv6 create <config json file>\n");
                exit();
        }


        if((jfd = open(argv[2], 0)) < 0)
        {
                printf(1, "ERROR opening container_config.json\n");
                exit();
        }else{
                while((n = read(jfd, buf, sizeof(buf))) > 0);
 
                if(n < 0) 
                {
                        printf(1, "Error reading container_config.json\n");
                        exit();
                }

        }
        //parse JSON for init, fs, and nproc

        jsmn_init(&p);

        r = jsmn_parse(&p, buf, strlen(buf), t, 128);

        for (int i = 1; i < r; i++)
        {
                if(jsoneq(buf, &t[i], "init") == 0){
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                init[j] = *(buf + t[i + 1].start + j);
                        }
                        i++;
                }else if(jsoneq(buf, &t[i], "fs") == 0){
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                fs[j] = *(buf + t[i + 1].start + j);
                        }
                        i++;         
                }else if(jsoneq(buf, &t[i], "nproc") == 0){
                        for(int j = 0; j < t[i + 1].end - t[i+1].start; j++)
                        {
                                nproc_array[j] = *(buf + t[i + 1].start + j);
                        }
                        nproc = atoi(nproc_array);
                        i++;
                }

        }

//        strcpy(init2,init);
//        strcpy(fs2,fs);

        printf(1,"dockv6 init: %s\n fs: %s\n",init2,fs2);
        printf(1,"\n\ndockv6 init: %s\n fs: %s\n",init,fs);
        //pass init, fs, and nproc to cm with shared mem
        char *shmem = shm_get("dockv6");

//        init2 = "/hello_world";
        strcpy(shmem, init);
 //       printf(1,"%p\n",shmem);
        shmem += (strlen(init)*sizeof(char) + sizeof(char));
        printf(1,"%p\n",shmem);
        strcpy(shmem,fs);
        shmem += (strlen(fs)*sizeof(char) + sizeof(char));
 //       fs2 = "/c1";

        *shmem = nproc;
        

        while(1);

        exit();   

}