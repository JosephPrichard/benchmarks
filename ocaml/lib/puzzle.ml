type move =
  | None
  | Left
  | Right
  | Up
  | Down

type board = int array

type t =
  { parent : t option
  ; tiles : board
  ; gscore : int
  ; fscore : int
  ; move : move
  }

type position = int * int
type direction = position * string

(* NxN size of the puzzle *)
let pos_of_index index size : position = (index / size, index mod size)
let index_of_pos ((row, col) : position) size = (row * size) + col
let str_of_pos ((row, col) : position) = Printf.sprintf "%d,%d" row col

let size_of_tiles tiles =
  int_of_float (sqrt (float_of_int (Array.length tiles)))

let string_of_move = function
  | None -> "None"
  | Left -> "Left"
  | Right -> "Right"
  | Up -> "Up"
  | Down -> "Down"

let compare puzzle1 puzzle2 = puzzle1.fscore - puzzle2.fscore

let tile_of_string str =
  try int_of_string str with
  | _ -> failwith (Printf.sprintf "Invalid str %s of tile" str)

let string_of_tile tile =
  if tile = 0 then
    " "
  else
    string_of_int tile

let in_bounds (pos : position) size =
  let row, col = pos in
  row >= 0 && row < size && col >= 0 && col < size

let ( ++ ) (row1, col1) (row2, col2) : position = (row1 + row2, col1 + col2)

(* Gets the tile at a position on the tiles, raises an exception if the position is invalid *)
let ( .%() ) tiles pos =
  let size = size_of_tiles tiles in
  if in_bounds pos size then
    tiles.(index_of_pos pos size)
  else
    let m =
      Printf.sprintf
        "Cannot get index %s - is an invalid position"
        (str_of_pos pos)
    in
    raise (Invalid_argument m)

(* Search for empty tile on the tiles and return the position - raise exception if the empty tile doesn't exist - should NOT happen*)
let find_empty tiles size =
  let rec loop i =
    if i < Array.length tiles then
      if tiles.(i) == 0 then
        pos_of_index i size
      else
        loop (i + 1)
    else
      raise (Invalid_argument "Puzzle doesn't have an empty tile")
  in
  loop 0

(* Get the adjacent positions conforming to a pattern relative to a position on the matrix - only containing positions within the boundary *)
let adjacent_directions pos size =
  let directions =
    List.map
      (fun (dpos, move) -> (pos ++ dpos, move))
      [ ((0, 1), Right); ((1, 0), Down); ((0, -1), Left); ((-1, 0), Up) ]
  in
  List.filter (fun (pos, _) -> in_bounds pos size) directions

(* Swap two tiles on the tiles for the given positions *)
let swap tiles pos1 pos2 =
  let size = size_of_tiles tiles in
  Array.mapi
    (fun i tile ->
      let pos = pos_of_index i size in
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
  let size = size_of_tiles tiles in
  let reduce_heuristic (sum_h, i) tile =
    if tile <> 0 then
      let tile_pos = pos_of_index i size in
      let goal_pos = pos_of_index tile size in
      let dist = manhattan_distance tile_pos goal_pos in
      (sum_h + dist, i + 1)
    else
      (sum_h, i + 1)
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
  List.rev (List.filter (( <> ) [||]) tiles)

let create_goal len =
  let rec loop i acc =
    if i >= 0 then
      loop (i - 1) (i :: acc)
    else
      acc
  in
  Array.of_list (loop (len - 1) [])

let hash_of_tiles tiles =
  let rec loop i buf =
    if i < Array.length tiles then
      let str = string_of_int tiles.(i) in
      let _ = Buffer.add_string buf str in
      loop (i + 1) buf
    else
      Buffer.contents buf
  in
  loop 0 (Buffer.create 16)

(* Get the next puzzles for a given puzzle - each generated next puzzle will be linked to this puzzle as a child*)
let next_puzzles puzzle =
  let size = size_of_tiles puzzle.tiles in
  let empty_pos = find_empty puzzle.tiles size in
  List.fold_left
    (fun acc (next_pos, move) ->
      let tiles = swap puzzle.tiles empty_pos next_pos in
      let gscore = puzzle.gscore + 1 in
      let fscore = gscore + calc_heurstic tiles in
      let puzzle = { parent = Some puzzle; tiles; gscore; fscore; move } in
      puzzle :: acc)
    []
    (adjacent_directions empty_pos size)

let print_puzzle endl puzzle =
  Printf.printf "%s\n" (string_of_move puzzle.move);
  let size = size_of_tiles puzzle.tiles in
  Array.iteri
    (fun i tile ->
      let term = if (i + 1) mod size == 0 then endl else " " in
      Printf.printf "%s" (string_of_tile tile ^ term))
    puzzle.tiles
