#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

typedef struct {
	double re;
	double im;						
} Complex;	

static Complex c_add(Complex a, Complex b) {
	Complex r = { a.re + b.re, a.im + b.im };
	return r;
}

static Complex c_sub(Complex a, Complex b) {
	Complex r = { a.re - b.re, a.im - b.im };
	return r;
}

static Complex c_mul(Complex a, Complex b) {
	Complex r = { 
		a.re * b.re - a.im * b.im ,
	    a.re * b.im + b.re * a.im
	};
	return r;
}

static void fft_rec(Complex* x, int n) {
    if (n <= 1) return;

    int half = n / 2;
    Complex* even = (Complex*)malloc(half * sizeof(Complex));
    Complex* odd = (Complex*)malloc(half * sizeof(Complex));

    for (int i = 0; i < half; i++) {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }

    fft_rec(even, half);
    fft_rec(odd, half);

    for (int k = 0; k < half; k++) {
        double ang = -2.0 * 3.141592653589793 * k / n;
        Complex w = { cos(ang), sin(ang) };
        Complex t = c_mul(w, odd[k]);
        x[k] = c_add(even[k], t);
        x[k + half] = c_sub(even[k], t);
    }

    free(even);
    free(odd);
}


int run_fft(int threads, uint64_t size) {
    (void)threads; 

    int n = (int)size;
    if ((n & (n - 1)) != 0) return 0; 

    Complex* x = (Complex*)malloc(n * sizeof(Complex));
    if (!x) return 0;

    for (int i = 0; i < n; i++) {
        x[i].re = sin((double)i);
        x[i].im = 0.0;
    }

    fft_rec(x, n);

    
    volatile double check = 0.0;
    for (int i = 0; i < n; i += (n / 16 + 1))
        check += x[i].re;
    (void)check;

    free(x);
    return 1;
}