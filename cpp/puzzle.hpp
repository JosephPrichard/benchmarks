//
// Created by Joseph on 2/25/2024.
//

#include <array>
#include <vector>
#include <functional>
#include <string>
#include <queue>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include "position.hpp"

#ifndef CPP_PUZZLE_H
#define CPP_PUZZLE_H

typedef char Tile;

template<std::size_t N = 3>
struct Puzzle {
    std::array<Tile, N * N> tiles;
    Action action = NONE;
    int g = 0;
    int f = 0;
    Puzzle<N>* prev = nullptr;

    Puzzle() : tiles(std::array<Tile, N * N>{0}) {};

    explicit Puzzle(const std::array<Tile, N * N> tiles) : tiles(tiles) {};

    explicit Puzzle(const std::array<Tile, N * N> tiles, Puzzle<N>* prev) : tiles(tiles) {
        this->prev = prev;
        this->g = prev->g + 1;
        this->f = this->g + this->heuristic();
    };
   
    explicit Puzzle(std::vector<Tile>& tiles) {
        for (int i = 0; i < N * N; i++) {
            this->tiles[i] = tiles[i];
        }
    };

    int heuristic() const {
        int h = 0;
        for (int i = 0; i < N * N; i++) {
            Tile tile = this->tiles[i];
            if (tile != 0) {
                auto [row1, col1] = pos_of_index(tile, N);
                auto [row2, col2] = pos_of_index(i, N);
                h += abs(row2 - row1) + abs(col2 - col1);
            }
        }
        return h;
    }

    Position find_zero() const {
        for (int i = 0; i < N * N; i++) {
            if (this->tiles[i] == 0) {
                return pos_of_index(i, N);
            }
        }
        std::cout << "Puzzle should contain a zero" << std::endl;
        exit(1);
    }
};

template<std::size_t N = 3>
const Puzzle<N>& get_goal() {
    static std::array<Tile, N * N> tiles{};
    for (int i = 0; i < N * N; i++) {
        tiles[i] = i;
    }
    static const Puzzle<N> puzzle(tiles);
    return puzzle;
}

template<std::size_t N = 3>
unsigned long long hash_tiles(const std::array<Tile, N * N>& tiles) {
    unsigned long long hash = 0;
    for (int i = 0; i < N * N; i++) {
        Tile tile = tiles[i];
        unsigned long long mask = ((unsigned long long) tile) << (i * 4);
        hash = (hash | mask);
    }
    return hash;
}

template<std::size_t N = 3>
void swap_positions(std::array<Tile, N * N>& tiles, Position new_pos, Position zero_pos) {
    auto new_index = pos_to_index(new_pos, N);
    auto zero_index = pos_to_index(zero_pos, N);
    auto temp = tiles[new_index];
    tiles[new_index] = tiles[zero_index];
    tiles[zero_index] = temp;
}

template<std::size_t N = 3>
using Path = std::tuple<std::vector<Puzzle<N>>, int>;

template<std::size_t N = 3>
std::vector<Puzzle<N>> reconstruct_path(Puzzle<N>* curr) {
    std::vector<Puzzle<N>> path;
    while (curr != nullptr) {
        path.push_back(*curr);
        curr = curr->prev;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

template<std::size_t N = 3>
static Path<N> find_path(Puzzle<N>& initial) {
    std::vector<Puzzle<N>*> puzzles;

    std::unordered_map<unsigned long long, bool> visited;

    auto cmp = [](Puzzle<N>* lhs, Puzzle<N>* rhs) { return lhs->f > rhs->f; };
    std::priority_queue<Puzzle<N>*, std::vector<Puzzle<N>*>, decltype(cmp)> frontier(cmp);
    frontier.push(&initial);

    const Puzzle<N>& goal = get_goal<N>();
    auto goal_hash = hash_tiles<N>(goal.tiles);

    int nodes = 0;
    while (!frontier.empty()) {
        Puzzle<N>* curr = frontier.top();

        frontier.pop();
        nodes += 1;

        unsigned long long curr_hash = hash_tiles<N>(curr->tiles);
        visited[curr_hash] = true;

        if (goal_hash == curr_hash) {
            return Path<N>{reconstruct_path(curr), nodes};
        }

        Position zero_pos = curr->find_zero();

        for (auto& direction : get_directions()) {
            auto new_pos = zero_pos + direction.vector;
            if (in_bounds(new_pos, N)) {
                std::array<Tile, N * N> next_tiles = curr->tiles;

                swap_positions<N>(next_tiles, new_pos, zero_pos);
                auto next_hash = hash_tiles<N>(next_tiles);
                
                if (visited.count(next_hash) <= 0) {
                    Puzzle<N>* neighbor = new Puzzle<N>(next_tiles, curr);
                    neighbor->action = direction.action;
                    puzzles.emplace_back(neighbor);
                    frontier.push(neighbor);
                }
            }
        }
    }

    for (Puzzle<N>* puzzle : puzzles) {
        delete puzzle;
    }

    return Path<N>{std::vector<Puzzle<N>>(), nodes};
}

template<std::size_t N>
void print_puzzle(const Puzzle<N>& puzzle) {
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        auto tile = (int) puzzle.tiles[i];
        if (tile == 0) {
            printf(" ");
        } else {
            printf("%d", tile);
        }
        if ((i + 1) % N == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}
#endif //CPP_PUZZLE_H