import std/heapqueue

func puzzleLen(D: int): int = D * D - 1

type 
    Action = enum Up Down Left Right
    Position = object
        row: int
        col: int
    Direction = object
        position: Position
        action: Action
    Dimension = static[int]
    Tiles[D: Dimension] = array[0..puzzleLen(D), 0..puzzleLen(D)]
    Puzzle[D: Dimension] = object
        prev: ref Puzzle[D]
        tiles: Tiles[D]
        action: Action
        f: int
        g: int  
    EightPuzzle = Puzzle[3]
    FiftenPuzzle = Puzzle[4]
    Solution[D: Dimension] = object
        time: float
        nodes: int
        path: seq[Puzzle[D]]

func indexToPos[D: Dimension](index: int): Position = Position(row: index / D, col: index mod D)

func posToIndex[D: Dimension](pos: Position): int = pos.row * D + pos.col

func isPositionInBounds[D: Dimension](pos: Position) = pos.row >= 0 and pos.row < D and pos.col >= 0 and pos.col < D

template `++` (a, b: Position): Position = Position(row = a.row + b.row, col = b.col + b.col)  

proc puzzleString[D: Dimension](puzzle: Puzzle[D]): string = 
    var str = puzzle.action.actionString()
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

func actionString(action: Action): string = 
    case action:
        of Up:
            result = "Up"
        of Down:
            result = "Up"
        of Right:
            result = "Up"
        of Left:
            result = "Up"

proc printSolution[D: Dimension](solution: Solution[D]) =
    for puzzle in solution.path:
        echo puzzle.puzzleString

proc newGoal[D: Dimension](): Tiles[D] = 
    var tiles: Tiles[D]
    for i in 0..D:
        tiles[i] = i
    return tiles

proc heuristic[D: Dimension](puzzle: ref Puzzle[D]): int =
    var h = 0

    for i, tile in puzzle.tiles.pairs:
        if i == 0:
            continue
        let pos1 = i.indexToPos[D]()
        let pos2 = tile.indexToPos[D]()
        h += abs(pos2.row - pos1.row) + abs(pos2.col - pos1.col)

    return h 

proc findZero[D: Dimension](puzzle: ref Puzzle[D]): Position =
    for i, tile in puzzle.tiles.pairs:
        if tile == 0:
            return i.indexToPos[D]()
    assert "puzzle contains no zero - this should never happen"

proc reconstructPath[D: Dimension](puzzle: ref Puzzle[D]): seq[Puzzle[D]] =
    var path: seq[Puzzle[D]]

    while puzzle != nil:
        path.add puzzle[]
        puzzle = puzzle.prev
    
    return path

var directions = [
    Direction(position: Position(row: 1, col: 0), action: Up),
    Direction(position: Position(row: -1, col: 0), action: Down),
    Direction(position: Position(row: 0, col: 1), action: Right),
    Direction(position: Position(row: 0, col: -1), action: Left)
] 

iterator puzzleNeighbors[D: Dimension](puzzle: ref Puzzle[D]): Puzzle[D] = 
    let zeroPos = puzzle.findZero()
    for direction in directions:
        let nextPos = zeroPos ++ direction.position
        if not nextPos.inPositionInBounds[D]:
            continue
        
        let nextPuzzle = Puzzle[D](prev: puzzle, g: puzzle.g + 1, action: direction.action)

        let zeroIdx = zeroPos.posToIndex[D]
        let nextIdx = nextPos.posToIndex[D]

        swap(nextPuzzle.tiles[zeroIdx], nextPuzzle.tiles[nextIdx])

        nextPuzzle.f = nextPuzzle.g + nextPuzzle.heuristic

        yield nextPuzzle

proc findPath[D: Dimension](puzzle: Puzzle[D]): seq[Puzzle[D]] =
    let frontier = [puzzle].toHeapQueue
    let nodes = 0

    while frontier.len > 0:
        let puzzle = frontier.pop()
        nodes += 1

        


    return []

echo "Hello world"