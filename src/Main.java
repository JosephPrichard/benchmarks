package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

/**
 *
 * @author Joseph Prichard
 */
public class Main
{
    /**
     * Parses a file and reads it into an initial puzzle state
     *
     * @param file to be parsed into the puzzle state
     * @return the puzzle state
     * @throws FileNotFoundException if the file doesn't exist
     */
    public static PuzzleState readPuzzle(File file) throws FileNotFoundException {
        ArrayList<ArrayList<Integer>> puzzleMatrix = new ArrayList<>();

        // scan through file line by line
        Scanner fileReader = new Scanner(file);
        while (fileReader.hasNext()) {
            // split each line into tokens, parse each token into a matrix value
            String line = fileReader.nextLine();
            String[] tokens = line.split(" ");

            ArrayList<Integer> puzzleRow = new ArrayList<>();

            for (String token : tokens) {
                if (!token.equals("")) {
                    puzzleRow.add(Integer.parseInt(token));
                }
            }

            puzzleMatrix.add(puzzleRow);
        }

        // copy the list to a fixed size matrix
        int[][] puzzle = new int[puzzleMatrix.size()][];
        for (int i = 0; i < puzzleMatrix.size(); i++) {
            puzzle[i] = puzzleMatrix.get(i).stream().mapToInt(num -> num).toArray();
        }

        // verify the the matrix is a square
        if (Utils.checkSquare(puzzle, 3) == -1) {
            System.out.println("Matrices must be square");
            System.exit(1);
        }

        return new PuzzleState(puzzle);
    }

    /**
     * Shell to allow for communication with puzzle solver algorithm
     *
     * @param args [inputFilePath](optional)
     */
    public static void main(String[] args) throws FileNotFoundException {
        File puzzleFile = null;
        if (args.length == 1) {
            puzzleFile = new File(args[0]);
            if (!puzzleFile.exists()) {
                System.out.println("Input file doesn't exist");
                System.exit(1);
            }
        }

        PuzzleSolver solver;
        PuzzleState initialState;

        if (puzzleFile != null) {
            initialState = readPuzzle(puzzleFile);
            solver = new PuzzleSolver(initialState.getBoardSize());
        } else {
            solver = new PuzzleSolver(4);
            initialState = solver.generateRandomSolvable();
        }

        if (!solver.isSolvable(initialState)) {
            System.out.println("Not Solvable");
        }

        System.out.println("Starting...");

        final long startTime = System.currentTimeMillis();

        ArrayList<PuzzleState> solution = solver.findSolution(initialState);

        final long endTime = System.currentTimeMillis();

        for (PuzzleState state : solution) {
            System.out.println(state.getAction());
            state.printPuzzle();
        }
        int size = solution.size() - 1;

        System.out.printf("Solved in %d steps\n", size);
        System.out.printf("Total execution time: %d ms\n", endTime - startTime);

        System.out.println("Finished!");
    }
}
