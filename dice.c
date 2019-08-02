#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>

#include "dice.h"


static int isnum(int c) {
	return isdigit(c) || c == '-' || c == '+';
 }


combo* parse_exp(char* s, char** end) {
	int not_first = 0;
	long rlen = 4;
	combo* b = calloc(1, sizeof(*b));
	
	b->rolls = calloc(1, sizeof(*b->rolls) * rlen);
	b->reps = 1;
	
	while(*s) {
		const char c = *s;
		
		if(isspace(c)) {
			s++;
			continue;
		}
		
		if(not_first && (c != '+' && c != '-')) {
			break;
		}
		
		if(isnum(c)) {
			not_first = 1;
			
			int sign = c == '+' ? 1 : (c == '-' ? -1 : 0);
			if(sign != 0) {
				s++;
				while(*s && isspace(*s)) s++;
			}
			long n = strtol(s, &s, 10);
			if(sign != 0) n *= sign;
			
 			while(*s && isspace(*s)) s++;
			
			if(*s == 'd' || *s == 'D') {
				if(sign < 0) {
					n = -n;
					b->rolls[b->num_rolls].sign = -1;
				}
				else b->rolls[b->num_rolls].sign = 1;
				
				b->rolls[b->num_rolls].n = n;
				
				// TODO: error handling on the strtol's
				n = strtol(++s, &s, 10);
				b->rolls[b->num_rolls].d = n;
				
				b->num_rolls++;
				if(b->num_rolls > rlen) {
					rlen *= 2;
					b->rolls = realloc(b->rolls, sizeof(*b->rolls) * rlen); 
				}
			}
			else {
				b->constant += n;
			}
			
		}
		else {
			
			free(b->rolls);
			free(b);
			
			return NULL;
		}
		
	}
	
	if(end) *end = s;
	
	
	if(b->num_rolls == 0 && b->constant == 0) {
		// the combo is empty
		free(b->rolls);
		free(b);
		return NULL;
	}
	
	return b;
}


long probe_reps(char* s, char** end) {
	long n;
	
	while(*s && isspace(*s)) s++;
	
	if(*s == 'x' || *s == 'X') {
		n = strtol(++s, &s, 10); 
	}
	else if(isdigit(*s)) {
		n = strtol(s, &s, 10);

		while(*s && isspace(*s)) s++;
		
		if(*s != 'x' && *s != 'X') {
			return -1;
		}
	}
	else {
		return -1;
	}
	
	if(end) *end = s;
	
	return n;
}









long roll(long d) {
	return floor(((float)rand() / (float)RAND_MAX) * (float)d) + 1;
}


long rolln(long n, long d) {
	long acc = 0;
	long multiplier = 1;
	if (n < 0) {
		multiplier = -1;
		n *= -1;
	}
	for(long i = 0; i < n; i++) acc += roll(d);
	return acc * multiplier;
}

long rolld(droll* r) {
	long acc = 0;
	for(long i = 0; i < r->n; i++) acc += roll(r->d);
	return acc * r->sign;
}


// roll a combo once; ignores b->reps
long rollcombo(combo* b) {
	long acc = b->constant;
	
	for(long i = 0; i < b->num_rolls; i++) {
		acc += rolld(&b->rolls[i]);
	}
	
	return acc;
}




static void print_help_and_die(char* invoked_as) {
	fprintf(stderr, 
		"Usage: %s [-f filename] [-p preset] [-n reps] [expression]\n"
		"\n"
		"Rolls some dice.\n"
		"If a preset is provided, expression is ignored.\n"
		"\n"
		"Options:\n"
		"\t-f filename    Sets the name of the file to search for presets\n"
		"\t               (default is 'dicefile')\n"
		"\t-p preset      Specifies a preset to use rather than an expression\n"
		"\t-n reps        Specifies number of times to repeat this roll\n"
		"\t-h             Display this message\n"
		"\n"
		"Notes:\n"
		"    Given the syntax:\n"
		"\n"
		"    <expression> ::= <expr-item> | <expr-item> <expression>\n"
		"     <expr-item> ::= <roll> | <numeric>\n"
		"          <roll> ::= <number> \"d\" <number>\n"
		"        <number> ::= <numeric> | <sign> <numeric>\n"
		"       <numeric> ::= <digit> | <digit> <numeric>\n"
		"          <sign> ::= \"+\" | \"-\"\n"
		"         <digit> ::= any digit 0 through 9\n"
		"\n"
		"    An expression is <expression> where each <expr-item> is summed together.\n"
		"    The result of evaluating a <roll> is the equivalent of rolling an m sided\n"
		"    die n times, where n is the first <number> and m is the second <number>.\n"
	, invoked_as);
	
	exit(1);
}


