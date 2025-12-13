#pragma once

#include <stdint.h>
#include "common.h" 

double now_seconds(void);

int write_result_jsonl(const char* out_path, const RunResult* r);

int parse_int_arg(int argc, char** argv, const char* key, int* out_value);
int parse_u64_arg(int argc, char** argv, const char* key, uint64_t* out_value);
int parse_str_arg(int argc, char** argv, const char* key, const char** out_value);
int run_sumsq(int threads, uint64_t size);
int run_matmul(int threads, uint64_t size);
int run_montecarlo(int threads, uint64_t size);
int run_mergesort(int threads, uint64_t size);
int run_fft(int threads, uint64_t size);