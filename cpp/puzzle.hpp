//
// Created by Joseph on 2/25/2024.
//

#include <array>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <queue>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <memory_resource>

#ifndef CPP_PUZZLE_H
#define CPP_PUZZLE_H

typedef char Tile;

struct Position {
    int row;
    int col;
};

Position operator+(Position lhs, Position rhs) {
    return Position{lhs.row + rhs.row, lhs.col + rhs.col};
}

bool in_bounds(Position pos, unsigned int n) {
    return pos.row >= 0 && pos.row < n && pos.col >= 0 && pos.col < n;
}

Position pos_of_index(int index, unsigned int n) {
    return Position{index / (int) n, index % (int) n};
}

unsigned int pos_to_index(Position pos, unsigned int n) {
    return pos.row * n + pos.col;
}

enum Action {
    NONE, LEFT, DOWN, UP, RIGHT
};

struct Direction {
    Position vec2d;
    Action action;
};

int heuristic(std::array<Tile, 16>& tiles, unsigned int n) {
    int h = 0;
    for (int i = 0; i < tiles.size(); i++) {
        Tile tile = tiles[i];
        if (tile != 0) {
            auto pos1 = pos_of_index(tile, n);
            auto pos2 = pos_of_index(i, n);
            h += abs(pos2.row - pos1.row) + abs(pos2.col - pos1.col);
        }
    }
    return h;
}

Position find_zero(std::array<Tile, 16>& tiles, unsigned int n) {
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i] == 0) {
            return pos_of_index(i, n);
        }
    }
    printf("Puzzle should contain a zero\n");
    exit(1);
}

std::array<Tile, 16> get_goal_tiles(unsigned int n) {
    std::array<Tile, 16> tiles{};
    for (int i = 0; i < n * n; i++) {
        tiles[i] = (Tile) i;
    }
    return tiles;
}

unsigned long long hash_tiles(const std::array<Tile, 16>& tiles, unsigned int size) {
    unsigned long long hash = 0;
    for (int i = 0; i < size; i++) {
        Tile tile = tiles[i];
        unsigned long long mask = static_cast<unsigned long long>(tile) << (i * 4);
        hash = (hash | mask);
    }
    return hash;
}

struct Puzzle {
    std::array<Tile, 16> tiles{};
    Action action = NONE;
    int g = 0;
    int f = 0;
    Puzzle* prev = nullptr;
};

void reconstruct_path(std::vector<Puzzle>& path, Puzzle* curr) {
    while (curr != nullptr) {
        path.push_back(*curr);
        curr = curr->prev;
    }
    std::reverse(path.begin(), path.end());
}

const static std::array<Direction, 4> DIRECTIONS = {
    Direction{Position{0, -1}, LEFT},
    Direction{Position{-1, 0}, UP},
    Direction{Position{1, 0}, DOWN},
    Direction{Position{0, 1}, RIGHT}
};

struct PuzzleInput {
    std::array<Tile, 16> tiles;
    unsigned int n;
};

struct Solution {
    double time = 0;
    int nodes = 0;
    std::vector<Puzzle> path;
};

std::unique_ptr<Solution> find_path(PuzzleInput input) {
    unsigned int n = input.n;
    unsigned int size = input.n * input.n;
    
    std::pmr::unsynchronized_pool_resource upr;
    std::pmr::monotonic_buffer_resource mbr;
    std::pmr::polymorphic_allocator<Puzzle> pa{ &mbr };

    Puzzle initial;
    initial.tiles = input.tiles;

    std::pmr::unordered_map<unsigned long long, bool> visited{ &upr };

    auto cmp = [](Puzzle* lhs, Puzzle* rhs) { return lhs->f > rhs->f; };
    std::priority_queue<Puzzle*, std::pmr::vector<Puzzle*>, decltype(cmp)> 
        frontier{ cmp, std::pmr::polymorphic_allocator<Puzzle*>{ &upr } };
    frontier.push(&initial);

    auto goal = get_goal_tiles(n);
    auto goal_hash = hash_tiles(goal, size);

    auto solution = std::make_unique<Solution>();

    while (!frontier.empty()) {
        Puzzle* curr = frontier.top();

        frontier.pop();
        solution->nodes += 1;

        auto curr_hash = hash_tiles(curr->tiles, size);
        visited[curr_hash] = true;

        if (goal_hash == curr_hash) {
            reconstruct_path(solution->path, curr);
            break;
        }

        Position zero_pos = find_zero(curr->tiles, n);

        for (auto& direction : DIRECTIONS) {
            Position new_pos = zero_pos + direction.vec2d;
            
            if (in_bounds(new_pos, n)) {
                std::array<Tile, 16> next_tiles = curr->tiles;

                auto new_index = pos_to_index(new_pos, n);
                auto zero_index = pos_to_index(zero_pos, n);
                auto temp = next_tiles[new_index];
                next_tiles[new_index] = next_tiles[zero_index];
                next_tiles[zero_index] = temp;

                auto next_hash = hash_tiles(next_tiles, size);
                
                auto has_key = visited.count(next_hash) > 0;
                if (!has_key) {
                    auto* neighbor = new(pa.allocate(1)) Puzzle();

                    neighbor->tiles = next_tiles; // copy
                    neighbor->prev = curr;
                    neighbor->action = direction.action;
                    neighbor->g = curr->g + 1;
                    neighbor->f = neighbor->g + heuristic(neighbor->tiles, n);
                    
                    frontier.push(neighbor);
                }
            }
        }
    }

    return solution;
}

void print_action(Action a) {
    switch (a) {
        case LEFT:
            printf("Left\n");
            break;
        case RIGHT:
            printf("Right\n");
            break;
        case DOWN:
            printf("Down\n");
            break;
        case UP:
            printf("Up\n");
            break;
        default:
            printf("Start\n");
            break;
    }
}

void print_puzzle(const Puzzle& puzzle, int n) {
    for (int i = 0; i < puzzle.tiles.size(); i++) {
        auto tile = static_cast<unsigned char>(puzzle.tiles[i]);
        if (tile == 0) {
            printf(" ");
        } else {
            printf("%d", tile);
        }
        if ((i + 1) % n == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

#endif //CPP_PUZZLE_H