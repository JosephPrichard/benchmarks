using System.Diagnostics;

namespace npuzzle
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Need at least 1 program argument");
                Environment.Exit(1);
            }

            var inputFile = new FileInfo(args[0]);
            if (!inputFile.Exists)
            {
                Console.WriteLine($"Input file {args[0]} does not exist");
                Environment.Exit(1);
            }

            StreamWriter? outbWriter = null;
            if (args.Length >= 2)
            {
                var file = new FileInfo(args[1]);
                 if (!file.Exists) 
                {
                    file.Create();
                }
                outbWriter = new StreamWriter(file.FullName);
            }

            StreamWriter? outWriter = null;
            if (args.Length >= 3)
            {
                var file = new FileInfo(args[2]);
                if (!file.Exists) 
                {
                    file.Create();
                }
                outWriter = new StreamWriter(file.FullName);
            }

            var states = Puzzle.FromFile(inputFile);
            Console.WriteLine($"Running for {states.Count} puzzle input(s)...\n");

            var times = new double[states.Count];
            for (var i = 0; i < states.Count; i++)
            {
                var initialState = states[i];

                var solver = new PuzzleSolver(initialState.Tiles.Length);
                var stopwatch = Stopwatch.StartNew();
                var solution = solver.FindSolution(initialState);
                stopwatch.Stop();

                times[i] = ((double) stopwatch.ElapsedTicks) / (Stopwatch.Frequency / 1000L);

                Console.WriteLine($"Solution for puzzle {i + 1}");
                foreach (var state in solution)
                {
                    Console.WriteLine(state.Action);
                    state.PrintPuzzle();
                }

                if (outWriter != null) {
                    outWriter.WriteLine($"{solution.Count - 1} steps");
                }  
                Console.WriteLine($"Solved in {solution.Count - 1} steps, expanded {solver.Nodes} nodes\n");
            }

            double totalTime = 0;
            for (var i = 0; i < states.Count; i++)
            {
                Console.WriteLine($"Puzzle {i + 1} took {times[i]} ms");
                totalTime += times[i];

                if (outbWriter != null) {
                    outbWriter.WriteLine($"{i + 1}, {times[i]}");
                }   
            }

            if (outbWriter != null) {
                outbWriter.WriteLine($"total, {totalTime}");
            }
            Console.WriteLine($"Took {totalTime} ms in total");

            if (outbWriter != null) {
                outbWriter.Flush();
                outbWriter.Close();
            }
            if (outWriter != null) {
                outWriter.Flush();
                outWriter.Close();
            }
        }
    }
}