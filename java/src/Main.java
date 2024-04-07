package src;

import java.io.*;

/**
 *
 * @author Joseph Prichard
 */
public class Main
{
    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println("Need at least 1 program argument");
            System.exit(1);
        }

        var inputFile = new File(args[0]);
        if (!inputFile.exists()) {
            System.out.println("Failed to read input file " + args[0]);
            System.exit(1);
        }

        var states = Puzzle.fromFile(inputFile);

        var times = new double[states.size()];
        var nodeResults = new int[states.size()];

        for (var i = 0; i < states.size(); i++) {
            var initialState = states.get(i);

            var startTime = System.nanoTime();

            var solver = new PuzzleSolver(initialState.length());
            var solution = solver.findSolution(initialState);

            var time = System.nanoTime() - startTime;
            var nodes = solver.getNodes();

            times[i] = ((double) time) / 1_000_000;
            nodeResults[i] = nodes;

            System.out.printf("Solution for puzzle %d\n", i + 1);
            for (var state : solution) {
                System.out.println(state.getAction());
            }

            System.out.printf("Solved in %d steps\n\n", solution.size() - 1);
        }

        double totalTime = 0;
        int totalNodes = 0;
        for (var i = 0; i < states.size(); i++) {
            System.out.printf("Puzzle %d: %f ms, %d nodes\n", i + 1, times[i], nodeResults[i]);
            totalTime += times[i];
            totalNodes += nodeResults[i];
        }

        System.out.printf("Total: %f ms, %d nodes\n", totalTime, totalNodes);
    }
}