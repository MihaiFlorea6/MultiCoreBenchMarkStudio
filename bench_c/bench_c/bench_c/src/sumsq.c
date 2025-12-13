#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct SumSqTask{
	uint64_t start;
	uint64_t end;
	double result;
} SumSqTask;	

static DWORD WINAPI sumsq_worker(LPVOID arg) {
	SumSqTask* t = (SumSqTask*)arg;
	double s = 0.0;

	for (uint64_t i = t->start; i < t->end; i++) {
		double x = (double)i;
		s += x * x;
	}

	t->result = s;
	return 0;
}

int run_sumsq(int threads, uint64_t size) {
    if (threads <= 0) return 0;
    if (size < (uint64_t)threads) return 0;

    HANDLE* hs = (HANDLE*)malloc(sizeof(HANDLE) * (size_t)threads);
    SumSqTask* tasks = (SumSqTask*)malloc(sizeof(SumSqTask) * (size_t)threads);
    if (!hs || !tasks) {
        free(hs);
        free(tasks);
        return 0;
    }

    uint64_t chunk = size / (uint64_t)threads;

    for (int i = 0; i < threads; i++) {
        tasks[i].start = (uint64_t)i * chunk;
        tasks[i].end = (i == threads - 1) ? size : (uint64_t)(i + 1) * chunk;
        tasks[i].result = 0.0;

        hs[i] = CreateThread(NULL, 0, sumsq_worker, &tasks[i], 0, NULL);
        if (hs[i] == NULL) {
            
            for (int j = 0; j < i; j++) CloseHandle(hs[j]);
            free(hs);
            free(tasks);
            return 0;
        }
    }

    WaitForMultipleObjects((DWORD)threads, hs, TRUE, INFINITE);

    double total = 0.0;
    for (int i = 0; i < threads; i++) {
        CloseHandle(hs[i]);
        total += tasks[i].result;
    }

    free(hs);
    free(tasks);

    
    (void)total;
    return 1;
}