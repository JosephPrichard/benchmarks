open Puzzle_solver

(* Tests for 8puzzle *)
let () =
  let tiles = Puzzle.tiles_of_str "8 6 7 2 0 4 3 5 1" in
  let number = Puzzle.int_of_tiles tiles 0 in
  Printf.printf "Number %d\n" number;

  let visited = Solver.VisitedSet.add number () Solver.VisitedSet.empty in
  let has = Solver.VisitedSet.find_opt (Puzzle.int_of_tiles tiles 0) visited in
  let _ =
    match has with
    | Some _ -> Printf.printf "Yes it is contained\n"
    | None -> Printf.printf "No it is not contained\n"
  in

  let tiles = Puzzle.tiles_of_str "8 6 7 2 5 4 3 0 1" in
  let h = Puzzle.calc_heurstic tiles in
  Printf.printf "Heuristic: %d\n\n" h;

  let row, col = Puzzle.find_empty tiles 0 in
  Printf.printf "Empty pos: %d,%d\n" row col;

  let neighbors =
    Puzzle.next_puzzles
      { parent = None; tiles; gscore = 0; fscore = 0; move = None }
  in
  let frontier =
    Solver.add_neighbors
      neighbors
      Solver.OrderedPuzzles.empty
      Solver.VisitedSet.empty
  in
  Solver.print_frontier frontier
