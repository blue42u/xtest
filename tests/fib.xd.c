#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xdata.h>

#define XDATA_STATE XD_s

static void add(void* dummy, xdata_state* XD_s, void* in[], void* out[]) {
	int *a = in[0], *b = in[1], *o = out[0];
	*o = *a + *b;
}

static void fib(void* dummy, xdata_state* XD_s, void* in[], void* out[]) {
	int *n = in[0], *o = out[0];

	if(*n <= 1) *o = *n;
	else {
		xd_C(int, a); xd_C(int, an);
		*an = *n-1;
		xd_P(fib, xd_L(an), xd_L(a));

		xd_C(int, b); xd_C(int, bn);
		*bn = *n-2;
		xd_P(fib, xd_L(bn), xd_L(b));

		xd_P(add, xd_L(a, b), xd_L(o));
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
	xd_R(fib, xc, xd_L(&fibindex), xd_L(&out));
	printf("%d\n", out);

	return 0;
}
