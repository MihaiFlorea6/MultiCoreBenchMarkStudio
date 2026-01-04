# MultiCore Benchmark Studio



A cross-language, multi-core CPU benchmarking suite + visualization app.



This project measures how different algorithms scale with the number of threads on a modern multi-core processor, then compares results across multiple programming languages and runtimes. The final goal is a single, modern GUI (dark mode) that runs benchmarks, parses results, and shows smooth, interactive charts.



## Current Status



\+ `bench\_c` implemented (C/C++ on Windows)  

\- `bench\_rs` planned (Rust)  

\- `bench\_java` planned (Java Native via GraalVM Native Image)  

\- `gui` planned (WPF desktop app, dark mode)



---



## What `bench\_c` does



`bench\_c` is the baseline benchmark runner. It executes a selected algorithm with a selected thread count and writes results in \*\*JSONL\*\* (one run per line).



### Algorithms implemented



1\. \*\*Sum of Squares\*\* (sumsq)  

&nbsp;  CPU-bound numeric loop stressing ALU/FPU and instruction throughput.



2\. \*\*Matrix Multiplication\*\* (matmul)  

&nbsp;  Heavy compute + memory access; stresses caches, memory bandwidth, and CPU vectorization/throughput.



3\. \*\*Monte Carlo PI\*\* (montecarlo)  

&nbsp;  Random sampling + floating point; stresses branch prediction + FP operations.



4\. \*\*Parallel Merge Sort\*\* (mergesort)  

&nbsp;  Divide-and-conquer + allocations + memory traffic; stresses scheduler and memory subsystem.



5\. \*\*FFT (Fast Fourier Transform)\*\* (fft)  

&nbsp;  Classic signal-processing transform; stresses floating point + recursion + memory access patterns.



---



## Output format (JSONL)



Each benchmark run writes one line like:



```json

{"language":"c","alg":5,"threads":8,"run\_index":1,"input\_size":1048576,"seconds":0.123456}



