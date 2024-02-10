(* Solving the 8puzzle *)
let () =
  let file = "8puzzle.txt" in
  let tiles = Puzzle.tiles_of_chan (open_in file) in

  let runs = 1000 in
  Printf.printf "Start\n";

  let s = Sys.time() *. 1000.0 in
  
  let rec run_solutions count =
    if count > 1 then
      let _ = Puzzle.solve tiles in
      run_solutions (count-1)
    else
      Puzzle.solve tiles
  in 
  let solution = run_solutions runs in

  let f = Sys.time() *. 1000.0 in

  Printf.printf "-Solution-\n\n";
  let () = Puzzle.print_solution solution in

  Printf.printf "Completed %d runs in %f ms\n" runs (f -. s);
  ()