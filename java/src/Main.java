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

        PrintStream outbWriter = null;
        if (args.length >= 2) {
            var file = new File(args[1]);
            if (inputFile.exists()) {
                outbWriter = new PrintStream(file);
            } else {
                System.out.println("Failed to open the benchmark file");
            }
        }

        PrintStream outWriter = null;
        if (args.length >= 3) {
            var file = new File(args[2]);
            if (inputFile.exists()) {
                outWriter = new PrintStream(file);
            } else {
                System.out.println("Failed to open the out file");
            }
        }

        var states = Puzzle.fromFile(inputFile);
        System.out.printf("Running for %d puzzle input(s)...\n\n", states.size());

        var totalNodes = 0;
        var times = new double[states.size()];
        for (var i = 0; i < states.size(); i++) {
            var initialState = states.get(i);

            var solver = new PuzzleSolver(initialState.length());
            var startTime = System.nanoTime();
            var solution = solver.findSolution(initialState);
            var time = System.nanoTime() - startTime;

            times[i] = ((double) time) / 1_000_000;

            System.out.printf("Solution for puzzle %d\n", i + 1);
            for (var state : solution) {
                System.out.println(state.getAction());
                state.printPuzzle();
            }

            var nodes = solver.getNodes();
            if (outWriter != null) {
                outWriter.printf("%d steps\n", solution.size() - 1);
            }
            System.out.printf("Solved in %d steps, expanded %d nodes\n\n", solution.size() - 1, nodes);
            totalNodes += nodes;
        }

        double totalTime = 0;
        for (var i = 0; i < states.size(); i++) {
            System.out.printf("Puzzle %d took %f ms\n", i + 1, times[i]);
            totalTime += times[i];

            if (outbWriter != null) {
                outbWriter.println((i + 1) + ", " + times[i]);
            }
        }

        System.out.printf("Took %f ms in total, expanded %d nodes in total\n", totalTime, totalNodes);
        if (outbWriter != null) {
            outbWriter.println("total, " + totalTime);
        }

        if (outWriter != null) {
            outWriter.close();
        }
        if (outbWriter != null) {
            outbWriter.close();
        }
    }
}