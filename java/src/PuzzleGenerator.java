/*
 * Generate random puzzle inputs for any solver implementation to use
 * 2/14/24
 */
package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;

/**
 *
 * @author Joseph Prichard
 */
public class PuzzleGenerator {

    public static void main(String[] args) throws IOException {
        if (args.length < 3) {
            System.out.println("Needs at least 3 arguments - the file path, number of puzzles, and nxn puzzle size");
            System.exit(1);
        }

        var outPath = args[0];
        var size = Integer.parseInt(args[1]);
        var n = Integer.parseInt(args[2]);

        var outFile = new File(outPath);

        try (var stream = new PrintStream(outFile)) {
            var solver = new PuzzleSolver(n);

            for (var i = 0; i < size; i++) {
                var puzzle = solver.generateRandomSolvable();
                puzzle.printPuzzle(stream, "0");
                stream.println();
            }
        }
    }
}