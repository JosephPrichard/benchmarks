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
    Position vector2;
    Action action;
};

struct Puzzle {
    std::array<Tile, 16> tiles{};
    Action action = NONE;
    int g = 0;
    int f = 0;
    Puzzle* prev = nullptr;

    // initializes a new puzzles from a dynamic array of tiles and does error checking
    explicit Puzzle(const std::vector<Tile>& tiles) {
        if (tiles.size() > 16) {
            printf("A puzzle must have at most 16 tiles, got %zu", tiles.size());
            exit(1);
        }
        for (int i = 0; i < tiles.size(); i++) {
            this->tiles[i] = tiles[i];
        }
    };

    // creates a new neighboring puzzle from another puzzle
    explicit Puzzle(const std::array<Tile, 16>& tiles, Puzzle* prev) : tiles(tiles) {
        this->prev = prev;
    };
};

 int heuristic(std::array<Tile, 16>& tiles, unsigned int n) {
    int h = 0;
    for (int i = 0; i < tiles.size(); i++) {
        Tile tile = tiles[i];
        if (tile != 0) {
            auto [row1, col1] = pos_of_index(tile, n);
            auto [row2, col2] = pos_of_index(i, n);
            h += abs(row2 - row1) + abs(col2 - col1);
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
    std::cout << "Puzzle should contain a zero" << std::endl;
    exit(1);
}

Puzzle get_goal(unsigned int n) {
    std::vector<Tile> tiles;
    for (int i = 0; i < n * n; i++) {
        tiles.push_back((Tile) i);
    }
    return Puzzle(tiles);
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

void reconstruct_path(std::vector<Puzzle>& path, Puzzle* curr) {
    while (curr != nullptr) {
        path.push_back(*curr);
        curr = curr->prev;
    }
    std::reverse(path.begin(), path.end());
}

const std::array<Direction, 4>& get_directions() {
    const static std::array<Direction, 4> DIRECTIONS = {
        Direction{Position{0, -1}, LEFT},
        Direction{Position{-1, 0}, UP},
        Direction{Position{1, 0}, DOWN},
        Direction{Position{0, 1}, RIGHT}
    };
    return DIRECTIONS;
}

struct Solution {
    float time = 0;
    int nodes = 0;
    std::vector<Puzzle> path;
};

unsigned int int_sqrt(unsigned int size) {
    float sqf = std::sqrt(static_cast<float>(size));
    float sq_flf = std::floor(sqf);
    if (sqf - sq_flf != 0) {
        return -1;
    }
    return static_cast<unsigned int>(sq_flf);
}

static std::unique_ptr<Solution> find_path(std::vector<Tile>& tiles) {
    Puzzle initial(tiles);

    unsigned int size = tiles.size();
    unsigned int n = int_sqrt(size);
    if (n <= 0) {
        printf("Size must be a perfect square, got %d", size);
        exit(1);
    }

    std::vector<std::unique_ptr<Puzzle>> puzzles;

    std::unordered_map<unsigned long long, bool> visited;

    auto cmp = [](Puzzle* lhs, Puzzle* rhs) { return lhs->f > rhs->f; };
    std::priority_queue<Puzzle*, std::vector<Puzzle*>, decltype(cmp)> frontier(cmp);
    frontier.push(&initial);

    const Puzzle& goal = get_goal(n);
    auto goal_hash = hash_tiles(goal.tiles, size);

    auto solution = std::make_unique<Solution>();

    while (!frontier.empty()) {
        Puzzle* curr = frontier.top();

        frontier.pop();
        solution->nodes += 1;

        unsigned long long curr_hash = hash_tiles(curr->tiles, size);
        visited[curr_hash] = true;

        if (goal_hash == curr_hash) {
            reconstruct_path(solution->path, curr);
            break;
        }

        Position zero_pos = find_zero(curr->tiles, n);

        for (auto& direction : get_directions()) {
            Position new_pos = zero_pos + direction.vector2;
            if (in_bounds(new_pos, n)) {
                std::array<Tile, 16> next_tiles = curr->tiles;

                auto new_index = pos_to_index(new_pos, n);
                auto zero_index = pos_to_index(zero_pos, n);
                auto temp = next_tiles[new_index];
                next_tiles[new_index] = next_tiles[zero_index];
                next_tiles[zero_index] = temp;

                unsigned long long next_hash = hash_tiles(next_tiles, size);
                
                if (visited.count(next_hash) <= 0) {
                    auto neighbor = std::make_unique<Puzzle>(next_tiles, curr);
                    neighbor->action = direction.action;
                    neighbor->g = curr->g + 1;
                    neighbor->f = neighbor->g + heuristic(neighbor->tiles, n);
                    
                    puzzles.emplace_back(std::move(neighbor));
                    frontier.push(puzzles.back().get());
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