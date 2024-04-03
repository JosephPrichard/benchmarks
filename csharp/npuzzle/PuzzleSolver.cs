using System;
using System.Collections.Generic;

namespace npuzzle
{
    public class PuzzleSolver
    {
        private readonly Puzzle _goalState;
        public int Nodes { get; private set; }

        public PuzzleSolver(int boardSize)
        {
            var num = 0;
            var goalPuzzle = new int[boardSize];
            for (var i = 0; i < goalPuzzle.Length; i++)
            {
                goalPuzzle[i] = num;
                num++;
            }

            _goalState = new Puzzle(goalPuzzle);
        }

        private static int Heuristic(Puzzle puzzleState)
        {
            var puzzle = puzzleState.Tiles;
            var dimension = puzzleState.Dimension;
            var h = 0;
            for (var i = 0; i < puzzle.Length; i++)
            {
                var row1 = i / dimension;
                var col1 = i % dimension;
                var row2 = puzzle[i] / dimension;
                var col2 = puzzle[i] % dimension;
                h += ManhattanDistance(row1, col1, row2, col2);
            }

            return h;
        }

        private static int ManhattanDistance(int row1, int col1, int row2, int col2)
        {
            return Math.Abs(row2 - row1) + Math.Abs(col2 - col1);
        }

        public List<Puzzle> FindSolution(Puzzle initialState)
        {
            var visited = new HashSet<string>();
            var frontier = new PriorityQueue<Puzzle, int>();
            frontier.Enqueue(initialState, initialState.F);

            Nodes = 0;
            while (frontier.Count > 0)
            {
                var currentState = frontier.Dequeue();
                visited.Add(currentState.ToString());
                Nodes += 1;

                if (currentState.Equals(_goalState))
                {
                    return ReconstructPath(currentState);
                }

                currentState.OnNeighbors((neighbor) =>
                {
                    if (!visited.Contains(neighbor.ToString()))
                    {
                        var h = Heuristic(neighbor);
                        neighbor.SetFScore(h);
                        frontier.Enqueue(neighbor, neighbor.F);
                    }
                });
            }

            return new List<Puzzle>();
        }

        private static List<Puzzle> ReconstructPath(Puzzle currentState)
        {
            var list = new List<Puzzle>();
            while (currentState != null)
            {
                list.Add(currentState);
                currentState = currentState.Parent;
            }

            list.Reverse();
            return list;
        }
    }
}