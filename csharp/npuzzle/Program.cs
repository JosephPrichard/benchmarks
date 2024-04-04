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

            var states = Puzzle.FromFile(inputFile);

            var times = new double[states.Count];
            var nodeResults = new int[states.Count];

            for (var i = 0; i < states.Count; i++)
            {
                var initialState = states[i];

                var solver = new PuzzleSolver(initialState.Tiles.Length);
                var stopwatch = Stopwatch.StartNew();
                var solution = solver.FindSolution(initialState);
                stopwatch.Stop();

                times[i] = ((double) stopwatch.ElapsedTicks) / (Stopwatch.Frequency / 1000L);
                nodeResults[i] = solver.Nodes;

                Console.WriteLine($"Solution for puzzle {i + 1}");
                foreach (var state in solution)
                {
                    Console.WriteLine(state.Action);
                    state.PrintPuzzle();
                }

                Console.WriteLine($"Solved in {solution.Count - 1} steps\n");
            }

            double totalTime = 0;
            int totalNodes = 0;
            for (var i = 0; i < states.Count; i++)
            {
                Console.WriteLine($"Puzzle {i + 1}: {times[i]} ms, {nodeResults[i]} nodes");
                totalTime += times[i];
                totalNodes += nodeResults[i];
            }

            Console.WriteLine($"Total: {totalTime} ms, {totalNodes} nodes");
        }
    }
}