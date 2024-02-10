# SlidingPuzzle Java

Java AI to solve the [sliding puzzle problem](https://en.wikipedia.org/wiki/15_puzzle) using the AStar algorithm. This AI is guaranteed to find the shortest number of steps to solve any solvable NPuzzle. 

This AI is capable of solving sliding puzzles of various sizes: 8puzzles, 15puzzles, NPuzzles, etc. However since this problem is considered NP-hard it can take a very long time to solve anything larger than a 15puzzle! As such, I reccomend you to stick to 8puzzles and 15puzzles unless you're going to run this AI in the background.

## Usage

To use this program, execute the jar file with the first argument the input file containing the sliding puzzle to be solved. 

You can leave the first argument out to let my program generate a random solvable sliding puzzle it will solve.

Examples for input and output files are contained in the root directory.

## About

I wrote this program originally for my highschool CS class in 2020. Since then I've graduated but I continue to make small changes and optimizations from time to time. Feel free to use this code as long as you give me credit.
