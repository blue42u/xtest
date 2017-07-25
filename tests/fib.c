#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined USE_xtask
#include <xtask.h>
typedef struct {
	xtask_task task;
	int n;
	int* out;
} fibdata;

typedef struct {
	xtask_task task;
	fibdata a;
	fibdata b;
	int aout, bout;
	int* out;
} fulldata;

static void* add(void* dummy, void* vdata) {
	fulldata* data = vdata;
	*data->out = data->aout + data->bout;
	free(data);
	return NULL;
}

static void* fib(void* dummy, void* vdata) {
	fibdata* data = vdata;
	if(data->n <= 1) {
		*data->out = data->n;
		return NULL;
	} else {
		fulldata* fd = malloc(sizeof(fulldata));
		*fd = (fulldata){
			{add, XTASK_FATE_LEAF, &fd->a, NULL},
			{{fib, 0, NULL, &fd->b}, data->n-1, &fd->aout},
			{{fib, 0, NULL, NULL},   data->n-2, &fd->bout},
			0, 0, data->out,
		};
		return fd;
	}
}
#elif defined USE_openmp
#include <omp.h>
static int fib(int n) {
	if(n <= 1) return n;
	int a,b;
	#pragma omp task shared(a)
	a = fib(n-1);
	#pragma omp task shared(b)
	b = fib(n-2);
	#pragma omp taskwait
	return a+b;
}
#elif defined USE_cilk
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
static int fib(int n) {
	if(n <= 1) return n;
	int a = cilk_spawn fib(n-1);
	int b = cilk_spawn fib(n-2);
	cilk_sync;
	return a+b;
}
#elif defined USE_single
static int fib(int n) {
	if(n <= 1) return n;
	else return fib(n-1) + fib(n-2);
}
#endif

int main(int argc, char** argv) {
#if defined USE_xtask
	xtask_config xc = {
		.workers=1,
		.max_leafing=10,
	};
#endif
	int fibindex = 20;
	int quiet = 0;

	char c;
	do {
		c = getopt(argc, argv, "w:f:q");
		switch(c) {
#if defined USE_xtask
		case 'w': xc.workers = atoi(optarg); break;
#elif defined USE_openmp
		case 'w': omp_set_num_threads(atoi(optarg)); break;
#elif defined USE_cilk
		case 'w': __cilkrts_set_param("nworkers", optarg); break;
#endif
		case 'f': fibindex = atoi(optarg); break;
		}
	} while(c != -1);

	int out;
#if defined USE_xtask
	xc.max_tailing = fibindex + 5;
	fibdata fd = {{fib, 0, NULL, NULL}, fibindex, &out};
	xtask_run(&fd, xc);
#elif defined USE_single || defined USE_cilk
	out = fib(fibindex);
#elif defined USE_openmp
	#pragma omp parallel
	{
		#pragma omp single
		{
			out = fib(fibindex);
		}
	}
#endif

	printf("%d\n", out);

	return 0;
}
