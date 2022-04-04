/*
 * An object of this class represents each puzzle state (node) for the puzzle problem
 * A puzzle state encapsulates the internal puzzle, the previous state, the score rankings, etc.
 * 4/15/20
 */
package src;

import java.io.PrintStream;

/**
 *
 * @author Joseph
 */
public class PuzzleState
{
    public static final int BOARD_SIZE = 3;

    private PuzzleState parent = null;
    private final int[][] puzzle;
    private String action = "Root";

    private final int zeroRow;
    private final int zeroCol;

    private int f = 0; // ranking value
    private int g = 0; // path cost

    /**
     * @param puzzle, a N x N column of any length, assuming row and column length is the same
     */
    public PuzzleState(int[][] puzzle) {
        this.puzzle = puzzle;
        //calculate the position of 0 in current
        int rowIn = -1;
        int colIn = -1;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (puzzle[i][j] == 0) {
                    rowIn = i;
                    colIn = j;
                    break;
                }
            }
        }
        this.zeroRow = rowIn;
        this.zeroCol = colIn;
    }
    
    /**
     * @param puzzle, a N x N column of any length, assuming row and column length is the same
     * @param rowIn, the row 0 is on
     * @param colIn, the column 0 is on
     */
    private PuzzleState(int[][] puzzle, int rowIn, int colIn) {
        this.puzzle = puzzle;
        this.zeroRow = rowIn;
        this.zeroCol = colIn;
    }

    private void setParent(PuzzleState parent) {
        this.parent = parent;
    }

    public PuzzleState getParent() {
        return parent;
    }

    private void setFScore(int f) {
        this.f = f;
    }

    public int getFScore() {
        return f;
    }

    private void setGScore(int g) {
        this.g = g;
    }

    public int getGScore() {
        return g;
    }

    public int getZeroRow() {
        return zeroRow;
    }

    public int getZeroCol() {
        return zeroCol;
    }

    private void setAction(String action) {
        this.action = action;
    }
    
    public String getAction() {
        return action;
    }

    public int[][] getPuzzle() {
        return puzzle;
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

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
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
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
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
        childPuzzleState.setParent(this);
        childPuzzleState.setAction("Up");
        childPuzzleState.setGScore(g + 1);
        return childPuzzleState;
    }

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted down
     * Automatically updates the parent pointer, action, and g-score
     *
     * @return puzzle shifted down, or null if it cannot be shifted
     */
    public PuzzleState shiftDown() {
        if (zeroRow + 1 > BOARD_SIZE-1) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow + 1][zeroCol];
        clonedPuzzle[zeroRow + 1][zeroCol] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow + 1, zeroCol);
        childPuzzleState.setParent(this);
        childPuzzleState.setAction("Down");
        childPuzzleState.setGScore(g + 1);
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
        childPuzzleState.setParent(this);
        childPuzzleState.setAction("Left");
        childPuzzleState.setGScore(g + 1);
        return childPuzzleState;
    }

    /**
     * Expands the puzzle state search tree by getting a version of the puzzle shifted right
     * Automatically updates the parent pointer, action, and g-score
     * 
     * @return puzzle shifted right, or null if it cannot be shifted
     */
    public PuzzleState shiftRight() {
        if (zeroCol + 1 > BOARD_SIZE-1) {
            return null;
        }
        int[][] clonedPuzzle = Utils.cloneArray(puzzle);

        int temp = clonedPuzzle[zeroRow][zeroCol];
        clonedPuzzle[zeroRow][zeroCol] = clonedPuzzle[zeroRow][zeroCol + 1];
        clonedPuzzle[zeroRow][zeroCol + 1] = temp;

        PuzzleState childPuzzleState = new PuzzleState(clonedPuzzle, zeroRow, zeroCol + 1);
        childPuzzleState.setParent(this);
        childPuzzleState.setAction("Right");
        childPuzzleState.setGScore(g + 1);
        return childPuzzleState;
    }

    /**
     * Calculates and updates the f score of the puzzle state
     *
     * @param goalPuzzle, the goal puzzle used to calculate heuristic
     */
    public void updateFScore(int[][] goalPuzzle) {
        int h = 0;
        //calculate the manhattan distance for each different tile
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                for (int k = 0; k < BOARD_SIZE; k++) {
                    for (int l = 0; l < BOARD_SIZE; l++) {
                        if (goalPuzzle[k][l] == this.puzzle[i][j] && goalPuzzle[k][l] != 0) {
                            h = h + calcManhattanDistance(i, j, k, l);
                        }
                    }
                }
            }
        }
        this.f = g + h;
    }

    /**
     * Calculates the Manhattan distance between two tiles on the puzzle state
     *
     * @param x1 x value of point1
     * @param y1 y value of point1
     * @param x2 x value of point2
     * @param y2 y value of point2
     * @return Returns the Manhattan distance
     */
    public static int calcManhattanDistance(int x1, int y1, int x2, int y2) {
        return Math.abs(x2 - x1) + Math.abs(y2 - y1);
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
            for (int j = 0; j < BOARD_SIZE - 1; j++) {
                printStream.print(puzzleRow[j] + " ");
            }
            printStream.println(puzzleRow[BOARD_SIZE - 1]);
        }
        printStream.println();
    }

    /**
     * Checks if the there is one 0 in the state
     *
     * @param start, the puzzle to check
     * @return boolean true or false
     */
    public static boolean properZeros(int[][] start) {
        int counter = 0;
        for (int[] start1 : start) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (start1[j] == 0) {
                    counter++;
                }
            }
        }
        return counter == 1;
    }

    /**
     * Finds the position of the 0 from bottom
     * @param puzzle to check
     * @return the position of the 0
     */
    public static int find0Position(int[][] puzzle) {
        for (int i = BOARD_SIZE - 1; i >= 0; i--) {
            for (int j = BOARD_SIZE  - 1; j >= 0; j--) {
                if (puzzle[i][j] == 0)
                    return BOARD_SIZE - i; 
            }
        }
        return -1;
    } 
  
    
    /**
     * Checks if an initial goal pair is solvable
     *
     * @param startPuzzle, the initial puzzle
     * @param goalPuzzle, the goal puzzle
     * @return true or false
     */
    public static boolean isSolvable(int[][] startPuzzle, int[][] goalPuzzle) {
        int invCount = calculateInversionCount(startPuzzle, goalPuzzle);
        if(BOARD_SIZE % 2 == 0) {
            int zeroStart = find0Position(startPuzzle);
            if(zeroStart % 2 == 0 && !(invCount % 2 == 0)) {
                return true;
            }
            else return !(zeroStart % 2 == 0) && (invCount % 2 == 0);
        }
        else {
            return invCount % 2 == 0; 
        }
    }
    
    /**
     * Calculates the number of inversions for a board setting
     * Useful for checking if an initial goal pair is solvable
     *
     * @param startPuzzle, the initial puzzle
     * @param goalPuzzle, the goal puzzle
     * @return the number of inversions
     */
    public static int calculateInversionCount(int[][] startPuzzle, int[][] goalPuzzle) {
        int[] startPuzzleList = new int[BOARD_SIZE * BOARD_SIZE];
        int[] goalPuzzleList = new int[BOARD_SIZE * BOARD_SIZE];

        int counter = 0;

        // copy the 2D puzzle arrays to a laid out 1D array
        for (int[] startRow : startPuzzle) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                startPuzzleList[counter] = startRow[j];
                counter++;
            }
        }
        counter = 0;
        for (int[] goalRow : goalPuzzle) {
            for (int j = 0; j < goalPuzzle.length; j++) {
                goalPuzzleList[counter] = goalRow[j];
                counter++;
            }
        }

        // counts the number of inversions in the puzzle
        int inversions = 0;
        for(int i = 0; i < BOARD_SIZE - 1; i++) {
            for(int j = i+1; j < BOARD_SIZE; j++) {
               int startValue1 = startPuzzleList[i];
               int startValue2 = startPuzzleList[j];
               int goalPos1 = -1;
               int goalPos2 = -1;
               if(startValue1 != 0 && startValue2 != 0) {
                   for (int l = 0; l < BOARD_SIZE; l++) {
                       if(startValue1 == goalPuzzleList[l]) {
                           goalPos1 = l;
                       }
                       else if(startValue2 == goalPuzzleList[l]) {
                           goalPos2 = l;
                       }
                   }
                   if(!(goalPos2 > goalPos1)) {
                        inversions++;
                   }
               }
            }
        }
        
        return inversions;
    } 
}
