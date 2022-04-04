package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Scanner;

import static src.PuzzleState.BOARD_SIZE;

public class Main
{

    public static int parsePuzzleTile(String value) {
        try {
            int numValue = Integer.parseInt(value);

            if (numValue < 0 || numValue > 9) {
                System.out.printf("%d must be between 0 and 8\n", numValue);
                System.exit(1);
            }

            return numValue;
        } catch(NumberFormatException e) {
            System.out.printf("%s must be an integer\n", value);
            System.exit(1);
            return -1;
        }
    }

    public static int[][] readPuzzle(File file) throws FileNotFoundException {
        int[][] puzzle = new int[BOARD_SIZE][BOARD_SIZE];

        int i = 0;
        Scanner fileReader = new Scanner(file);
        while (fileReader.hasNext()) {
            if (i >= BOARD_SIZE) {
                System.out.printf("Puzzle must be %d x %d\n", BOARD_SIZE, BOARD_SIZE);
                System.exit(1);
            }
            String line = fileReader.nextLine();
            String[] columns = line.split(" ");
            int j = 0;
            for (String column : columns) {
                if (j >= BOARD_SIZE) {
                    System.out.printf("Puzzle must be %d x %d\n", BOARD_SIZE, BOARD_SIZE);
                    System.exit(1);
                }
                if (!column.equals("")) {
                    puzzle[i][j] = parsePuzzleTile(column);
                    j++;
                }
            }
            if (j < BOARD_SIZE) {
                System.out.printf("Puzzle must be %d x %d\n", BOARD_SIZE, BOARD_SIZE);
                System.exit(1);
            }
            i++;
        }
        if (i < BOARD_SIZE) {
            System.out.printf("Puzzle must be %d x %d\n", BOARD_SIZE, BOARD_SIZE);
            System.exit(1);
        }

        return puzzle;
    }

    /**
     * Shell to allow for communication with puzzle solver algorithm
     *
     * @param args [initialStateFilePath] [goalStateFilePath] -o [outputFilePath]
     *             Output file is optional, if not specified program will output to console
     */
    public static void main(String[] args) throws FileNotFoundException {
        if (args.length < 2) {
            System.out.println("Proper usage: <initialStateFilePath> <goalStateFilePath> -o <outputFilePath>");
            System.exit(1);
        }

        File initialFile = new File(args[0]);
        File goalFile = new File(args[1]);

        if (!initialFile.exists()) {
            System.out.printf("%s isn't a file\n", initialFile.getName());
            System.exit(1);
        }
        if (!goalFile.exists()) {
            System.out.printf("%s isn't a file", goalFile.getName());
            System.exit(1);
        }

        PrintStream printStream = System.out;
        if (args.length >= 4) {
            File outputFile = new File(args[3]);
            if (!outputFile.exists()) {
                System.out.printf("%s doesn't exist\n", outputFile.getName());
                System.exit(1);
            }
            printStream = new PrintStream(outputFile);
        }

        int[][] initial = readPuzzle(initialFile);
        int[][] goal = readPuzzle(goalFile);

        boolean isValidProblem = PuzzleState.isSolvable(initial, goal)
            && PuzzleState.properZeros(initial)
            && PuzzleState.properZeros(goal);

        System.out.println("Starting...");

        if(isValidProblem) {
            final long startTime = System.currentTimeMillis();

            PuzzleSolver solver = new PuzzleSolver();
            ArrayList<PuzzleState> solution = solver.findSolution(initial, goal);

            final long endTime = System.currentTimeMillis();

            for (PuzzleState state : solution) {
                printStream.println(state.getAction());
                state.printPuzzle(printStream);
            }
            int size = solution.size() - 1;

            printStream.printf("Solved in %d steps\n", size);
            printStream.printf("Total execution time: %d ms\n", endTime - startTime);
        } else {
            System.out.println("Not Solvable");
        }
        System.out.println("Finished!");
    }
}
