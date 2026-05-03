import std/heapqueue
import std/tables
import std/strutils
import std/bitops
import std/times
import std/os
import std/strformat
import std/algorithm

func puzzleLen(D: int): int = D * D - 1

type 
    Action = enum Start Up Down Left Right
    Position = object
        row: int
        col: int
    Direction = object
        position: Position
        action: Action
    Dimension = static[int]
    Tiles[D: Dimension] = array[0..puzzleLen(D), 0..puzzleLen(D)]
    Puzzle[D: Dimension] {.byref.} = object
        prev: ref Puzzle[D]
        tiles: Tiles[D]
        action: Action
        f: int
        g: int  
    EightPuzzle = Puzzle[3]
    FifteenPuzzle = Puzzle[4]
    Solution[D: Dimension] = object
        time: float
        nodes: int
        path: seq[Puzzle[D]]
    PuzzleKind = enum EightPuzzleKind FifteenPuzzleKind
    AnyPuzzle = object
        case kind: PuzzleKind
        of EightPuzzleKind:
            eight: EightPuzzle
        of FifteenPuzzleKind:
            fifteen: FifteenPuzzle
    AnySolution = object
        duration: Duration
        nodes: int
        case kind: PuzzleKind
        of EightPuzzleKind:
            eightPath: seq[EightPuzzle]
        of FifteenPuzzleKind:
            fifteenPath: seq[FifteenPuzzle]
    
proc puzzleFromTiles[D: Dimension](tilesSeq: seq[int]): Puzzle[D] =
    var tilesArr: Tiles[D]
    for i, tile in tilesSeq.pairs:
        # throws `RangeDefect` if the tile is not in the tilesArr ordinal range
        tilesArr[i] = tile
    return Puzzle[D](tiles: tilesArr)

proc newPuzzle[D: Dimension](puzzle: Puzzle[D]): ref Puzzle[D] =
    var puzzleRef = new Puzzle[D]
    puzzleRef[] = puzzle
    return puzzleRef

func indexToPos[D: Dimension](index: int): Position = Position(row: int(index / D), col: index mod D)

func posToIndex[D: Dimension](pos: Position): int = pos.row * D + pos.col

func isPositionInBounds[D: Dimension](pos: Position): bool = pos.row >= 0 and pos.row < D and pos.col >= 0 and pos.col < D

proc add (a, b: Position): Position = Position(row: a.row + b.row, col: a.col + b.col)  

proc puzzleToString[D: Dimension](puzzle: Puzzle[D]): string = 
    var str = puzzle.action.actionToString()
    str.add "\n"

    for i, tile in puzzle.tiles.pairs:
        if tile == 0:
            str.add "  "
        else:
            str.add $tile
            str.add " "
        if (i+1) mod D == 0:
            str.add "\n"

    return str

proc puzzleSeqToString[D: Dimension](puzzles: seq[Puzzle[D]]): string = 
    var str = ""
    for puzzle in puzzles:
        str.add &"{puzzle}\n"
    return str

func actionToString(action: Action): string = 
    case action:
        of Start:
            result = "Start"
        of Up:
            result = "Up"
        of Down:
            result = "Down"
        of Right:
            result = "Right"
        of Left:
            result = "Left"

proc printPath[D: Dimension](path: seq[Puzzle[D]]): string = 
    var str = ""
    for i, puzzle in path.pairs:
        str.add (actionToString puzzle.action)
        if i != path.len()-1:
            str.add "\n"
    return str

proc printSolution(solution: AnySolution) =
    case solution.kind:
        of EightPuzzleKind:
            echo printPath(solution.eightPath)
        of FifteenPuzzleKind:
            echo printPath(solution.fifteenPath)

proc steps(solution: AnySolution): int = 
    case solution.kind:
        of EightPuzzleKind:
            return solution.eightPath.len()-1
        of FifteenPuzzleKind:
            return solution.fifteenPath.len()-1
                
proc newGoal[D: Dimension](): Tiles[D] = 
    var tiles: Tiles[D]
    for i in 0..puzzleLen(D):
        tiles[i] = i
    return tiles

proc hashTiles[D: Dimension](tiles: Tiles[D]): uint64 = 
    var hash: uint64

    for i, tile in tiles.pairs:
        let mask = uint64(tile) shl (i * 4)
        hash = bitor(hash, mask)

    return hash

proc hashPuzzle[D: Dimension](puzzle: Puzzle[D]): uint64 = 
    return hashTiles(puzzle.tiles)

proc heuristic[D: Dimension](puzzle: Puzzle[D]): int =
    var h = 0

    for i, tile in puzzle.tiles.pairs:
        if tile == 0:
            continue
        let pos1 = indexToPos[D](i)
        let pos2 = indexToPos[D](tile)
        h += abs(pos2.row - pos1.row) + abs(pos2.col - pos1.col)

    return h 

proc findZero[D: Dimension](puzzle: Puzzle[D]): Position =
    for i, tile in puzzle.tiles.pairs:
        if tile == 0:
            return indexToPos[D](i)
    raise newException(AssertionDefect, "A puzzle must contain at least one zero")

