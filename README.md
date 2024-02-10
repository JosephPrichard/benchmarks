# SlidingPuzzle

A collection of implementations solve the [sliding puzzle problem](https://en.wikipedia.org/wiki/15_puzzle) using the AStar algorithm in a variety of languages. The program is guaranteed to find the shortest number of steps to solve any solvable NPuzzle. 

Currently includes implementations for Java, C, and OCaml. Many more to come - implementing a sliding puzzle solver has become my primary way to learn a new language.

## Benchmarks

Currently working on some benchmarks to measure whether which implementation is the fastest. The C version needs some fixes to support solving 15puzzles. The benchmarks will be performed with a large 8puzzle 100 times in a row and a large 15puzzle 1 time. The former will test which implementation has the lowest overhead and the latter which scales the best into larger puzzles.