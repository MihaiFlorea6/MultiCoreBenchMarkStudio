using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using System.Windows;

namespace MultiCoreBenchmarkStudio.GUI
{
    public partial class MainWindow : Window
    {
        private readonly string root = @"C:\Users\Mihai Florea\Desktop\UTCN\ANII_DE_STUDIU\AnUniv_III_'25-'26\SSC\PROIECT\MultiCoreBenchmarkStudio";

        public MainWindow()
        {
            InitializeComponent();
        }

        private async void RunBenchmark_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                StatusBox.Clear();

                int alg = AlgCombo.SelectedIndex + 1;

                if (!int.TryParse(ThreadsBox.Text.Trim(), out int threads) || threads <= 0)
                    throw new Exception("Threads must be a positive integer.");

                if (!long.TryParse(SizeBox.Text.Trim(), out long size) || size <= 0)
                    throw new Exception("Size must be a positive integer.");

                if (!int.TryParse(RunsBox.Text.Trim(), out int runs) || runs <= 0)
                    throw new Exception("Runs must be a positive integer.");

                string exe = GetExePath();
                string outFile = Path.Combine(root, "results", "results.jsonl");

                Directory.CreateDirectory(Path.GetDirectoryName(outFile)!);

                string args = $"--alg {alg} --threads {threads} --runs {runs} --size {size} --out \"{outFile}\"";

                Log($"EXE: {exe}");
                Log($"ARGS: {args}");
                Log($"OUT:  {outFile}");
                Log("");

                var (exitCode, stdout, stderr) = await RunProcessAsync(exe, args);

                Log($"ExitCode: {exitCode}");
                if (!string.IsNullOrWhiteSpace(stdout))
                {
                    Log("STDOUT:");
                    Log(stdout);
                }
                if (!string.IsNullOrWhiteSpace(stderr))
                {
                    Log("STDERR:");
                    Log(stderr);
                }

                Log("\nDONE  (check results.jsonl)");
            }
            catch (Exception ex)
            {
                Log("ERROR  " + ex.Message);
            }
        }

        private string GetExePath()
        {
            var choice = (LanguageCombo.SelectedIndex);

            string root = @"C:\Users\Mihai Florea\Desktop\UTCN\ANII_DE_STUDIU\AnUniv_III_'25-'26\SSC\PROIECT\MultiCoreBenchmarkStudio";

            return choice switch
            {
                0 => Path.Combine(root, "bench_c", "bench_c", "x64", "Release", "bench_c.exe"),
                1 => Path.Combine(root, "bench_rs", "target", "release", "bench_rs.exe"),
                2 => Path.Combine(root, "bench_java", "bench_java", "bench_java_native.exe"),
                _ => throw new Exception("Unknown language selection.")
            };
        }

        private void Log(string msg)
        {
            StatusBox.AppendText(msg + Environment.NewLine);
            StatusBox.ScrollToEnd();
        }

        private static Task<(int exitCode, string stdout, string stderr)> RunProcessAsync(string fileName, string arguments)
        {
            return Task.Run(() =>
            {
                var psi = new ProcessStartInfo
                {
                    FileName = fileName,
                    Arguments = arguments,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true,
                    WorkingDirectory = Path.GetDirectoryName(fileName) ?? Environment.CurrentDirectory
                };

                using var p = new Process { StartInfo = psi };
                p.Start();

                string stdout = p.StandardOutput.ReadToEnd();
                string stderr = p.StandardError.ReadToEnd();

                p.WaitForExit();
                return (p.ExitCode, stdout, stderr);
            });
        }
    }
}
