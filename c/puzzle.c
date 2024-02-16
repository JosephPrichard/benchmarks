//
// Joseph Prichard 2023
//

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define SIZE 9
#define ROWS 3
#define tile_t char
#define NEIGHBOR_CNT 4
#define CHILD_CNT 4
#define LF_THRESHOLD 0.7f
#define MAX_RUNS 1000
#define MAX_LINE 128
#define ARENA_SIZE 409600

// TYPE AND FUNCTION DEFINITIONS

typedef enum move_t {
    NONE, UP, DOWN, LEFT, RIGHT
} move_t;

typedef tile_t board_t[SIZE];

typedef struct puzzle_t {
    struct puzzle_t* parent;
    board_t board;
    move_t move;
    int g;
    int f;
} puzzle_t;

typedef struct action_t {
    board_t board;
    move_t move;
} action_t;

typedef struct result_t {
    action_t* solution;
    int steps;
    int time;
} result_t;

typedef struct priority_q_t {
    puzzle_t** min_heap;
    int size;
    int capacity;
} priority_q_t;

typedef struct hash_table_t {
    int* table;
    int size;
    int capacity;
} hash_table_t;

typedef struct arena_t {
    struct arena_t* prev;
    char mem[ARENA_SIZE];
    int offset;
} arena_t;

static const board_t GOAL_BRD = {0, 1, 2, 3, 4, 5, 6, 7, 8};
static const int NEIGHBOR_OFFSETS[NEIGHBOR_CNT][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
static const int NEIGHBOR_MOVES[NEIGHBOR_CNT] = {RIGHT, DOWN, LEFT, UP};
static const char* MOVE_STRINGS[] = {"Start", "Up", "Down", "Left", "Right"};

// ARENA IMPLEMENTATION

arena_t* new_arena() {
    arena_t* a = malloc(sizeof(arena_t));
    if (a == NULL) {
        printf("Failed to allocate arena");
        exit(1);
    }
    memset(a->mem, 0, ARENA_SIZE);
    a->prev = NULL;
    a->offset = 0;
    return a;
}

void* arena_alloc_rec(arena_t* curr, arena_t** root, int size, int is_first) {
    int next_offset = curr->offset + size;
    if (next_offset >= ARENA_SIZE) {
        if (!is_first) {
            return NULL;
        }
        arena_t* next_a = new_arena();
        next_a->prev = curr;
        *root = next_a;
        return arena_alloc_rec(next_a, root, size, 0);
    } else {
        int last_offset = curr->offset;
        curr->offset += size;
        return curr->mem + last_offset;
    }
}

void* arena_alloc(arena_t** a, int size) {
    return arena_alloc_rec(*a, a, size, 1);
}

void free_arena(arena_t* a) {
    if (a != NULL) {
        free_arena(a->prev);
        free(a);
    }
}

// HASH TABLE IMPLEMENTATION

int is_prime(int n) {
    // iterate from 2 to sqrt(n)
    for (int i = 2; i <= sqrt(n); i++) {
        // if n is divisible by any number between 2 and n/2, it is not prime
        if (n % i == 0) {
            return 0;
        }
    }
    if (n <= 1)
        return 0;
    return 1;
}

int next_prime(int n) {
    for (int i = n;; i++) {
        if (is_prime(i)) {
            return i;
        }
    }
}

hash_table_t* new_ht() {
    hash_table_t* ht = malloc(sizeof(hash_table_t));
    if (ht == NULL) {
        printf("Failed to allocate hash_table_t");
        exit(1);
    }
    ht->capacity = next_prime(10);
    ht->table = calloc(ht->capacity, sizeof(int));
    ht->size = 0;
    if (ht->table == NULL) {
        printf("Failed to allocate hash_table_t table");
        exit(1);
    }
    return ht;
}

int hash_board(const board_t board) {
    // hash function takes each symbol in the puzzle_t from start to end as a digit
    int hash = 0;
    for (int i = 0; i < SIZE; i++) {
        hash += board[i] * (int) pow(10, i);
    }
    return hash;
}

int probe(hash_table_t* ht, int h, int i) {
    return (h + i) % ht->capacity; // linear probe
}

void probe_ht(hash_table_t* ht, int key) {
    // probe until we find a slot to insert
    for (int i = 0;; i++) {
        int p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            ht->table[p] = key;
            break;
        }
    }
}

