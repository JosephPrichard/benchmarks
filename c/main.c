//
// Joseph Prichard 2023
//

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include "puzzle.h"

#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
#endif

#define MAX_LINE 128

int get_num_cores() {
    int num_cores = 16;
    #ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        num_cores = sysinfo.dwNumberOfProcessors;
    #elif __linux__
        num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    #endif
    return num_cores;
}

int int_sqrt(int size) {
    float sqf = sqrtf((float) size);
    float sq_flf = floorf(sqf);
    if (sqf - sq_flf != 0) {
        return -1;
    }
    return (int) sq_flf;
}

void init_run(Run* run, int size) {
    run->time = 0;
    run->steps = 0;
    run->nodes = 0;
    run->rows = int_sqrt(size);
    if (run->rows < 0) {
        printf("Board size must be a perfect square, %d is not\n", size);
        exit(1);
    }
}

int is_space_string(char* str) {
    for (int i = 0; str[i] != 0; i++) {
        if (!isspace(str[i])) {
            return 0;
        }
    }
    return 1;
}

typedef struct {
    Run* mem;
    int size;
    int capacity;
} Runs;

Runs parse_inputs(FILE* input_file) {
    Runs runs;
    runs.size = 0;
    runs.capacity = 10;

    runs.mem = (Run*) malloc(sizeof(Run) * runs.capacity);
    if (runs.mem == NULL) {
        printf("Failed to allocate runs array");
        exit(1);
    }

    int tile_index = 0;

    char line[MAX_LINE] = {0};
    while (fgets(line, sizeof(line), input_file)) {
        if (is_space_string(line)) {
            init_run(&runs.mem[runs.size], tile_index);
            tile_index = 0;
            runs.size += 1;

            if (runs.size >= runs.capacity) {
                runs.capacity *= 2;
                Run* next_runs = (Run*) realloc(runs.mem, sizeof(Run) * runs.capacity);
                if (next_runs == NULL) {
                    printf("Failed to reallocate runs array");
                    exit(1);
                }
                runs.mem = next_runs;
            }
        } else {
            char* tok;
            char* delim = " \n";
            tok = strtok(line, delim);

            while (tok != NULL) {
                if (is_space_string(tok)) {
                    tok = strtok(NULL, delim);
                    continue;
                }

                Tile tile = (Tile) strtol(tok, NULL, 10);
                if (errno) {
                    printf("Failed to parse a token to tile %s with errno %d\n", tok, errno);
                    exit(1);
                }
            
                if (tile_index >= MAX_SIZE){
                    printf("A puzzle must have no more than %d tiles\n", tile_index);
                    exit(1);
                }

                runs.mem[runs.size].initial_brd[tile_index] = tile;
                tile_index += 1;

                tok = strtok(NULL, delim);
            }
        }
        memset(line, 0, MAX_LINE);
    }

    return runs;
}

double get_time(struct timeval* start, struct timeval* end) {
    long elapsed_usec = (end->tv_sec - start->tv_sec) * 1000000 + (end->tv_usec - start->tv_usec);
    return ((double) elapsed_usec) / 1000.0;
}

void find_paths(Runs runs) {
    for (int i = 0; i < runs.size; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        Run* run = &runs.mem[i];
        solve(run);

        gettimeofday(&end, NULL);
        run->time = get_time(&start, &end);
    }
}

typedef struct {
    Runs runs;
    int index;
    pthread_mutex_t mu;
} TaskPool;

TaskPool* new_tp(Runs runs) {
    TaskPool* task_pool = (TaskPool*) malloc(sizeof(TaskPool));
    if (task_pool == NULL) {
        printf("Failed to allocate task pool\n");
        exit(1);
    }
    task_pool->index = 0;
    task_pool->runs = runs;
    
    int err = pthread_mutex_init(&task_pool->mu, NULL);
    if (err != 0) {
        printf("Failed to initialize the mutex lock\n");
        exit(1);
    }
    return task_pool;
}

Run* take_task(TaskPool* pool) {
    int err = pthread_mutex_lock(&pool->mu);
    if (err != 0) {
        printf("Failed to lock task pool mutex: err %d\n", err);
        exit(1);
    }

    Run* run = NULL;
    if (pool->index < pool->runs.size) {
        run = &pool->runs.mem[pool->index++];
    }

    err = pthread_mutex_unlock(&pool->mu);
    if (err != 0) {
        printf("Failed to unlock task pool mutex: err %d\n", err);
        exit(1);
    }
    return run;
}

void* do_task(void* arg) {
    TaskPool* task_pool = (TaskPool*) arg;
    for (;;) {
        Run* run = take_task(task_pool);
        if (run == NULL) {
            // we stop the thread when we run out of tasks aka we cannot take anymore
            return NULL;
        }
        
        struct timeval start, end;
        gettimeofday(&start, NULL);

        solve(run);

        gettimeofday(&end, NULL);
        run->time = get_time(&start, &end);
    }
}

void find_paths_parallel(Runs runs) {
    int thread_count = get_num_cores();
    pthread_t threads[thread_count];

    TaskPool* task_pool = new_tp(runs);

    for (int i = 0; i < thread_count; i++) {
        int err = pthread_create(&threads[i], NULL, do_task, task_pool);
        if (err != 0) {
            printf("Fail to create pthread %d: err %d\n", i, err);
            exit(1);
        }
    }
    for (int i = 0; i < thread_count; i++) {
        int err = pthread_join(threads[i], NULL);
        if (err != 0) {
            printf("Fail to join pthread %d: err %d\n", i, err);
            exit(1);
        }
    }

    free(task_pool);
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
        return 1;
    }

    char* flag = "seq";
    if (argc >= 3) {
        flag = argv[2];
    }

    Runs runs = parse_inputs(input_file);
    fclose(input_file);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (strcmp(flag, "seq") == 0) {
        find_paths(runs);
    } else if (strcmp(flag, "par") == 0) {
        find_paths_parallel(runs);
    } else {
        printf("Parallelism flag must be seq or par, got %s\n", flag);
        return 1;
    }

    gettimeofday(&end, NULL);
    double ete_time = get_time(&start, &end);

    for (int i = 0; i < runs.size; i++) {
        printf("Solution for puzzle %d\n", i + 1);
        Run* run = &runs.mem[i];
        print_solution(run);
        free(run->solution);
    }

    int total_nodes = 0;
    double total_time = 0;
    for (int i = 0; i < runs.size; i++) {
        Run* run = &runs.mem[i];
        printf("Puzzle %d: %f ms, %d nodes\n", i + 1, run->time, run->nodes);
        total_time += run->time;
        total_nodes += run->nodes;
    }

    free(runs.mem);

    printf("\nTotal: %f ms, %d nodes\n", total_time, total_nodes);
    printf("End-to-end: %f ms", ete_time);
    return 0;
}