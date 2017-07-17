#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xdata.h>

static xd_F(add, dummy, in) {
	int *a = in[0], *b = in[1];
	xd_O(int, *a + *b);
}

static xd_F(fib, dummy, in) {
	int *n = in[0];

	if(*n <= 1) xd_O(int, *n);
	else {
		xd_C(int, a);
		xd_CV(int, an, *n-1);
		xd_P(a, fib, an);

		xd_C(int, b);
		xd_CV(int, bn, *n-2);
		xd_P(b, fib, bn);

		xd_P(XD_OUT, add, a, b);
	}
}

int main(int argc, char** argv) {
	xtask_config xc = { .workers=1, .max_leafing=10, };
	int fibindex = 20;

	char c;
	do {
		c = getopt(argc, argv, "w:f:");
		switch(c) {
		case 'w': xc.workers = atoi(optarg); break;
		case 'f': fibindex = atoi(optarg); break;
		}
	} while(c != -1);

	int out;
	xc.max_tailing = fibindex + 5;
	xd_R(xc, &out, fib, &fibindex);
	printf("%d\n", out);

	return 0;
}
