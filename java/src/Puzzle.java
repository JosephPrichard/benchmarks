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
import java.util.function.Consumer;

/**
 *
 * @author Joseph Prichard
 */
public class Puzzle
{
    private static final int[][] DIRECTIONS = {{0, -1}, {0, 1}, {1, 0}, {-1, 0}};
    private static final String[] ACTIONS = {"Left", "Right", "Down", "Up"};

    private Puzzle parent = null;
    private final int[] puzzle;
    private String action = "Start";
    private int f = 0;
    private int g = 0;

    public Puzzle(int[] puzzle) {
        this.puzzle = puzzle;
    }

    public static Puzzle ofList(List<Integer> puzzleList) {
        var puzzle = puzzleList.stream().mapToInt(x -> x).toArray();
        var dimension = Math.sqrt(puzzle.length);
        if (dimension - Math.floor(dimension) != 0) {
            throw new InvalidPuzzleException("Matrices must be square");
        }
        return new Puzzle(puzzle);
    }

    public static List<Puzzle> fromFile(File file) throws FileNotFoundException {
        List<Puzzle> states = new ArrayList<>();
        List<Integer> currPuzzle = new ArrayList<>();

        var fileReader = new Scanner(file);
        while (fileReader.hasNext()) {
            var line = fileReader.nextLine();
            var tokens = line.split(" ");
            if (tokens.length == 0 || line.isEmpty()) {
                states.add(ofList(currPuzzle));
                currPuzzle = new ArrayList<>();
            } else {
                for (var token : tokens) {
                    if (!token.isEmpty()) {
                        currPuzzle.add(Integer.parseInt(token));
                    }
                }
            }
        }
        
        fileReader.close();

        states.add(ofList(currPuzzle));
        return states;
    }

    public int length() {
        return puzzle.length;
    }

    public int getDimension() {
        return (int) Math.sqrt(length());
    }

    public Puzzle getParent() {
        return parent;
    }

    public void calcFScore(int h) {
        this.f = g + h;
    }

    public int getFScore() {
        return f;
    }
    
    public String getAction() {
        return action;
    }

    public int[] getPuzzle() {
        return puzzle;
    }

    public void unlink() {
        action = "Start";
        parent = null;
    }

    public boolean equals(Puzzle other) {
        for (var i = 0; i < length(); i++) {
            if (puzzle[i] != other.getPuzzle()[i]) {
                return false;
            }
        }
        return true;
    }

    @Override
    public String toString() {
        var stringBuilder = new StringBuilder();
        for (var tile : puzzle) {
            stringBuilder.append(tile);
        }
        return stringBuilder.toString();
    }

    public static boolean inBounds(int row, int col, int dimension) {
        return row >= 0 && row < dimension && col >= 0 && col < dimension;
    }

    public void onNeighbors(Consumer<Puzzle> onNeighbor) {
        var dimension = getDimension();
        var zeroIndex = findZero();
        var zeroRow = zeroIndex / dimension;
        var zeroCol = zeroIndex % dimension;

        for (var i = 0; i < DIRECTIONS.length; i++) {
            var direction = DIRECTIONS[i];
            var nextRow = zeroRow + direction[0];
            var nextCol = zeroCol + direction[1];

            if (!inBounds(nextRow, nextCol, dimension)) {
                continue;
            }

            var nextPuzzle = new int[puzzle.length];
            System.arraycopy(puzzle, 0, nextPuzzle, 0, puzzle.length);

            var nextIndex = nextRow * dimension + nextCol;
            var temp = nextPuzzle[zeroIndex];
            nextPuzzle[zeroIndex] = nextPuzzle[nextIndex];
            nextPuzzle[nextIndex] = temp;

            var neighbor = new Puzzle(nextPuzzle);
            neighbor.parent = this;
            neighbor.action = ACTIONS[i];
            neighbor.g = g + 1;

            onNeighbor.accept(neighbor);
        }
    }

    public List<Puzzle> getNeighbors() {
        List<Puzzle> neighbors = new ArrayList<>();
        onNeighbors(neighbors::add);
        return neighbors;
    }

    public void printPuzzle() {
        printPuzzle(System.out, " ");
    }

    public void printPuzzle(PrintStream stream, String empty) {
        var dimension = getDimension();
        for (var i = 0; i < puzzle.length; i++) {
            var tile = puzzle[i];
            if (tile == 0) {
                stream.print(empty + " ");
            } else {
                stream.print(tile + " ");
            }
            if ((i + 1) % dimension == 0) {
                stream.println();
            }
        }
    }

    public int findZero() {
        for (var i = 0; i < puzzle.length; i++) {
            if (puzzle[i] == 0) {
               return i;
            }
        }
        throw new InvalidPuzzleException("Puzzle must have a 0 tile");
    }
}