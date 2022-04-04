/*
 * Implements AStar algorithm to solve the puzzle
 * 4/16/20
 */
package src;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.PriorityQueue;

/**
 *
 * @author Joseph
 */
public class PuzzleSolver
{
    /**
     * Wrapper function for calling the AStar algorithm with raw arrays
     * @param initial, the initial puzzle
     * @param goal, the goal puzzle
     * @return the solution as a list of PuzzleNodes
     */
    public ArrayList<PuzzleState> findSolution(int[][] initial, int[][] goal) {
        return findSolution(new PuzzleState(initial), new PuzzleState(goal));
    }

    /**
     * Implementation of AStar algorithm to solve puzzle
     * @param initialState, the initial puzzle
     * @param goalState, the goal puzzle
     * @return the solution as a list of PuzzleNodes
     */
    private ArrayList<PuzzleState> findSolution(PuzzleState initialState, PuzzleState goalState) {
        // creates the closed set to stores nodes we've already expanded
        HashSet<PuzzleState> closedSet = new HashSet<>(1000);
        closedSet.add(initialState);

        // creates the open set to choose the closest state to the solution at each iteration
        PriorityQueue<PuzzleState> openSet = new PriorityQueue<>(1000, new FScoreComparator());
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
            ArrayList<PuzzleState> neighborStates = calculateNeighborStates(currentState);

            // for each neighbor, if it isn't already visited update the FScore for ranking and add it to the open set
            for (PuzzleState neighborState : neighborStates) {
                if (!closedSet.contains(neighborState)) {
                    neighborState.updateFScore(goalState.getPuzzle());
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
    public ArrayList<PuzzleState> calculateNeighborStates(PuzzleState current) {
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
    private ArrayList<PuzzleState> reconstructPath(PuzzleState bottomLeaf) {
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
}
