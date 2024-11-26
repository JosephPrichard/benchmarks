module Puzzle = struct
  type move =
    | None
    | Left
    | Right
    | Up
    | Down

  type board =
    { tiles : int array
    ; size : int
    }

  type t =
    { parent : t option
    ; board : board
    ; gscore : int
    ; fscore : int
    ; move : move
    }

  type position = int * int
  type direction = position * string

  let pos_of_index ~size index : position = index / size, index mod size
  let index_of_pos ~size ((row, col) : position) = (row * size) + col
  let str_of_pos ((row, col) : position) = Printf.sprintf "%d,%d" row col
  let size_of_tiles tiles = tiles |> Array.length |> float_of_int |> sqrt |> int_of_float

  let string_of_move = function
    | None -> "None"
    | Left -> "Left"
    | Right -> "Right"
    | Up -> "Up"
    | Down -> "Down"
  ;;

  let compare puzzle1 puzzle2 = puzzle1.fscore - puzzle2.fscore

  let tile_of_string str =
    try int_of_string str with
    | _ -> failwith (Printf.sprintf "Invalid str %s of tile" str)
  ;;

  let string_of_tile tile = if tile = 0 then " " else string_of_int tile

  let in_bounds ((row, col) : position) size =
    row >= 0 && row < size && col >= 0 && col < size
  ;;

  let ( ++ ) ((row1, col1) : position) ((row2, col2) : position) : position =
    row1 + row2, col1 + col2
  ;;

  let ( .%() ) board pos =
    if in_bounds pos board.size
    then board.tiles.(index_of_pos ~size:board.size pos)
    else
      raise
        (Invalid_argument
           (Printf.sprintf
              "Cannot get index %s - is an invalid position"
              (str_of_pos pos)))
  ;;

  let find_empty board =
    let i =
      board.tiles
      |> Array.mapi (fun i tile -> i, tile)
      |> Array.find_opt (fun (_, tile) -> tile == 0)
      |> Option.get
      |> fst
    in
    pos_of_index ~size:board.size i
  ;;

  let swap board (pos1 : position) (pos2 : position) =
    let tiles =
      Array.mapi
        (fun i tile ->
          let pos = pos_of_index ~size:board.size i in
          if pos = pos1 then board.%(pos2) else if pos = pos2 then board.%(pos1) else tile)
        board.tiles
    in
    { tiles; size = board.size }
  ;;

  let manhattan_dist (row1, col1) (row2, col2) = abs (row2 - row1) + abs (col2 - col1)

  let heuristic board =
    let foldh (sum, i) tile =
      if tile == 0
      then sum, i + 1
      else (
        let dst =
          manhattan_dist
            (pos_of_index ~size:board.size i)
            (pos_of_index ~size:board.size tile)
        in
        sum + dst, i + 1)
    in
    fst (Array.fold_left foldh (0, 0) board.tiles)
  ;;

  let goal len =
    let rec aux i acc = if i >= 0 then aux (i - 1) (i :: acc) else acc in
    Array.of_list (aux (len - 1) [])
  ;;

  let hash_tiles tiles : int64 =
    fst
      (Array.fold_left
         (fun (hash, i) tile ->
           let tile = Int64.of_int tile in
           let mask = Int64.shift_left tile (i * 4) in
           Int64.logor hash mask, i + 1)
         (0L, 0)
         tiles)
  ;;

  let directions = [ (0, 1), Right; (1, 0), Down; (0, -1), Left; (-1, 0), Up ]

  let move_puzzle puzzle from_pos (to_pos, move) =
    let board = swap puzzle.board from_pos to_pos in
    let gscore = puzzle.gscore + 1 in
    let fscore = gscore + heuristic board in
    { parent = Some puzzle; board; gscore; fscore; move }
  ;;

  let adjacent puzzle =
    let empty_pos = find_empty puzzle.board in
    directions
    |> List.map (fun (direction, move) -> empty_pos ++ direction, move)
    |> List.filter (fun (pos, _) -> in_bounds pos puzzle.board.size)
    |> List.map (move_puzzle puzzle empty_pos)
  ;;

  let print_action puzzle = Printf.printf "%s\n" (string_of_move puzzle.move)

  let print_puzzle endl puzzle =
    print_action puzzle;
    Array.iteri
      (fun i tile ->
        let endl = if (i + 1) mod puzzle.board.size == 0 then endl else " " in
        Printf.printf "%s" (string_of_tile tile ^ endl))
      puzzle.board.tiles
  ;;

  let rec lines_of_chan ic =
    try
      let line = input_line ic in
      line :: lines_of_chan ic
    with
    | End_of_file -> []
    | e ->
      close_in_noerr ic;
      raise e
  ;;

  let tiles_of_str str =
    str
    |> Str.split (Str.regexp "[ \n\r\x0c\t]+")
    |> Array.of_list
    |> Array.map tile_of_string
  ;;

  let tiles_of_lines lines =
    List.fold_left
      (fun acc line ->
        let tiles = tiles_of_str line in
        match acc with
        | [] -> [ tiles ]
        | array :: acc ->
          if Array.length tiles = 0
          then [||] :: array :: acc
          else Array.concat [ array; tiles ] :: acc)
      []
      lines
  ;;

  let tiles_of_chan ic =
    lines_of_chan ic |> tiles_of_lines |> List.filter (( <> ) [||]) |> List.rev
  ;;
