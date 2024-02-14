type move =
  | None
  | Left
  | Right
  | Up
  | Down

type tile =
  | Empty
  | One
  | Two
  | Three
  | Four
  | Five
  | Six
  | Seven
  | Eight

type board = tile array

type t =
  { parent : t option
  ; tiles : board
  ; gscore : int
  ; fscore : int
  ; move : move
  }

type puzzle = t
type position = int * int
type direction = position * string

let size = 3 (* NxN size of the puzzle *)
let goal_tiles = [| Empty; One; Two; Three; Four; Five; Six; Seven; Eight |]
let pos_of_index index : position = (index / size, index mod size)
let index_of_pos ((row, col) : position) = (row * size) + col
let str_of_pos ((row, col) : position) = Printf.sprintf "%d,%d" row col

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

let tile_of_int int =
  match int with
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

let compare puzzle1 puzzle2 = puzzle1.fscore - puzzle2.fscore

let tile_of_string str =
  try tile_of_int (int_of_string str) with
  | _ -> failwith (Printf.sprintf "Invalid str %s of tile" str)

let string_of_tile tile =
  match tile with
  | Empty -> " "
  | _ -> string_of_int (int_of_tile tile)

let in_bounds (pos : position) =
  let row, col = pos in
  row >= 0 && row < size && col >= 0 && col < size

let ( ++ ) (row1, col1) (row2, col2) : position = (row1 + row2, col1 + col2)

(* Gets the tile at a position on the tiles, raises an exception if the position is invalid *)
let ( .%() ) tiles pos =
  if in_bounds pos then
    tiles.(index_of_pos pos)
  else
    let m =
      Printf.sprintf
        "Cannot get index %s - is an invalid position"
        (str_of_pos pos)
    in
    raise (Invalid_argument m)

(* Search for empty tile on the tiles and return the position - raise exception if the empty tile doesn't exist - should NOT happen*)
let find_empty tiles =
  let rec loop i = 
    if i < Array.length tiles then
      if tiles.(i) == Empty then
        pos_of_index i
      else
        loop (i + 1)
    else
      raise (Invalid_argument "Puzzle doesn't have an empty tile")
in
loop 0

(* Get the adjacent positions conforming to a pattern relative to a position on the matrix - only containing positions within the boundary *)
let adjacent_directions pos =
  let directions =
    List.map
      (fun (dpos, move) -> (pos ++ dpos, move))
      [ ((0, 1), Right); ((1, 0), Down); ((0, -1), Left); ((-1, 0), Up) ]
  in
  List.filter (fun (pos, _) -> in_bounds pos) directions

(* Swap two tiles on the tiles for the given positions *)
let swap tiles pos1 pos2 =
  Array.mapi
    (fun i tile ->
      let pos = pos_of_index i in
      if pos = pos1 then
        tiles.%(pos2)
      else if pos = pos2 then
        tiles.%(pos1)
      else
        tile)
    tiles

let manhattan_distance pos1 pos2 =
  let row1, col1 = pos1 in
  let row2, col2 = pos2 in
  abs (row2 - row1) + abs (col2 - col1)

(* Calculate the heuristic of the given tiles relative to a PREDEFINED GOAL STATE *)
let calc_heurstic tiles =
  let reduce_heuristic (sum_h, i) tile =
    (* Position of the tile on tiles *)
    let tile_pos = pos_of_index i in
    (* Position of the tile on goal_tiles - same value as the tile itself *)
    let goal_pos = pos_of_index (int_of_tile tile) in
    let md = manhattan_distance tile_pos goal_pos in
    (sum_h + md, i + 1)
  in
  fst (Array.fold_left reduce_heuristic (0, 0) tiles)

let rec lines_of_chan ic =
  try
    let line = input_line ic in
    line :: lines_of_chan ic
  with
  | End_of_file -> []
  | e ->
    close_in_noerr ic;
    raise e

let tiles_of_str str =
  Array.map
    tile_of_string
    (Array.of_list (Str.split (Str.regexp "[ \n\r\x0c\t]+") str))

let tiles_of_chan ic =
  let lines = lines_of_chan ic in
  let tiles = 
    List.fold_left
      (fun acc line ->
        let tiles = tiles_of_str line in
        match acc with
        | [] -> [ tiles ]
        | h :: acc ->
          if Array.length tiles = 0 then
            (* No tiles on a line means the next line is a new subarray *)
            [||] :: h :: acc
          else
            Array.concat [ h; tiles ] :: acc)
      []
      lines
  in
  List.filter (( <> ) [||]) tiles 

let rec int_of_tiles tiles i =
  let exp = int_of_float (10. ** float_of_int i) in
  if i < Array.length tiles then
    let num = int_of_tile tiles.(i) * exp in
    num + int_of_tiles tiles (i + 1)
  else
    0

(* Get the next puzzles for a given puzzle - each generated next puzzle will be linked to this puzzle as a child*)
let next_puzzles puzzle =
  let empty_pos = find_empty puzzle.tiles in
  List.fold_left
    (fun acc (next_pos, move) -> 
      let tiles = swap puzzle.tiles empty_pos next_pos in
      let gscore = puzzle.gscore + 1 in
      let fscore = gscore + calc_heurstic tiles in
      let puzzle = { parent = Some puzzle; tiles; gscore; fscore; move } in
      puzzle :: acc)
    []
    (adjacent_directions empty_pos)

let print_puzzle endl puzzle =
  Printf.printf "%s\n" (string_of_move puzzle.move);
  Array.iteri
    (fun i tile ->
      let term = if (i + 1) mod 3 == 0 then endl else " " in
      Printf.printf "%s" (string_of_tile tile ^ term))
    puzzle.tiles
