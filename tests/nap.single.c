#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void nap() {}

static void longnap() {
	sleep(0);
}

int main(int argc, char** argv) {
	int samples = 1000;
	void (*n)() = nap;

	char c;
	do {
		c = getopt(argc, argv, "s:l");
		switch(c) {
		case 's': samples = atoi(optarg); break;
		case 'l': n = longnap; break;
		}
	} while(c != -1);

	for(int i=0; i<samples; i++)
		n();

	return 0;
}


