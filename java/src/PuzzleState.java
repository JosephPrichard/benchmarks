/*
 * An object of this class represents each puzzle state (node) for the puzzle problem
 * A puzzle state encapsulates the internal puzzle, the previous state, the score rankings, etc.
 * 4/15/20
 */
package src;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 *
 * @author Joseph Prichard
 */
public class PuzzleState
{
    private PuzzleState parent = null;
    private final int[][] puzzle;
    private String action = "Start";

    private final int zeroRow;
    private final int zeroCol;

    private int f = 0; // ranking value
    private int g = 0; // path cost

    public PuzzleState(int[][] puzzle) {
        this.puzzle = puzzle;
        //calculate the position of 0 in current
        int zeroRow = -1;
        int zeroCol = -1;
        for (int i = 0; i < size(); i++) {
            for (int j = 0; j < size(); j++) {
                if (puzzle[i][j] == 0) {
                    zeroRow = i;
                    zeroCol = j;
                    break;
                }
            }
        }
        this.zeroRow = zeroRow;
        this.zeroCol = zeroCol;
    }

    private PuzzleState(int[][] puzzle, int zeroRow, int zeroCol) {
        this.puzzle = puzzle;
        this.zeroRow = zeroRow;
        this.zeroCol = zeroCol;
    }

    public static PuzzleState fromMatrix(List<List<Integer>> curr) {
        int[][] puzzle = Utils.listMatrixToArray(curr);
        // verify the matrix is a square
        if (Utils.checkSquare(puzzle, 3) == -1) {
            throw new InvalidPuzzleException("Matrices must be square");
        }
        return new PuzzleState(puzzle);
    }

    public static List<PuzzleState> fromFile(File file) throws FileNotFoundException {
        List<PuzzleState> states = new ArrayList<>();
        List<List<Integer>> curr = new ArrayList<>();

        // scan through file line by line
        Scanner fileReader = new Scanner(file);
        while (fileReader.hasNext()) {
            // split each line into tokens, parse each token into a matrix value
            String line = fileReader.nextLine();
            String[] tokens = line.split(" ");
            if (tokens.length == 0 || line.isEmpty()) {
                states.add(fromMatrix(curr));
                curr = new ArrayList<>();
            } else {
                List<Integer> row = new ArrayList<>();
                for (String token : tokens) {
                    if (!token.isEmpty()) {
                        row.add(Integer.parseInt(token));
                    }
                }
                curr.add(row);
            }
        }

        states.add(fromMatrix(curr));
        return states;
    }

    public int size() {
        return puzzle.length;
    }

    public PuzzleState getParent() {
        return parent;
    }

    public void setFScore(int h) {
        this.f = g + h;
    }

    public int getFScore() {
        return f;
    }
    
    public String getAction() {
        return action;
    }

    public int[][] getPuzzle() {
        return puzzle;
    }

    /**
     * Unlink state from the state tree
     */
    public void unlink() {
        action = "Start";
        parent = null;
    }

    @Override
    public boolean equals(Object puzzleState) {
        if (puzzleState.getClass() != PuzzleState.class) {
            return false;
        }

        int[][] puzzle1 = ((PuzzleState) puzzleState).getPuzzle();

        for (int i = 0; i < size(); i++) {
            for (int j = 0; j < size(); j++) {
                if (puzzle[i][j] != puzzle1[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    @Override
    public int hashCode() {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i = 0; i < size(); i++) {
            for (int j = 0; j < size(); j++) {
                stringBuilder.append(puzzle[i][j]);
            }
        }
        return stringBuilder.toString().hashCode();
    }

    public PuzzleState shiftUp() {
        if (zeroRow - 1 < 0) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow - 1][zeroCol];
        clonedPuzzle[zeroRow - 1][zeroCol] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow - 1, zeroCol);
        childPuzzleState.parent = this;
        childPuzzleState.action = "Up";
        childPuzzleState.g = g + 1;
        return childPuzzleState;
    }

    public PuzzleState shiftDown() {
        if (zeroRow + 1 > size() -1) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow + 1][zeroCol];
        clonedPuzzle[zeroRow + 1][zeroCol] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow + 1, zeroCol);
        childPuzzleState.parent = this;
        childPuzzleState.action = "Down";
        childPuzzleState.g = g + 1;
        return childPuzzleState;
    }

    public PuzzleState shiftLeft() {
        if (zeroCol - 1 < 0) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow][zeroCol - 1];
        clonedPuzzle[zeroRow][zeroCol - 1] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow, zeroCol - 1);
        childPuzzleState.parent = this;
        childPuzzleState.action = "Left";
        childPuzzleState.g = g + 1;
        return childPuzzleState;
    }

    public PuzzleState shiftRight() {
        if (zeroCol + 1 > size() -1) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow][zeroCol + 1];
        clonedPuzzle[zeroRow][zeroCol + 1] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow, zeroCol + 1);
        childPuzzleState.parent = this;
        childPuzzleState.action = "Right";
        childPuzzleState.g = g + 1;
        return childPuzzleState;
    }

    public void printPuzzle() {
        printPuzzle(System.out, " ");
    }

    public void printPuzzle(PrintStream stream, String empty) {
        for (int[] puzzleRow : puzzle) {
            for (int tile : puzzleRow) {
                if (tile == 0) {
                    stream.print(empty  + " ");
                } else {
                    stream.print(tile + " ");
                }
            }
            stream.println();
        }
    }

    public boolean properZeros() {
        int counter = 0;
        for (int[] puzzleRow : puzzle) {
            for (int tile : puzzleRow) {
                if (tile == 0) {
                    counter++;
                }
            }
        }
        return counter == 1;
    }

    public int find0Position() {
        for (int i = size() - 1; i >= 0; i--) {
            for (int j = size() - 1; j >= 0; j--) {
                if (puzzle[i][j] == 0) {
                    return size() - i;
                }
            }
        }
        return -1;
    }
}
