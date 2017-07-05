import sys;
import random;

int n = string2int(argv("n", "20"));
int m = string2int(argv("m", int2string(n)));

(int A[][]) fill() {
	foreach i in [0:n-1] {
		foreach j in [0:m-1] {
			A[i][j] = randint(0,10000);
		}
	}
}

(int B[][]) rot(int A[][]) {
	foreach i in [0:n-1] {
		foreach j in [0:m-1] {
			B[i][j] = A[j][i];
		}
	}
}

rot(fill())
