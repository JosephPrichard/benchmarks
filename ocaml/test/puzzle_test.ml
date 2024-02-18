open Puzzle_solver
open Puzzle_solver.Solver

module PureSolver = Solver.Make (PureSearchState)

(* Tests for 8puzzle *)
let () =
  let tiles = Puzzle.tiles_of_str "12 13 2 3 4 5 6 7 8 9 10 11 0 1 14 15 16" in
  let hash = Puzzle.hash_of_tiles tiles in
  Printf.printf "Number %s\n" hash;

  let tiles = Puzzle.tiles_of_str "8 6 7 2 0 4 3 5 1" in
  let hash = Puzzle.hash_of_tiles tiles in
  Printf.printf "Number %s\n" hash;

  let state = PureSearchState.add_visited hash PureSearchState.empty in
  let has = PureSearchState.is_visited (Puzzle.hash_of_tiles tiles) state in
  if has then
    Printf.printf "Yes it is contained\n"
  else
    Printf.printf "No it is not contained\n";

  let tiles = Puzzle.tiles_of_str "8 6 7 2 5 4 3 0 1" in
  let h = Puzzle.calc_heurstic tiles in
  Printf.printf "Heuristic: %d\n\n" h;

  let row, col = Puzzle.find_empty tiles 3 in
  Printf.printf "Empty pos: %d,%d\n" row col;

  let neighbors =
    Puzzle.next_puzzles
      { parent = None; tiles; gscore = 0; fscore = 0; move = None }
  in
  let frontier =
    PureSolver.add_neighbors
      neighbors
      PureSearchState.empty
  in
  PureSearchState.debug_frontier frontier
