/*
 * An object of this class represents each puzzle state (node) for the puzzle problem
 * A puzzle state encapsulates the internal puzzle, the previous state, the score rankings, etc.
 * 4/15/20
 */
package src;

import java.io.PrintStream;

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

    /**
     * @param puzzle, an N x N square matrix
     */
    public PuzzleState(int[][] puzzle) {
        this.puzzle = puzzle;
        //calculate the position of 0 in current
        int zeroRow = -1;
        int zeroCol = -1;
        for (int i = 0; i < getBoardSize(); i++) {
            for (int j = 0; j < getBoardSize(); j++) {
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
    
    /**
     * @param puzzle, an N x N square matrix
     * @param zeroRow, the row 0 is on
     * @param zeroCol, the column 0 is on
     */
    private PuzzleState(int[][] puzzle, int zeroRow, int zeroCol) {
        this.puzzle = puzzle;
        this.zeroRow = zeroRow;
        this.zeroCol = zeroCol;
    }

    public int getBoardSize() {
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

    /**
     * Checks whether this puzzle state is equal to another puzzle state (if the internal puzzles match)
     *
     * @param puzzleState, the puzzle state to check
     * @return True or false
     */
    @Override
    public boolean equals(Object puzzleState) {
        if (puzzleState.getClass() != PuzzleState.class) {
            return false;
        }

        int[][] puzzle1 = ((PuzzleState) puzzleState).getPuzzle();

        for (int i = 0; i < getBoardSize(); i++) {
            for (int j = 0; j < getBoardSize(); j++) {
                if (puzzle[i][j] != puzzle1[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Generates a unique hashcode for a puzzle state so it can be stored in a hash table
     *
     * @return the hash code for the puzzle state
     */
    @Override
    public int hashCode() {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i = 0; i < getBoardSize(); i++) {
            for (int j = 0; j < getBoardSize(); j++) {
                stringBuilder.append(puzzle[i][j]);
            }
        }
        return stringBuilder.toString().hashCode();
    }

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted up
     * Automatically updates the parent pointer, action, and g-score
     *
     * @return puzzle shifted up, or null if it cannot be shifted
     */
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

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted down
     * Automatically updates the parent pointer, action, and g-score
     *
     * @return puzzle shifted down, or null if it cannot be shifted
     */
    public PuzzleState shiftDown() {
        if (zeroRow + 1 > getBoardSize() -1) {
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

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted left
     * Automatically updates the parent pointer, action, and g-score
     *
     * @return puzzle shifted left, or null if it cannot be shifted
     */
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

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted right
     * Automatically updates the parent pointer, action, and g-score
     * 
     * @return puzzle shifted right, or null if it cannot be shifted
     */
    public PuzzleState shiftRight() {
        if (zeroCol + 1 > getBoardSize() -1) {
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

    /**
     * Prints the puzzleState to sys out
     */
    public void printPuzzle() {
        printPuzzle(System.out);
    }

    /**
     * Prints the puzzleState
     *
     * @param printStream the printStream to output to
     */
    public void printPuzzle(PrintStream printStream) {
        for (int[] puzzleRow : puzzle) {
            for (int j = 0; j < getBoardSize() - 1; j++) {
                printStream.print(puzzleRow[j] + " ");
            }
            printStream.println(puzzleRow[getBoardSize() - 1]);
        }
        printStream.println();
    }

    /**
     * Checks if the there is one 0 in the puzzle state
     *
     * @return boolean true or false
     */
    public boolean properZeros() {
        int counter = 0;
        for (int[] puzzleRow : puzzle) {
            for (int j = 0; j < getBoardSize(); j++) {
                if (puzzleRow[j] == 0) {
                    counter++;
                }
            }
        }
        return counter == 1;
    }

    /**
     * Finds the position of the 0 from bottom

     * @return the position of the 0, -1 if not found
     */
    public int find0Position() {
        for (int i = getBoardSize() - 1; i >= 0; i--) {
            for (int j = getBoardSize() - 1; j >= 0; j--) {
                if (puzzle[i][j] == 0)
                    return getBoardSize() - i;
            }
        }
        return -1;
    }
}
