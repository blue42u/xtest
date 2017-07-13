#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#if defined USE_openmp
#include <omp.h>
#elif defined USE_cilk
#include <cilk/cilk.h>
#elif defined USE_xtask
#include <xtask.h>
typedef struct {
	xtask_task task;
	int i, j, m;
} taskdata;

typedef struct {
	xtask_task task;
	int n,m;
	taskdata subs[];
} maindata;
#endif

static int* matrixA;
static int* matrixB;

#if defined USE_xtask
static void* rot(void* dummy, void* vdata) {
	taskdata* d = vdata;
	matrixB[d->i*d->m + d->j] = matrixA[d->j*d->m + d->i];
	return NULL;
}

static void* copy(void* dummy, void* vdata) {
	taskdata* d = vdata;
	matrixA[d->i*d->m + d->j] = matrixB[d->i*d->m + d->j];
	return NULL;
}
#else
static void rot(int i, int j, int m) {
	matrixB[i*m + j] = matrixA[j*m + i];
}

static void copy(int i, int j, int m) {
	matrixA[i*m + j] = matrixB[i*m + j];
}
#endif

#if defined USE_xtask
static void* mainfree(void* dummy, void* vdata) {
	free(vdata);
	return NULL;
}

static void* maincp(void* dummy, void* vdata) {
	maindata* d = vdata;
	for(int i=0; i < d->n*d->m; i++)
		d->subs[i].task = (xtask_task){
			copy, XTASK_FATE_LEAF, NULL, &d->subs[i+1]};
	d->subs[d->n*d->m - 1].task.sibling = NULL;
	d->task = (xtask_task){ mainfree, 0, &d->subs[0], NULL };
	return d;
}
#endif

int main(int argc, char** argv) {
	srandom(time(NULL));
#if defined USE_xtask
	xtask_config xc = {.workers=1};
#endif
	int n = 20, m = 0;

	char c;
	do {
		c = getopt(argc, argv, "w:n:m:");
		switch(c) {
#if defined USE_xtask
		case 'w': xc.workers = atoi(optarg); break;
#elif defined USE_openmp
		case 'w': omp_set_num_threads(atoi(optarg)); break;
#endif
		case 'n': n = atoi(optarg); break;
		case 'm': m = atoi(optarg); break;
		}
	} while(c != -1);

	if(!m) m = n;

	matrixA = malloc(n*m*sizeof(int));
	matrixB = malloc(n*m*sizeof(int));
	for(int i=0; i<n*m; i++) matrixA[i] = random();

#if defined USE_xtask
	xc.max_leafing = n*m;
	xc.max_tailing = 1;
	maindata* d = malloc(sizeof(maindata) + n*m*sizeof(taskdata));
	*d = (maindata){ {maincp, 0, &d->subs[0], NULL}, n, m };
	for(int i=0; i<n; i++) for(int j=0; j<m; j++) {
		int x = i*m + j;
		d->subs[x] = (taskdata){
			{rot, XTASK_FATE_LEAF, NULL, &d->subs[x+1]}, i,j,m};
	}
	d->subs[n*m - 1].task.sibling = NULL;

	xtask_run(d, xc);
#elif defined USE_openmp || defined USE_single
#ifdef USE_openmp
	#pragma omp parallel
#endif
	{
#if defined USE_openmp
		#pragma omp for
#endif
		for(int i=0; i<n; i++) for(int j=0; j<m; j++) rot(i,j,m);
#ifdef USE_openmp
		#pragma omp for
#endif
		for(int i=0; i<n; i++) for(int j=0; j<m; j++) copy(i,j,m);
	}
#elif defined USE_cilk
	cilk_for(int i=0; i<n; i++) cilk_for(int j=0; j<m; j++) rot(i,j,m);
	cilk_for(int i=0; i<n; i++) cilk_for(int j=0; j<m; j++) copy(i,j,m);
#endif

	free(matrixA);
	free(matrixB);

	return 0;
}
