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
#include "arena.h"

#ifndef CPP_PUZZLE_H
#define CPP_PUZZLE_H

enum Action {
    NONE, LEFT, DOWN, UP, RIGHT
};

std::ostream& operator<<(std::ostream& os, Action a);

typedef char Tile;

typedef std::tuple<int, int> Position;

typedef std::tuple<Position, Action> direction;

Position operator+(Position lhs, Position rhs) {
    auto row1 = std::get<0>(lhs);
    auto col1 = std::get<1>(lhs);
    auto row2 = std::get<0>(rhs);
    auto col2 = std::get<1>(rhs);
    return Position{row1 + row2, col1 + col2};
}

template<std::size_t N = 3>
class Puzzle {
public:
    static constexpr std::size_t SIZE = N * N;

    explicit Puzzle(const std::array<Tile, SIZE> tiles) : tiles(tiles) {};

    Puzzle() : tiles(std::array<Tile, SIZE>{0}) {};

    Puzzle(Puzzle<N> const& other) {
        this->tiles = other.tiles;
        this->f = other.f;
        this->g = other.g;
        this->prev = other.prev;
        this->action = other.action;
    }

    static bool in_bounds(Position pos) {
        auto row = std::get<0>(pos);
        auto col = std::get<1>(pos);
        return row >= 0 && row < N && col >= 0 && col < N;
    }

    static const Puzzle<N>& get_goal() {
        static std::array<Tile, SIZE> tiles{};
        for (int i = 0; i < SIZE; i++) {
            tiles[i] = i;
        }
        static const Puzzle puzzle(tiles);
        return puzzle;
    }

    Action get_action() const {
        return action;
    };

    int get_fscore() const {
        return f;
    }

    Position pos_of_index(int index) {
        return Position{index / N, index % N};
    }

    std::string hash_tiles() const {
        std::string str(std::begin(tiles), std::end(tiles));
        return str;
    }

    int heuristic() {
        int h = 0;
        for (int i = 0; i < SIZE; i++) {
            auto pos1 = pos_of_index(this->get_tile(i));
            auto row1 = std::get<0>(pos1);
            auto col1 = std::get<1>(pos1);

            auto pos2 = pos_of_index(i);
            auto row2 = std::get<0>(pos2);
            auto col2 = std::get<1>(pos2);

            h += abs(row2 - row1) + abs(col2 - col1);
        }
        return h;
    }

    Puzzle<N> make_neighbor(Position new_pos, Position zero_pos, Action a) {
        auto new_puzzle = *this;

        auto temp = new_puzzle.get_tile(new_pos);
        new_puzzle.get_tile(new_pos) = new_puzzle.get_tile(zero_pos);
        new_puzzle.get_tile(zero_pos) = temp;

        new_puzzle.g += 1;
        new_puzzle.f = new_puzzle.g + new_puzzle.heuristic();
        new_puzzle.prev = this;
        new_puzzle.action = a;

        return new_puzzle;
    }

    Position find_zero() {
        for (int i = 0; i < SIZE; i++) {
            if (this->get_tile(i) == 0) {
                return pos_of_index(i);
            }
        }
        throw std::invalid_argument("Puzzle should contain a zero");
    }

    Tile get_tile(int index) const {
        return tiles[index];
    }

    Tile& get_tile(Position pos) {
        auto row = std::get<0>(pos);
        auto col = std::get<1>(pos);
        return tiles.at(row * N + col);
    }

    static std::vector<Puzzle<N>> reconstruct_path(Puzzle<N>* curr) {
        std::vector<Puzzle<N>> path;
        while (curr != nullptr) {
            path.push_back(*curr);
            curr = curr->prev;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    typedef std::tuple<std::vector<Puzzle>, int> Path;

    static void debug(std::priority_queue<Puzzle<N>*, std::vector<Puzzle<N>*>> frontier) {
        while (!frontier.empty()) {
            auto puzzle = frontier.top();
            frontier.pop();
            std::cout << puzzle->get_fscore() << " ";
        }
        std::cout << std::endl;
    }

    static Path find_path(Puzzle<N> initial) {
        Arena<Puzzle> arena;

        std::unordered_map<std::string, bool> visited;

        auto cmp = [](Puzzle<N>* lhs, Puzzle<N>* rhs)
            { return lhs->get_fscore() > rhs->get_fscore(); };
        std::priority_queue<Puzzle<N>*, std::vector<Puzzle<N>*>, decltype(cmp)> frontier(cmp);
        frontier.push(&initial);

        auto& goal = Puzzle::get_goal();

        int nodes = 0;
        while (!frontier.empty()) {
            auto puzzle = frontier.top();
            frontier.pop();
            nodes++;

            if (*puzzle == goal) {
                return Path{reconstruct_path(puzzle), nodes};
            }

            visited[puzzle->hash_tiles()] = true;

            const static std::array<direction, 4> DIRECTIONS =
                {direction{Position{0, -1}, LEFT},
                 direction{Position{-1, 0}, UP},
                 direction{Position{1, 0}, DOWN},
                 direction{Position{0, 1}, RIGHT}};

            auto zero_pos = puzzle->find_zero();
            for (auto& direction: DIRECTIONS) {
                auto new_pos = zero_pos + std::get<0>(direction);
                if (in_bounds(new_pos)) {
                    auto neighbor = arena.alloc(puzzle->make_neighbor(new_pos, zero_pos, std::get<1>(direction)));
                    auto is_visited = visited.count(neighbor->hash_tiles()) > 0;
                    if (!is_visited) {
                        frontier.push(neighbor);
                    }
                }
            }
        }

        std::vector<Puzzle> no_sol;
        return Path{no_sol, nodes};
    }
private:
    std::array<Tile, SIZE> tiles;
    Puzzle* prev = nullptr;
    int g = 0;
    int f = 0;
    Action action = NONE;
};

template<std::size_t N>
bool operator==(const Puzzle<N>& lhs, const Puzzle<N>& rhs) {
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        if (lhs.get_tile(i) != rhs.get_tile(i)) {
            return false;
        }
    }
    return true;
}

template<std::size_t N>
bool operator<(const Puzzle<N>& lhs, const Puzzle<N>& rhs) {
    return lhs.get_fscore() > rhs.get_fscore();
}

std::ostream& operator<<(std::ostream& os, Action a) {
    switch (a) {
        case LEFT:
            os << "Left" << std::endl;
            break;
        case RIGHT:
            os << "Right" << std::endl;
            break;
        case DOWN:
            os << "Down" << std::endl;
            break;
        case UP:
            os << "Up" << std::endl;
            break;
        default:
            os << "None" << std::endl;
            break;
    }
    return os;
}

template<std::size_t N>
std::ostream& operator<<(std::ostream& os, const Puzzle<N>& puzzle) {
    os << puzzle.get_action();
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        auto t = (int) puzzle.get_tile(i);
        if (t == 0) {
            os << " ";
        } else {
            os << t;
        }
        if ((i + 1) % N == 0) {
            os << std::endl;
        } else {
            os << " ";
        }
    }
    return os;
}

#endif //CPP_PUZZLE_H