struct work {
	char* preset_name;
	long reps;
	
	combo* combo;
	
	struct work* next;
};




int main(int argc, char* argv[]) {
	int debug = 0;
	
	preset_list* pl;
	
	struct work* work_h = NULL;
	struct work* work_t = NULL;
	
	long reps = 1;
	
	// derp
	if(argc < 2) {
		print_help_and_die(argv[0]);
	}
	
	
	// seed the rng
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ut = 1000000 * tv.tv_sec + tv.tv_usec;
	
	srand(ut);
	
	
	
	pl = preset_list_create(); 
	
	
	
	for(int ar = 1; ar < argc; ar++) {
		char* arg = argv[ar];
		
		if(0 == strcasecmp("--debug", arg)) {
			debug = 1;
		}
		else if(0 == strcasecmp("-f", arg)) {
			if(++ar >= argc) print_help_and_die(argv[0]);
			
			dicefile_load_presets(argv[ar], pl);
		}
		else if(0 == strcasecmp("-h", arg)) {
			print_help_and_die(argv[0]);
		}
		else if(0 == strcasecmp("-n", arg)) {
			if(++ar >= argc) print_help_and_die(argv[0]);
			
			reps = strtol(argv[ar], NULL, 10);
		}
		else if(0 == strcasecmp("-p", arg)) {
			if(++ar >= argc) print_help_and_die(argv[0]);
			
			struct work* w = calloc(1, sizeof(*w));
			
			w->preset_name = argv[ar];
			w->reps = reps;
			
			if(work_h == NULL) work_h = w;
			if(work_t != NULL) work_t->next = w;
			work_t = w;
		}
		else {
			// bare arg
			
			
			char* e = arg;
			combo* head = NULL;
			combo* tail = NULL;
			
			
			while(*e) {
				long r = probe_reps(e, &e);
				
				if(r > 0) {
					reps = r;
					continue;
				}
				else {
					combo* b = parse_exp(e, &e);
					if(b) {
						b->reps = reps;
						
						if(head == NULL) head = b;
						if(tail != NULL) tail->next = b;
						tail = b;
					}
					else {
						break;
					}
				}
			}
			
			// add the combo if if's valid
			if(head) {
				struct work* w = calloc(1, sizeof(*w));
			
				w->combo = head;
				
				if(work_h == NULL) work_h = w;
				if(work_t != NULL) work_t->next = w;
				work_t = w;
			}
			
		}
	}
	
	
	
	
	struct work* w = work_h;
	
	while(w) {
		combo* cmb = NULL;
		preset* p;
		
		if(w->combo) {
			cmb = w->combo;
		}
		else {
			if(debug) printf("preset name: '%s'\n", w->preset_name);
			p = preset_list_find(pl, w->preset_name); 
			if(p) cmb = p->cmb;
			else printf("no such preset: '%s'\n", w->preset_name);
		}
		
		if(cmb) {
			if(debug) {
				printf("reps: %ld, constant: %ld\n", cmb->reps, cmb->constant);
				for(int i = 0; i < cmb->num_rolls; i++) {
					printf("  n %ld, d %ld, s %ld\n", cmb->rolls[i].n,cmb->rolls[i].d,cmb->rolls[i].sign);
				}
				printf("\n");
			}
			
			for(int r = 0; r < cmb->reps; r++) {
				if(p) printf("%s: ", p->name);
				
				combo* b = cmb;
				while(b) {
					long a = rollcombo(b);
					
					printf("%ld ", a);
					
					b = b->next;
				}
				
				printf("\n");
			}
		}
		
		
		w = w->next;
		if(w) printf("\n");
	}
	
	
	return 0;
}
