(* Tests for 8puzzle *)
let () =
  let tiles = Puzzle.tiles_of_str "8 6 7 2 0 4 3 5 1" in
  let number = Puzzle.int_of_tiles tiles 0 in
  Printf.printf "Number %d\n" number;

  let visited = Puzzle.VisitedSet.add number () Puzzle.VisitedSet.empty in
  let has = Puzzle.VisitedSet.find_opt (Puzzle.int_of_tiles tiles 0) visited in
  let _ = match has with
  | Some _ -> Printf.printf "Yes it is contained\n"
  | None -> Printf.printf "No it is not contained\n"
  in

  let tiles = Puzzle.tiles_of_str "8 6 7 2 5 4 3 0 1" in
  let h = Puzzle.calc_heurstic tiles in
  Printf.printf "Heuristic: %d\n\n" h;

  let (row, col) = Puzzle.find_empty tiles 0 in
  Printf.printf "Empty pos: %d,%d\n" row col;

  let neighbors = Puzzle.next_puzzles {parent = None; tiles = tiles; gscore = 0; fscore = 0; move = None} in
  let frontier = Puzzle.add_neighbors neighbors Puzzle.OrderedPuzzles.empty Puzzle.VisitedSet.empty in

  Puzzle.print_frontier frontier in
  ()