#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#if defined USE_xtask
#include <xtask.h>
#elif defined USE_openmp
#include <omp.h>
#endif

static void* nap(void* dummy, void* data) {
	return NULL;
}

static void* longnap(void* dummy, void* data) {
	sleep(0);
	return NULL;
}

int main(int argc, char** argv) {
#if defined USE_xtask
	xtask_config xc = {.workers=1};
#endif
	int samples = 1000;
	void* (*n)(void*, void*) = nap;

	char c;
	do {
		c = getopt(argc, argv, "w:s:l");
		switch(c) {
#if defined USE_xtask
		case 'w': xc.workers = atoi(optarg); break;
#elif defined USE_openmp
		case 'w': omp_set_num_threads(atoi(optarg)); break;
#endif
		case 's': samples = atoi(optarg); break;
		case 'l': n = longnap; break;
		}
	} while(c != -1);

#if defined USE_xtask
	xtask_task* tasks = malloc(samples*sizeof(xtask_task));
	for(int i=0; i<samples; i++)
		tasks[i] = (xtask_task){n, XTASK_FATE_LEAF, NULL, &tasks[i+1]};
	tasks[samples-1].sibling = NULL;
	xc.max_leafing = samples+5;
	xtask_run(tasks, xc);
	free(tasks);
#elif defined USE_single
	for(int i=0; i<samples; i++)
		n(NULL, NULL);
#elif defined USE_openmp
	#pragma omp parallel
	{
		#pragma omp for
		for(int i=0; i<samples; i++)
			n(NULL, NULL);
	}
#endif

	return 0;
}


