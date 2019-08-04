#ifndef __DICE__dice_h__ 
#define __DICE__dice_h__ 

typedef struct droll {
	long n, d, sign;
} droll;


typedef struct combo {
	droll* rolls;
	long num_rolls;
	long constant;
	long reps;
	
	struct combo* next;
} combo;



long probe_reps(char* s, char** end);
combo* parse_exp(char* s, char** end);




typedef struct preset {
	char* name;
	char* pretty_name;
	char* source;
	combo* cmb;
} preset;

typedef struct preset_list {
	// vector for now
	preset** data;
	size_t len, alloc;
} preset_list;


int preset_list_set(preset_list* pl, preset* p);
preset* preset_list_find(preset_list* pl, char* name);

preset_list* preset_list_create();

long dicefile_load_presets(char* path, preset_list* pl);


#endif // __DICE__dice_h__ 
