#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

inline double gettime() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000);
}

int main(int argc, char* const* argv) {
	if(argc < 2) {
		printf("Usage: %s <program> [<args>...]\n", argv[0]);
		return 1;
	}

	double real = gettime();

	pid_t p = fork();
	if(p == -1) {
		perror("while forking");
		return 1;
	}
	if(p == 0) {
		dup2(2, 1);
		execvp(argv[1], &argv[1]);
		perror("in child's exec");
		return 1;
	}
	int exit;
	waitpid(p, &exit, 0);

	real = gettime() - real;

	if(exit != 0) {
		printf("-1 -1 -1 TIME\n");
		return 1;
	}

	struct rusage ru;
	getrusage(RUSAGE_CHILDREN, &ru);
	double usr = (double)ru.ru_utime.tv_sec
		+ ((double)ru.ru_utime.tv_usec / 1000000);
	double sys = (double)ru.ru_stime.tv_sec
		+ ((double)ru.ru_stime.tv_usec / 1000000);

	printf("%lf %lf %lf TIME\n", real, usr, sys);

	return 0;
}
