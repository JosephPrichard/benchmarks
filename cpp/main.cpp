#include <iostream>
#include <fstream>
#include <chrono>
#include <variant>
#include "puzzle.h"

template<typename F>
void split_string(std::string s, std::string delim, const F& f) {
    std::string token;
    for (int i = 0; ; i++) {
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

struct EightPuzzle {
    Puzzle<3> value;
    EightPuzzle(Puzzle<3> v) : value(v) {}
};

struct FifteenPuzzle {
    Puzzle<4> value;
    FifteenPuzzle(Puzzle<4> v) : value(v) {}
};

typedef std::variant<EightPuzzle, FifteenPuzzle> AnyPuzzle;

std::vector<AnyPuzzle> read_puzzles(std::istream& is) {
    std::vector<Tile> curr_tiles;
    std::vector<AnyPuzzle> puzzles;
    std::string line;

    while (is.good()) {
        getline(is, line);

        if (line.empty()) {
            if (curr_tiles.size() == 9) {
                auto puzzle = EightPuzzle(Puzzle<3>(curr_tiles));
                puzzles.emplace_back(puzzle);
                curr_tiles.clear();
            } else if (curr_tiles.size() == 16) {
                auto puzzle = FifteenPuzzle(Puzzle<4>(curr_tiles));
                puzzles.emplace_back(puzzle);
                curr_tiles.clear();
            } else if (!curr_tiles.empty()) {
                std::cout << "A puzzle must be size 16 or 9" << std::endl;
                exit(1);
            }
        }

        split_string(line, " ", [&curr_tiles](std::string& token) {
            if (!token.empty()) {
                Tile tile = (Tile) stoi(token);
                curr_tiles.emplace_back(tile);
            }
        });
    }

    return puzzles;
}

template<std::size_t N>
std::tuple<float, int> run_puzzle(Puzzle<N> puzzle, int i) {
    auto start = std::chrono::high_resolution_clock::now();
    auto path = Puzzle<N>::find_path(puzzle);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration =  std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    auto time = ((float) duration.count()) / 1000.0f;

    auto solution = std::get<0>(path);
    auto nodes = std::get<1>(path);

    for (auto &p : solution) {
        std::cout << p;
    }
    std::cout << "Solved in " << (solution.size() - 1) << " steps\n" << std::endl;

    return std::tuple(time, nodes);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Need at least 1 program argument" << std::endl;
        exit(1);
    }

    std::fstream fs_in;
    fs_in.open(argv[1]);
    if (!fs_in.is_open()) {
        std::cout << "Failed to open input file " << argv[2] << std::endl;
        exit(1);
    }

    std::vector<std::tuple<float, int>> results;
    auto puzzles = read_puzzles(fs_in);

    for (int i = 0; i < puzzles.size(); i++) {
        auto& puzzle_var = puzzles[i];

        if (std::holds_alternative<EightPuzzle>(puzzle_var)) {
            auto puzzle = std::get<EightPuzzle>(puzzle_var);
            auto result = run_puzzle(puzzle.value, i);

            results.emplace_back(result);
        } else if (std::holds_alternative<FifteenPuzzle>(puzzle_var)) {
            auto puzzle = std::get<FifteenPuzzle>(puzzle_var);
            auto result = run_puzzle(puzzle.value, i);

            results.emplace_back(result);
        }
    }

    float totalTime = 0;
    int totalNodes = 0;

    for (int i = 0; i < results.size(); i++) {
        auto result = results[i];
        auto time = std::get<0>(result);
        auto nodes = std::get<1>(result);

        std::cout << "Puzzle " << (i + 1) << ": " << time << " ms, " << nodes << " nodes" << std::endl;

        totalTime += time;
        totalNodes += nodes;
    }

    std::cout << "Total: " << totalTime << " ms, " << totalNodes << " nodes" << std::endl;

    return 0;
}
