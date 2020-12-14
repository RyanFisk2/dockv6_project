#include "types.h"
#include "user.h"
#include "cm.h"
#include "jsmn.h"

/*
 * jsoneq function copied from jsmn.c
 * had visibility issues with it being at jsmn/jsmn.c, so moved it here 
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
 * dockv6 reads a json config file for container specifications
 * including the initial process to run in the container, the
 * root fs for the container, and the maximum number of processes
 * for that container
 * 
 * dockv6 passes those specs to the container manager via shared memory
 * 
 * TODO: integrate with condition variables
 *       to signal CM to wakeup when dockv6 writes
 *       specs into shared memory 
 */
int
main(int argc, char* argv[])
{
        int jfd, n, r, muxid, nproc = 0;
        char buf[512];
        char init[32], fs[32], nproc_array[32];
        jsmn_parser p;
        jsmntok_t t[128];

        if(argc != 3){
                printf(1, "Usage: dockv6 create <config json file>\n");
                exit();
        }


        if((jfd = open(argv[2], 0)) < 0)
        {
                printf(1, "ERROR opening json\n");
                exit();
        }else{
                while((n = read(jfd, buf, sizeof(buf))) > 0);
 
                if(n < 0) 
                {
                        printf(1, "Error reading config file\n");
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


        //pass init, fs, and nproc to cm with shared mem
        char *shmem = shm_get("dockv6");

        strcpy(shmem, init);
        shmem += ((strlen(init) * sizeof(char)) + sizeof(char));

        strcpy(shmem,fs);
        shmem += ((strlen(fs) * sizeof(char)) + sizeof(char));

        *shmem = nproc;

        
        muxid = mutex_create("cmcomm");
        cv_signal(muxid);


        exit();   

}