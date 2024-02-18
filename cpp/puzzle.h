//
// Created by Joseph on 2/25/2024.
//

#include <array>
#include <vector>
#include <functional>
#include <string>
#include <queue>

#ifndef CPP_PUZZLE_H
#define CPP_PUZZLE_H

typedef char tile;

typedef std::tuple<int, int> position;

typedef std::tuple<position, std::string> direction;

position operator+(position lhs, position rhs);

template<std::size_t N>
class Puzzle {
public:
    static constexpr std::size_t SIZE = N * N;

    static const Puzzle& get_goal();

    explicit Puzzle(const std::array<tile, SIZE> tiles) : tiles(tiles) {};

    explicit Puzzle(Puzzle<N> const&);

    Puzzle() : tiles(std::array<tile, SIZE>(0)) {};

    Puzzle *get_prev() const;

    int get_fscore() const;

    std::string hash_tiles() const;

    tile &operator[](position);

    friend bool operator==(const Puzzle<N> &, const Puzzle<N> &);

    template<class T>
    friend std::ostream &operator<<(std::ostream &, const Puzzle<N> &);

    bool in_bounds(position pos);

    position find_zero();

    position pos_of_index(int);

    template <typename F>
    void on_neighbors(const F&);

    int heuristic();
private:
    std::array<tile, SIZE> tiles;
    Puzzle *prev = nullptr;
    int g = 0;
    int f = 0;
    std::string action;
};

template<std::size_t N>
std::vector<Puzzle<N>> reconstruct_path(Puzzle<N> &root) {
    std::vector<Puzzle<N>> path;
    while (root != nullptr) {
        path.push_back(root);
        root = root.get_prev();
    }
    return path;
}

template<std::size_t N>
std::vector<Puzzle<N>> find_path(Puzzle<N> &initial) {
    std::unordered_map<std::string, bool> visited;

    auto cmp = [](Puzzle<N> left, Puzzle<N> right) { return left.get_fscore() < right.get_fscore(); };
    std::priority_queue<Puzzle<N>&, std::vector<Puzzle<N>>, decltype(cmp)> frontier(cmp);
    frontier.push(initial);

    auto goal = Puzzle<N>::get_goal();

    while (!frontier.empty()) {
        Puzzle<N> puzzle = frontier.top();
        frontier.pop();

        if (puzzle == goal) {
            return reconstruct_path(puzzle);
        }

        visited[puzzle.hash_tiles()] = true;

        puzzle.on_neighbors([&visited, &frontier](Puzzle<N> &neighbor) {
            if (visited.count(neighbor.hash_tiles()) <= 0) {
                frontier.push(neighbor);
            }
        });
    }
}

#endif //CPP_PUZZLE_H
