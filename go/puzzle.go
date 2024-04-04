package main

import (
	"bufio"
	"container/heap"
	"fmt"
	"math"
	"os"
	"slices"
	"strconv"
	"strings"
	"time"
)

const (
	Up    = 1
	Down  = 2
	Left  = 3
	Right = 4
)

type Position struct {
	row int
	col int
}

func IndexToPos(i int, dim int) Position {
	return Position{row: i / dim, col: i % dim}
}

func (pos Position) ToIndex(dim int) int {
	return pos.row*dim + pos.col
}

func (pos Position) InBounds(dim int) bool {
	return pos.row >= 0 && pos.row < dim &&
		pos.col >= 0 && pos.col < dim
}

func (pos Position) Add(rhs Position) Position {
	return Position{row: pos.row + rhs.row, col: pos.col + rhs.col}
}

type Tile = byte

type Puzzle struct {
	prev   *Puzzle
	tiles  []Tile
	g      int
	f      int
	action int
	dim    int
}

func NewPuzzle(prev *Puzzle, tiles []Tile, dim int) Puzzle {
	return Puzzle{prev: prev, tiles: tiles, dim: dim}
}

func NewGoal(len int) []Tile {
	tiles := make([]Tile, len)
	for i := 0; i < len; i++ {
		tiles[i] = byte(i)
	}
	return tiles
}

func IntSqrt(x int) int {
	return int(math.Sqrt(float64(x)))
}

func (puzzle Puzzle) PrintPuzzle() string {
	var sb strings.Builder
	switch puzzle.action {
	case Down:
		sb.WriteString("Down\n")
	case Up:
		sb.WriteString("Up\n")
	case Left:
		sb.WriteString("Left\n")
	case Right:
		sb.WriteString("Right\n")
	default:
		sb.WriteString("Start\n")
	}
	dim := IntSqrt(len(puzzle.tiles))
	for i, tile := range puzzle.tiles {
		if tile == 0 {
			sb.WriteString("  ")
		} else {
			sb.WriteString(fmt.Sprintf("%d ", tile))
		}
		if (i+1)%dim == 0 {
			sb.WriteString("\n")
		}
	}
	return sb.String()
}

func Equals(tiles []Tile, other []Tile) bool {
	for i, tile := range tiles {
		if tile != other[i] {
			return false
		}
	}
	return true
}

func abs(i int) int {
	if i < 0 {
		return -i
	}
	return i
}

func (puzzle Puzzle) Heuristic() int {
	h := 0
	for i, tile := range puzzle.tiles {
		pos1 := IndexToPos(i, puzzle.dim)
		pos2 := IndexToPos(int(tile), puzzle.dim)
		if tile != 0 {
			h += abs(pos2.row-pos1.row) + abs(pos2.col-pos1.col)
		}
	}
	return h
}

func (puzzle Puzzle) FindZero() Position {
	for i, tile := range puzzle.tiles {
		if tile == 0 {
			return IndexToPos(i, puzzle.dim)
		}
	}
	panic("Puzzle contains no zero - this should never happen")
}

func (puzzle Puzzle) Hash() string {
	var sb strings.Builder
	for _, tile := range puzzle.tiles {
		sb.WriteByte(tile)
	}
	return sb.String()
}

type Direction struct {
	pos    Position
	action int
}

var directions = []Direction{
	{pos: Position{row: 0, col: 1}, action: Right},
	{pos: Position{row: 0, col: -1}, action: Left},
	{pos: Position{row: 1, col: 0}, action: Down},
	{pos: Position{row: -1, col: 0}, action: Up},
}

func (puzzle Puzzle) OnNeighbors(onNeighbor func(puzzle *Puzzle)) {
	zeroPos := puzzle.FindZero()
	for _, direction := range directions {
		newPos := zeroPos.Add(direction.pos)
		if !newPos.InBounds(puzzle.dim) {
			continue
		}

		nextPuzzle := NewPuzzle(&puzzle, slices.Clone(puzzle.tiles), puzzle.dim)

		temp := nextPuzzle.tiles[newPos.ToIndex(puzzle.dim)]
		nextPuzzle.tiles[newPos.ToIndex(puzzle.dim)] = nextPuzzle.tiles[zeroPos.ToIndex(puzzle.dim)]
		nextPuzzle.tiles[zeroPos.ToIndex(puzzle.dim)] = temp

		nextPuzzle.g = puzzle.g + 1
		nextPuzzle.f = nextPuzzle.g + nextPuzzle.Heuristic()
		nextPuzzle.action = direction.action

		onNeighbor(&nextPuzzle)
	}
}

type PuzzleHeap struct {
	array []*Puzzle
}

func (h PuzzleHeap) Len() int {
	return len(h.array)
}

