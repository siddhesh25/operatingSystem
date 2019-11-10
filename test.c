#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <linux/sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <err.h>

#define SCHED_CASIO 9
#define millis 1000000

struct sched_attr {
	uint32_t size;              /* Size of this structure */
	uint32_t sched_policy;      /* Policy (SCHED_*) */
	uint64_t sched_flags;       /* Flags */
	int32_t sched_nice;         /* Nice value (SCHED_OTHER,
				  SCHED_BATCH) */
	uint32_t sched_priority;    /* Static priority (SCHED_FIFO,
				  SCHED_RR) */
	/* Remaining fields are for SCHED_DEADLINE */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;

	/* New casio fields */
	uint64_t casio_deadline;
	uint64_t casio_id;
};



static int sched_setattr (pid_t pid, const struct sched_attr *attr, unsigned int flags) {
    return syscall (SYS_sched_setattr, pid, attr, flags);
}

static int sched_getattr (pid_t pid, const struct sched_attr *attr, unsigned int size, unsigned int flags) {
    return syscall (SYS_sched_getattr, pid, attr, size, flags);
}



int child_id = -1; //set globally when child starts



// Child entry point, will get successive values of i
void run_child(int n) {
	child_id = n;
	printf("Child %d starting\n", child_id);


	struct sched_attr attr;

	int retval;
	retval = sched_getattr(0, &attr, sizeof(attr), 0);
	printf("c%d: retval: %d\n", child_id, retval);



	attr.sched_policy = SCHED_CASIO;
	attr.casio_id = child_id;
	// children have deadlines ranges from 2s down to 400ms
	// later children have sooner deadlines, so should schedule first
	attr.casio_deadline = 2000 * millis -  child_id * 100 * millis; //later children have sooner deadlines
	
	retval = sched_setattr(0, &attr, 0);

	printf("c%d: retval: %d\n", child_id, retval);
	usleep(500 * 1000);

	printf("c%d: starting 1st work \n", child_id);
	for (int i = 0; i < 2 * 1000 * 1000; i++) { }
	printf("c%d: finished 1st work, sleeping \n", child_id);
	usleep(500 * 1000);


	for (int i = 0; i < 2 * 1000 * 1000; i++) { }
	printf("c%d: finished 2nd work, sleeping \n", child_id);
	usleep(500 * 1000);


	printf("Child %d finished\n", child_id);
	exit(0);
}


int main() {
	printf("Casio test program, 16 tasks\n");
	printf("Restricting affinity to cpu 0\n");
	/* Set cpu affinity to only schedule on processor 0*/
	unsigned long mask = 1; 
	unsigned int len = sizeof(mask);
	if (sched_setaffinity(0, len, &mask) < 0) {
		perror("sched_setaffinity");
	}

	printf("Spawning Children\n");


	int num_children = 16;

	for ( int i = 0; i < num_children; i++) {
		int pid = fork();

		if(pid == 0) { //child
			run_child(i);
		}

		if(pid < 0) { err(1, "failed to spawn child"); }
	}
	

	printf("Main spawned %d children, waiting for children\n", num_children);
	for( int i = 0; i < num_children; i++) {
		wait(NULL);
	}
	printf("Main exiting\n");
}



