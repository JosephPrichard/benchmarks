#include <iostream>
#include <fstream>
#include <chrono>
#include <variant>
#include <thread>
#include <future>
#include <semaphore>
#include <cstring>
#include "puzzle.hpp"

bool is_space_string(std::string& str) {
    for (char c : str) {
        if (!isspace(c)) {
            return false;
        }
    }
    return true;
}

unsigned int int_sqrt(unsigned int size) {
    float sqf = std::sqrt(static_cast<float>(size));
    float sq_flf = std::floor(sqf);
    if (sqf - sq_flf != 0) {
        return -1;
    }
    return static_cast<unsigned int>(sq_flf);
}

std::vector<PuzzleInput> read_inputs(std::istream& is) {
    std::vector<PuzzleInput> puzzles;
    std::vector<Tile> curr_tiles;

    std::string line;

    while (is.good()) {
        getline(is, line);

        if (is_space_string(line)) {
           if (!curr_tiles.empty()) {
                unsigned int n = int_sqrt(curr_tiles.size());
                if (n <= 0) {
                    printf("Size must be a perfect square, got %zu", curr_tiles.size());
                    exit(1);
                }

                PuzzleInput input{};
                input.n = n;
                for (int i = 0; i < curr_tiles.size(); i++) {
                    input.tiles[i] = curr_tiles[i];
                }

                puzzles.emplace_back(input);
                curr_tiles.clear();
           }
        } else {
            std::string token;
            for (;;) {
                std::string* toparse = &line;
                unsigned long long pos = line.find(' ');
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
                    catch(std::invalid_argument& e) {
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

void run_puzzles_parallel(std::vector<PuzzleInput>& inputs, std::vector<std::unique_ptr<Solution>>& solutions) {
    unsigned int threads_count = std::thread::hardware_concurrency();

    for (int i = 0; i < inputs.size(); i++) {
        solutions.push_back(nullptr);
    }

    std::vector<std::jthread> threads;
    std::atomic<int> index = 0;

    for (int i = 0; i < threads_count; i++) {
        std::jthread thread([&index, &inputs, &solutions](){
            for (;;) {
                int curr_index = index++;
                if (curr_index >= inputs.size()) {
                    return;
                }
                auto& tiles = inputs[curr_index];
                
                auto start = std::chrono::steady_clock::now();

                auto solution = find_path(tiles);

                auto stop = std::chrono::steady_clock::now();
                solution->time = std::chrono::duration<double, std::milli>(stop - start).count();

                solutions[curr_index] = std::move(solution);
            }
        });
        threads.emplace_back(std::move(thread));
    }
}

void print_solutions(std::vector<std::unique_ptr<Solution>>& solutions) {
    for (int i = 0; i < solutions.size(); i++) {
        auto& solution = solutions[i];

        printf("Solution for puzzle %d\n", i + 1);
        for (auto &p : solution->path) {
            print_action(p.action);
        }
        printf("Solved in %zu steps, expanded %d nodes\n\n", solution->path.size() - 1, solution->nodes);
    }

    float totalTime = 0;
    int totalNodes = 0;
    for (int i = 0; i < solutions.size(); i++) {
        auto& solution = solutions[i];

        printf("Puzzle %d: %f ms, %d nodes\n", (i + 1), solution->time, solution->nodes);
        totalTime += solution->time;
        totalNodes += solution->nodes;
    }
    printf("\nTotal: %.2f ms, %d nodes\n", totalTime, totalNodes);
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
    auto inputs = read_inputs(fs_in);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::unique_ptr<Solution>> solutions;

    switch (par_flag) {
        case SEQ: {
            for (auto& tiles : inputs) {
                auto start = std::chrono::steady_clock::now();
    
                auto solution = find_path(tiles);

                auto stop = std::chrono::steady_clock::now();
                solution->time = std::chrono::duration<double, std::milli>(stop - start).count();

                solutions.emplace_back(std::move(solution));
            }
            break;
        }
        case PAR: {
            run_puzzles_parallel(inputs, solutions);
            break;
        }
    }
    
    auto stop = std::chrono::steady_clock::now();
    auto eteTime = std::chrono::duration<double, std::milli>(stop - start);

    print_solutions(solutions);

    printf("End-to-end: %.2fms\n", eteTime.count());
    return 0;
}