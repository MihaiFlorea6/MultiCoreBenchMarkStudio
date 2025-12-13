#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h> 


static uint64_t xorshift64star(uint64_t* state) {
	uint64_t x = *state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	*state = x;
	return x * UINT64_C(2685821657736338717);
}

static double rand01(uint64_t* state) {
	uint64_t r = xorshift64star(state);
	return (r >> 11) * (1.0 / 9007199254740992.0);
}


typedef struct MC_Task{
	uint64_t iters;
	uint64_t inside;
	uint64_t seed;
} MC_Task;

static DWORD WINAPI montecarlo_worker(LPVOID arg);

int run_montecarlo(int threads, uint64_t size) {
    if (threads <= 0) return 0;
    if (size < (uint64_t)threads) return 0;

    HANDLE* hs = (HANDLE*)malloc(sizeof(HANDLE) * (size_t)threads);
    MC_Task* tasks = (MC_Task*)malloc(sizeof(MC_Task) * (size_t)threads);
    if (!hs || !tasks) {
        free(hs);
        free(tasks);
        return 0;
    }

    uint64_t base = size / (uint64_t)threads;
    uint64_t rem = size % (uint64_t)threads;

    uint64_t tseed = (uint64_t)time(NULL) ^ 0x9E3779B97F4A7C15ULL;

    for (int i = 0; i < threads; i++) {
        uint64_t it = base + (i < (int)rem ? 1ULL : 0ULL);
        tasks[i].iters = it;
        tasks[i].inside = 0;
        tasks[i].seed = tseed ^ (uint64_t)(i + 1) * 0xBF58476D1CE4E5B9ULL;

        hs[i] = CreateThread(NULL, 0, montecarlo_worker, &tasks[i], 0, NULL);
        if (!hs[i]) {
            for (int j = 0; j < i; j++) CloseHandle(hs[j]);
            free(hs);
            free(tasks);
            return 0;
        }
    }

    WaitForMultipleObjects((DWORD)threads, hs, TRUE, INFINITE);

    uint64_t inside_total = 0;
    for (int i = 0; i < threads; i++) {
        CloseHandle(hs[i]);
        inside_total += tasks[i].inside;
    }

    free(hs);
    free(tasks);


    double pi = 4.0 * (double)inside_total / (double)size;
    (void)pi;

    return 1;
}

static DWORD WINAPI montecarlo_worker(LPVOID arg) {
    MC_Task* t = (MC_Task*)arg;
	uint64_t state = t->seed ? t->seed : 88172645463325252ULL;

    uint64_t inside = 0;
    for (uint64_t i = 0; i < t->iters; i++) {
        double x = rand01(&state);
        double y = rand01(&state);
        double d = x * x + y * y;
        if (d <= 1.0) inside++;
    }
    t->inside = inside;
    return 0;
}

