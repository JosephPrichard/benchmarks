package main

import (
	"fmt"
	pq "github.com/emirpasic/gods/queues/priorityqueue"
	"github.com/emirpasic/gods/utils"
	"log"
	"os"
	"runtime"
	"slices"
	"strconv"
	"strings"
	"sync"
	"time"
)

type Action int

const (
	_ Action = iota
	Up
	Down
	Left
	Right
)

func (action Action) String() string {
	switch action {
	case Down:
		return "Down\n"
	case Up:
		return "Up\n"
	case Left:
		return "Left\n"
	case Right:
		return "Right\n"
	default:
		return "Start\n"
	}
}

type Position struct {
	row int
	col int
}

func IndexToPos(i int, dimension int) Position {
	return Position{row: i / dimension, col: i % dimension}
}

func (pos Position) ToIndex(dimension int) int {
	return pos.row*dimension + pos.col
}

func (pos Position) InBounds(dimension int) bool {
	return pos.row >= 0 && pos.row < dimension && pos.col >= 0 && pos.col < dimension
}

func (pos Position) Add(rhs Position) Position {
	return Position{row: pos.row + rhs.row, col: pos.col + rhs.col}
}

type Tile = uint8

type Puzzle struct {
	prev      *Puzzle
	tiles     []Tile
	g         int
	f         int
	action    Action
	dimension int
}

func NewGoal(len int) []Tile {
	tiles := make([]Tile, len)
	for i := range len {
		tiles[i] = byte(i)
	}
	return tiles
}

func (puzzle *Puzzle) PrintPuzzle() string {
	var sb strings.Builder
	for i, tile := range puzzle.tiles {
		if tile == 0 {
			sb.WriteString("  ")
		} else {
			fmt.Fprintf(&sb, "%d ", tile)
		}
		if (i+1)%puzzle.dimension == 0 {
			sb.WriteString("\n")
		}
	}
	return sb.String()
}

func abs(i int) int {
	if i < 0 {
		return -i
	}
	return i
}

func (puzzle *Puzzle) Heuristic() int {
	h := 0
	for i, tile := range puzzle.tiles {
		if tile == 0 {
			continue
		}
		pos1 := IndexToPos(i, puzzle.dimension)
		pos2 := IndexToPos(int(tile), puzzle.dimension)
		h += abs(pos2.row-pos1.row) + abs(pos2.col-pos1.col)
	}
	return h
}

func (puzzle *Puzzle) FindZero() Position {
	for i, tile := range puzzle.tiles {
		if tile == 0 {
			return IndexToPos(i, puzzle.dimension)
		}
	}
	panic("Puzzle contains no zero - this should never happen")
}

func HashTiles(tiles []Tile) uint64 {
	var hash uint64
	for i, tile := range tiles {
		mask := (uint64(tile)) << (i * 4)
		hash = hash | mask
	}
	return hash
}

func (puzzle *Puzzle) Hash() uint64 {
	return HashTiles(puzzle.tiles)
}

type Direction struct {
	pos    Position
	action Action
}

var directions = []Direction{
	{pos: Position{row: 0, col: 1}, action: Right},
	{pos: Position{row: 0, col: -1}, action: Left},
	{pos: Position{row: 1, col: 0}, action: Down},
	{pos: Position{row: -1, col: 0}, action: Up},
}

func (puzzle *Puzzle) OnNeighbors(onNeighbor func(puzzle Puzzle)) {
	zeroPos := puzzle.FindZero()
	for _, direction := range directions {
		nextPos := zeroPos.Add(direction.pos)
		if !nextPos.InBounds(puzzle.dimension) {
			continue
		}

		nextPuzzle := Puzzle{
			prev:      puzzle,
			tiles:     slices.Clone(puzzle.tiles),
			g:         puzzle.g + 1,
			dimension: puzzle.dimension,
			action:    direction.action,
		}

		zeroIdx := zeroPos.ToIndex(puzzle.dimension)
		nextIdx := nextPos.ToIndex(puzzle.dimension)

		temp := nextPuzzle.tiles[nextIdx]
		nextPuzzle.tiles[nextIdx] = nextPuzzle.tiles[zeroIdx]
		nextPuzzle.tiles[zeroIdx] = temp

		onNeighbor(nextPuzzle)
	}
}

func ReconstructPath(puzzle *Puzzle) []Puzzle {
	path := make([]Puzzle, 0)
	for puzzle != nil {
		path = append(path, *puzzle)
		puzzle = puzzle.prev
	}
	slices.Reverse(path)
	return path
}

