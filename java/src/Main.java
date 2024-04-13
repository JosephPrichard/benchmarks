package src;

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;

/**
 *
 * @author Joseph Prichard
 */
public class Main
{
    private static double nanosToMs(long nanos) {
        return ((double) nanos) / 1_000_000;
    }

    private static List<Solution> runSolvers(List<Puzzle> states) {
        List<Solution> solutions = new ArrayList<>();
        for (var initialState : states) {
            var startTime = System.nanoTime();

            var solver = new PuzzleSolver(initialState.length());
            var solution = solver.findSolution(initialState);

            var time = nanosToMs(System.nanoTime() - startTime);
            var nodes = solver.getNodes();

            solutions.add(new Solution(time, nodes, solution));
        }
        return solutions;
    }

    private static List<Solution> runSolversParallel(List<Puzzle> states) throws InterruptedException, ExecutionException {
        var tp = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        List<CompletableFuture<Solution>> futures = new ArrayList<>();

        for (Puzzle state : states) {
            var future = CompletableFuture.supplyAsync(() -> {
                var startTime = System.nanoTime();

                var solver = new PuzzleSolver(state.length());
                var solution = solver.findSolution(state);

                var time = nanosToMs(System.nanoTime() - startTime);
                var nodes = solver.getNodes();

                return new Solution(time, nodes, solution);
            }, tp);
            futures.add(future);
        }

        CompletableFuture.allOf(futures.toArray(new CompletableFuture[0])).get();

        List<Solution> solutions = new ArrayList<>();
        for (var future : futures) {
            solutions.add(future.get());
        }

        tp.shutdown();
        return solutions;
    }

    public static void main(String[] args) throws IOException, InterruptedException, ExecutionException {
        if (args.length < 1) {
            System.out.println("Need at least 1 program argument");
            System.exit(1);
        }

        var inputFile = new File(args[0]);
        if (!inputFile.exists()) {
            System.out.println("Failed to read input file " + args[0]);
            System.exit(1);
        }

        var flag = "seq";
        if (args.length >= 2) {
            flag = args[1];
        }

        var states = Puzzle.fromFile(inputFile);

        var startTime = System.nanoTime();

        var solutions = switch (flag) {
            case "seq" -> runSolvers(states);
            case "par" -> runSolversParallel(states);
            default -> {
                System.out.println("Flag must be seq or par, got " + flag);
                System.exit(1);
                yield null;
            }
        };

        var eteTime = nanosToMs(System.nanoTime() - startTime);
        
        for (var i = 0; i < solutions.size(); i++) {
            var solution = solutions.get(i);
            System.out.printf("Solution for puzzle %d\n", i + 1);
            for (var state : solution.path()) {
                System.out.println(state.getAction());
            }
            System.out.printf("Solved in %d steps\n\n", solution.path().size() - 1);
        }

        var totalTime = 0d;
        var totalNodes = 0;
        for (var i = 0; i < solutions.size(); i++) {
            var solution = solutions.get(i);

            var time = solution.time();
            var nodes = solution.nodes();

            System.out.printf("Puzzle %d: %f ms, %d nodes\n", i + 1, time, nodes);
            totalTime += time;
            totalNodes += nodes;
        }
        System.out.printf("Total: %f ms, %d nodes\n", totalTime, totalNodes);

        System.out.printf("End-to-end: %f ms\n", eteTime);
    }
}