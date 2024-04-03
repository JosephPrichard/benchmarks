using System;
using System.Diagnostics;

namespace npuzzle
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Needs at least 1 argument - the file");
                Environment.Exit(1);
            }

            var initialFile = new System.IO.FileInfo(args[0]);
            if (!initialFile.Exists)
            {
                Console.WriteLine("Input initial state file doesn't exist");
                Environment.Exit(1);
            }

            var states = Puzzle.FromFile(initialFile);
            Console.WriteLine($"Running for {states.Count} puzzle input(s)...\n");

            var times = new double[states.Count];
            for (var i = 0; i < states.Count; i++)
            {
                var initialState = states[i];

                var solver = new PuzzleSolver(initialState.Tiles.Length);
                var stopwatch = Stopwatch.StartNew();
                var solution = solver.FindSolution(initialState);
                stopwatch.Stop();

                times[i] = stopwatch.ElapsedMilliseconds;

                Console.WriteLine($"Solution for puzzle {i + 1}");
                foreach (var state in solution)
                {
                    Console.WriteLine(state.Action);
                    state.PrintPuzzle();
                }

                Console.WriteLine($"Solved in {solution.Count - 1} steps, expanded {solver.Nodes} nodes\n");
            }

            double totalTime = 0;
            for (var i = 0; i < states.Count; i++)
            {
                Console.WriteLine($"Puzzle {i + 1} took {times[i]} ms");
                totalTime += times[i];
            }

            Console.WriteLine($"Took {totalTime} ms in total");
        }
    }
}