#include <xtask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct {
	xtask_task task;
	int i, j, m;
} taskdata;

typedef struct {
	xtask_task task;
	int n,m;
	taskdata subs[];
} maindata;

static int* matrixA;
static int* matrixB;

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

int main(int argc, char** argv) {
	srandom(time(NULL));
	xtask_config xc = {0};
	int n = 20, m = 0;

	char c;
	do {
		c = getopt(argc, argv, "w:n:m:");
		switch(c) {
		case 'w': xc.workers = atoi(optarg); break;
		case 'n': n = atoi(optarg); break;
		case 'm': m = atoi(optarg); break;
		}
	} while(c != -1);

	if(!m) m = n;

	matrixA = malloc(n*m*sizeof(int));
	matrixB = malloc(n*m*sizeof(int));

	maindata* d = malloc(sizeof(maindata) + n*m*sizeof(taskdata));
	*d = (maindata){ {maincp, 0, &d->subs[0], NULL}, n, m };
	for(int i=0; i<n; i++) for(int j=0; j<m; j++) {
		int x = i*m + j;
		matrixA[x] = random();
		d->subs[x] = (taskdata){
			{rot, XTASK_FATE_LEAF, NULL, &d->subs[x+1]}, i,j,m};
	}
	d->subs[n*m - 1].task.sibling = NULL;

	xtask_run(d, xc);

	free(matrixA);
	free(matrixB);

	return 0;
}
