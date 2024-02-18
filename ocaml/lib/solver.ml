module type SearchState = sig 
  type t
  val empty : t
  val is_visited : string -> t -> bool
  val add_visited : string -> t -> t
  val add_frontier : string -> Puzzle.t -> t -> t
  val min_frontier : t -> (string * Puzzle.t) option
  val remove_frontier : string -> t -> t
end

module Make (State: SearchState) = struct 
  let rec reconstruct_path (puzzle : Puzzle.t) path =
    let path = puzzle :: path in
    match puzzle.parent with
    | Some parent -> reconstruct_path parent path
    | None -> path

  let rec add_neighbors (puzzles : Puzzle.t list) s =
    (* Try to add the neighbor to frontier but only if it is not visited *)
    match puzzles with
    | puzzle :: puzzles ->
      let key = Puzzle.hash_of_tiles puzzle.tiles in
      (* A neighbor can only be added into the frontier if it has not already been visited *)
      if State.is_visited key s then
        add_neighbors puzzles s
      else
        add_neighbors puzzles (State.add_frontier key puzzle s)
    | [] -> s
  
  let rec search s goal_tiles =
    (* Get the BEST puzzle from the frontier - no puzzle means we end the search with no solution *)
    match State.min_frontier s with
    | Some (key, puzzle) ->
      let s = State.remove_frontier key s in
      (* A puzzle must be removed from the frontier and added to visited set - we don't want to search it again *)
      let s = State.add_visited (Puzzle.hash_of_tiles puzzle.tiles) s in
      (* Check if the puzzle matches the goal solution *)
      if puzzle.tiles = goal_tiles then
        reconstruct_path puzzle []
      else
        let neighbors = Puzzle.next_puzzles puzzle in
        let s = add_neighbors neighbors s in
        search s goal_tiles
    | None -> []
  
  let solve tiles =
    let initial = Puzzle.{ parent = None; tiles; gscore = 0; fscore = 0; move = None } in
    search
      (State.add_frontier
         (Puzzle.hash_of_tiles initial.tiles)
         initial
         State.empty)
      (Puzzle.create_goal (Array.length initial.tiles))
end

let print_solution path =
  List.iter (Puzzle.print_puzzle "\n") path;
  Printf.printf "Solved in %d steps\n\n" (List.length path - 1)

(* Functionally pure implementation of the solver e using a map and a psq *)
module PureSearchState = struct 
  module OrderedPuzzles = Psq.Make (String) (Puzzle)
  module VisitedSet = Map.Make (String)

  type t = (OrderedPuzzles.t * unit VisitedSet.t)

  let empty = (OrderedPuzzles.empty, VisitedSet.empty)

  let is_visited key (_, visited) = Option.is_some (VisitedSet.find_opt key visited)

  let add_visited key (frontier, visited) = (frontier, VisitedSet.add key () visited)

  let add_frontier key puzzle (frontier, visited) = (OrderedPuzzles.add key puzzle frontier, visited)

  let min_frontier (frontier, _) = OrderedPuzzles.min frontier

  let remove_frontier key (frontier, visited) = (OrderedPuzzles.remove key frontier, visited)

  let debug_frontier (frontier, _) =
    Printf.printf "Frontier %d\n" (OrderedPuzzles.size frontier);
    OrderedPuzzles.iter (fun _ p -> Puzzle.print_puzzle " " p) frontier;
    Printf.printf "\n"
end