end

open Puzzle
module OrderedPuzzles = Psq.Make (Int64) (Puzzle)
module VisitedSet = Map.Make (Int64)

let rec reconstruct_path puzzle path =
  let path = puzzle :: path in
  match puzzle.parent with
  | Some parent -> reconstruct_path parent path
  | None -> path
;;

let rec add_neighbors puzzles frontier visited =
  match puzzles with
  | puzzle :: puzzles ->
    let key = Puzzle.hash_tiles puzzle.board.tiles in
    (match VisitedSet.find_opt key visited with
     | Some _ -> add_neighbors puzzles frontier visited
     | None -> add_neighbors puzzles (OrderedPuzzles.add key puzzle frontier) visited)
  | [] -> frontier
;;

let rec search frontier visited goal_tiles nodes =
  match OrderedPuzzles.min frontier with
  | Some (key, puzzle) ->
    let visited = VisitedSet.add (Puzzle.hash_tiles puzzle.board.tiles) () visited in
    if puzzle.board.tiles = goal_tiles
    then reconstruct_path puzzle [], nodes
    else (
      let frontier =
        add_neighbors
          (Puzzle.adjacent puzzle)
          (OrderedPuzzles.remove key frontier)
          visited
      in
      search frontier visited goal_tiles (nodes + 1))
  | None -> [], nodes
;;

let solve tiles =
  let initial =
    { parent = None
    ; board = { tiles; size = size_of_tiles tiles }
    ; gscore = 0
    ; fscore = 0
    ; move = None
    }
  in
  let frontier =
    OrderedPuzzles.add
      (Puzzle.hash_tiles initial.board.tiles)
      initial
      OrderedPuzzles.empty
  in
  let goal = Puzzle.goal (Array.length initial.board.tiles) in
  search frontier VisitedSet.empty goal 0
;;

let run_solutions tiles =
  List.mapi
    (fun i tiles ->
      let s = Unix.gettimeofday () *. 1000.0 in
      let solution, nodes = solve tiles in
      let f = Unix.gettimeofday () *. 1000.0 in
      Printf.printf "Solution for puzzle %d\n" (i + 1);
      List.iter Puzzle.print_action solution;
      Printf.printf "Solved in %d steps\n\n" (List.length solution - 1);
      f -. s, nodes)
    tiles
;;

let total_solutions solutions =
  List.fold_left
    (fun (i, total_time, total_nodes) (time, nodes) ->
      i + 1, total_time +. time, total_nodes + nodes)
    (1, 0.0, 0)
    solutions
;;

let () =
  match Array.to_list Sys.argv with
  | _ :: ifile :: _ ->
    let tiles = Puzzle.tiles_of_chan (open_in ifile) in
    let solutions = run_solutions tiles in
    List.iteri
      (fun i (time, nodes) ->
        Printf.printf "Puzzle %d: %f ms, %d nodes\n" (i + 1) time nodes)
      solutions;
    let _, total_time, total_nodes = total_solutions solutions in
    Printf.printf "\nTotal: %f ms, %d nodes" total_time total_nodes;
    Printf.printf "\nEnd-to-end: %f ms\n" total_time
  | _ -> print_endline "Needs at least 1 program argument"
;;
