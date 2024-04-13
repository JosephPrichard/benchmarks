#include <iostream>
#include <fstream>
#include <chrono>
#include <variant>
#include <thread>
#include <future>
#include <semaphore>
#include "puzzle.h"

template<typename F>
void split_string(std::string str, std::string delim, const F& func) {
    std::string token;
    for (int i = 0; ; i++) {
        int pos = str.find(delim);
        if (pos == std::string::npos) {
            break;
        }
        token = str.substr(0, pos);
        str.erase(0, pos + 1);
        func(token);
    }
    func(str);
}

typedef std::variant<Puzzle<3>, Puzzle<4>> AnyPuzzle;

std::vector<AnyPuzzle> read_puzzles(std::istream& is) {
    std::vector<Tile> curr_tiles;
    std::vector<AnyPuzzle> puzzles;
    std::string line;

    while (is.good()) {
        getline(is, line);

        if (line.empty()) {
            if (curr_tiles.size() == 9) {
                auto puzzle = Puzzle<3>(curr_tiles);
                puzzles.emplace_back(puzzle);

                curr_tiles.clear();
            } else if (curr_tiles.size() == 16) {
                auto puzzle = Puzzle<4>(curr_tiles);
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

float time_difference(std::chrono::_V2::system_clock::time_point start, std::chrono::_V2::system_clock::time_point stop) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return ((float) duration.count()) / 1000.0f;
}

template<std::size_t N = 3>
using Result = std::tuple<float, int, std::vector<Puzzle<N>>>;

template<std::size_t N>
Result<N> run_puzzle(Puzzle<N>& puzzle, const std::string& mem_flag) {
    FindPathFunc<N> find_path = nullptr;
    if (mem_flag == "arena") {
        find_path = SolverArena<N>::find_path;
    } else if (mem_flag == "sp") {
        find_path = SolverSharedPtr<N>::find_path;
    } else {
        throw std::invalid_argument("Invalid flag - must be 'arena' or 'sp'");
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto path = find_path(puzzle);
    auto stop = std::chrono::high_resolution_clock::now();

    auto time = time_difference(start, stop);

    auto [solution, nodes] = path;
    return std::tuple(time, nodes, solution);
}

typedef std::tuple<float, int, std::variant<std::vector<Puzzle<3>>, std::vector<Puzzle<4>>>> AnyResult;

AnyResult run_any_puzzle(AnyPuzzle& any_puzzle, const std::string& mem_flag) {
    if (std::holds_alternative<Puzzle<3>>(any_puzzle)) {
        auto& puzzle = std::get<Puzzle<3>>(any_puzzle);
        auto [time, nodes, solution] = run_puzzle(puzzle, mem_flag);

        auto result = AnyResult(time, nodes, solution);
        return result;
    } else if (std::holds_alternative<Puzzle<4>>(any_puzzle)) {
        auto& puzzle = std::get<Puzzle<4>>(any_puzzle);
        auto [time, nodes, solution] = run_puzzle(puzzle, mem_flag);

        auto result = AnyResult(time, nodes, solution);
        return result;
    } else {
        std::cout << "Unknown variant for AnyPuzzle\n";
        exit(1);
    }
}

std::vector<AnyResult> run_puzzles(std::vector<AnyPuzzle>& puzzles, const std::string& mem_flag) {
    std::vector<AnyResult> results;
    for (auto any_puzzle : puzzles) {
        auto result = run_any_puzzle(any_puzzle, mem_flag);
        results.emplace_back(result);
    }
    return results;
}

AnyResult run_any_puzzle_task(AnyPuzzle& any_puzzle, const std::string& mem_flag, std::counting_semaphore<24>& sem) {
    sem.acquire();
    auto result = run_any_puzzle(any_puzzle, mem_flag);
    sem.release();
    return result;
}

std::vector<AnyResult> run_puzzles_parallel(std::vector<AnyPuzzle>& puzzles, const std::string& mem_flag) {
    std::counting_semaphore<24> sem(10); 

    std::vector<std::future<AnyResult>> futures;
    for (auto any_puzzle : puzzles) {
        std::future<AnyResult> future = std::async(&run_any_puzzle_task, any_puzzle, mem_flag, &sem);
        futures.emplace_back(future);
    }

    std::vector<AnyResult> results;
    for (auto& future : futures) {
        results.emplace_back(future.get());
    }
    return results;
}

template<std::size_t N = 3>
void print_solution(std::vector<Puzzle<N>> solution, int nodes, int i) {
    std::cout << "Solution for puzzle " << i + 1 << std::endl;
    for (auto &p : solution) {
        std::cout << p.get_action();
    }
    std::cout << "Solved in " << (solution.size() - 1) << " steps, expanded " << nodes << " nodes" << std::endl;
}

void print_results(std::vector<AnyResult> results) {
    for (int i = 0; i < results.size(); i++) {
        auto nodes = std::get<1>(results[i]);
        auto& any_result = std::get<2>(results[i]);

        if (std::holds_alternative<std::vector<Puzzle<3>>>(any_result)) {
            auto solution = std::get<std::vector<Puzzle<3>>>(any_result);

            print_solution(solution, nodes, i);
        } else if (std::holds_alternative<std::vector<Puzzle<4>>>(any_result)) {
            auto solution = std::get<std::vector<Puzzle<4>>>(any_result);

            print_solution(solution, nodes, i);
        } else {
            std::cout << "Unknown variant for AnyResult\n";
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Need at least 1 program argument" << std::endl;
        exit(1);
    }

    std::fstream fs_in;
    fs_in.open(argv[1]);
    if (!fs_in.is_open()) {
        std::cout << "Failed to open input file " << argv[1] << std::endl;
        exit(1);
    }

    std::string mem_flag = "arena";
    if (argc >= 3) {
        mem_flag = std::string(argv[2]);
    }
    std::string par_flag = "par";
    if (argc >= 4) {
        mem_flag = std::string(argv[3]);
    }

    auto puzzles = read_puzzles(fs_in);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<AnyResult> results;
    if (par_flag == "par") {
        results = run_puzzles_parallel(puzzles, mem_flag);
    } else if (par_flag == "seq") {
        results = run_puzzles(puzzles, mem_flag);
    } else {
        std::cout << "Flag must be par or seq, got " << par_flag << std::endl;
        exit(1);
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto eteTime = time_difference(start, stop);

    print_results(results);

    float totalTime = 0;
    int totalNodes = 0;
    for (int i = 0; i < results.size(); i++) {
        auto& [time, nodes, _] = results[i];
        std::cout << "Puzzle " << (i + 1) << ": " << time << " ms, " << nodes << " nodes" << std::endl;
        totalTime += time;
        totalNodes += nodes;
    }

    std::cout << "Total: " << totalTime << " ms, " << totalNodes << " nodes" << std::endl;

    std::cout << "End-to-end: " << eteTime << "ms " << std::endl;
    return 0;
}
