/*
 * Implements AStar algorithm to solve the puzzle
 * 4/16/20
 */
package src;

import java.util.*;

/**
 *
 * @author Joseph Prichard
 */
public class PuzzleSolver
{
    private final PuzzleState goalState;
    private final int boardSize;

    public PuzzleState getGoalState() {
        return goalState;
    }

    public int getBoardSize() {
        return boardSize;
    }

    public PuzzleSolver(int boardSize, PuzzleState goalState) {
        this.boardSize = boardSize;
        this.goalState = goalState;
    }

    public PuzzleSolver(int boardSize) {
        this.boardSize = boardSize;

        // creates the goal state for the specified board size
        int num = 1;
        int[][] goalPuzzle = new int[boardSize][];
        for (int i = 0; i < boardSize; i++) {
            goalPuzzle[i] = new int[boardSize];
            for (int j = 0; j < boardSize; j++) {
                goalPuzzle[i][j] = num;
                num++;
            }
        }
        goalPuzzle[boardSize - 1][boardSize - 1] = 0;

        this.goalState = new PuzzleState(goalPuzzle);
    }

    /**
     * Calculates the heuristic for the AStar algorithm
     *
     * @param puzzleState, to calculate heuristic for relative to goal
     * @return the heuristic h value
     */
    public int heuristic(PuzzleState puzzleState) {
        int[][] puzzle = puzzleState.getPuzzle();
        int[][] goalPuzzle = goalState.getPuzzle();
        int h = 0;
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                for (int k = 0; k < boardSize; k++) {
                    for (int l = 0; l < boardSize; l++) {
                        if (goalPuzzle[k][l] == puzzle[i][j] && goalPuzzle[k][l] != 0) {
                            h += manhattanDistance(i, j, k, l);
                        }
                    }
                }
            }
        }
        return h;
    }

    public static int manhattanDistance(int row, int col1, int row2, int col2) {
        return Math.abs(row2 - row) + Math.abs(col2 - col1);
    }

    /**
     * Implementation of AStar algorithm to solve puzzle
     * @param initialState, the initial puzzle
     * @return the solution as a list of PuzzleNodes
     */
    public ArrayList<PuzzleState> findSolution(PuzzleState initialState) {
        // creates the closed set to stores nodes we've already expanded
        HashSet<PuzzleState> closedSet = new HashSet<>(100);
        closedSet.add(initialState);

        // creates the open set to choose the closest state to the solution at each iteration
        PriorityQueue<PuzzleState> openSet = new PriorityQueue<>(100, Comparator.comparingInt(PuzzleState::getFScore));
        openSet.add(initialState);

        // iterate until we find a solution or are out of puzzle states
        while(!openSet.isEmpty()) {
            // pop the closest state off the priority queue
            PuzzleState currentState = openSet.poll();
            closedSet.add(currentState);

            // check if we've reached the goal state, if so we can return the solution
            if(currentState.equals(goalState)) {
                return reconstructPath(currentState);
            }

            // expand our search by getting the neighbor states for the current state
            ArrayList<PuzzleState> neighborStates = neighborStates(currentState);

            // for each neighbor, if it isn't already visited update the FScore for ranking and add it to the open set
            for (PuzzleState neighborState : neighborStates) {
                if (!closedSet.contains(neighborState)) {
                    neighborState.setFScore(heuristic(neighborState));
                    openSet.add(neighborState);
                }
            }
        }
        return null;
    }

    /**
     * Calculates the next possible states from the current state
     * @param current, the current state
     * @return a list of states
     */
    public ArrayList<PuzzleState> neighborStates(PuzzleState current) {
        ArrayList<PuzzleState> neighborStates = new ArrayList<>();
        PuzzleState upPuzzle = current.shiftUp();
        if (upPuzzle != null) {
            neighborStates.add(upPuzzle);
        }

        PuzzleState downPuzzle = current.shiftDown();
        if (downPuzzle != null) {
            neighborStates.add(downPuzzle);
        }

        PuzzleState rightPuzzle = current.shiftRight();
        if (rightPuzzle != null) {
            neighborStates.add(rightPuzzle);
        }

        PuzzleState leftPuzzle = current.shiftLeft();
        if (leftPuzzle != null) {
            neighborStates.add(leftPuzzle);
        }
        return neighborStates;
    }

    /**
     * Reconstructs the shortest path from the bottomLeaf in the tree
     *
     * @param bottomLeaf, the leaf to climb up from
     * @return the shortest path
     */
    public ArrayList<PuzzleState> reconstructPath(PuzzleState bottomLeaf) {
        boolean atRoot = false;
        PuzzleState current = bottomLeaf;
        ArrayList<PuzzleState> list = new ArrayList<>();
        // traverse tree up through parents until we reach the root
        while (!atRoot) {
            list.add(current);
            if (current.getParent() != null) {
                current = current.getParent();
            } else {
                atRoot = true;
            }
        }
        // reverse the list since we traversed from goal state to initial
        Collections.reverse(list);
        return list;
    }

    /**
     * Generates a puzzle state guaranteed to be solvable
     *
     * @return solvable puzzle state
     */
    public PuzzleState generateRandomSolvable() {
        int moves = Utils.rand(15,20);

        PuzzleState currentState = goalState;
        for (int i = 0; i < moves; i++) {
            ArrayList<PuzzleState> neighborStates = neighborStates(currentState);
            int move = Utils.rand(0, neighborStates.size() - 1);
            currentState = neighborStates.get(move);
        }

        currentState.unlink();

        return currentState;
    }

    /**
     * Checks if a puzzle is solvable to goal state
     * THIS ONLY WORKS AGAINST THE DEFAULT GOAL STATE
     *
     * @param puzzleState to check if it is solvable
     * @return true or false
     */
    public boolean isSolvable(PuzzleState puzzleState) {
        int invCount = countInversions(puzzleState);
        if(puzzleState.getBoardSize() % 2 == 0) {
            // even board size
            int zeroStart = puzzleState.find0Position();
            return (zeroStart + invCount) % 2 != 0;
        }
        else {
            // odd board size
            return invCount % 2 == 0;
        }
    }

    /**
     * Calculates the number of inversions between a puzzle and the goal puzzle
     *
     * @param puzzleState, the initial state to count inversions for
     * @return the number of inversions
     */
    public int countInversions(PuzzleState puzzleState) {
        int[] puzzle = new int[boardSize * boardSize];

        // copy the 2D puzzle arrays to a laid out 1D array
        int counter = 0;
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                puzzle[counter] = puzzleState.getPuzzle()[i][j];
                counter++;
            }
        }
        // counts the number of inversions in the puzzle
        // an inversion is any pair i and j where i < j but i appears after j on the solved state
        int inversions = 0;
        for (int i = 0; i < puzzle.length; i++) {
            if (puzzle[i] == 0) {
                continue;
            }
            for (int j = i + 1; j < puzzle.length; j++) {
                if (puzzle[j] == 0) {
                    continue;
                }
                if (puzzle[i] > puzzle[j]) {
                    inversions++;
                }
            }
        }

        return inversions;
    }
}
