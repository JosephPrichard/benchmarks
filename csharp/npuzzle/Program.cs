using System.Diagnostics;

namespace npuzzle
{
    internal class Program
    {
        struct Solution {
            public double Time;
            public int Nodes;
            public List<Puzzle> Path;

            public Solution(double time, int nodes, List<Puzzle> path)
            {
                Time = time;
                Nodes = nodes;
                Path = path;
            }
        }

        static List<Solution> RunSolvers(List<Puzzle> states) {
            List<Solution> solutions = [];
            foreach (var state in states)
            {
                var solver = new PuzzleSolver(state.Tiles.Length);
                var stopwatch = Stopwatch.StartNew();
                var solution = solver.FindSolution(state);
                stopwatch.Stop();

                var time = ((double) stopwatch.ElapsedTicks) / (Stopwatch.Frequency / 1000L);

                solutions.Add(new Solution(time, solver.Nodes, solution));
            }
            return solutions;
        }

        static List<Solution> RunSolversParallel(List<Puzzle> states) {
            ThreadPool.SetMaxThreads(Environment.ProcessorCount, 0);

            List<Task<Solution>> tasks = [];
            foreach (var state in states)
            {
                var task = Task.Run(() => 
                {
                    var solver = new PuzzleSolver(state.Tiles.Length);
                    var stopwatch = Stopwatch.StartNew();
                    var solution = solver.FindSolution(state);
                    stopwatch.Stop();

                    var time = ((double) stopwatch.ElapsedTicks) / (Stopwatch.Frequency / 1000L);
                    return new Solution(time, solver.Nodes, solution);
                });
                tasks.Add(task);
            }

            var joinTask = Task.WhenAll(tasks.ToArray());
            joinTask.Wait();
            return [.. joinTask.Result];
        }

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

            var flag = "seq";
            if (args.Length >= 2) {
                flag = args[1];
            }

            var states = Puzzle.FromFile(inputFile);

            var stopwatch = Stopwatch.StartNew();

            List<Solution> solutions = [];
            if (flag == "seq") {
                solutions = RunSolvers(states);
            } else if (flag == "par") {
                solutions = RunSolversParallel(states);
            } else {
                Console.WriteLine("Flag must be seq or par, got " + flag);
                Environment.Exit(1);
            }

            stopwatch.Stop();
            var eteTime = ((double) stopwatch.ElapsedTicks) / (Stopwatch.Frequency / 1000L);
            
            for (var i = 0; i < solutions.Count; i++) 
            {
                var sol = solutions[i];

                Console.WriteLine($"Solution for puzzle {i + 1}");
                foreach (var state in sol.Path)
                {
                    Console.WriteLine(state.Action);
                }
                Console.WriteLine($"Solved in {sol.Path.Count - 1} steps\n");
            }

            double totalTime = 0;
            int totalNodes = 0;

            for (var i = 0; i < solutions.Count; i++)
            {
                var sol = solutions[i];

                Console.WriteLine($"Puzzle {i + 1}: {sol.Time} ms, {sol.Nodes} nodes");
                totalTime += sol.Time;
                totalNodes += sol.Nodes;
            }

            Console.WriteLine($"\nTotal: {totalTime} ms, {totalNodes} nodes");

            Console.WriteLine($"End-to-end: {eteTime} ms");
        }
    }
}