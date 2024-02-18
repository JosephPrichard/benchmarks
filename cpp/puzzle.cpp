//
// Created by Joseph on 2/25/2024.
//

#include <iostream>
#include "puzzle.h"

template<std::size_t N>
Puzzle<N>::Puzzle(Puzzle<N> const& other) {
    this->tiles = other;
    this->f = other.f;
    this->g = other.g;
    this->prev = other.prev;
}

template<std::size_t N>
const Puzzle<N>& Puzzle<N>::get_goal() {
    static const std::array<tile, SIZE> tiles;
    for (int i = 0; i < SIZE; i++) {
        tiles[i] = i;
    }
    static const Puzzle<N> puzzle(tiles);
    return puzzle;
}

template<std::size_t N>
Puzzle<N> *Puzzle<N>::get_prev() const {
    return prev;
}

template<std::size_t N>
int Puzzle<N>::get_fscore() const {
    return f;
}

template<std::size_t N>
std::string Puzzle<N>::hash_tiles() const {
    std::string str(std::begin(tiles), std::end(tiles));
    return str;
}

position operator+(position lhs, position rhs) {
    auto row1 = std::get<0>(lhs);
    auto col1 = std::get<1>(lhs);
    auto row2 = std::get<0>(rhs);
    auto col2 = std::get<1>(rhs);
    return position{row1 + row2, col1 + col2};
}

template<std::size_t N>
int Puzzle<N>::heuristic() {
    int h = 0;
    for (int i = 0; i < SIZE; i++) {
        auto pos1 = pos_of_index(this->tiles[i]);
        auto row1 = std::get<0>(pos1);
        auto col1 = std::get<1>(pos1);
        auto pos2 = pos_of_index(i);
        auto row2 = std::get<0>(pos2);
        auto col2 = std::get<1>(pos2);
        h += abs(row2 - row1) + abs(col2 - col1);
    }
    return h;
}

template<std::size_t N>
position Puzzle<N>::find_zero() {
    for (int i = 0; i < SIZE; i++) {
        tile tile = this->tiles[i];
        if (tile == 0) {
            return pos_of_index(i);
        }
    }
}

template<std::size_t N>
position Puzzle<N>::pos_of_index(int index) {
    return position{index / SIZE, index % SIZE};
}


template<std::size_t N>
template <typename F>
void Puzzle<N>::on_neighbors(const F& on_neighbor) {
    const static std::array<direction, 4> DIRECTIONS =
        {direction{position{0, -1}, "Left"},
         direction{position{-1, 0}, "Down"},
         direction{position{1, 0}, "Up"},
         direction{position{0, 1}, "Right"}};

    auto zero_pos = find_zero();
    for (auto &direction : DIRECTIONS) {
        auto pos_vec = std::get<0>(direction);
        auto new_pos = zero_pos + pos_vec;
        if (in_bounds(new_pos)) {
            auto new_puzzle = *this;
            new_puzzle.g += 1;
            new_puzzle.f = new_puzzle.g + new_puzzle.heuristic();
            new_puzzle.prev = this;

            auto temp = new_puzzle[new_pos];
            new_puzzle[new_pos] = new_puzzle[zero_pos];
            new_puzzle[zero_pos] = temp;

            new_puzzle.action = std::get<1>(direction);
            on_neighbor(new_puzzle);
        }
    }
}

template<std::size_t N>
bool Puzzle<N>::in_bounds(position pos) {
    auto row = std::get<0>(pos);
    auto col = std::get<1>(pos);
    return row > 0 && row < N && col > 0 && col < N;
}

template<std::size_t N>
tile &Puzzle<N>::operator[](position pos) {
    auto row = std::get<0>(pos);
    auto col = std::get<1>(pos);
    return tiles[row * N + col];
}

template<std::size_t N>
bool operator==(const Puzzle<N> &lhs, const Puzzle<N> &rhs) {
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

template<std::size_t N>
std::ostream &operator<<(std::ostream &os, const Puzzle<N> &puzzle) {
    for (int i = 0; i < Puzzle<N>::SIZE; i++) {
        os << puzzle[i];
        if ((i + 1) % N == 0) {
            os << std::endl;
        } else {
            os << " ";
        }
    }
    return os;
}