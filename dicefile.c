#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "dice.h"



static char* read_file(char* path, size_t* len) {
	size_t l;
	char* buf;
	FILE* f;
	
	f = fopen(path, "rb");
	
	if(!f) goto ERROR_0;
	
	fseek(f, 0, SEEK_END);
	l = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	buf = malloc(l+1);
	
	if(l != fread(buf, 1, l, f)) {
		goto ERROR_1;
	}
	buf[l] = 0;
	
	fclose(f);
	
	if(len) *len = l;
	return buf;
	
	
ERROR_1:
	free(buf);
	fclose(f);
	
ERROR_0:
	if(len) *len = -1;
	
	fprintf(stderr, "Could not read file: '%s'\n", path);
	
	return NULL;
}


preset* preset_list_find(preset_list* pl, char* name) {
	for(size_t i = 0; i < pl->len; i++) {
		if(0 == strcmp(pl->data[i]->name, name)) 
			return pl->data[i]; 
	}
	
	return NULL;
}


int preset_list_set(preset_list* pl, preset* p) {
	for(size_t i = 0; i < pl->len; i++) {
		if(0 == strcmp(pl->data[i]->name, p->name)) {
			pl->data[i] = p;
			return 1;
		}
	}
	
	if(pl->len >= pl->alloc) {
		pl->alloc *= 2;
		pl->data = realloc(pl->data, pl->alloc * sizeof(*pl->data));
	}
	
	pl->data[pl->len] = p;
	pl->len++;
	
	return 0;
}


preset_list* preset_list_create() {
	preset_list* pl;
	
	pl = calloc(1, sizeof(*pl));
	pl->alloc = 4;
	pl->data = calloc(1, pl->alloc * sizeof(*pl->data));
	
	return pl;
}

#define skipspace(s) while(*(s) && isspace(*(s))) { s++; }


char* strtrimdupn(char* s, size_t n) {
	if(n == 0) {
		char* x = malloc(1);
		*x = 0;
		return x;
	}
	
	skipspace(s);
	
	char* o = malloc(n + 1);
	char* e = o;
	
	int i = 0;
	int has_space = 0;
	while(*s && i < n) {
		if(isspace(*s)) {
			if(!has_space) {
				*e++ = ' ';
				has_space = 1;
			}
			s++;
		}
		else {
			*e++ = *s++;
			has_space = 0;
		}
		
		i++;
	}
	
	// trim off a trailing space
	if(*(e-1) == ' ') e--;
	
	*e = 0;
	
	return o;
}


void skipline(char** s) {
	while(**s && **s != '\n') (*s)++;
	if(**s) (*s)++;
}


long dicefile_load_presets(char* path, preset_list* pl) {
	long added = 0;
	
	char* source = read_file(path, NULL);
	if(!source) return 0;
	
	char* s = source;
	
	
	while(*s) {
		
		skipspace(s);
		
		// skip comments
		if(*s == '#') {
			skipline(&s);
			continue;
		}
		
		
		// TODO: check for null's in the strchr
		
		// read the name
		char* e = strchr(s, ':');
		if(!e) continue;
		char* name = strtrimdupn(s, e - s);
// 		printf("name: '%s'\n", name);
		
		// read the definition
		e++;
		char* eol = strchr(e, '\n');
		char* def = strtrimdupn(e, eol - e);
// 		printf("exp: '%s'\n", def);
		
		s = eol;
		
		// parse the roll expression 
		long reps = 1;
		combo* head = NULL;
		combo* tail = NULL;
		
		e = def;
		
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
					fprintf(stderr, "bad expression in file '%s': %s\n", path, def);
					break;
				}
			}
		}
		
		// add the preset if if's valid
		if(head) {
			preset* p = calloc(1, sizeof(*p));
			p->name = name;
			p->source = def;
			p->cmb = head;
			
			if(preset_list_set(pl, p)) {
				fprintf(stderr, "Duplicate preset definition for '%s' found in '%s'\n", name, path);
			}
			
			added++;
		}
		else {
			free(name);
			free(def);
		}
	
	}
	
	return added;
}