void rehash(hash_table_t* ht) {
    // keep references to old structures before creating new structures
    int old_capacity = ht->capacity;
    int* old_table = ht->table;
    // allocate a new hash table and rehash all old elements into it
    ht->capacity = next_prime(ht->capacity * 2);
    ht->table = calloc(ht->capacity, sizeof(int));
    // check for allocation errors
    if (ht->table == NULL) {
        printf("hash_table_t reallocation failed");
        exit(1);
    }
    // add all keys from the old to the new min_heap
    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i] != 0) {
            probe_ht(ht, old_table[i]);
        }
    }
    // free the old hash table
    free(old_table);
}

void insert_into_ht(hash_table_t* ht, int key) {
    // rehash when load factor exceeds threshold
    if ((float) ht->size / (float) ht->capacity > LF_THRESHOLD) {
        rehash(ht);
    }
    probe_ht(ht, key);
    ht->size++;
}

int ht_has_key(hash_table_t* ht, int key) {
    // probe until we find a match or the first empty slot
    for (int i = 0;; i++) {
        int p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            // probed until empty slot so board_t isn't in table
            return 0;
        } else if (ht->table[p] == key) {
            // hash values match so board_t is in table
            return 1;
        }
    }
}

// PQ IMPLEMENTATION

priority_q_t* new_pq() {
    priority_q_t* pq = malloc(sizeof(priority_q_t));
    if (pq == NULL) {
        printf("Failed to allocate priority_q_t");
        exit(1);
    }
    pq->capacity = 10;
    pq->min_heap = malloc(sizeof(puzzle_t) * pq->capacity);
    pq->size = 0;
    if (pq->min_heap == NULL) {
        printf("Failed to allocate priority_q_t heap");
        exit(1);
    }
    return pq;
}

void ensure_capacity(priority_q_t* pq) {
    // ensure min_heap's capacity is large enough
    if (pq->size >= pq->capacity) {
        pq->capacity = pq->capacity * 2;
        puzzle_t** min_heap = realloc(pq->min_heap, sizeof(puzzle_t) * pq->capacity);
        // check for allocation errors
        if (min_heap == NULL) {
            printf("priority_q_t reallocation failed");
            exit(1);
        }
        pq->min_heap = min_heap;
    }
}

void push_pq(priority_q_t* pq, puzzle_t* puz) {
    ensure_capacity(pq);
    // add element to end of min_heap
    pq->min_heap[pq->size] = puz;
    // sift the min_heap up
    int pos = pq->size;
    int parent = (pos - 1) / CHILD_CNT;
    // sift up until parent score is larger
    while (parent >= 0) {
        if (pq->min_heap[pos]->f < pq->min_heap[parent]->f) {
            // swap parent with child
            puzzle_t* temp = pq->min_heap[pos];
            pq->min_heap[pos] = pq->min_heap[parent];
            pq->min_heap[parent] = temp;
            // climb up the tree
            pos = parent;
            parent = (pos - 1) / CHILD_CNT;
        } else {
            parent = -1;
        }
    }
    pq->size++;
}

puzzle_t* pop_pq(priority_q_t* pq) {
    // check for empty min_heap
    if (pq->size == 0) {
        printf("Can't pop an empty min_heap");
        exit(1);
    }
    // extract top element and move_t bottom to top
    puzzle_t* top = pq->min_heap[0];
    pq->min_heap[0] = pq->min_heap[pq->size - 1];
    // sift top element down
    int pos = 0;
    for (;;) {
        // get the smallest child
        int first_child = CHILD_CNT * pos + 1;
        int child = first_child;
        if (child >= pq->size) {
            break;
        }
        // iterate from leftmost to rightmost child to find the smallest at level
        for (int i = 1; i < CHILD_CNT; i++) {
            int new_child = first_child + i;
            if (new_child >= pq->size) {
                break;
            }
            if(pq->min_heap[new_child]->f < pq->min_heap[child]->f) {
                child = new_child;
            }
        }
        // swap child with parent if child is smaller
        if (pq->min_heap[pos]->f > pq->min_heap[child]->f) {
            // swap parent with child
            puzzle_t* temp = pq->min_heap[pos];
            pq->min_heap[pos] = pq->min_heap[child];
            pq->min_heap[child] = temp;
            // climb down tree
            pos = child;
        } else {
            break;
        }
    }
    pq->size--;
    return top;
}

