#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>


int roll(int d) {
	return floor(((float)rand() / (float)RAND_MAX) * (float)d) + 1;
}


int rolln(int n, int d) {
	int acc = 0;
	for(int i = 0; i < n; i++) acc += roll(d);
	return acc;
}


char* stripspace(char* s) {
	char* o = malloc(strlen(s) + 1);
	char* p = o;
	
	while(*s) {
		if(!isspace(*s)) {
			*p = *s;
			p++;
		}
		s++;
	}
	
	*p = 0;
	
	return o;
}



int main(int argc, char* argv[]) {
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	uint64_t ut = 1000000 * tv.tv_sec + tv.tv_usec;
	
	srand(ut);
	
	int64_t acc;
	
	if(argc < 2) return 0;
	
	char* s = argv[1];
	
	int rep = 1;
	if(*s == 'x' || *s == 'X') {
		rep = strtol(++s, NULL, 10);
		s = argv[2];
	}
	
	
	char* os = stripspace(s);
	for(int r = 0; r < rep; r++) { 
		s = os;
		acc = 0;
		
		while(*s) {
			int n = 0;
			
			n = strtol(s, &s, 10);
			
			if(*s == 'd' || *s == 'D') {
				int d = strtol(++s, &s, 10);
				acc += rolln(n, d);
			}
			else {
				acc += n;
			}
		
		}
		
		printf("%ld\n", acc);
	}
	
	return 0;
}
