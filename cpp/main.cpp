#include <iostream>
#include <fstream>
#include <chrono>
#include "puzzle.h"

template<typename F>
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
    f(s);
}

template<std::size_t N>
std::vector<std::array<Tile, N * N>> read_tiles(std::istream& is) {
    constexpr std::size_t SIZE = N * N;

    int index = 0;
    std::array<Tile, SIZE> curr_tiles;
    std::vector<std::array<Tile, SIZE>> puzzles;
    std::string line;

    while (is.good()) {
        getline(is, line);

        if (index >= SIZE) {
            index = 0;
            puzzles.emplace_back(curr_tiles);
        }

        split_string(line, " ", N, [&curr_tiles, &index](std::string& token) {
            if (token != "") {
                Tile tile = stoi(token);
                curr_tiles[index] = tile;
                index++;
            }
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

    std::vector<float> times;

    auto tiles = read_tiles<4>(fs);
    for (int i = 0; i < tiles.size(); i++) {
        Puzzle<4> puzzle(tiles[i]);

        auto start = std::chrono::high_resolution_clock::now();
        auto path = Puzzle<4>::find_path(puzzle);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration =  std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        times.push_back(duration.count() / 1000.0f);

        auto solution = std::get<0>(path);
        auto nodes = std::get<1>(path);

        std::cout << "Solution for puzzle " << (i + 1) << std::endl;
        for (auto p : solution) {
            std::cout << p;
        }
        std::cout << "Solved in " << (solution.size() - 1) << " steps, expanded " << nodes << " nodes \n" << std::endl;
    }

    float total = 0;
    for (int i = 0; i < times.size(); i++) {
        std::cout << "Puzzle " << (i + 1) << " took " << times[i] << " ms" << std::endl;
        total += times[i];
    }
    std::cout << "Took " << total << " ms in total" << std::endl;

    return 0;
}
