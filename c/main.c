//
// Joseph Prichard 2023
//

#include <stdio.h>
#include "puzzle.h"

#define MAX_RUNS 1000
#define MAX_LINE 128

int int_sqrt(int size) {
    float sqf = sqrtf((float) size);
    float sq_flf = floorf(sqf);
    if (sqf - sq_flf != 0) {
        return -1;
    }
    return (int) sq_flf;
}

void init_input(board_input_t* input, int size) {
    input->rows = int_sqrt(size);
    if (input->rows < 0) {
        printf("Board rows must be a perfect square\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        input->goal_brd[i] = (tile_t) i;
    }
}

int parse_inputs(board_input_t inputs[MAX_RUNS], FILE* input_file) {
    int board_index = 0;
    int tile_index = 0;

    char line[MAX_LINE] = {0};
    while (fgets(line, sizeof(line), input_file)) {
        if (strcmp(line, "\n") == 0) {
            if (board_index >= MAX_RUNS) {
                printf("Maximum of %d inputs is allowed\n", MAX_RUNS);
                exit(1);
            }
            init_input(&inputs[board_index], tile_index);

            tile_index = 0;
            board_index++;
        } else {
            char* tok;
            char* delim = " \n";
            tok = strtok(line, delim);

            while (tok != NULL) {
                tile_t t = (tile_t) strtol(tok, NULL, 10);
                if (errno) {
                    printf("Failed to parse a token to tile_t %s with errno %d\n", tok, errno);
                    exit(1);
                }

                if (tile_index >= MAX_SIZE){
                    printf("A puzzle must have no more than %d tiles\n", tile_index);
                    exit(1);
                }

                inputs[board_index].initial_brd[tile_index] = t;
                tile_index++;

                tok = strtok(NULL, delim);
            }
        }
        memset(line, 0, MAX_LINE);
    }

    return board_index;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Need at least 1 program arguments\n");
        return 1;
    }

    char* file_path = argv[1];
    FILE* input_file = fopen(file_path, "r");
    if (input_file == NULL) {
        printf("Failed to read input file %s\n", file_path);
        exit(1);
    }

    board_input_t inputs[MAX_RUNS] = {0};
    int count = parse_inputs(inputs, input_file);
    fclose(input_file);

    result_t results[MAX_RUNS];

    for (int i = 0; i < count; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        results[i] = solve(inputs[i]);

        gettimeofday(&end, NULL);

        long elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        results[i].time = ((double) elapsed_usec) / 1000.0;
    }

    for (int i = 0; i < count; i++) {
        printf("Solution for puzzle %d\n", i + 1);
        print_solution(results[i], inputs[i].rows);
    }

    int total_nodes = 0;
    double total_time = 0;
    for (int i = 0; i < count; i++) {
        printf("Puzzle %d: %f ms, %d nodes\n", i + 1, results[i].time, results[i].nodes);
        total_time += results[i].time;
        total_nodes += results[i].nodes;
    }

    printf("Total: %f ms, %d nodes\n", total_time, total_nodes);

    return 0;
}