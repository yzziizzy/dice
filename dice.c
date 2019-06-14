#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include "dicefile.h"

int roll(int d) {
	return floor(((float)rand() / (float)RAND_MAX) * (float)d) + 1;
}


int rolln(int n, int d) {
	int acc = 0;
    int multiplier = 1;
    if (n < 0) {
        multiplier = -1;
        n *= -1;
    }
	for(int i = 0; i < n; i++) acc += roll(d);
	return acc * multiplier;
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
    char *dicefile = "dicefile";
    char *preset = NULL;
    
    int rep = 1;
    char *rep_end;
    
    int opt = 0;
    while (opt >= 0) {
        opt = getopt(argc, argv, "p:n:f:h");
        if (opt < 0) {
          break;  
        }
        switch (opt) {
            case 'f':
                dicefile = optarg;
                break;
            case 'p':
                preset = optarg;
                break;
            case 'n': 
                rep = strtol(optarg, &rep_end, 10);
                if (*rep_end) {
                    fprintf(stderr, "Invalid number specified for reps (%s)\n", optarg);
                    return 1;
                }
                break;
            default:
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
            , argv[0]);
                return 0;
        }
    }
    char expression[1000];
    size_t expr_len = sizeof(expression) - 1;
    if (preset != NULL) {
        if (!dicefile_find_line(dicefile, preset, expression, expr_len)) {
            return 1;
        }
    }
    else {
        char **cursor = argv + optind;
        size_t offset = 0;
        while (*cursor) {
            if (offset > 0 && offset + 1 < expr_len) {
                expression[offset++] = ' ';
            }
            size_t len = strlen(*cursor);
            if (len + offset > expr_len) {
                len = expr_len - offset;
            }
            memcpy(expression + offset, *cursor, len);
            offset += len;
            ++cursor;
        }
        expression[offset] = '\0';
    }
    
    
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	
	uint64_t ut = 1000000 * tv.tv_sec + tv.tv_usec;
	
	srand(ut);
	
	int64_t acc;
	
	if(argc < 2) return 0;
	
	char* s = expression;
	
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
