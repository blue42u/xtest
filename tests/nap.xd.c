#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <xdata.h>

static xd_F(nap, dummy, in) {}

static xd_F(longnap, dummy, in) {
	sleep(0);
}

static xd_F(split, dummy, in) {
	int samples = *(int*)in[0];
	int dolong = *(int*)in[1];

	xdata_line* lines = malloc(samples*sizeof(xdata_line));
	for(int i=0; i<samples; i++) {
		lines[i] = xdata_create(XD_STATE, 0);
		if(dolong) xd_P0(lines[i], longnap);
		else xd_P0(lines[i], nap);
	}

	xdata_prepare(XD_STATE, nap, XD_OUT, samples, lines);
	free(lines);
}

int main(int argc, char** argv) {
	xtask_config xc = {.workers=1};
	int* samples = malloc(sizeof(int));
	*samples = 1000;
	int* dolong = malloc(sizeof(int));
	*dolong = 0;

	char c;
	do {
		c = getopt(argc, argv, "w:s:l");
		switch(c) {
		case 'w': xc.workers = atoi(optarg); break;
		case 's': *samples = atoi(optarg); break;
		case 'l': *dolong = 1; break;
		}
	} while(c != -1);

	xd_R(xc, NULL, split, samples, dolong);

	free(samples);
	free(dolong);
	return 0;
}


