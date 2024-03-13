import copy
import heapq
import sys
import time
from enum import Enum
from math import sqrt, floor
from typing import Self
from typing import Callable

class Action(Enum):
    none = 1
    up = 2
    down = 3
    left = 4
    right = 5

    def __str__(self):
        match self:
            case self.up:
                return "Up"
            case self.down:
                return "Down"
            case self.left:
                return "Left"
            case self.right:
                return "Right"
            case _:
                return "None"


Position = tuple[int, int]


def index_of_pos(pos: Position, dim: int) -> int:
    return pos[0] * dim + pos[1]


def pos_of_index(i: int, dim: int) -> Position:
    return i // dim, i % dim


def in_bounds(pos: Position, dim: int) -> bool:
    return 0 <= pos[0] < dim and 0 <= pos[1] < dim


def add_positions(pos1: Position, pos2: Position) -> Position:
    return pos1[0] + pos2[0], pos1[1] + pos2[1]


Direction = tuple[Position, Action]
directions: list[Direction] = [((1, 0), Action.down), ((-1, 0), Action.up),
                               ((0, 1), Action.right), ((0, -1), Action.left)]


class Puzzle:
    def __init__(self, tiles):
        self.g = 0
        self.f = 0
        self.prev = None
        self.tiles = tiles
        self.action = Action.none
        self.dim = floor(sqrt(len(tiles)))

    def equals(self, other: list[int]) -> bool:
        for i in range(0, len(self.tiles)):
            if self.tiles[i] != other[i]:
                return False
        return True

    def heuristic(self) -> int:
        h = 0
        for i in range(0, len(self.tiles)):
            row1, col1 = pos_of_index(i, self.dim)
            row2, col2 = pos_of_index(self.tiles[i], self.dim)
            h += abs(row2 - row1) + abs(col2 - col1)
        return h

    def find_zero(self) -> Position:
        for i in range(0, len(self.tiles)):
            if self.tiles[i] == 0:
                return pos_of_index(i, self.dim)
        raise Exception("Puzzles should contain a 0 tile")

    def hash(self):
        s = ''
        for tile in self.tiles:
            s += str(tile)
        return s

    def print(self):
        print(self.action)
        for i in range(0, len(self.tiles)):
            if self.tiles[i] == 0:
                print("  ", end="")
            else:
                print(self.tiles[i], end=" ")
            if (i + 1) % self.dim == 0:
                print("\n", end="")

    def __lt__(self, other):
        return self.f < other.f

    def neighbors(self, callback: Callable[[Self], None]):
        zero_pos = self.find_zero()
        zero_index = index_of_pos(zero_pos, self.dim)

        for d in directions:
            next_pos = add_positions(zero_pos, d[0])
            next_index = index_of_pos(next_pos, self.dim)

            if not in_bounds(next_pos, self.dim):
                continue

            next_puzzle = Puzzle(copy.deepcopy(self.tiles))

            temp = next_puzzle.tiles[zero_index]
            next_puzzle.tiles[zero_index] = next_puzzle.tiles[next_index]
            next_puzzle.tiles[next_index] = temp

            next_puzzle.prev = self
            next_puzzle.action = d[1]
            next_puzzle.g = self.g + 1
            next_puzzle.f = next_puzzle.g + next_puzzle.heuristic()

            callback(next_puzzle)


def create_goal(size: int):
    tiles = []
    for i in range(0, size):
        tiles.append(i)
    return tiles


def reconstruct_path(curr: Puzzle):
    path = []
    while curr is not None:
        path.append(curr)
        curr = curr.prev
    path.reverse()
    return path


def find_path(initial: Puzzle):
    visited = {}
    frontier = [initial, initial, initial]

    goal = create_goal(len(initial.tiles))

    nodes = 0
    while len(frontier) > 0:
        curr_puzzle = heapq.heappop(frontier)
        nodes += 1

        if curr_puzzle.equals(goal):
            return reconstruct_path(curr_puzzle), nodes

        visited[curr_puzzle.hash()] = True

        def on_neighbor(neighbor: Puzzle):
            if not visited.get(neighbor.hash(), False):
                heapq.heappush(frontier, neighbor)

        curr_puzzle.neighbors(on_neighbor)

    return [], nodes

def read_puzzles(s: str) -> list[Puzzle]:
    puzzles = []
    curr_tiles = []

    lines = s.split("\n")
    for line in lines:
        tokens = line.split(" ")

        if len(tokens) > 1:
            for token in tokens:
                if token != "":
                    curr_tiles.append(int(token))
        else:
            if len(curr_tiles) > 0:
                puzzles.append(Puzzle(curr_tiles))
                curr_tiles = []
    return puzzles


def main():
    if len(sys.argv) >= 2:
        with open(sys.argv[1], "r") as file:
            file_contents = file.read()
            puzzles = read_puzzles(file_contents)

            times = []

            print(f"Starting for {len(puzzles)} puzzle(s)...\n")
            for puzzle in puzzles:
                start = time.perf_counter()

                solution, nodes = find_path(puzzle)

                end = time.perf_counter()
                times.append((end - start) * 1000.0)

                for p in solution:
                    p.print()
                print(f"Solved in {len(solution) - 1} steps, explored {nodes} nodes\n")

            total = 0
            for i in range(0, len(times)):
                print(f"Puzzle {i + 1} took {times[i]} ms")
                total += times[i]
            print(f"Took {total} ms in total")

    else:
        print("Needs at least one program argument as the input file")


main()
