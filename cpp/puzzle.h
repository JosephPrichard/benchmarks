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

    Puzzle() : tiles(std::array<Tile, SIZE>{0}) {};

    explicit Puzzle(const std::array<Tile, SIZE> tiles) : tiles(tiles) {};
   
    explicit Puzzle(std::vector<Tile>& tiles) {
        for (int i = 0; i < SIZE; i++) {
            this->tiles[i] = tiles[i];
        }
    };

    Puzzle(Puzzle<N> const& other) {
        this->tiles = other.tiles;
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

    [[nodiscard]] 
    Action get_action() const {
        return action;
    };

    [[nodiscard]] 
    Position pos_of_index(int index) const {
        return Position{index / N, index % N};
    }

    // 64 BIT HASH FOR AN ARRAY OF LENGTH 16 WHERE EACH ELEMENT IS 4 BITS
    [[nodiscard]] 
    unsigned long long hash_tiles() const {
        unsigned long long hash = 0;
        for (int i = 0; i < SIZE; i++) {
            Tile tile = tiles[i];
            long long mask = ((unsigned long long) tile) << (i * 4);
            hash = (hash | mask);
        }
        return hash;
    }

    [[nodiscard]] 
    int heuristic() const {
        int h = 0;
        for (int i = 0; i < SIZE; i++) {
            Tile tile = this->tiles[i];
            if (tile != 0) {
                auto pos1 = pos_of_index(tile);
                auto row1 = std::get<0>(pos1);
                auto col1 = std::get<1>(pos1);

                auto pos2 = pos_of_index(i);
                auto row2 = std::get<0>(pos2);
                auto col2 = std::get<1>(pos2);

                h += abs(row2 - row1) + abs(col2 - col1);
            }
        }
        return h;
    }

    Puzzle<N> get_neighbor(Position new_pos, Position zero_pos, Action a) const {
        auto new_puzzle = *this;

        auto temp = new_puzzle[new_pos];
        new_puzzle[new_pos] = new_puzzle[zero_pos];
        new_puzzle[zero_pos] = temp;

        new_puzzle.action = a;

        return new_puzzle;
    }

    [[nodiscard]] 
    Position find_zero() const {
        for (int i = 0; i < SIZE; i++) {
            if (this->tiles[i] == 0) {
                return pos_of_index(i);
            }
        }
        throw std::invalid_argument("Puzzle should contain a zero");
    }

    [[nodiscard]] 
    Tile get_tile(int index) const {
        return tiles[index];
    }

    Tile& operator[](Position pos) {
        auto row = std::get<0>(pos);
        auto col = std::get<1>(pos);
        return tiles[row * N + col];
    }

    static const std::array<direction, 4>& get_directions() {
        const static std::array<direction, 4> DIRECTIONS =
            {direction{Position{0, -1}, LEFT},
            direction{Position{-1, 0}, UP},
            direction{Position{1, 0}, DOWN},
            direction{Position{0, 1}, RIGHT}};
        return DIRECTIONS;
    }
private:
    std::array<Tile, SIZE> tiles;
    Action action = NONE;
};

template<std::size_t N = 3>
using Path = std::tuple<std::vector<Puzzle<N>>, int>;

template<std::size_t N = 3>
using FindPathFunc = std::tuple<std::vector<Puzzle<N>>, int> (*)(Puzzle<N>);

template<std::size_t N = 3>
class Node {
public:
    Node() : puzzle(Puzzle<N>()) {}

    explicit Node(Puzzle<N>&& p) : puzzle(p) {}

    explicit Node(Puzzle<N>&& p, Node<N>* prev) : puzzle(p), prev(prev) {
        g = prev->g + 1;
        f = g + p.heuristic();
    }

    const Puzzle<N>& get_puzzle() const {
        return puzzle;
    }

    Node<N>* get_prev() {
        return prev;
    }

    [[nodiscard]] 
    int get_fscore() const {
        return f;
    }
private:
    int g = 0;
    int f = 0;
    Puzzle<N> puzzle;
    Node<N>* prev = nullptr;
};