func FindPath(initial Puzzle) ([]Puzzle, int) {
	visited := make(map[uint64]struct{})

	frontier := pq.NewWith(func(a, b interface{}) int {
		priorityA := a.(*Puzzle).f
		priorityB := b.(*Puzzle).f
		return utils.IntComparator(priorityA, priorityB)
	})

	frontier.Enqueue(&initial)

	goal := NewGoal(len(initial.tiles))
	goalHash := HashTiles(goal)

	nodes := 0
	for {
		top, ok := frontier.Dequeue()
		if !ok {
			break
		}
		puzzle := top.(*Puzzle)
		nodes += 1

		currHash := puzzle.Hash()
		visited[currHash] = struct{}{}

		if currHash == goalHash {
			return ReconstructPath(puzzle), nodes
		}

		puzzle.OnNeighbors(func(puzzle Puzzle) {
			_, isVisited := visited[puzzle.Hash()]
			if !isVisited {
				puzzle.f = puzzle.g + puzzle.Heuristic()
				frontier.Enqueue(&puzzle)
			}
		})
	}

	return make([]Puzzle, 0), nodes
}

func ReadPuzzles(path string) ([]Puzzle, error) {
	contents, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to read input file: %v\n", err)
	}

	var puzzles []Puzzle
	var current []Tile

	for line := range strings.SplitSeq(string(contents), "\n") {
		line := strings.TrimRight(line, "\n\r")
		tokens := strings.Split(line, " ")

		if len(tokens) > 1 {
			for _, token := range tokens {
				if token == "" {
					continue
				}
				tile, err := strconv.Atoi(token)
				if err != nil {
					return nil, fmt.Errorf("tile %s must be integers", token)
				}
				current = append(current, byte(tile))
			}
		} else {
			if len(current) == 0 {
				continue
			}

			var dimension int
			switch len(current) {
			case 9:
				dimension = 3
			case 16:
				dimension = 4
			default:
				return nil, fmt.Errorf("puzzle must have 9 or 16 tiles, got %d", len(current))
			}

			puzzles = append(puzzles, Puzzle{prev: nil, tiles: current, dimension: dimension})
			current = make([]Tile, 0)
		}
	}

	return puzzles, nil
}

type Solution struct {
	time  float64
	nodes int
	path  []Puzzle
}

func FindPaths(puzzles []Puzzle) []Solution {
	var solutions []Solution
	for _, puzzle := range puzzles {
		start := time.Now()

		solution, nodes := FindPath(puzzle)

		duration := time.Since(start).Microseconds()
		t := float64(duration) / 1000.0

		solutions = append(solutions, Solution{
			time:  t,
			nodes: nodes,
			path:  solution,
		})
	}
	return solutions
}

func FindPathsParallel(puzzles []Puzzle) []Solution {
	solutions := make([]Solution, len(puzzles))

	var wg sync.WaitGroup
	sem := make(chan struct{}, runtime.NumCPU())

	for i, puzzle := range puzzles {
		wg.Go(func() {
			sem <- struct{}{}

			start := time.Now()
			solution, nodes := FindPath(puzzle)

			duration := time.Since(start).Microseconds()
			t := float64(duration) / 1000.0

			solutions[i] = Solution{
				time:  t,
				nodes: nodes,
				path:  solution,
			}

			<-sem
		})
	}

	wg.Wait()
	return solutions
}

func main() {
	if len(os.Args) < 2 {
		log.Fatalf("Needs at least argument for input file")
	}

	inputFile := os.Args[1]

	flag := "seq"
	if len(os.Args) >= 3 {
		flag = os.Args[2]
	}

	puzzles, err := ReadPuzzles(inputFile)
	if err != nil {
		log.Fatalf("failed to read puzzles: %s", err)
	}

	start := time.Now()

	var solutions []Solution
	switch flag {
	case "seq":
		solutions = FindPaths(puzzles)
	case "par":
		solutions = FindPathsParallel(puzzles)
	default:
		log.Fatalf("Parallelism flag must be par or seq, got %s \n", flag)
	}

	duration := time.Since(start).Microseconds()
	eteTime := float64(duration) / 1000.0

	for i, s := range solutions {
		fmt.Printf("Solution for puzzle %d\n", i+1)
		for _, puzzle := range s.path {
			fmt.Print(puzzle.action.String())
		}

		fmt.Printf("Solved in %d steps\n\n", len(s.path)-1)
	}

	var totalTime float64
	var totalNodes int
	for i, s := range solutions {
		fmt.Printf("Puzzle %d: %f ms, %d nodes\n", i+1, s.time, s.nodes)
		totalTime += s.time
		totalNodes += s.nodes
	}
	fmt.Printf("\nTotal: %f ms, %d nodes\n", totalTime, totalNodes)

	fmt.Printf("End-to-end: %f ms\n", eteTime)
}
