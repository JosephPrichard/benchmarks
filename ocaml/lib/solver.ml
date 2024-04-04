open Puzzle
module OrderedPuzzles = Psq.Make (String) (Puzzle)
module VisitedSet = Map.Make (String)

let rec reconstruct_path puzzle path =
  let path = puzzle :: path in
  match puzzle.parent with
  | Some parent -> reconstruct_path parent path
  | None -> path

let rec add_neighbors puzzles frontier visited =
  (* Try to add the neighbor to frontier but only if it is not visited *)
  match puzzles with
  | puzzle :: puzzles ->
    let key = Puzzle.hash_of_tiles puzzle.tiles in
    (* A neighbor can only be added into the frontier if it has not already been visited*)
    (match VisitedSet.find_opt key visited with
    | Some _ -> add_neighbors puzzles frontier visited
    | None ->
      add_neighbors puzzles (OrderedPuzzles.add key puzzle frontier) visited)
  | [] -> frontier

let rec search frontier visited goal_tiles nodes =
  (* Get the BEST puzzle from the frontier - no puzzle means we end the search with no solution *)
  match OrderedPuzzles.min frontier with
  | Some (key, puzzle) ->
    let frontier = OrderedPuzzles.remove key frontier in
    (* A puzzle must be removed from the frontier and added to visited set - we don't want to search it again *)
    let visited =
      VisitedSet.add (Puzzle.hash_of_tiles puzzle.tiles) () visited
    in
    (* Check if the puzzle matches the goal solution *)
    if puzzle.tiles = goal_tiles then
      (reconstruct_path puzzle [], nodes)
    else
      let neighbors = next_puzzles puzzle in
      let frontier = add_neighbors neighbors frontier visited in
      search frontier visited goal_tiles (nodes + 1)
  | None -> ([], nodes)

let solve tiles =
  let initial = { parent = None; tiles; gscore = 0; fscore = 0; move = None } in
  search
    (OrderedPuzzles.add
       (Puzzle.hash_of_tiles initial.tiles)
       initial
       OrderedPuzzles.empty)
    VisitedSet.empty
    (Puzzle.create_goal (Array.length initial.tiles))
    0
