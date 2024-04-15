#include <iostream>
#include <fstream>
#include <chrono>
#include <variant>
#include <thread>
#include <future>
#include <semaphore>
#include <cstring>
#include "puzzle.hpp"

using AnyPuzzle = std::variant<Puzzle<3>, Puzzle<4>>;

bool is_space_string(std::string& str) {
    for (int i = 0; i < str.length(); i++) {
        char c = str[i];
        if (!isspace(c)) {
            return false;
        }
    }
    return true;
}

std::vector<AnyPuzzle> read_puzzles(std::istream& is) {
    std::vector<Tile> curr_tiles;
    std::vector<AnyPuzzle> puzzles;
    std::string line;

    while (is.good()) {
        getline(is, line);

        if (is_space_string(line)) {
            if (curr_tiles.size() == 9) {
                auto puzzle = Puzzle<3>(curr_tiles);
                puzzles.emplace_back(puzzle);

                curr_tiles.clear();
            } else if (curr_tiles.size() == 16) {
                auto puzzle = Puzzle<4>(curr_tiles);
                puzzles.emplace_back(puzzle);

                curr_tiles.clear();
            } else if (!curr_tiles.empty()) {
                printf("A puzzle must be size 16 or 9\n");
                exit(1);
            }
        } else {
            std::string token;
            for (;;) {
                std::string* toparse = &line;
                int pos = line.find(' ');
                if (pos != std::string::npos) {
                    token = line.substr(0, pos);
                    line.erase(0, pos + 1);
                    toparse = &token;
                }
                if (!is_space_string(*toparse)) {
                    try {
                        int tile = stoi(*toparse);
                        curr_tiles.emplace_back((Tile) tile);
                    }
                    catch(std::invalid_argument e) {
                        printf("Failed to parse a token: %s\n", toparse->c_str());
                        exit(1);
                    }
                }
                if (pos == std::string::npos) {
                    break;
                }
            }
        }        
    }

    return puzzles;
}

float time_difference(std::chrono::_V2::system_clock::time_point start, std::chrono::_V2::system_clock::time_point stop) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return ((float) duration.count()) / 1000.0f;
}

template<std::size_t N = 3>
struct Solution {
    float time;
    int nodes;
    std::vector<Puzzle<N>> path;
};

template<std::size_t N = 3>
Solution<N> run_puzzle(Puzzle<N>& puzzle) {
    auto start = std::chrono::high_resolution_clock::now();
    auto path = find_path(puzzle);
    auto stop = std::chrono::high_resolution_clock::now();

    auto time = time_difference(start, stop);

    auto [solution, nodes] = path;
    return Solution<N>{time, nodes, solution};
}

struct AnySolution {
    float time;
    int nodes;
    std::variant<std::vector<Puzzle<3>>, std::vector<Puzzle<4>>> path;
};

AnySolution run_any_puzzle(AnyPuzzle& any_puzzle) {
    if (auto* puzzle = std::get_if<Puzzle<3>>(&any_puzzle)) {
        auto solution = run_puzzle(*puzzle);
        AnySolution any_solution{solution.time, solution.nodes, solution.path};
        return any_solution;
    } else if (auto* puzzle = std::get_if<Puzzle<4>>(&any_puzzle)) {
        auto solution = run_puzzle(*puzzle);
        AnySolution any_solution{solution.time, solution.nodes, solution.path};
        return any_solution;
    } else {
        printf("Unknown tag for AnyPuzzle\n");
        exit(1);
    }
}

std::vector<AnySolution> run_puzzles(std::vector<AnyPuzzle>& puzzles) {
    std::vector<AnySolution> solutions;
    for (auto& any_puzzle : puzzles) {
        auto sol = run_any_puzzle(any_puzzle);
        solutions.emplace_back(std::move(sol));
    }
    return solutions;
}

std::vector<AnySolution> run_puzzles_parallel(std::vector<AnyPuzzle>&puzzles) {
    std::vector<std::future<AnySolution>> futures;
    for (int i = 0; i < puzzles.size(); i++) {
        auto future = std::async(std::launch::async, run_any_puzzle, std::ref(puzzles[i]));
        futures.emplace_back(std::move(future));
    }
    
    std::vector<AnySolution> solutions;
    for (auto& future : futures) {
        solutions.push_back(future.get());
    }
    return solutions;
}

template<std::size_t N = 3>
void print_solution(std::vector<Puzzle<N>>& solution, int nodes, int i) {
    printf("Solution for puzzle %d\n", i + 1);
    for (auto &p : solution) {
        print_action(p.action);
    }
    printf("Solved in %d steps, expanded %d nodes\n", (solution.size() - 1), nodes);
}

void print_results(std::vector<AnySolution> solutions) {
    for (int i = 0; i < solutions.size(); i++) {
        auto& solution = solutions[i];
        auto& any_path = solution.path;

        if (auto* path = std::get_if<std::vector<Puzzle<3>>>(&any_path)) {
            print_solution(*path, solution.nodes, i);
        } else if (auto* path = std::get_if<std::vector<Puzzle<4>>>(&any_path)) {
            print_solution(*path, solution.nodes, i);
        } else {
            printf("Unknown variant for AnySolution\n");
            exit(1);
        }
    }
}

enum ParFlag { SEQ = 1, PAR = 2 };

ParFlag get_par_flag(int argc, char* argv[]) {
    ParFlag par_flag = SEQ;
    if (argc > 2) {
        std::string arg(argv[2]);
        if ("par" == arg) {
            par_flag = PAR;
        } else if ("seq" == arg) {
            par_flag = SEQ;
        } else {
            printf("Par flag must be par or seq, got %s\n", arg.c_str());
            exit(1);
        }
    }
    return par_flag;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Need at least 1 program argument\n");
        exit(1);
    }

    std::fstream fs_in;
    fs_in.open(argv[1]);
    if (!fs_in.is_open()) {
        printf("Failed to open input file %s\n", argv[1]);
        exit(1);
    }

    ParFlag par_flag = get_par_flag(argc, argv);
    auto puzzles = read_puzzles(fs_in);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<AnySolution> solutions;

    switch (par_flag) {
        case SEQ: {
            solutions = run_puzzles(puzzles);
            break;
        }
        case PAR: {
            solutions = run_puzzles_parallel(puzzles);
            break;
        }
        default: {
            printf("Invalid par flag %d\n", par_flag);
            exit(1);
        }
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto eteTime = time_difference(start, stop);

    print_results(solutions);

    float totalTime = 0;
    int totalNodes = 0;
    for (int i = 0; i < solutions.size(); i++) {
        auto& sol = solutions[i];
        printf("Puzzle %d: %.2f ms, %d nodes\n", (i + 1), sol.time, sol.nodes);
        totalTime += sol.time;
        totalNodes += sol.nodes;
    }

    printf("\nTotal: %.2f ms, %d nodes\n", totalTime, totalNodes);

    printf("End-to-end: %.2fms\n", eteTime);
    return 0;
}