#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>


int roll(int d) {
	return floor(((float)rand() / (float)RAND_MAX) * (float)d) + 1;
}


int rolln(int n, int d) {
	int acc = 0;
	for(int i = 0; i < n; i++) acc += roll(d);
	return acc;
}





int main(int argc, char* argv[]) {
	srand(time(NULL));
	
	if(argc < 2) return 0;
	
	char* s = argv[1];
	char* e = NULL;
	
	int n = strtol(s, &e, 10);
	if(*e != 'd') {
		printf("%d\n", roll(n));
		return 0;
	}
	
	int d = strtol(++e, &e, 10);
	if(*e != '+' && *e != '-') {
		printf("%d\n", rolln(n, d));
		return 0;
	}
	
	int p = strtol(e, &e, 10);
	
	printf("%d\n", rolln(n, d) + p);
	
	
	return 0;
}
