#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

static int* arr;

// Lomuto Partition Scheme, as written on Wikipedia
static int part(int lo, int hi) {
	int p = arr[hi];
	int i = lo-1;
	for(int j=lo; j<hi; j++)
		if(arr[j] <= p) {
			i++;
			int v = arr[i];
			arr[i] = arr[j];
			arr[j] = v;
		}
	i++;
	int v = arr[i];
	arr[i] = arr[hi];
	arr[hi] = v;
	return i;
}


#if defined USE_xtask
#include <xtask.h>
typedef struct {
	xtask_task task;
	int lo, hi;
} qdata;

static void* empty(void* d, void* v) { free(v); return NULL; }

static void* sort(void* dummy, void* vdata) {
	qdata* d = vdata;
	int lo = d->lo;
	int hi = d->hi;
	free(d);

	if(lo < hi) {
		int p = part(lo, hi);
		qdata* a = malloc(sizeof(qdata));
		qdata* b = malloc(sizeof(qdata));
		*a = (qdata){{sort, 0, NULL, b}, lo, p-1};
		*b = (qdata){{sort, 0, NULL, NULL}, p+1, hi};
		//xtask_task* x = malloc(sizeof(xtask_task));
		//*x = (xtask_task){empty, 0, a, NULL};
		return a;
	} else return NULL;
}
#elif defined USE_openmp
#include <omp.h>
static void sort(int lo, int hi) {
	if(lo < hi) {
		int p = part(lo, hi);
		#pragma task shared(lo, hi, p)
		sort(lo, p-1);
		#pragma task shared(lo, hi, p)
		sort(p+1, hi);
		#pragma taskwait
	}
}
#elif defined USE_cilk
#include <cilk/cilk.h>
static void sort(int lo, int hi) {
	if(lo < hi) {
		int p = part(lo, hi);
		cilk_spawn sort(lo, p-1);
		cilk_spawn sort(p+1, hi);
		cilk_sync;
	}
}
#elif defined USE_single
static void sort(int lo, int hi) {
	if(lo < hi) {
		int p = part(lo, hi);
		sort(lo, p-1);
		sort(p+1, hi);
	}
}
#endif

int main(int argc, char** argv) {
#if defined USE_xtask
	xtask_config xc = {.workers=1};
#endif
	int size = 50;

	char c;
	do {
		c = getopt(argc, argv, "w:n:");
		switch(c) {
#if defined USE_xtask
		case 'w': xc.workers = atoi(optarg); break;
#elif defined USE_openmp
		case 'w': omp_set_num_threads(atoi(optarg)); break;
#endif
		case 'n': size = atoi(optarg); break;
		}
	} while(c != -1);

	arr = malloc(size*sizeof(int));
	srandom(8472847);
	for(int i=0; i<size; i++) arr[i] = random();

#if defined USE_xtask
	xc.max_leafing = size;
	xc.max_tailing = ceil(log2(size));
	qdata* qd = malloc(sizeof(qdata));
	*qd = (qdata){{sort, 0, NULL, NULL}, 0, size-1};
	xtask_run(qd, xc);
#elif defined USE_single || defined USE_cilk
	sort(0, size-1);
#elif defined USE_openmp
	#pragma omp parallel
	{
		#pragma omp single
		{
			sort(0, size-1);
		}
	}
#endif

	for(int i=1; i<size; i++)
		if(arr[i-1] > arr[i]) {
			fprintf(stderr, "Sorting out of order!");
			return 1;
		}

	free(arr);

	return 0;
}
