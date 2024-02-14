/*
 * Generate random puzzle inputs for any solver implementation to use
 * 2/14/24
 */
package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;

/**
 *
 * @author Joseph Prichard
 */
public class PuzzleGenerator {

    public static void main(String[] args) throws FileNotFoundException {
        if (args.length < 3) {
            System.out.println("Needs at least 3 arguments - the file path, number of puzzles, and nxn puzzle size");
            System.exit(1);
        }

        String outPath = args[0];
        int size = Integer.parseInt(args[1]);
        int n = Integer.parseInt(args[2]);

        File outFile = new File(outPath);

        try (PrintStream stream = new PrintStream(outFile)) {
            PuzzleSolver solver = new PuzzleSolver(n);

            for (int i = 0; i < size; i++) {
                PuzzleState puzzle = solver.generateRandomSolvable();
                puzzle.printPuzzle(stream);
                stream.println();
            }
        }
    }
}
