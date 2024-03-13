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
    private final Puzzle goalState;
    private int nodes = 0;

    public PuzzleSolver(int boardSize) {
        var num = 0;
        var goalPuzzle = new int[boardSize * boardSize];
        for (var i = 0; i < goalPuzzle.length; i++) {
            goalPuzzle[i] = num;
            num++;
        }
        this.goalState = new Puzzle(goalPuzzle);
    }

    public int getNodes() {
        return nodes;
    }

    public int heuristic(Puzzle puzzleState) {
        var puzzle = puzzleState.getPuzzle();
        var dimension = puzzleState.getDimension();
        var h = 0;
        for (var i = 0; i < puzzle.length; i++) {
            var row1 = i / dimension;
            var col1 = i % dimension;
            var row2 = puzzle[i] / dimension;
            var col2 = puzzle[i] % dimension;
            h += manhattanDistance(row1, col1, row2, col2);
        }
        return h;
    }

    public static int manhattanDistance(int row1, int col1, int row2, int col2) {
        return Math.abs(row2 - row1) + Math.abs(col2 - col1);
    }

    public void debug(PriorityQueue<Puzzle> frontier) {
        var clonedFrontier = frontier.toArray(new Puzzle[0]);
        Arrays.sort(clonedFrontier, Comparator.comparingInt(Puzzle::getFScore));
        for (var puzzle : clonedFrontier) {
            System.out.print(puzzle.getFScore() + " ");
        }
        System.out.println();
    }

    public List<Puzzle> findSolution(Puzzle initialState) {
        var visited = new HashSet<String>();
        var frontier = new PriorityQueue<>(Comparator.comparingInt(Puzzle::getFScore));
        frontier.add(initialState);

        nodes = 0;
        while(!frontier.isEmpty()) {
            var currentState = frontier.poll();
            visited.add(currentState.toString());
            nodes += 1;

            if(currentState.equals(goalState)) {
                return reconstructPath(currentState);
            }

            currentState.onNeighbors((neighbor) -> {
                if (!visited.contains(neighbor.toString())) {
                    var h = heuristic(neighbor);
                    neighbor.calcFScore(h);
                    frontier.add(neighbor);
                }
            });
        }
        return new ArrayList<>();
    }

    public List<Puzzle> reconstructPath(Puzzle currentState) {
        List<Puzzle> list = new ArrayList<>();
        while (currentState != null) {
            list.add(currentState);
            currentState = currentState.getParent();
        }
        Collections.reverse(list);
        return list;
    }

    private int randRange(int min, int max) {
        return (int) (Math.random() * (max + 1 - min)) + min;
    }

    public Puzzle generateRandomSolvable() {
        var moves = randRange(30, 50);

        var currentState = goalState;
        for (var i = 0; i < moves; i++) {
            var neighborStates = currentState.getNeighbors();
            var move = randRange(0, neighborStates.size() - 1);
            currentState = neighborStates.get(move);
        }

        currentState.unlink();

        return currentState;
    }
}
