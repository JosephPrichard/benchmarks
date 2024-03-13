open Puzzle_solver

(* Solving the 8puzzle *)
let run_solutions tiles =
  List.map
    (fun tiles ->
      let s = Unix.gettimeofday () *. 1000.0 in
      let solution, nodes = Solver.solve tiles in
      let f = Unix.gettimeofday () *. 1000.0 in
      (solution, nodes, f -. s))
    tiles

let () =
  if Array.length Sys.argv < 2 then
    print_endline "Needs path to file input as a program argument"
  else
    let file = Sys.argv.(1) in
    let tiles = Puzzle.tiles_of_chan (open_in file) in
    Printf.printf "Running for %d puzzle input(s)...\n\n" (List.length tiles);

    let solutions = run_solutions tiles in

    List.iteri
      (fun i (path, nodes, _) ->
        Printf.printf "Solution for puzzle %d\n" (i + 1);
        List.iter (Puzzle.print_puzzle "\n") path;
        Printf.printf "Solved in %d steps, exploring %d nodes\n\n" (List.length path - 1) nodes)
      solutions;

    List.iteri
      (fun i (_, _, time) -> Printf.printf "Puzzle %d took %f ms\n" (i + 1) time)
      solutions;

    let total_time =
      List.fold_left (fun acc (_, _, time) -> acc +. time) 0.0 solutions
    in
    Printf.printf "Took %f ms in total\n" total_time;
    ()
