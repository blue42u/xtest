#include <xtask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static int* matrixA;
static int* matrixB;

static void rot(int i, int j, int m) {
	matrixB[i*m + j] = matrixA[j*m + i];
}

static void copy(int i, int j, int m) {
	matrixA[i*m + j] = matrixB[i*m + j];
}

int main(int argc, char** argv) {
	srandom(time(NULL));
	int n = 20, m = 0;

	char c;
	do {
		c = getopt(argc, argv, "n:m:");
		switch(c) {
		case 'n': n = atoi(optarg); break;
		case 'm': m = atoi(optarg); break;
		}
	} while(c != -1);

	if(!m) m = n;

	matrixA = malloc(n*m*sizeof(int));
	matrixB = malloc(n*m*sizeof(int));

	for(int i=0; i<n*m; i++) matrixA[i] = random();

	for(int i=0; i<n; i++) for(int j=0; j<m; j++) rot(i, j, m);
	for(int i=0; i<n; i++) for(int j=0; j<m; j++) copy(i, j, m);

	free(matrixA);
	free(matrixB);

	return 0;
}
