//
// Joseph Prichard 2023
//

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "puzzle.h"

#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
#endif

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

void init_input(Run* input, int size) {
    input->rows = int_sqrt(size);
    if (input->rows < 0) {
        printf("Board rows must be a perfect square\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        input->goal_brd[i] = (Tile) i;
    }
}

int parse_inputs(Run inputs[MAX_RUNS], FILE* input_file) {
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
                Tile t = (Tile) strtol(tok, NULL, 10);
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

double get_time(struct timeval* start, struct timeval* end) {
    long elapsed_usec = (end->tv_sec - start->tv_sec) * 1000000 + (end->tv_usec - start->tv_usec);
    return ((double) elapsed_usec) / 1000.0;
}

void find_paths(Run runs[], int count) {
    for (int i = 0; i < count; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        Run* run = &runs[i];
        solve(run);

        gettimeofday(&end, NULL);
        run->time = get_time(&start, &end);
    }
}

typedef struct Task {
    Run* run;
    sem_t* sem;
    int i;
} Task;

void* find_path_task(void* arg) {
    Task* task = (Task*) arg;
    sem_wait(task->sem);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    solve(task->run);

    gettimeofday(&end, NULL);
    task->run->time = get_time(&start, &end);

    sem_post(task->sem);
    return NULL;
}

int get_num_cores() {
    int num_cores = 16;
    #ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        num_cores = sysinfo.dwNumberOfProcessors;
    #elif __linux__
        num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    #endif
    printf("System cores: %d\n", num_cores);
    return num_cores;
}

void find_paths_parallel(Run runs[], int count) {
    pthread_t threads[count];

    sem_t sem;
    sem_init(&sem, 0, get_num_cores());

    Task* tasks = malloc(count * sizeof(Task));
    if (tasks == NULL) {
        printf("Failed to allocate tasks\n");
        exit(1);
        return;
    }

    for (int i = 0; i < count; i++) {
        Task task = {
            .run = &runs[i], 
            .sem = &sem,
            .i = i
        };
        tasks[i] = task;
        pthread_create(&threads[i], NULL, find_path_task, &tasks[i]);
    }
    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }

    free(tasks);
    sem_destroy(&sem);
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

    char* flag = "seq";
    if (argc >= 3) {
        flag = argv[2];
    }

    Run runs[MAX_RUNS] = {0};
    int count = parse_inputs(runs, input_file);
    fclose(input_file);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (strcmp(flag, "seq") == 0) {
        find_paths(runs, count);
    } else if (strcmp(flag, "par") == 0) {
        find_paths_parallel(runs, count);
    } else {
        printf("Parallelism flag must be seq or par, got %s\n", flag);
        return 1;
    }

    gettimeofday(&end, NULL);
    double ete_time = get_time(&start, &end);

    for (int i = 0; i < count; i++) {
        printf("Solution for puzzle %d\n", i + 1);
        print_solution(&runs[i]);
    }

    int total_nodes = 0;
    double total_time = 0;
    for (int i = 0; i < count; i++) {
        double time = runs[i].time;
        int nodes = runs[i].nodes;

        printf("Puzzle %d: %f ms, %d nodes\n", i + 1, time, nodes);
        total_time += time;
        total_nodes += nodes;
    }

    printf("\nTotal: %f ms, %d nodes\n", total_time, total_nodes);

    printf("End-to-end: %f ms", ete_time);
    return 0;
}