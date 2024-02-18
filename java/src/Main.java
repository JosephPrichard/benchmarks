package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.List;

/**
 *
 * @author Joseph Prichard
 */
public class Main
{
    public static void main(String[] args) throws FileNotFoundException {
        if (args.length < 1) {
            System.out.println("Needs at least 1 argument - the file");
            System.exit(1);
        }

        var initialFile = new File(args[0]);
        if (!initialFile.exists()) {
            System.out.println("Input initial state file doesn't exist");
            System.exit(1);
        }

        var states = PuzzleState.fromFile(initialFile);
        System.out.printf("Running for %d puzzle input(s)...\n\n", states.size());

        var times = new double[states.size()];
        for (var i = 0; i < states.size(); i++) {
            var initialState = states.get(i);

            var solver = new PuzzleSolver(initialState.size());
            var startTime = System.nanoTime();
            var solution = solver.findSolution(initialState);
            var time = System.nanoTime() - startTime;

            times[i] = ((double) time) / 1_000_000;

            System.out.printf("Solution for puzzle %d\n", i + 1);
            for (var state : solution) {
                System.out.println(state.getAction());
                state.printPuzzle();
            }
            System.out.printf("Solved in %d steps\n\n", solution.size() - 1);
        }

        double totalTime = 0;
        for (var i = 0; i < states.size(); i++) {
            System.out.printf("Puzzle %d took %f ms\n", i + 1, times[i]);
            totalTime += times[i];
        }

        System.out.printf("Took %f ms in total\n", totalTime);
    }
}