func (h PuzzleHeap) Less(i, j int) bool {
	return h.array[i].f < h.array[j].f
}

func (h PuzzleHeap) Swap(i, j int) {
	h.array[i], h.array[j] = h.array[j], h.array[i]
}

func (h *PuzzleHeap) Push(x interface{}) {
	h.array = append(h.array, x.(*Puzzle))
}

func (h PuzzleHeap) lastIndex() int {
	return len(h.array) - 1
}

func (h *PuzzleHeap) Pop() interface{} {
	last := h.array[h.lastIndex()]
	h.array = h.array[:h.lastIndex()]
	return last
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
	visited := make(map[string]bool)

	frontier := PuzzleHeap{array: make([]*Puzzle, 0)}
	frontier.Push(&initial)
	heap.Init(&frontier)

	goal := NewGoal(len(initial.tiles))

	nodes := 0
	for frontier.Len() > 0 {
		puzzle := heap.Pop(&frontier).(*Puzzle)
		nodes += 1

		visited[puzzle.Hash()] = true

		if Equals(puzzle.tiles, goal) {
			return ReconstructPath(puzzle), nodes
		}

		puzzle.OnNeighbors(func(puzzle *Puzzle) {
			_, exists := visited[puzzle.Hash()]
			if !exists {
				heap.Push(&frontier, puzzle)
			}
		})
	}

	return make([]Puzzle, 0), nodes
}

func ReadPuzzles(path string) []Puzzle {
	contents, err := os.ReadFile(path)
	if err != nil {
		fmt.Printf("Failed to read input file %s\n", path)
		os.Exit(1)
	}

	var puzzles []Puzzle
	var current []Tile

	lines := strings.Split(string(contents), "\n")
	for _, line := range lines {
		line := strings.TrimRight(line, "\n\r")
		tokens := strings.Split(strings.TrimRight(line, "\n\r"), " ")

		if len(tokens) > 1 {
			for _, token := range tokens {
				if token == "" {
					continue
				}
				tile, err := strconv.Atoi(token)
				if err != nil {
					fmt.Printf("Tile %s must be integers", token)
					os.Exit(1)
				}
				current = append(current, byte(tile))
			}
		} else {
			if len(current) == 0 {
				continue
			}
			dim := IntSqrt(len(current))
			puzzles = append(puzzles, NewPuzzle(nil, current, dim))
			current = make([]Tile, 0)
		}
	}

	return puzzles
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Needs at least argument for input file")
		os.Exit(1)
	}
	filePath := os.Args[1]

	var outbWriter *bufio.Writer
	if len(os.Args) >= 3 {
		file, err := os.OpenFile(os.Args[2], os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
		if err != nil {
			fmt.Printf("Failed to open the bench file")
			os.Exit(1)
		}
		outbWriter = bufio.NewWriter(file)
		defer func() {
			outbWriter.Flush()
			file.Close()
		}()
	}

	var outWriter *bufio.Writer
	if len(os.Args) >= 4 {
		file, err := os.OpenFile(os.Args[3], os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
		if err != nil {
			fmt.Printf("Failed to open the out file")
			os.Exit(1)
		}
		outWriter = bufio.NewWriter(file)
		defer func() {
			outWriter.Flush()
			file.Close()
		}()
	}

	var totalNodes int
	var times []float64

	puzzles := ReadPuzzles(filePath)

	fmt.Printf("Running for %d puzzle input(s)...\n\n", len(puzzles))
	for i, puzzle := range puzzles {
		start := time.Now()
		solution, nodes := FindPath(puzzle)
		duration := time.Since(start).Microseconds()

		times = append(times, float64(duration)/1000.0)

		fmt.Printf("Solution for puzzle %d\n", i+1)
		for _, puzzle := range solution {
			fmt.Print(puzzle.PrintPuzzle())
		}

		outWriter.WriteString(fmt.Sprintf("%d steps\n", len(solution)-1))
		fmt.Printf("Solved in %d steps, expanded %d nodes\n\n", len(solution)-1, nodes)
		totalNodes += nodes
	}

	var totalTime float64
	for i, time := range times {
		fmt.Printf("Puzzle %d took %f ms\n", i+1, time)
		totalTime += time

		_, err := outbWriter.WriteString(fmt.Sprintf("%d, %f\n", i+1, time))
		if err != nil {
			fmt.Printf("Failed to write to output file")
			os.Exit(1)
		}
	}

	_, err := outbWriter.WriteString(fmt.Sprintf("total, %f", totalTime))
	if err != nil {
		fmt.Printf("Failed to write to output file")
		os.Exit(1)
	}
	fmt.Printf("Took %f ms in total, expanded %d nodes in total\n", totalTime, totalNodes)
}
