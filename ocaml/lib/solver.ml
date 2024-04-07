open Puzzle
module OrderedPuzzles = Psq.Make (Int64) (Puzzle)
module VisitedSet = Map.Make (Int64)

let rec reconstruct_path puzzle path =
  let path = puzzle :: path in
  match puzzle.parent with
  | Some parent -> reconstruct_path parent path
  | None -> path

let rec add_neighbors puzzles frontier visited =
  match puzzles with
  | puzzle :: puzzles ->
    let key = Puzzle.hash_of_tiles puzzle.tiles in

    (match VisitedSet.find_opt key visited with
    | Some _ -> add_neighbors puzzles frontier visited
    | None ->
      add_neighbors puzzles (OrderedPuzzles.add key puzzle frontier) visited)

  | [] -> frontier

let rec search frontier visited goal_tiles nodes =
  match OrderedPuzzles.min frontier with
  | Some (key, puzzle) ->
    let frontier = OrderedPuzzles.remove key frontier in

    let visited =
      VisitedSet.add (Puzzle.hash_of_tiles puzzle.tiles) () visited
    in

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
