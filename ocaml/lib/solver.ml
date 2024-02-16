open Puzzle

let rec reconstruct_path puzzle path =
  let path = puzzle :: path in
  match puzzle.parent with
  | Some parent -> reconstruct_path parent path
  | None -> path

module OrderedPuzzles = Psq.Make (Int) (Puzzle)
module VisitedSet = Map.Make (Int)

let rec add_neighbors puzzles frontier visited =
  (* Try to add the neighbor to frontier but only if it is not visited *)
  match puzzles with
  | puzzle :: puzzles ->
    let key = int_of_tiles puzzle.tiles 0 in
    (* A neighbor can only be added into the frontier if it has not already been visited*)
    (match VisitedSet.find_opt key visited with
    | Some _ -> add_neighbors puzzles frontier visited
    | None ->
      add_neighbors puzzles (OrderedPuzzles.add key puzzle frontier) visited)
  | [] -> frontier

let rec search frontier visited goal_tiles =
  (* Get the BEST puzzle from the frontier - no puzzle means we end the search with no solution *)
  match OrderedPuzzles.min frontier with
  | Some (key, puzzle) ->
    let frontier = OrderedPuzzles.remove key frontier in
    (* A puzzle must be removed from the frontier and added to visited set - we don't want to search it again *)
    let visited = VisitedSet.add (int_of_tiles puzzle.tiles 0) () visited in
    (* Check if the puzzle matches the goal solution *)
    if puzzle.tiles = goal_tiles then
      reconstruct_path puzzle []
    else
      let neighbors = next_puzzles puzzle in
      let frontier = add_neighbors neighbors frontier visited in
      search frontier visited goal_tiles
  | None -> []

let create_goal len =
  let rec loop i acc =
    if i >= 0 then
      loop (i - 1) (i :: acc)
    else
      acc
  in
  Array.of_list (loop (len - 1) [])

let solve tiles =
  let initial = { parent = None; tiles; gscore = 0; fscore = 0; move = None } in
  search
    (OrderedPuzzles.add
       (int_of_tiles initial.tiles 0)
       initial
       OrderedPuzzles.empty)
    VisitedSet.empty
    (create_goal (Array.length initial.tiles))

let print_solution path =
  List.iter (print_puzzle "\n") path;
  Printf.printf "Solved in %d steps\n\n" (List.length path - 1)

let print_frontier frontier =
  Printf.printf "Frontier %d\n" (OrderedPuzzles.size frontier);
  OrderedPuzzles.iter (fun _ p -> Puzzle.print_puzzle " " p) frontier;
  Printf.printf "\n"
