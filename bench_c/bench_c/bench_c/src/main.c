#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "common.h"

static void print_usage(const char* exe) {
    printf("Usage:\n");
    printf("  %s --alg <1..5> --threads <n> --runs <k> --size <n> --out <file.jsonl>\n", exe);
    printf("Example:\n");
    printf("  %s --alg 5 --threads 8 --runs 3 --size 1048576 --out results.jsonl\n", exe);
}

int main(int argc, char** argv) {
    int alg = 0, threads = 0, runs = 0;
    uint64_t size = 0;
    const char* out_path = NULL;

    
    if (!parse_int_arg(argc, argv, "--alg", &alg)) {
        fprintf(stderr, "Missing --alg\n"); print_usage(argv[0]); return 1;
    }
    if (!parse_int_arg(argc, argv, "--threads", &threads)) {
        fprintf(stderr, "Missing --threads\n"); print_usage(argv[0]); return 1;
    }
    if (!parse_int_arg(argc, argv, "--runs", &runs)) {
        fprintf(stderr, "Missing --runs\n"); print_usage(argv[0]); return 1;
    }
    if (!parse_u64_arg(argc, argv, "--size", &size)) {
        fprintf(stderr, "Missing --size\n"); print_usage(argv[0]); return 1;
    }
    if (!parse_str_arg(argc, argv, "--out", &out_path)) {
        fprintf(stderr, "Missing --out\n"); print_usage(argv[0]); return 1;
    }

    
    if (alg < 1 || alg > 5) { fprintf(stderr, "Invalid --alg\n"); return 1; }
    if (threads <= 0 || threads > 256) { fprintf(stderr, "Invalid --threads\n"); return 1; }
    if (runs <= 0 || runs > 1000) { fprintf(stderr, "Invalid --runs\n"); return 1; }
    if (size == 0) { fprintf(stderr, "Invalid --size\n"); return 1; }

    
    {
        FILE* f = fopen(out_path, "a");
        if (!f) { fprintf(stderr, "Cannot open --out file\n"); return 1; }
        fclose(f);
    }

    
    for (int r = 0; r < runs; r++) {
        double t0 = now_seconds();

        int ok = 0;
        switch (alg) {
        case ALG_SUMSQ:      ok = run_sumsq(threads, size); break;
        case ALG_MATMUL:     ok = run_matmul(threads, size); break;
        case ALG_MONTECARLO: ok = run_montecarlo(threads, size); break;
        case ALG_MERGESORT:  ok = run_mergesort(threads, size); break;
        case ALG_FFT:        ok = run_fft(threads, size); break;
        default: ok = 0; break;
        }

        double t1 = now_seconds();
        if (!ok) {
            fprintf(stderr, "Algorithm failed (alg=%d)\n", alg);
            return 2;
        }

        RunResult rr;
        rr.language = "c";
        rr.alg = alg;
        rr.threads = threads;
        rr.run_index = r;
        rr.input_size = size;
        rr.seconds = (t1 - t0);

        if (!write_result_jsonl(out_path, &rr)) {
            fprintf(stderr, "Failed writing JSONL\n");
            return 3;
        }
    }

    printf("OK. Wrote results to %s\n", out_path);
    return 0;
}