// PUZZLE SOLVER IMPLEMENTATION

puzzle_t* new_puzzle(arena_t** arena, const board_t brd) {
    puzzle_t* puz = arena_alloc(arena, sizeof(puzzle_t));
    if (puz == NULL) {
        printf("Failed to allocate puzzle_t on arena");
        exit(1);
    }
    memcpy(puz->board, brd, sizeof(board_t));
    puz->move = NONE;
    puz->parent = NULL;
    puz->f = 0;
    puz->g = 0;
    return puz;
}

int find_zero(const board_t brd) {
    for (int i = 0; i < SIZE; i++)
        if (brd[i] == 0)
            return i;
    printf("board_t doesn't contain 0");
    exit(1);
}

int move_board(board_t brd_in, board_t brd_out, int row_offset, int col_offset) {
    // copy input to output (overrides output board_t)
    memcpy(brd_out, brd_in, sizeof(board_t));
    // find the location of the zero on the board_t
    int zero_loc = find_zero(brd_in);
    int zero_row = zero_loc / ROWS;
    int zero_col = zero_loc % ROWS;
    // find the location of the tile_t to be swapped
    int swap_row = zero_row + row_offset;
    int swap_col = zero_col + col_offset;
    int swap_loc = swap_col + ROWS * swap_row;
    // check if puzzle_t is out of bounds
    if (swap_row < 0 || swap_row >= ROWS || swap_col < 0 || swap_col >= ROWS) {
        return 1;
    }
    // swap location of 0 with new location
    tile_t temp = brd_out[zero_loc];
    brd_out[zero_loc] = brd_out[swap_loc];
    brd_out[swap_loc] = temp;
    return 0;
}

int heuristic(const board_t brd) {
    int h = 0;
    for (int i = 0; i < SIZE; i++) {
        int row1 = i / ROWS;
        int col1 = i % ROWS;
        int row2 = brd[i] / ROWS;
        int col2 = brd[i] % ROWS;
        int manhattan_distance = abs(row2 - row1) + abs(col2 - col1);
        h += manhattan_distance;
    }
    return h;
}

void reconstruct_path(puzzle_t* leaf_puz, result_t* result) {
    int size = 10;
    result->solution = malloc(sizeof(action_t) * size);
    if (result->solution == NULL) {
        printf("Failed to allocate solution buffer");
        exit(1);
    }

    int i;
    for(i = 0; leaf_puz != NULL; i++) {
        if (i >= size) {
            size *= 2;
            action_t* new_solution = realloc(result->solution, sizeof(action_t) * size);
            if (new_solution == NULL) {
                printf("Failed to reallocate solution buffer");
                exit(1);
            }
            result->solution = new_solution;
        }

        memcpy(result->solution[i].board, leaf_puz->board, sizeof(board_t));
        result->solution[i].move = leaf_puz->move;

        leaf_puz = leaf_puz->parent;
    }
   result->steps = i;
}

