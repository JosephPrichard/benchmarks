# Benchmarks

Benchmark of popular programming languages by solving the [sliding puzzle problem](https://en.wikipedia.org/wiki/15_puzzle) using the AStar algorithm.
Each implementation is guaranteed to find the shortest number of steps to solve any solvable NPuzzle. 

The sliding puzzle problem is a space-exploration problem involving list/array traversal, map/priority queue usage, copying, and lots of memory allocation. This makes it an effective way of benchmarking how efficient a programming language is.

Currently includes implementations for Java, C, Elixir, and OCaml. Many more to come - implementing a sliding puzzle solver has become my primary way to learn a new language. 

# Usage
Any puzzle solver executable can be invoked with a command line argument. The command line argument is a path to a file containing newline seperated puzzles to solve.
An example of how valid puzzle files look like is in 8puzzles.txt and 15puzzles.txt. Random puzzle file inputs can be generated in `PuzzleGenerator.java` in the java implementation.

An example 3x3 puzzle input looks like so
```
0 1 2
3 4 5
6 7 8
```

A 4x4 like so
```
0 1 2 3
4 5 6 7
8 9 10 11
12 13 14 15
```

A legal puzzle input must contain a 0 representing the empty space, a space between each column, and a newline between rows. Nonzero tiles can be any integers but the program isn't guaranteed to find the shortest path (or any path) nonconventional puzzles. Generally nonzero tiles for a legal puzzle are numbers 1:(N*N-1). N being the dimension of the puzzle. An 8puzzle would contain integers 1:8 and a 15puzzle integers 1:15 in any order.

If the solver cannot find a solution - it will run until it exhausts all possible states! I'm working on implementing nonsolvable state detection for puzzles of any size.

# Results

Working on a graph that can show the different results for each language.
