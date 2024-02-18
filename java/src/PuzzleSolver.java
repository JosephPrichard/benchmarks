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
        var num = 0;
        var goalPuzzle = new int[boardSize][];
        for (var i = 0; i < boardSize; i++) {
            goalPuzzle[i] = new int[boardSize];
            for (var j = 0; j < boardSize; j++) {
                goalPuzzle[i][j] = num;
                num++;
            }
        }

        this.goalState = new PuzzleState(goalPuzzle);
    }

    public int heuristic(PuzzleState puzzleState) {
        var puzzle = puzzleState.getPuzzle();
        var goalPuzzle = goalState.getPuzzle();
        var h = 0;
        for (var i = 0; i < boardSize; i++) {
            for (var j = 0; j < boardSize; j++) {
                for (var k = 0; k < boardSize; k++) {
                    for (var l = 0; l < boardSize; l++) {
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

    public List<PuzzleState> findSolution(PuzzleState initialState) {
        // creates the closed set to stores nodes we've already expanded
        var closedSet = new HashSet<PuzzleState>(100);
        closedSet.add(initialState);

        // creates the open set to choose the closest state to the solution at each iteration
        var openSet = new PriorityQueue<PuzzleState>(100, Comparator.comparingInt(PuzzleState::getFScore));
        openSet.add(initialState);

        // iterate until we find a solution or are out of puzzle states
        while(!openSet.isEmpty()) {
            // pop the closest state off the priority queue
            var currentState = openSet.poll();
            closedSet.add(currentState);

            // check if we've reached the goal state, if so we can return the solution
            if(currentState.equals(goalState)) {
                return reconstructPath(currentState);
            }

            // expand our search by getting the neighbor states for the current state
            var neighborStates = neighborStates(currentState);

            // for each neighbor, if it isn't already visited update the FScore for ranking and add it to the open set
            for (var neighborState : neighborStates) {
                if (!closedSet.contains(neighborState)) {
                    neighborState.setFScore(heuristic(neighborState));
                    openSet.add(neighborState);
                }
            }
        }
        return new ArrayList<>();
    }

    public List<PuzzleState> neighborStates(PuzzleState current) {
        List<PuzzleState> neighborStates = new ArrayList<>();
        var upPuzzle = current.shiftUp();
        if (upPuzzle != null) {
            neighborStates.add(upPuzzle);
        }

        var downPuzzle = current.shiftDown();
        if (downPuzzle != null) {
            neighborStates.add(downPuzzle);
        }

        var rightPuzzle = current.shiftRight();
        if (rightPuzzle != null) {
            neighborStates.add(rightPuzzle);
        }

        var leftPuzzle = current.shiftLeft();
        if (leftPuzzle != null) {
            neighborStates.add(leftPuzzle);
        }
        return neighborStates;
    }

    public List<PuzzleState> reconstructPath(PuzzleState bottomLeaf) {
        var atRoot = false;
        var current = bottomLeaf;
        List<PuzzleState> list = new ArrayList<>();
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

    public PuzzleState generateRandomSolvable() {
        var moves = Utils.rand(30,50);

        var currentState = goalState;
        for (var i = 0; i < moves; i++) {
            var neighborStates = neighborStates(currentState);
            var move = Utils.rand(0, neighborStates.size() - 1);
            currentState = neighborStates.get(move);
        }

        currentState.unlink();

        return currentState;
    }

    public boolean isSolvable(PuzzleState puzzleState) {
        var invCount = countInversions(puzzleState);
        if(puzzleState.size() % 2 == 0) {
            // even board size
            var zeroStart = puzzleState.find0Position();
            return (zeroStart + invCount) % 2 != 0;
        }
        else {
            // odd board size
            return invCount % 2 == 0;
        }
    }

    public int countInversions(PuzzleState puzzleState) {
        var puzzle = Utils.flattenArray(puzzleState.getPuzzle());

        var inversions = 0;
        for (var i = 0; i < puzzle.length; i++) {
            if (puzzle[i] == 0) {
                continue;
            }
            for (var j = i + 1; j < puzzle.length; j++) {
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
