import sys;
import random;

int n = string2int(argv("n", "20"));
int m = string2int(argv("m", int2string(n)));

int A[][];
int B[][];

foreach i in [0:n-1] {
	foreach j in [0:m-1] {
		A[i][j] = randint(0,10000);
	}
}

foreach i in [0:n-1] {
	foreach j in [0:m-1] {
		B[i][j] = A[j][i];
	}
}

foreach i in [0:n-1] {
	foreach j in [0:m-1] {
		A[i][j] = B[i][j];
	}
}
