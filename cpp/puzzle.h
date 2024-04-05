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
   
    explicit Puzzle(std::vector<Tile>& tiles_vec) {
        for (int i = 0; i < SIZE; i++) {
            tiles[i] = tiles_vec[i];
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

    Action get_action() const {
        return action;
    };

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
            Tile tile = this->get_tile(i);
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

    Puzzle<N> get_neighbor(Position new_pos, Position zero_pos, Action a) {
        auto new_puzzle = *this;

        auto temp = new_puzzle.get_tile(new_pos);
        new_puzzle.get_tile(new_pos) = new_puzzle.get_tile(zero_pos);
        new_puzzle.get_tile(zero_pos) = temp;

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
private:
    std::array<Tile, SIZE> tiles;
    Action action = NONE;
};

template<std::size_t N = 3>
class Node {
public:
    Node() : puzzle(Puzzle<N>()) {}

    Node(Node<N> const& other) {
        puzzle = other.puzzle;
        g = other.g;
        f = other.f;
        prev = other.prev;
    }

    explicit Node(Puzzle<N> puzzle) : puzzle(puzzle) {}

    explicit Node(Puzzle<N> puzzle, Node<N>* prev) : puzzle(puzzle), prev(prev) {
        g = prev->g + 1;
        f = g + puzzle.heuristic();
    }

    const Puzzle<N>& get_puzzle() const {
        return puzzle;
    }

    Node<N>* get_prev() {
        return prev;
    }
    
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
class NodeSp {
public:
    NodeSp() : puzzle(Puzzle<N>()) {}

    NodeSp(NodeSp<N> const& other) {
        puzzle = other.puzzle;
        g = other.g;
        f = other.f;
        prev = other.prev;
    }

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
using Path = std::tuple<std::vector<Puzzle<N>>, int>;

template<std::size_t N = 3>
using FindPathFunc = std::tuple<std::vector<Puzzle<N>>, int> (*)(Puzzle<N>);

const std::array<direction, 4>& get_directions() {
    const static std::array<direction, 4> DIRECTIONS =
        {direction{Position{0, -1}, LEFT},
        direction{Position{-1, 0}, UP},
        direction{Position{1, 0}, DOWN},
        direction{Position{0, 1}, RIGHT}};
    return DIRECTIONS;
}

template<std::size_t N = 3>
class SolverArena {
public:
    static Path<N> find_path(Puzzle<N> initial) {
        Arena<Node<N>> arena;

        std::unordered_map<std::string, bool> visited;

        auto cmp = [](Node<N>* lhs, Node<N>* rhs)
            { return lhs->get_fscore() > rhs->get_fscore(); };
        std::priority_queue<Node<N>*, std::vector<Node<N>*>, decltype(cmp)> frontier(cmp);

        Node<N> initial_node(initial);
        frontier.push(&initial_node);

        auto& goal = Puzzle<N>::get_goal();

        int nodes = 0;
        while (!frontier.empty()) {
            auto node = frontier.top();
            auto curr_puzzle = node->get_puzzle();

            frontier.pop();
            nodes++;

            if (curr_puzzle == goal) {
                return Path<N>{reconstruct_path(node), nodes};
            }

            visited[curr_puzzle.hash_tiles()] = true;

            auto zero_pos = curr_puzzle.find_zero();

            for (auto& direction : get_directions()) {
                auto d = std::get<0>(direction);
                auto a = std::get<1>(direction);

                auto new_pos = zero_pos + d;
                if (Puzzle<N>::in_bounds(new_pos)) {
                    auto next_puzzle = curr_puzzle.get_neighbor(new_pos, zero_pos, a);
                    Node<N> next_node(next_puzzle, node);
                    
                    if (visited.count(next_puzzle.hash_tiles()) <= 0) {
                        auto neighbor = arena.alloc(next_node);
                        frontier.push(neighbor);
                    }
                }
            }
        }

        std::vector<Puzzle<N>> no_sol;
        return Path<N>{no_sol, nodes};
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
class SolverSp {
public:
    static Path<N> find_path(Puzzle<N> initial) {
        std::unordered_map<std::string, bool> visited;

        auto cmp = [](std::shared_ptr<NodeSp<N>> lhs, std::shared_ptr<NodeSp<N>> rhs)
            { return lhs->get_fscore() > rhs->get_fscore(); };
        std::priority_queue<std::shared_ptr<NodeSp<N>>, std::vector<std::shared_ptr<NodeSp<N>>>, decltype(cmp)> frontier(cmp);

        NodeSp<N> initial_node(initial);
        frontier.push(std::make_shared<NodeSp<N>>(initial_node));

        auto& goal = Puzzle<N>::get_goal();

        int nodes = 0;
        while (!frontier.empty()) {
            auto node = frontier.top();
            auto curr_puzzle = node->get_puzzle();

            frontier.pop();
            nodes++;

            if (curr_puzzle == goal) {
                return Path<N>{reconstruct_path(node), nodes};
            }

            visited[curr_puzzle.hash_tiles()] = true;

            auto zero_pos = curr_puzzle.find_zero();

            for (auto& direction : get_directions()) {
                auto d = std::get<0>(direction);
                auto a = std::get<1>(direction);

                auto new_pos = zero_pos + d;
                if (Puzzle<N>::in_bounds(new_pos)) {
                    auto next_puzzle = curr_puzzle.get_neighbor(new_pos, zero_pos, a);
                    NodeSp<N> next_node(next_puzzle, node);
                    
                    if (visited.count(next_puzzle.hash_tiles()) <= 0) {
                        auto neighbor = std::make_shared<NodeSp<N>>(next_node);
                        frontier.push(neighbor);
                    }
                }
            }
        }

        std::vector<Puzzle<N>> no_sol;
        return Path<N>{no_sol, nodes};
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
    os << puzzle.get_action();
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
