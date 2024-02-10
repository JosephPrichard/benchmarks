type move = None | Left | Right | Up | Down
type tile = Empty | One | Two | Three | Four | Five | Six | Seven | Eight
type board = tile array
type puzzle = { 
  parent: puzzle option;
  tiles: board;
  gscore: int; 
  fscore: int;
  move: move;
}
type position = (int * int)
type direction = (position * string)

let size = 3 (* NxN size of the puzzle *)
let goal_tiles = [|Empty; One; Two; Three; Four; Five; Six; Seven; Eight;|]

let pos_of_index index = (index / size, index mod size)

let index_of_pos (row, col) = row * size + col

let str_of_pos (row, col) = Printf.sprintf "%d,%d" row col

let int_of_tile = function
  | Empty -> 0
  | One -> 1
  | Two -> 2
  | Three -> 3
  | Four -> 4
  | Five -> 5
  | Six -> 6
  | Seven -> 7
  | Eight -> 8

let tile_of_int int = match int with
  | 0 -> Empty
  | 1 -> One
  | 2 -> Two
  | 3 -> Three
  | 4 -> Four
  | 5 -> Five
  | 6 -> Six
  | 7 -> Seven
  | 8 -> Eight
  | _ -> failwith (Printf.sprintf "Invalid integer %d for tile" int)

let string_of_move = function
  | None -> "None"
  | Left -> "Left"
  | Right -> "Right"
  | Up -> "Up"
  | Down -> "Down"

let tile_of_string str =
  try tile_of_int (int_of_string str)
  with _ -> failwith (Printf.sprintf "Invalid str %s of tile" str)

let string_of_tile tile = string_of_int (int_of_tile tile)

let in_bounds pos = let (row, col) = pos in row >= 0 && row < size && col >= 0 && col < size

let in_bounds_direction direction = let (pos, _) = direction in in_bounds pos

(* Gets the tile at a position on the tiles, raises an exception if the position is invalid *)
let get_pos tiles pos =
  if in_bounds pos then
    tiles.(index_of_pos pos)
  else 
    raise (Invalid_argument 
      (Printf.sprintf "Cannot get index %s - is an invalid position" 
        (str_of_pos pos)))

(* New tiles with the position at tiles set to the tile - raises an exception if the position is invalid *)
let set_pos tiles pos new_tile =
  if in_bounds pos then
    let index = index_of_pos pos in
    let to_tile i tile =
      if i == index then new_tile
      else tile
    in
    Array.mapi to_tile tiles
  else
    raise (Invalid_argument 
      (Printf.sprintf "Cannot set position %s - is an invalid position" 
        (str_of_pos pos)))

