#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
	int tid;
	int threads;
	int n;
	const double* A;
	const double* B;
	double* C;
	int row_start;
	int row_end;
} MatMulTask;		


static DWORD WINAPI matmul_worker(LPVOID arg) {
	MatMulTask* t = (MatMulTask*)arg;
	int n = t->n;
	const double* A = t->A;
	const double* B = t->B;
	double* C = t->C;

	for (int i = t->row_start; i < t->row_end; i++) {
		int iN = i * n;
		for (int j = 0; j < n; j++) {
			double sum = 0.0;
			for (int k = 0; k < n; k++) {
				sum += A[iN + k] * B[k * n + j];
				
			}
			C[iN + j] = sum;
		}

	}
	return 0;
}

int run_matmul(int threads, uint64_t size) {
    if (threads <= 0) return 0;

    int n = (int)size;
    if (n <= 0) return 0;
    if (n > 6000) return 0; 

    size_t nn = (size_t)n * (size_t)n;

    double* A = (double*)malloc(nn * sizeof(double));
    double* B = (double*)malloc(nn * sizeof(double));
    double* C = (double*)malloc(nn * sizeof(double));
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return 0;
    }

   
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            A[idx] = sin((double)(i + j));
            B[idx] = cos((double)(i - j));
            C[idx] = 0.0;
        }
    }

    HANDLE* hs = (HANDLE*)malloc(sizeof(HANDLE) * (size_t)threads);
    MatMulTask* tasks = (MatMulTask*)malloc(sizeof(MatMulTask) * (size_t)threads);
    if (!hs || !tasks) {
        free(hs); free(tasks);
        free(A); free(B); free(C);
        return 0;
    }

    int base = n / threads;
    int rem = n % threads;

    int row = 0;
    for (int t = 0; t < threads; t++) {
        int rows = base + (t < rem ? 1 : 0);
        tasks[t].tid = t;
        tasks[t].threads = threads;
        tasks[t].n = n;
        tasks[t].A = A;
        tasks[t].B = B;
        tasks[t].C = C;
        tasks[t].row_start = row;
        tasks[t].row_end = row + rows;
        row += rows;

        hs[t] = CreateThread(NULL, 0, matmul_worker, &tasks[t], 0, NULL);
        if (!hs[t]) {
            for (int j = 0; j < t; j++) CloseHandle(hs[j]);
            free(hs); free(tasks);
            free(A); free(B); free(C);
            return 0;
        }
    }

    WaitForMultipleObjects((DWORD)threads, hs, TRUE, INFINITE);
    for (int t = 0; t < threads; t++) CloseHandle(hs[t]);

    
    double checksum = 0.0;
    for (size_t i = 0; i < nn; i += (nn / 16 + 1)) checksum += C[i];
    (void)checksum;

    free(hs); free(tasks);
    free(A); free(B); free(C);
    return 1;
}
