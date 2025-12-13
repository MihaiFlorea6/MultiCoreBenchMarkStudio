#include "utils.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

double now_seconds(void) {
#ifdef _WIN32
	static LARGE_INTEGER freq;
	static int initialized = 0;
	if(!initialized) {
		QueryPerformanceFrequency(&freq);
		initialized = 1;
	}
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (double)t.QuadPart / (double)freq.QuadPart;

 #else
	return 0.0;
#endif
}

int write_result_jsonl(const char* out_path, const RunResult* r) {
	FILE* f = fopen(out_path, "a");
	if(!f) {
		return 0;
	}
	fprintf(f,
		"{\"language\":\"%s\",\"alg\":%d,\"threads\":%d,\"run_index\":%d,\"input_size\":%llu,\"seconds\":%.6f}\n",
		r->language ? r->language : "c",
		r->alg,
		r->threads,
		r->run_index,
		(unsigned long long)r->input_size,
		r->seconds
	);
	fclose(f);
	return 1;
}

static int str_eq(const char* a, const char* b) {
	return a && b && strcmp(a, b) == 0;
}

int parse_int_arg(int argc, char** argv, const char* key, int* out_value) {
	for(int i = 1; i + 1 < argc; i++) {
		if(str_eq(argv[i], key)) {
			*out_value = atoi(argv[i + 1]);
			return 1;
		}
	}
	return 0;
}

int parse_u64_arg(int argc, char** argv, const char* key, uint64_t* out_value) {
	for(int i = 1; i + 1 < argc; i++) {
		if(str_eq(argv[i], key)) {
			*out_value = (uint64_t)_strtoui64(argv[i + 1], NULL, 10);
			return 1;
		}
	}
	return 0;
}

int parse_str_arg(int argc, char** argv, const char* key, const char** out_value) {
	for(int i = 1; i + 1 < argc; i++) {
		if(str_eq(argv[i], key)) {
			*out_value = argv[i + 1];
			return 1;
		}
	}
	return 0;
}