result_t solve(const board_t initial_brd) {
    result_t result = {};

    int goal_hash = hash_board(GOAL_BRD);

    arena_t* arena = new_arena();

    puzzle_t* root = new_puzzle(&arena, initial_brd);
    priority_q_t* open_set = new_pq();
    hash_table_t* closed_set = new_ht();

    push_pq(open_set, root);

    // iterate until we find a solution
    while(open_set->size > 0) {
        // pop off the state with the best heuristic
        puzzle_t* current_puz = pop_pq(open_set);
        int current_hash = hash_board(current_puz->board);
        insert_into_ht(closed_set, current_hash);

        // check if we've reached the goal state
        if (current_hash == goal_hash) {
            reconstruct_path(current_puz, &result);
            break;
        }

        // add neighbor states to the priority queue
        for (int i = 0; i < NEIGHBOR_CNT; i++) {
            board_t neighbor_board = {0};
            int row_offset = NEIGHBOR_OFFSETS[i][0];
            int col_offset = NEIGHBOR_OFFSETS[i][1];
            // write a moved board_t state into the neighbor board_t, then check for error states and if neighbor bord is closed
            if (move_board(current_puz->board, neighbor_board, row_offset, col_offset) != 0) {
                continue;
            }
            int not_visited = !ht_has_key(closed_set, hash_board(neighbor_board));
            if (not_visited) {
                // create a new neighbor with the new board_t and calculated states
                puzzle_t* neighbor_puz = new_puzzle(&arena, neighbor_board);
                neighbor_puz->parent = current_puz;
                neighbor_puz->g = current_puz->g + 1;
                neighbor_puz->f = neighbor_puz->g + heuristic(neighbor_board);
                neighbor_puz->move = NEIGHBOR_MOVES[i];

                // add neighbor board_t to pq
                push_pq(open_set, neighbor_puz);
            }
        }
    }

    free(closed_set->table);
    free(closed_set);
    free(open_set->min_heap);
    free(open_set);
    free_arena(arena);

    return result;
}

void print_board(const board_t brd) {
    for (int i = 0; i < SIZE; i++) {
        if (brd[i] != 0) {
            printf("%d ", brd[i]);
        } else {
            printf("  ");
        }
        if ((i + 1) % 3 == 0) {
            printf("\n");
        }
    }
}

void print_solution(action_t solution[], int count) {
    for (int i = count - 1; i >= 0; i--) {
        printf("%s\n", MOVE_STRINGS[solution[i].move]);
        print_board(solution[i].board);
    }
    printf("Solved in %d steps\n", count - 1);
}

int parse_boards(board_t boards[MAX_RUNS], FILE* input_file) {
    int board_index = 0;
    int tile_index = 0;

    char line[MAX_LINE] = {0};
    while (fgets(line, sizeof(line), input_file)) {
        if (strcmp(line, "\n") == 0) {
            tile_index = 0;
            board_index++;
            if (board_index >= MAX_RUNS) {
                printf("Maximum of %d input boards is allowed", MAX_RUNS);
                exit(1);
            }
        } else {
            char* tok;
            char* delim = " \n";
            tok = strtok(line, delim);

            while (tok != NULL) {
                tile_t t = (tile_t) strtol(tok, NULL, 10);
                if (errno) {
                    printf("Failed to parse a token to tile_t %s with errno %d", tok, errno);
                    exit(1);
                }

                boards[board_index][tile_index] = t;
                tile_index++;

                tok = strtok(NULL, delim);
            }
        }
        memset(line, 0, MAX_LINE);
    }

    return board_index;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Need at least 1 program argument");
        return 1;
    }

    char* file_path = argv[1];
    FILE* input_file = fopen(file_path, "r");
    if (input_file == NULL) {
        printf("Failed to open input file %s.", file_path);
        exit(1);
    }

    board_t initial_boards[MAX_RUNS] = {0};
    int count = parse_boards(initial_boards, input_file);
    fclose(input_file);

    printf("Running for %d puzzle input(s)...\n", count);

    result_t results[MAX_RUNS];
    for (int i = 0; i < count; i++) {
        clock_t start_time = clock();
        results[i] = solve(initial_boards[i]);
        results[i].time = clock() - start_time;
    }

    for (int i = 0; i < count; i++) {
        printf("\nSolution for puzzle %d\n", i + 1);
        print_solution(results[i].solution, results[i].steps);
        free(results[i].solution);
    }

    int total_time = 0;
    for (int i = 0; i < count; i++) {
        printf("\nPuzzle %d took %d ms", i + 1, results[i].time);
        total_time += results[i].time;
    }
    printf("\nTook %d ms in total\n", total_time);

    return 0;
}