(* Search for empty tile on the tiles and return the position - raise exception if the empty tile doesn't exist - should NOT happen*)  
let rec find_empty tiles i =
  if i < Array.length tiles then
    (if tiles.(i) == Empty then pos_of_index i
    else
      find_empty tiles (i+1))
  else
    raise (Invalid_argument "Puzzle doesn't have an empty tile")

(* Get the adjacent positions conforming to a pattern relative to a position on the matrix - only containing positions within the boundary *)
let adjacent_directions pos = 
  let (og_row, og_col) = pos in

  let do_move direction =
    let ((r, c), move) = direction in 
    ((og_row + r, og_col + c), move)
  in

  List.filter 
    in_bounds_direction
    (List.map do_move
      [((0, 1), Right); ((1, 0), Down); ((0, -1), Left); ((-1, 0), Up)])

(* Swap two tiles on the tiles for the given positions *)
let swap tiles pos_i pos_j =
  (set_pos 
    (set_pos tiles pos_i (get_pos tiles pos_j)) 
    pos_j 
    (get_pos tiles pos_i))

let manhattan_distance pos1 pos2 =
  let (row1, col1) = pos1 in
  let (row2, col2) = pos2 in
  abs (row2 - row1) + abs (col2 - col1)

(* Calculate the heuristic of the given tiles relative to a PREDEFINED GOAL STATE *)
let calc_heurstic tiles =
  let reduce_heuristic acc tile = 
    let (sum_h, i) = acc in
    (* Position of the tile on tiles *)
    let tile_pos = pos_of_index i in 
    (* Position of the tile on goal_tiles - same value as the tile itself *)
    let goal_pos = pos_of_index (int_of_tile tile) in
    let md = manhattan_distance tile_pos goal_pos in
    (sum_h + md, i + 1) 
  in
  let (h, _) = Array.fold_left reduce_heuristic (0, 0) tiles in h 

let tiles_of_str str = 
  let split = Str.split (Str.regexp "[ \n\r\x0c\t]+") str in
  Array.of_list (List.map tile_of_string split)

let tiles_of_chan ic =
  let rec str_of_ch ic = 
    try
      let line = input_line ic in
      line ^ "\n" ^ (str_of_ch ic)
    with
    | End_of_file -> ""
    | e ->
      close_in_noerr ic;
      raise e
  in
  let str = str_of_ch ic in
  tiles_of_str str

let rec int_of_tiles tiles i = 
  let exp = int_of_float (10. ** float_of_int i) in
  if i < Array.length tiles then
    int_of_tile tiles.(i) * exp + int_of_tiles tiles (i+1)
  else 
    0

(* Get the next puzzles for a given puzzle - each generated next puzzle will be linked to this puzzle as a child*)
let next_puzzles puzzle =
  let empty_pos = find_empty puzzle.tiles 0 in

  let rec next_puzzles directions acc = 
    match directions with 
    | direction :: directions ->
      let (next_pos, move) = direction in
      let tiles = swap puzzle.tiles empty_pos next_pos in

      let gscore = puzzle.gscore + 1 in
      let fscore = gscore + calc_heurstic tiles in
      let puzzle = {parent = Some puzzle; tiles = tiles; gscore = gscore; fscore = fscore; move = move} in

      next_puzzles directions (puzzle :: acc)
    | [] -> acc
  in  
  next_puzzles (adjacent_directions empty_pos) []

let print_tiles comb tiles = 
  let print_tile i tile =
    let term = if (i+1) mod 3 == 0 then comb else " " in
    Printf.printf "%s" (string_of_tile tile ^ term);
    () 
  in
  let () = Array.iteri print_tile tiles in
  Printf.printf "\n"

let print_puzzle_dbg comb puzzle  =
  Printf.printf "Puzzle\nG: %d, F: %d\n" puzzle.gscore puzzle.fscore;
  print_tiles comb puzzle.tiles 

let print_puzzle puzzle  =
  Printf.printf "%s\n" (string_of_move puzzle.move);
  print_tiles "\n" puzzle.tiles 

module OrderedPuzzles = Psq.Make(
  struct
    type t = int
    let compare num1 num2 = num1 - num2
  end
)(
  struct
    type t = puzzle
    let compare puzzle1 puzzle2 = puzzle1.fscore - puzzle2.fscore
  end
) 

module VisitedSet = Map.Make(struct
  type t = int
  let compare num1 num2 = num1 - num2
end)

let rec reconstruct_path puzzle path =
  let path = puzzle :: path in
  match puzzle.parent with 
  | Some parent -> reconstruct_path parent path
  | None -> path

let print_frontier frontier =
  Printf.printf "Frontier %d\n" (OrderedPuzzles.size frontier);
  OrderedPuzzles.iter (fun _ p -> print_puzzle_dbg " " p) frontier;
  Printf.printf "\n"

let print_visited visited =
  Printf.printf "Visited %d\n" (VisitedSet.cardinal visited);
  VisitedSet.iter (fun k _ -> Printf.printf "%d, " k;) visited;
  Printf.printf "\n"

let always _ = true

let rec add_neighbors puzzles frontier visited =
  (* Try to add the neighbor to frontier but only if it is not visited *)
  match puzzles with
  | puzzle :: puzzles ->
    let key = (int_of_tiles puzzle.tiles 0) in
    (* A neighbor can only be added into the frontier if it has not already been visited*)
    (match VisitedSet.find_opt key visited with
    | Some _ ->
      add_neighbors puzzles frontier visited
    | None ->
      add_neighbors puzzles (OrderedPuzzles.add key puzzle frontier) visited)
  | [] -> frontier

let rec search frontier visited bound =
  if bound <= 0 then [] else
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
      search frontier visited (bound-1)
  | None -> []
    
let solve tiles =
  let root = {parent = None; tiles = tiles; gscore = 0; fscore = 0; move = None} in
  let root_key = int_of_tiles root.tiles 0 in
  let frontier = OrderedPuzzles.add root_key root OrderedPuzzles.empty in
  let visited = VisitedSet.empty in
  search frontier visited max_int

let rec print_solution path = 
  match path with
  | puzzle :: path -> 
    print_puzzle puzzle;
    print_solution path
  | [] -> ()