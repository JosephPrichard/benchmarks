#include <iostream>
#include <fstream>
#include "puzzle.h"

template <typename F>
void split_string(std::string s, std::string delim, std::size_t max, const F& f) {
    std::string token;
    for (int i = 0; i < max; i++) {
        int pos = s.find(delim);
        if (pos == std::string::npos) {
            break;
        }
        token = s.substr(0, pos);
        s.erase(0, pos + 1);
        f(token);
    }
}

template<std::size_t N>
std::vector<std::array<tile, N * N>> read_tiles(std::istream &is) {
    constexpr std::size_t SIZE = N * N;

    int index = 0;
    std::array<tile, SIZE> curr_tiles;
    std::vector<std::array<tile, SIZE>> puzzles;
    std::string line;

    while (is.good()) {
        getline(is, line);

        if (index >= SIZE) {
            index = 0;
            puzzles.emplace_back(curr_tiles);
        }

        split_string(line, " ", N, [&curr_tiles, &index](std::string &token) {
            tile tile = stoi(token);
            curr_tiles[index] = tile;
            index++;
        });
    }

    return puzzles;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "First program argument must be file path to inputs." << std::endl;
        exit(1);
    }

    std::fstream fs;
    fs.open(argv[1]);

    auto tiles_vec = read_tiles<3>(fs);
    for (auto tiles : tiles_vec) {
        Puzzle<3> puzzle(tiles);
        Puzzle<3> puzzle1(tiles);

        auto solution = find_path<3>(puzzle);
    }

    return 0;
}
