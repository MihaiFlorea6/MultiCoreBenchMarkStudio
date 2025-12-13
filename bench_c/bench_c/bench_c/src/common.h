#pragma once

#include <stdint.h>

typedef enum {
	ALG_SUMSQ = 1,
	ALG_MATMUL = 2,
	ALG_MONTECARLO = 3,
	ALG_MERGESORT = 4,
	ALG_FFT = 5
} AlgId;

typedef struct {
	const char* language;
	int alg;
	int threads;
	int run_index;
	uint64_t input_size;
	double seconds;
} RunResult;