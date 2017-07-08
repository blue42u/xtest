import sys;

(int o) fib(int n) {
	if(n <= 1) {
		o = n;
	} else {
		o = fib(n-1) + fib(n-2);
	}
}

int f = fib(string2int(argv("f", "20")));
if(!argv_contains("q")) {
	trace(f);
}
