using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace npuzzle
{
    public class Puzzle
    {
        private static readonly int[][] Directions = { new[] { 0, -1 }, new[] { 0, 1 }, new[] { 1, 0 }, new[] { -1, 0 } };

        private static readonly string[] Actions = { "Left", "Right", "Down", "Up" };

        public Puzzle? Parent { get; private set; }
        public int[] Tiles { get; }
        public string Action = "Start";
        public int F { get; set; }
        private int G { get; set; }

        public int Dimension => (int)Math.Sqrt(Tiles.Length);

        public Puzzle(int[] tiles)
        {
            Tiles = tiles;
            F = 0;
            G = 0;
        }

        private static Puzzle OfList(List<int> puzzleList)
        {
            var puzzle = puzzleList.ToArray();
            
            var dimension = Math.Sqrt(puzzle.Length);
            if ((dimension - (int)Math.Floor(dimension)) != 0)
            {
                throw new InvalidPuzzleException("Matrices must be square");
            }

            return new Puzzle(puzzle);
        }

        public static List<Puzzle> FromFile(FileInfo file)
        {
            var states = new List<Puzzle>();
            var currPuzzle = new List<int>();

            using (var reader = new StreamReader(file.FullName))
            {
                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    if (line == null) continue;

                    var tokens = line.Split(' ');
                    if (tokens.Length == 0 || line == string.Empty)
                    {
                        states.Add(OfList(currPuzzle));
                        currPuzzle = new List<int>();
                    }
                    else
                    {
                        foreach (var token in tokens)
                        {
                            if (!string.IsNullOrEmpty(token))
                            {
                                currPuzzle.Add(int.Parse(token));
                            }
                        }
                    }
                }
            }

            states.Add(OfList(currPuzzle));
            return states;
        }

        public void SetFScore(int h)
        {
            F = G + h;
        }

        public bool Equals(Puzzle other) => Tiles.SequenceEqual(other.Tiles);
        
        public override string ToString() => string.Join("", Tiles);

        private static bool InBounds(int row, int col, int dimension) =>
            row >= 0 && row < dimension && col >= 0 && col < dimension;

        public void OnNeighbors(Action<Puzzle> onNeighbor)
        {
            var dimension = Dimension;
            var zeroIndex = FindZero();
            var zeroRow = zeroIndex / dimension;
            var zeroCol = zeroIndex % dimension;

            for (var i = 0; i < Directions.Length; i++)
            {
                var direction = Directions[i];
                var nextRow = zeroRow + direction[0];
                var nextCol = zeroCol + direction[1];

                if (!InBounds(nextRow, nextCol, dimension))
                    continue;

                var nextPuzzle = new int[Tiles.Length];
                Array.Copy(Tiles, nextPuzzle, Tiles.Length);

                var nextIndex = nextRow * dimension + nextCol;
                (nextPuzzle[zeroIndex], nextPuzzle[nextIndex]) = (nextPuzzle[nextIndex], nextPuzzle[zeroIndex]);

                var neighbor = new Puzzle(nextPuzzle)
                {
                    Parent = this,
                    Action = Actions[i],
                    G = G + 1
                };

                onNeighbor(neighbor);
            }
        }

        public List<Puzzle> GetNeighbors()
        {
            var neighbors = new List<Puzzle>();
            OnNeighbors(neighbors.Add);
            return neighbors;
        }

        public void PrintPuzzle() => PrintPuzzle(Console.Out, " ");

        public void PrintPuzzle(TextWriter stream, string empty)
        {
            var dimension = Dimension;
            for (var i = 0; i < Tiles.Length; i++)
            {
                var tile = Tiles[i];
                if (tile == 0)
                    stream.Write(empty + " ");
                else
                    stream.Write(tile + " ");
                if ((i + 1) % dimension == 0)
                    stream.WriteLine();
            }
        }

        private int FindZero()
        {
            for (var i = 0; i < Tiles.Length; i++)
            {
                if (Tiles[i] == 0)
                    return i;
            }

            throw new InvalidPuzzleException("Puzzle must have a 0 tile");
        }
    }
}