#include "types.h"
#include "user.h"

#define NCHILDREN  8		/* should be > 1 to have contention */
#define CRITSECTSZ 3
#define DATASZ (1024 * 32 / NCHILDREN)


/*
This code tests for race conditions between a shared mutex across 8 forks.
Should print "RACE!!!" if a race condition is encountered. 
*/

void
child(int lockid, int pipefd, char tosend)
{
	mutex_create("test1");
    int i, j;
	for (i = 0 ; i < DATASZ ; i += CRITSECTSZ) {
		
		/*
		 * If the critical section works, then each child
		 * writes CRITSECTSZ times to the pipe before another
		 * child does.  Thus we can detect race conditions on
		 * the "shared resource" that is the pipe.
		 */
		//printf(1, "t, %d\n", i);
		mutex_lock(lockid);
		for (j = 0 ; j < CRITSECTSZ ; j++) {
			write(pipefd, &tosend, 1);
		}
		mutex_unlock(lockid);
	}
    mutex_delete(lockid);
	exit();
}

int
main(void)
{
    int i;
	int pipes[2];
	char data[NCHILDREN], first = 'a';
	int lockid;
	printf(1, "Testing for race conditions on mutexes, should see 'RACE' on error.\n");
	for (i = 0 ; i < NCHILDREN ; i++) {
		data[i] = (char)(first + i);
	}

	if (pipe(&pipes[0])) {
		printf(1, "Pipe error\n");
		exit();
	}

	if ((lockid = mutex_create("test1")) < 0) {
		printf(1, "Lock creation error\n");
		exit();
	}
	
	for (i = 0 ; i < NCHILDREN; i++) {
		if (fork() == 0) child(lockid, pipes[1], data[i]);
	}
	close(pipes[1]);
	while (1) {
		char fst, c;
		int cnt;

		fst = '_';
		for (cnt = 0 ; cnt < CRITSECTSZ ; cnt++) {
			if (read(pipes[0], &c, 1) == 0) goto done;
			if (fst == '_') {
				fst = c;
			} else if (fst != c) {
				printf(1, "RACE!!!\n");
			}
		}
	}
done:
	for (i = 0 ; i < NCHILDREN ; i++) {
		if (wait() < 0) {
			printf(1, "Wait error\n");
			exit();
		}
	}

	mutex_delete(lockid);

    exit();
}