proc reconstructPath[D: Dimension](puzzle: ref Puzzle[D]): seq[Puzzle[D]] =
    var path: seq[Puzzle[D]]
    var currPuzzle = puzzle

    while not isNil currPuzzle:
        path.add currPuzzle[]
        currPuzzle = currPuzzle.prev
    
    path.reverse()
    return path

var directions = [
    Direction(position: Position(row: -1, col: 0), action: Up),
    Direction(position: Position(row: 1, col: 0), action: Down),
    Direction(position: Position(row: 0, col: 1), action: Right),
    Direction(position: Position(row: 0, col: -1), action: Left)
] 

iterator neighbors[D: Dimension](puzzle: ref Puzzle[D]): Puzzle[D] = 
    let zeroPos = (puzzle[]).findZero()
    for direction in directions:
        let nextPos = zeroPos.add direction.position
        if not isPositionInBounds[D](nextPos):
            continue
        
        var nextPuzzle = Puzzle[D](prev: puzzle, g: puzzle.g + 1, action: direction.action)
        nextPuzzle.tiles = puzzle.tiles

        let zeroIdx = posToIndex[D](zeroPos)
        let nextIdx = posToIndex[D](nextPos)

        swap(nextPuzzle.tiles[zeroIdx], nextPuzzle.tiles[nextIdx])

        yield nextPuzzle

proc `<` [D: Dimension](a, b: ref Puzzle[D]): bool = a.f < b.f

proc findPath[D: Dimension](inputPuzzle: Puzzle[D]): Solution[D] =
    var goal = newGoal[D]()
    let goalHash = hashTiles goal

    var visited = initTable[uint64, bool]()

    let rootPuzzle = newPuzzle inputPuzzle

    var frontier = initHeapQueue[ref Puzzle[D]]()
    frontier.push rootPuzzle

    var nodes = 0

    while frontier.len > 0:
        var items: seq[Puzzle[D]] = @[]
        for item in frontier.items():
            items.add item[]

        let currPuzzle = frontier.pop()
        nodes += 1
      
        let currHash = hashPuzzle currPuzzle[]
        visited[currHash] = true    

        if currHash == goalHash:
            return Solution[D](path: reconstructPath currPuzzle, nodes: nodes)

        for p in currPuzzle.neighbors():
            var puzzle = p
            let isVisited = visited.hasKey (hashPuzzle puzzle)
            if not isVisited:
                puzzle.f = puzzle.g + puzzle.heuristic()
                frontier.push (newPuzzle puzzle)

    return Solution[D](path: @[], nodes: nodes)

proc findPaths(puzzles: seq[AnyPuzzle]): seq[AnySolution] =
    var solutions: seq[AnySolution] = @[]

    for puzzle in puzzles:
        var anySolution: AnySolution

        let startTime = now()
        case puzzle.kind:
            of EightPuzzleKind:
                let solution = findPath puzzle.eight
                anySolution = AnySolution(kind: EightPuzzleKind, eightPath: solution.path, nodes: solution.nodes)
            of FifteenPuzzleKind:
                let solution = findPath puzzle.fifteen
                anySolution = AnySolution(kind: FifteenPuzzleKind, fifteenPath: solution.path, nodes: solution.nodes)

        anySolution.duration = now() - startTime

        solutions.add anySolution

    return solutions

proc readPuzzles(path: string): seq[AnyPuzzle] =
    let contents = readFile(path)

    var tiles: seq[int] = @[]
    var puzzles: seq[AnyPuzzle] = @[]

    for lineItr in contents.split("\n"):
        let line = lineItr.strip()
        let tokens = line.split(" ")
        if tokens.len() > 1:
            for token in tokens:
                if token == "":
                    continue
                tiles.add (parseInt token)
        else:
            var puzzle: AnyPuzzle
            case tiles.len():
                of 0:
                    continue
                of 9:
                    puzzle = AnyPuzzle(kind: EightPuzzleKind, eight: puzzleFromTiles[3](tiles))
                of 16:
                    puzzle = AnyPuzzle(kind: FifteenPuzzleKind, fifteen: puzzleFromTiles[4](tiles))
                else:
                    raise newException(ValueError, "A puzzle must be of length 9 or 16")
            puzzles.add puzzle
            tiles.setLen(0)

    return puzzles

func microseconds(duration: Duration): float = float(duration.inMicroseconds()) / 1000.0 

let params = commandLineParams()
if params.len() < 2:
    echo "Expected at least one argument for input file"
    quit(1)

let inputFilePath = params[1]

let inputPuzzles = readPuzzles(inputFilePath)

let startTime = now()

let solutions = findPaths(inputPuzzles)

let e2eTime = now() - startTime

for i, solution in solutions.pairs:
    echo &"Solution for puzzle {i+1}"
    printSolution(solution)
    echo &"Solved in {solution.steps()} steps"

var
    totalTime: Duration
    totalNodes: int

echo ""
for i, solution in solutions.pairs:
    echo &"Puzzle {i+1}: {microseconds(solution.duration)} ms, {solution.nodes} nodes"
    totalTime += solution.duration
    totalNodes += solution.nodes

echo &"\nTotal: {microseconds(totalTime)} ms, {totalNodes} nodes"
echo &"End-to-end: {microseconds(e2eTime)} ms"