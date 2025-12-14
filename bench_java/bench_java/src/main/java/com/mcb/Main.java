package com.mcb;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;
import java.util.*;
import java.util.concurrent.*;

public class Main {

    static final String LANG = "java_native";

    static class Args {
        int alg;
        int threads;
        int runs;
        long size;
        Path out;
    }

    private static void usageAndExit() {
        System.out.println("Usage: bench_java --alg <1..5> --threads <n> --runs <n> --size <n> --out <file>");
        System.exit(2);
    }

    private static Args parseArgs(String[] args) {
        if (args.length == 0) usageAndExit();

        Map<String, String> m = new HashMap<>();
        for (int i = 0; i < args.length; i++) {
            String k = args[i];
            if (!k.startsWith("--")) usageAndExit();
            if (i + 1 >= args.length) usageAndExit();
            String v = args[++i];
            m.put(k, v);
        }

        Args a = new Args();
        try {
            a.alg = Integer.parseInt(req(m, "--alg"));
            a.threads = Integer.parseInt(req(m, "--threads"));
            a.runs = Integer.parseInt(req(m, "--runs"));
            a.size = Long.parseLong(req(m, "--size"));
            a.out = Path.of(req(m, "--out"));
        } catch (Exception e) {
            usageAndExit();
        }

        if (a.alg < 1 || a.alg > 5 || a.threads < 1 || a.runs < 1 || a.size < 1) usageAndExit();
        return a;
    }

    private static String req(Map<String, String> m, String key) {
        String v = m.get(key);
        if (v == null) usageAndExit();
        return v;
    }

    static void writeJsonl(String file, String language, int alg, int threads, int runIndex, long inputSize, double seconds) {
        String line = String.format(
                "{\"language\":\"%s\",\"alg\":%d,\"threads\":%d,\"run_index\":%d,\"input_size\":%d,\"seconds\":%.9f}%n",
                language, alg, threads, runIndex, inputSize, seconds
        );
        try (BufferedWriter bw = new BufferedWriter(new FileWriter(file, true))) {
            bw.write(line);
        } catch (Exception e) {
            throw new RuntimeException("Failed writing JSONL: " + file, e);
        }
    }


    static void runSumSq(int threads, long n) {
            ExecutorService pool = Executors.newFixedThreadPool(threads);
            List<Future<Double>> futures = new ArrayList<>(threads);

            long chunk = n / threads;

            for (int t = 0; t < threads; t++) {
                final long start = t * chunk;
                final long end = (t == threads - 1) ? n : (t + 1) * chunk;

                futures.add(pool.submit(() -> {
                    double sum = 0.0;
                    for (long i = start; i < end; i++) {
                        double di = (double) i;
                        sum += di * di;
                    }
                    return sum;
                }));
            }

            double total = 0.0;
            try {
                for (Future<Double> f : futures) total += f.get();
            } catch (Exception e) {
                throw new RuntimeException(e);
            } finally {
                pool.shutdown();
            }


            if (total == -1.0) System.out.println(total);
    }


    static void runMatMul(int threads, long nLong) {
            int n = (int) nLong;
            int nn = n * n;

            double[] A = new double[nn];
            double[] B = new double[nn];
            double[] C = new double[nn];

            for (int i = 0; i < nn; i++) {
                A[i] = Math.sin(i * 0.001);
                B[i] = Math.cos(i * 0.001);
            }

            ExecutorService pool = Executors.newFixedThreadPool(threads);
            List<Future<?>> futures = new ArrayList<>(threads);

            int rowsPer = n / threads;

            for (int t = 0; t < threads; t++) {
                final int r0 = t * rowsPer;
                final int r1 = (t == threads - 1) ? n : (t + 1) * rowsPer;

                futures.add(pool.submit(() -> {
                    for (int i = r0; i < r1; i++) {
                        int iBase = i * n;
                        for (int k = 0; k < n; k++) {
                            double a = A[iBase + k];
                            int kBase = k * n;
                            for (int j = 0; j < n; j++) {
                                C[iBase + j] += a * B[kBase + j];
                            }
                        }
                    }
                }));
            }

            try {
                for (Future<?> f : futures) f.get();
            } catch (Exception e) {
                throw new RuntimeException(e);
            } finally {
                pool.shutdown();
            }

            if (C[0] == -123.0) System.out.println(C[0]);
    }


    static void runMonteCarlo(int threads, long iterations) {
            ExecutorService pool = Executors.newFixedThreadPool(threads);
            List<Future<Long>> futures = new ArrayList<>(threads);

            long chunk = iterations / threads;
            long seedBase = System.nanoTime();

            for (int t = 0; t < threads; t++) {
                final long start = t * chunk;
                final long end = (t == threads - 1) ? iterations : (t + 1) * chunk;
                final long seed = seedBase ^ (0x9E3779B97F4A7C15L * (t + 1));

                futures.add(pool.submit(() -> {
                    SplittableRandom rng = new SplittableRandom(seed);
                    long inside = 0;
                    for (long i = start; i < end; i++) {
                        double x = rng.nextDouble();
                        double y = rng.nextDouble();
                        if (x * x + y * y <= 1.0) inside++;
                    }
                    return inside;
                }));
            }

            long insideTotal = 0;
            try {
                for (Future<Long> f : futures) insideTotal += f.get();
            } catch (Exception e) {
                throw new RuntimeException(e);
            } finally {
                pool.shutdown();
            }

            double pi = 4.0 * (double) insideTotal / (double) iterations;
            if (pi == -1.0) System.out.println(pi);
    }


