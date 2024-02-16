open Puzzle_solver

(* Solving the 8puzzle *)
let run_solutions tiles =
  List.map
    (fun tiles ->
      let s = Unix.gettimeofday () *. 1000.0 in
      let solution = Solver.solve tiles in
      let f = Unix.gettimeofday () *. 1000.0 in
      (solution, f -. s))
    tiles

let () =
  let file = "8puzzles.txt" in
  let tiles = Puzzle.tiles_of_chan (open_in file) in
  Printf.printf "Running for %d puzzle input(s)...\n\n" (List.length tiles);

  let solutions = run_solutions tiles in

  List.iteri
    (fun i (solution, _) ->
      Printf.printf "Solution for puzzle %d\n" (i + 1);
      Solver.print_solution solution)
    solutions;

  List.iteri
    (fun i (_, time) -> Printf.printf "Puzzle %d took %f ms\n" (i + 1) time)
    solutions;

  let total_time =
    List.fold_left (fun acc (_, time) -> acc +. time) 0.0 solutions
  in
  Printf.printf "Took %f ms in total\n" total_time;
  ()
