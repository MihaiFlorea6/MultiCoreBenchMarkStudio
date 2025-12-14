use clap::Parser;
use serde::Serialize;
use std::fs::OpenOptions;
use std::io::Write;
use std::time::Instant;


#[derive(Parser, Debug)]
struct Args {
    #[arg(long)]
    alg: u32,
    #[arg(long)]
    threads: u32,
    #[arg(long)]
    runs: u32,
    #[arg(long)]
    size: u64,
    #[arg(long)]
    out: String,
}


#[derive(Serialize)]
struct RunResult<'a> {
    language: &'a str,
    alg: u32,
    threads: u32,
    run_index: u32,
    input_size: u64,
    seconds: f64,
}


fn run_sumsq(size: u64, threads: usize) {
    use rayon::prelude::*;

    
    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(threads)
        .build()
        .expect("failed to build rayon thread pool");

    let sum: f64 = pool.install(|| {
        (0u64..size)
            .into_par_iter()
            .map(|i| {
                let x = i as f64;
                x * x
            })
            .sum()
    });

    
    std::hint::black_box(sum);
}


fn run_matmul(n: u64, threads: usize) {
    use rayon::prelude::*;

    let n = n as usize;
    if n == 0 { return; }

    // Matrici contigue 1D: A[i*n + j]
    let mut a = vec![0.0f64; n * n];
    let mut b = vec![0.0f64; n * n];
    let mut c = vec![0.0f64; n * n];

    // inițializare deterministă (fără RNG, să fie stabil între rulări)
    for i in 0..n {
        for j in 0..n {
            a[i * n + j] = ((i as f64) + (j as f64)).sin();
            b[i * n + j] = ((i as f64) - (j as f64)).cos();
        }
    }

    // Pool cu număr fix de thread-uri
    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(threads)
        .build()
        .expect("failed to build rayon thread pool");

    // Paralelizăm pe rânduri
    pool.install(|| {
        c.par_chunks_mut(n)
            .enumerate()
            .for_each(|(i, c_row)| {
                for j in 0..n {
                    let mut sum = 0.0f64;
                    for k in 0..n {
                        sum += a[i * n + k] * b[k * n + j];
                    }
                    c_row[j] = sum;
                }
            });
    });

    // anti-optimizare
    std::hint::black_box(&c);
}


fn run_montecarlo(size: u64, threads: usize) {
    use rayon::prelude::*;
    use rand::rngs::SmallRng;
    use rand::{Rng, SeedableRng};

    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(threads)
        .build()
        .expect("failed to build rayon thread pool");

    let inside: u64 = pool.install(|| {
        
        let chunk = (size / threads as u64).max(1);
        (0..threads)
            .into_par_iter()
            .map(|tid| {
                let start = tid as u64 * chunk;
                let end = if tid == threads - 1 {
                    size
                } else {
                    ((tid as u64 + 1) * chunk).min(size)
                };

                
                let mut rng = SmallRng::seed_from_u64(0x9E37_79B9_7F4A_7C15u64 ^ (tid as u64 + 1));

                let mut local_inside = 0u64;
                for _ in start..end {
                    let x: f64 = rng.r#gen::<f64>();
                    let y: f64 = rng.r#gen::<f64>();
                    if x * x + y * y <= 1.0 {
                        local_inside += 1;
                    }
                }
                local_inside
            })
            .sum()
    });

    let pi = 4.0 * (inside as f64) / (size as f64);
    std::hint::black_box(pi);
}


fn merge_sort_parallel(arr: &mut [i32], threads: usize) {
    use rayon::prelude::*;

    // prag: sub dimensiunea asta, sortăm secvențial (altfel overhead mare)
    const THRESHOLD: usize = 1 << 15; // 32768

    // buffer auxiliar (alocat o singură dată)
    let mut tmp = vec![0i32; arr.len()];

    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(threads)
        .build()
        .expect("failed to build rayon thread pool");

    pool.install(|| mergesort_rec(arr, &mut tmp, THRESHOLD));
}


