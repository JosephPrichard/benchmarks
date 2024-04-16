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

std::vector<std::vector<Tile>> read_puzzles(std::istream& is) {
    std::vector<std::vector<Tile>> puzzles;
    std::vector<Tile> curr_tiles;

    std::string line;

    while (is.good()) {
        getline(is, line);

        if (is_space_string(line)) {
           if (!curr_tiles.empty()) {
               puzzles.emplace_back(curr_tiles);
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

void run_puzzles(std::vector<std::unique_ptr<Solution>>& solutions, std::vector<std::vector<Tile>>& puzzles) {
    for (auto& tiles : puzzles) {
        auto start = std::chrono::high_resolution_clock::now();

        auto solution = find_path(tiles);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        solution->time = static_cast<float>(duration.count()) / 1000.0f;

        solutions.emplace_back(std::move(solution));
    }
}

void run_puzzles_parallel(std::vector<std::unique_ptr<Solution>>& solutions, std::vector<std::vector<Tile>>& puzzles) {
    unsigned int cores = std::thread::hardware_concurrency();
    unsigned int count = std::min(cores, 64u);

    std::counting_semaphore<64u> sem(10);

    std::vector<std::future<std::unique_ptr<Solution>>> futures;
    for (int i = 0; i < puzzles.size(); i++) {
        sem.acquire();
        auto future = std::async(std::launch::async, [](std::vector<Tile>* tiles, std::counting_semaphore<64u>* sem){
            auto start = std::chrono::high_resolution_clock::now();

            auto solution = find_path(*tiles);

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            solution->time = static_cast<float>(duration.count()) / 1000.0f;

            sem->release();

            return solution;
        }, &puzzles[i], &sem);
        futures.emplace_back(std::move(future));
    }

    for (auto& future : futures) {
        solutions.emplace_back(std::move(future.get()));
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
    auto puzzles = read_puzzles(fs_in);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::unique_ptr<Solution>> solutions;

    switch (par_flag) {
        case SEQ: {
            run_puzzles(solutions, puzzles);
            break;
        }
        case PAR: {
            run_puzzles_parallel(solutions, puzzles);
            break;
        }
    }
    
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    auto eteTime = ((float) duration.count()) / 1000.0f;

    print_solutions(solutions);

    printf("End-to-end: %.2fms\n", eteTime);
    return 0;
}