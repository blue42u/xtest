#include <xtask.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int fib(int n) {
	if(n <= 1) return n;
	else return fib(n-1) + fib(n-2);
}

int main(int argc, char** argv) {
	int fibindex = 20;
	if(argc > 1) fibindex = atoi(argv[1]);

	printf("%d\n", fib(fibindex));

	return 0;
}