fn mergesort_rec(a: &mut [i32], tmp: &mut [i32], threshold: usize) {
    use rayon::join;

    let n = a.len();
    if n <= 1 { return; }

    // dacă e mic, secvențial (folosim sort_unstable ca bază)
    if n <= threshold {
        a.sort_unstable();
        return;
    }

    let mid = n / 2;
    let (left, right) = a.split_at_mut(mid);
    let (tmp_left, tmp_right) = tmp.split_at_mut(mid);

    // paralelizăm cele două jumătăți
    join(
        || mergesort_rec(left, tmp_left, threshold),
        || mergesort_rec(right, tmp_right, threshold),
    );

    // merge în tmp, apoi copiem înapoi
    merge_into(left, right, tmp);
    a.copy_from_slice(&tmp[..n]);
}


fn merge_into(left: &[i32], right: &[i32], out: &mut [i32]) {
    let mut i = 0usize;
    let mut j = 0usize;
    let mut k = 0usize;

    while i < left.len() && j < right.len() {
        if left[i] <= right[j] {
            out[k] = left[i];
            i += 1;
        } else {
            out[k] = right[j];
            j += 1;
        }
        k += 1;
    }
    while i < left.len() {
        out[k] = left[i];
        i += 1; k += 1;
    }
    while j < right.len() {
        out[k] = right[j];
        j += 1; k += 1;
    }
}


fn run_mergesort(size: u64, threads: usize) {
    use rand::Rng;

    let n = size as usize;
    let mut rng = rand::thread_rng();

    let mut arr = vec![0i32; n];
    for x in &mut arr {
        *x = rng.r#gen::<i32>();
    }

    merge_sort_parallel(&mut arr, threads);

    std::hint::black_box(&arr);

}


fn fft_rec(x: &mut [num_complex::Complex64]) {
    let n = x.len();
    if n <= 1 {
        return;
    }

    let mut even: Vec<_> = x.iter().step_by(2).cloned().collect();
    let mut odd: Vec<_> = x.iter().skip(1).step_by(2).cloned().collect();

    fft_rec(&mut even);
    fft_rec(&mut odd);

    for k in 0..n / 2 {
        let ang = -2.0 * std::f64::consts::PI * (k as f64) / (n as f64);
        let w = num_complex::Complex64::new(ang.cos(), ang.sin());
        let t = w * odd[k];
        x[k] = even[k] + t;
        x[k + n / 2] = even[k] - t;
    }
}


fn run_fft(size: u64, _threads: usize) {
    let n = size as usize;

    // trebuie putere a lui 2
    if n == 0 || (n & (n - 1)) != 0 {
        return;
    }

    let mut x: Vec<num_complex::Complex64> = (0..n)
        .map(|i| num_complex::Complex64::new((i as f64).sin(), 0.0))
        .collect();

    fft_rec(&mut x);

    std::hint::black_box(&x);
}


fn main() -> anyhow::Result<()> {
    let args = Args::parse();

    if !(1..=5).contains(&args.alg) {
        anyhow::bail!("--alg must be 1..5");
    }
    if args.threads == 0 {
        anyhow::bail!("--threads must be >= 1");
    }
    if args.runs == 0 {
        anyhow::bail!("--runs must be >= 1");
    }
    if args.size == 0 {
        anyhow::bail!("--size must be >= 1");
    }

    
    std::fs::write(&args.out, "")?;

    for r in 0..args.runs {
        let t0 = Instant::now();

        match args.alg {
    1 => run_sumsq(args.size, args.threads as usize),
    2 => run_matmul(args.size, args.threads as usize),
    3 => run_montecarlo(args.size, args.threads as usize),
    4 => run_mergesort(args.size, args.threads as usize),
    5 => run_fft(args.size, args.threads as usize),
    _ => { std::hint::black_box(args.size); }
    
}


        let sec = t0.elapsed().as_secs_f64();

        let rr = RunResult {
            language: "rust",
            alg: args.alg,
            threads: args.threads,
            run_index: r,
            input_size: args.size,
            seconds: sec,
        };

        let mut f = OpenOptions::new().append(true).open(&args.out)?;
        writeln!(f, "{}", serde_json::to_string(&rr)?)?;
    }

    println!("OK. Wrote results to {}", args.out);
    Ok(())
}
