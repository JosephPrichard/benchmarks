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

        File initialFile = new File(args[0]);
        if (!initialFile.exists()) {
            System.out.println("Input initial state file doesn't exist");
            System.exit(1);
        }

        List<PuzzleState> states = PuzzleState.fromFile(initialFile);
        System.out.printf("Running for %d puzzle input(s)...\n\n", states.size());

        long[] times = new long[states.size()];
        for (int i = 0; i < states.size(); i++) {
            PuzzleState initialState = states.get(i);

            PuzzleSolver solver = new PuzzleSolver(initialState.size());
            long startTime = System.currentTimeMillis();
            List<PuzzleState> solution = solver.findSolution(initialState);
            long time = System.currentTimeMillis() - startTime;

            times[i] = time;

            System.out.printf("Solution for puzzle %d\n", i + 1);
            for (PuzzleState state : solution) {
                System.out.println(state.getAction());
                state.printPuzzle();
            }
            System.out.printf("Solved in %d steps\n\n", solution.size() - 1);
        }

        long totalTime = 0;
        for (int i = 0; i < states.size(); i++) {
            System.out.printf("Puzzle %d took %d ms\n", i + 1, times[i]);
            totalTime += times[i];
        }

        System.out.printf("Took %d ms in total\n", totalTime);
    }
}
