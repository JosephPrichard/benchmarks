open Puzzle_solver
open Unix

let run_solutions tiles =
  List.mapi
    (fun i tiles ->
      let s = Unix.gettimeofday () *. 1000.0 in
      let solution, nodes = Solver.solve tiles in
      let f = Unix.gettimeofday () *. 1000.0 in

      Printf.printf "Solution for puzzle %d\n" (i + 1);
      List.iter Puzzle.print_action solution;

      Printf.printf "Solved in %d steps\n\n" (List.length solution - 1);
      (f -. s, nodes))
    tiles

let total_solutions solutions = 
  List.fold_left 
    (fun acc x ->
      let (i, total_time, total_nodes) = acc in
      let (time, nodes) = x in

      (i + 1, total_time +. time, total_nodes + nodes))
    (1, 0.0, 0)
    solutions

let () =
  match Array.to_list Sys.argv with
    | _ :: ifile :: _ -> 
      let tiles = Puzzle.tiles_of_chan (open_in ifile) in
    
        let solutions = run_solutions tiles in
    
        List.iteri
          (fun i ( time, nodes) -> Printf.printf "Puzzle %d: %f ms, %d nodes\n" (i + 1) time nodes)
          solutions;
    
        let _, total_time, total_nodes = total_solutions solutions in
    
        Printf.printf "\nTotal: %f ms, %d nodes" total_time total_nodes;
        Printf.printf "\nEnd-to-end: %f ms" total_time;
    | _ -> print_endline "Needs at least 1 program argument"
