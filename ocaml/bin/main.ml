open Puzzle_solver

let run_solutions tiles out_chan =
  List.mapi
    (fun i tiles ->
      let s = Unix.gettimeofday () *. 1000.0 in
      let solution, nodes = Solver.solve tiles in
      let f = Unix.gettimeofday () *. 1000.0 in

      Printf.printf "Solution for puzzle %d\n" (i + 1);
      List.iter (Puzzle.print_puzzle "\n") solution;

      Printf.printf
        "Solved in %d steps, exploring %d nodes\n\n"
        (List.length solution - 1)
        nodes;

      (match out_chan with
      | Some c -> output_string c (Printf.sprintf "%d steps\n" (List.length solution - 1))
      | None -> ());

      (nodes, f -. s))
    tiles

let total_solutions out_chan solutions = 
  List.fold_left 
    (fun acc x ->
      let (i, total_nodes, total_time) = acc in
      let (nodes, time) = x in

      (match out_chan with
      | Some c -> output_string c (Printf.sprintf "%d, %f\n" i time)
      | None -> ());

      (i + 1, total_nodes + nodes, total_time +. time))
    (1, 0, 0.0)
    solutions

let run in_chan outb_chan out_chan =
  let tiles = Puzzle.tiles_of_chan in_chan in

  Printf.printf "Running for %d puzzle input(s)...\n\n" (List.length tiles);

  let solutions = run_solutions tiles out_chan in

  List.iteri
    (fun i (_, time) -> Printf.printf "Puzzle %d took %f ms\n" (i + 1) time)
    solutions;

  let _, total_nodes, total_time = total_solutions outb_chan solutions in

  Printf.printf
    "Took %f ms in total, expanded %d nodes in total"
    total_time
    total_nodes;

  match outb_chan with
  | Some c -> output_string c (Printf.sprintf "total, %f" total_time)
  | None -> ()

let () =
  match Array.to_list Sys.argv with
  | [] | [ _ ] -> print_endline "Needs at least 1 program argument"
  | _ :: ifile :: obfile :: ofile :: _ ->
    run (open_in ifile) (Some (open_out obfile)) (Some (open_out ofile))
  | _ :: ifile :: obfile :: _ -> run (open_in ifile) (Some (open_out obfile)) None
  | _ :: ifile :: _ -> run (open_in ifile) None None