    static void runMergeSort(int threads, long nLong) {

            int n = (int) nLong;
            int[] arr = new int[n];
            SplittableRandom rng = new SplittableRandom(12345);

            for (int i = 0; i < n; i++) arr[i] = rng.nextInt();

            ForkJoinPool fj = new ForkJoinPool(threads);
            int[] tmp = new int[n];

            fj.invoke(new MergeSortTask(arr, tmp, 0, n));
            fj.shutdown();


            if (n >= 2 && arr[0] > arr[n - 1]) System.out.println("?");
        }

        static class MergeSortTask extends RecursiveAction {
            private static final int THRESH = 1 << 14; // 16384
            final int[] a, tmp;
            final int lo, hi; // [lo, hi)

            MergeSortTask(int[] a, int[] tmp, int lo, int hi) {
                this.a = a; this.tmp = tmp; this.lo = lo; this.hi = hi;
            }

            @Override protected void compute() {
                int len = hi - lo;
                if (len <= 1) return;
                if (len <= THRESH) {
                    Arrays.sort(a, lo, hi);
                    return;
                }
                int mid = lo + (len / 2);
                MergeSortTask left = new MergeSortTask(a, tmp, lo, mid);
                MergeSortTask right = new MergeSortTask(a, tmp, mid, hi);
                invokeAll(left, right);
                merge(a, tmp, lo, mid, hi);
            }

            static void merge(int[] a, int[] tmp, int lo, int mid, int hi) {
                int i = lo, j = mid, k = lo;
                while (i < mid && j < hi) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
                while (i < mid) tmp[k++] = a[i++];
                while (j < hi) tmp[k++] = a[j++];
                System.arraycopy(tmp, lo, a, lo, hi - lo);
            }
    }


    static void runFFT(int threadsIgnored, long nLong) {
            int n = (int) nLong;
            if ((n & (n - 1)) != 0) throw new IllegalArgumentException("FFT size must be power of 2");

            double[] re = new double[n];
            double[] im = new double[n];

            for (int i = 0; i < n; i++) {
                re[i] = Math.sin(i * 0.01);
                im[i] = 0.0;
            }


            for (int i = 1, j = 0; i < n; i++) {
                int bit = n >>> 1;
                for (; (j & bit) != 0; bit >>>= 1) j ^= bit;
                j ^= bit;
                if (i < j) {
                    double tr = re[i]; re[i] = re[j]; re[j] = tr;
                    double ti = im[i]; im[i] = im[j]; im[j] = ti;
                }
            }

            for (int len = 2; len <= n; len <<= 1) {
                double ang = -2.0 * Math.PI / len;
                double wLenRe = Math.cos(ang);
                double wLenIm = Math.sin(ang);

                for (int i = 0; i < n; i += len) {
                    double wRe = 1.0, wIm = 0.0;
                    int half = len >>> 1;

                    for (int j = 0; j < half; j++) {
                        int u = i + j;
                        int v = u + half;

                        double vr = re[v] * wRe - im[v] * wIm;
                        double vi = re[v] * wIm + im[v] * wRe;

                        re[v] = re[u] - vr;
                        im[v] = im[u] - vi;
                        re[u] = re[u] + vr;
                        im[u] = im[u] + vi;

                        double nwRe = wRe * wLenRe - wIm * wLenIm;
                        double nwIm = wRe * wLenIm + wIm * wLenRe;
                        wRe = nwRe; wIm = nwIm;
                    }
                }
            }

            if (re[0] == 123.0) System.out.println(re[0]);

    }

    public static void main(String[] argv) {
        Args args = parseArgs(argv);

        for (int r = 0; r < args.runs; r++) {
            long t0 = System.nanoTime();

            switch (args.alg) {
                case 1 -> runSumSq(args.threads, args.size);
                case 2 -> runMatMul(args.threads, args.size);
                case 3 -> runMonteCarlo(args.threads, args.size);
                case 4 -> runMergeSort(args.threads, args.size);
                case 5 -> runFFT(args.threads, args.size);
                default -> throw new IllegalArgumentException("alg must be 1..5");
            }

            long t1 = System.nanoTime();
            double seconds = (t1 - t0) / 1e9;

            writeJsonl(args.out.toString(), LANG, args.alg, args.threads, r, args.size, seconds);
            System.out.printf("OK. Wrote run %d to %s%n", r, args.out);
        }
    }
}