template<std::size_t N = 3>
class SolverArena {
public:
    static Path<N> find_path(Puzzle<N> initial) {
        Arena<Node<N>> arena;

        std::unordered_map<unsigned long long, bool> visited;

        auto cmp = [](Node<N>* lhs, Node<N>* rhs)
            { return lhs->get_fscore() > rhs->get_fscore(); };
        std::priority_queue<Node<N>*, std::vector<Node<N>*>, decltype(cmp)> frontier(cmp);

        Node<N> initial_node(std::move(initial));
        frontier.push(&initial_node);

        const Puzzle<N>& goal = Puzzle<N>::get_goal();
        auto goal_hash = goal.hash_tiles();

        int nodes = 0;
        while (!frontier.empty()) {
            Node<N>* node = frontier.top();
            const Puzzle<N>& curr_puzzle = node->get_puzzle();

            frontier.pop();
            nodes++;

            auto curr_hash = curr_puzzle.hash_tiles();
            visited[curr_hash] = true;

            if (goal_hash == curr_hash) {
                return Path<N>{reconstruct_path(node), nodes};
            }

            auto zero_pos = curr_puzzle.find_zero();

            for (auto& direction : Puzzle<N>::get_directions()) {
                auto d = std::get<0>(direction);
                auto a = std::get<1>(direction);

                auto new_pos = zero_pos + d;
                if (Puzzle<N>::in_bounds(new_pos)) {
                    auto next_puzzle = curr_puzzle.get_neighbor(new_pos, zero_pos, a);
                    
                    if (visited.count(next_puzzle.hash_tiles()) <= 0) {
                        auto neighbor = new(arena.alloc()) Node<N>(std::move(next_puzzle), node);
                        frontier.push(neighbor);
                    }
                }
            }
        }

        return Path<N>{std::vector<Puzzle<N>>(), nodes};
    }

private:
    static std::vector<Puzzle<N>> reconstruct_path(Node<N>* curr) {
        std::vector<Puzzle<N>> path;
        while (curr != nullptr) {
            path.push_back(curr->get_puzzle());
            curr = curr->get_prev();
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

template<std::size_t N = 3>
class NodeSp {
public:
    NodeSp() : puzzle(Puzzle<N>()) {}

    explicit NodeSp(Puzzle<N> puzzle) : puzzle(puzzle) {}

    explicit NodeSp(Puzzle<N> puzzle, std::shared_ptr<NodeSp<N>> prev) : puzzle(puzzle), prev(prev) {
        g = this->prev->g + 1;
        f = g + puzzle.heuristic();
    }

    const Puzzle<N>& get_puzzle() const {
        return puzzle;
    }

    std::shared_ptr<NodeSp<N>> get_prev() {
        return prev;
    }

    [[nodiscard]] 
    int get_fscore() const {
        return f;
    }
private:
    int g = 0;
    int f = 0;
    Puzzle<N> puzzle;
    std::shared_ptr<NodeSp<N>> prev = nullptr;
};

template<std::size_t N = 3>
class SolverSharedPtr {
public:
    static Path<N> find_path(Puzzle<N> initial) {
        std::unordered_map<unsigned long long, bool> visited;

        auto cmp = [](std::shared_ptr<NodeSp<N>> lhs, std::shared_ptr<NodeSp<N>> rhs)
            { return lhs->get_fscore() > rhs->get_fscore(); };
        std::priority_queue<std::shared_ptr<NodeSp<N>>, std::vector<std::shared_ptr<NodeSp<N>>>, decltype(cmp)> frontier(cmp);

        NodeSp<N> initial_node(initial);
        frontier.push(std::make_shared<NodeSp<N>>(initial_node));

        const Puzzle<N>& goal = Puzzle<N>::get_goal();

        int nodes = 0;
        while (!frontier.empty()) {
            std::shared_ptr<NodeSp<N>> node = frontier.top();
            const Puzzle<N>& curr_puzzle = node->get_puzzle();

            frontier.pop();
            nodes++;

            if (curr_puzzle == goal) {
                return Path<N>{reconstruct_path(node), nodes};
            }

            visited[curr_puzzle.hash_tiles()] = true;

            auto zero_pos = curr_puzzle.find_zero();

            for (auto& direction : Puzzle<N>::get_directions()) {
                auto d = std::get<0>(direction);
                auto a = std::get<1>(direction);

                auto new_pos = zero_pos + d;
                if (Puzzle<N>::in_bounds(new_pos)) {
                    auto next_puzzle = curr_puzzle.get_neighbor(new_pos, zero_pos, a);

                    if (visited.count(next_puzzle.hash_tiles()) <= 0) {
                        auto neighbor = std::make_shared<NodeSp<N>>(next_puzzle, node);
                        frontier.push(neighbor);
                    }
                }
            }
        }
        
        return Path<N>{std::vector<Puzzle<N>>(), nodes};
    }

private:
    static std::vector<Puzzle<N>> reconstruct_path(std::shared_ptr<NodeSp<N>> curr) {
        std::vector<Puzzle<N>> path;
        while (curr != nullptr) {
            path.push_back(curr->get_puzzle());
            curr = curr->get_prev();
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
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
            os << "Start" << std::endl;
            break;
    }
    return os;
}

template<std::size_t N>
std::ostream& operator<<(std::ostream& os, const Puzzle<N>& puzzle) {
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        auto tile = (int) puzzle.get_tile(i);
        if (tile == 0) {
            os << " ";
        } else {
            os << tile;
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
