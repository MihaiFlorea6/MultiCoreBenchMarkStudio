#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	int* arr;
	int* tmp;
	int left;
	int right;
	int depth;
	int max_depth;
} MergeTask;


static void merge(int* arr, int* tmp, int l, int m, int r) {
	int i = l, j = m + 1, k = l;

	while (i <= m && j <= r) 
		if (arr[i] <= arr[j]) {
			tmp[k++] = arr[i++];
		}
		else {
			tmp[k++] = arr[j++];
		}

		while (i <= m) tmp[k++] = arr[i++];
		while (j <= r) tmp[k++] = arr[j++];

		for (i = l; i <= r; i++)
			arr[i] = tmp[i];
}

static DWORD WINAPI mergesort_worker(LPVOID arg) {
    MergeTask* t = (MergeTask*)arg;
    int l = t->left;
    int r = t->right;

    if (l >= r) return 0;

    int m = (l + r) / 2;

    if (t->depth < t->max_depth) {
        MergeTask left = { t->arr, t->tmp, l, m, t->depth + 1, t->max_depth };
        MergeTask right = { t->arr, t->tmp, m + 1, r, t->depth + 1, t->max_depth };

        HANDLE h = CreateThread(NULL, 0, mergesort_worker, &left, 0, NULL);
        mergesort_worker(&right);

        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }
    else {
        MergeTask left = { t->arr, t->tmp, l, m, t->depth + 1, t->max_depth };
        MergeTask right = { t->arr, t->tmp, m + 1, r, t->depth + 1, t->max_depth };
        mergesort_worker(&left);
        mergesort_worker(&right);
    }

    merge(t->arr, t->tmp, l, m, r);
    return 0;
}

int run_mergesort(int threads, uint64_t size) {
    if (threads <= 0) return 0;
    if (size < 2) return 0;

    int n = (int)size;
    int* arr = (int*)malloc(n * sizeof(int));
    int* tmp = (int*)malloc(n * sizeof(int));
    if (!arr || !tmp) {
        free(arr); free(tmp);
        return 0;
    }

    for (int i = 0; i < n; i++)
        arr[i] = rand();

    int max_depth = 0;
    while ((1 << max_depth) < threads)
        max_depth++;

    MergeTask root = { arr, tmp, 0, n - 1, 0, max_depth };
    mergesort_worker(&root);

    
    volatile int check = 0;
    for (int i = 0; i < n; i += (n / 16 + 1))
        check += arr[i];
    (void)check;

    free(arr);
    free(tmp);
    return 1;
}