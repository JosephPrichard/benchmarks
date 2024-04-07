//
// Joseph Prichard 2023
//

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>

#define LONGEST_SOL 100
#define MAX_SIZE (4 * 4)
#define NEIGHBOR_CNT 4
#define CHILD_CNT 2
#define LF_THRESHOLD 0.7f
#define INITIAL_ARENA_SIZE 1000

int is_prime(int n) {
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

typedef char tile_t;

typedef tile_t board_t[MAX_SIZE];

// 64 BIT HASH FOR AN ARRAY OF LENGTH 16 WHERE EACH ELEMENT IS 4 BITS
unsigned long long hash_board(const board_t brd, int size) {
    unsigned long long hash = 0;
    for (int i = 0; i < size; i++) {
        tile_t tile = brd[i];
        long long mask = ((unsigned long long) tile) << (i * 4);
        hash = (hash | mask);
    }
    return hash;
}

typedef struct hash_table_t {
    unsigned long long* table; // 64 BIT KEYS
    int size;
    int capacity;
} hash_table_t;

hash_table_t* new_ht() {
    hash_table_t* ht = (hash_table_t*) malloc(sizeof(hash_table_t));
    if (ht == NULL) {
        printf("Failed to allocate hash_table\n");
        exit(1);
    }
    ht->capacity = next_prime(10);
    ht->table = (unsigned long long*) calloc(ht->capacity, sizeof(unsigned long long));
    ht->size = 0;
    if (ht->table == NULL) {
        printf("Failed to allocate hash_table\n");
        exit(1);
    }
    return ht;
}

unsigned long long probe(hash_table_t* ht, unsigned long long h, int i) {
    return (h + (unsigned long long) i) % ((unsigned long long) ht->capacity); // linear probe
}

void probe_ht(hash_table_t* ht, unsigned long long key) {
    for (int i = 0;; i++) {
        unsigned long long p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            ht->table[p] = key;
            break;
        }
    }
}

void rehash(hash_table_t* ht) {
    int old_capacity = ht->capacity;
    unsigned long long* old_table = ht->table;

    ht->capacity = next_prime(ht->capacity * 2);
    ht->table = (unsigned long long*) calloc(ht->capacity, sizeof(unsigned long long));

    if (ht->table == NULL) {
        printf("hash_table reallocation failed\n");
        exit(1);
    }

    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i] != 0) {
            probe_ht(ht, old_table[i]);
        }
    }

    free(old_table);
}

void insert_into_ht(hash_table_t* ht, unsigned long long key) {
    if ((float) ht->size / (float) ht->capacity > LF_THRESHOLD) {
        rehash(ht);
    }
    probe_ht(ht, key);
    ht->size++;
}

int ht_has_key(hash_table_t* ht, unsigned long long key) {
    for (int i = 0;; i++) {
        unsigned long long p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            return 0;
        } else if (ht->table[p] == key) {
            return 1;
        }
    }
}

typedef struct priorityq_t {
    int* min_heap;
    int size;
    int capacity;
} priorityq_t;

priorityq_t* new_pq() {
    priorityq_t* pq = (priorityq_t*) malloc(sizeof(priorityq_t));
    if (pq == NULL) {
        printf("Failed to allocate priority queue\n");
        exit(1);
    }
    pq->capacity = 10;
    pq->min_heap = (int*) malloc(sizeof(int) * pq->capacity);
    pq->size = 0;
    if (pq->min_heap == NULL) {
        printf("Failed to allocate priority queue heap\n");
        exit(1);
    }
    return pq;
}

void ensure_capacity(priorityq_t* pq) {
    if (pq->size >= pq->capacity) {
        pq->capacity = pq->capacity * 2;
        int* min_heap = (int*) realloc(pq->min_heap, sizeof(int) * pq->capacity);
        if (min_heap == NULL) {
            printf("priority queue reallocation failed\n");
            exit(1);
        }
        pq->min_heap = min_heap;
    }
}

typedef enum move_t {
    NONE, UP, DOWN, LEFT, RIGHT
} move_t;

typedef struct puzzle_t {
    int parent_offset;
    board_t board;
    move_t move;
    int g;
    int f;
} puzzle_t;

void push_pq(priorityq_t* pq, puzzle_t* arena, int arena_offset) {
    ensure_capacity(pq);

    pq->min_heap[pq->size] = arena_offset;

    int pos = pq->size;
    int parent = (pos - 1) / CHILD_CNT;

    while (parent >= 0) {
        if (arena[pq->min_heap[pos]].f < arena[pq->min_heap[parent]].f) {

            int temp = pq->min_heap[pos];
            pq->min_heap[pos] = pq->min_heap[parent];
            pq->min_heap[parent] = temp;

            pos = parent;
            parent = (pos - 1) / CHILD_CNT;
        } else {
            parent = -1;
        }
    }
    pq->size++;
}

int pop_pq(priorityq_t* pq, puzzle_t* arena) {
    if (pq->size == 0) {
        printf("Can't pop an empty priority queue\n");
        exit(1);
    }

    int top = pq->min_heap[0];
    pq->min_heap[0] = pq->min_heap[pq->size - 1];

    int pos = 0;
    for (;;) {
        int first_child = CHILD_CNT * pos + 1;
        int child = first_child;
        if (child >= pq->size) {
            break;
        }

        for (int i = 1; i < CHILD_CNT; i++) {
            int new_child = first_child + i;
            if (new_child >= pq->size) {
                break;
            }
            if(arena[pq->min_heap[new_child]].f < arena[pq->min_heap[child]].f) {
                child = new_child;
            }
        }

        if (arena[pq->min_heap[pos]].f > arena[pq->min_heap[child]].f) {
            int temp = pq->min_heap[pos];
            pq->min_heap[pos] = pq->min_heap[child];
            pq->min_heap[child] = temp;

            pos = child;
        } else {
            break;
        }
    }
    pq->size--;
    return top;
}

static const int NEIGHBOR_OFFSETS[NEIGHBOR_CNT][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

int move_board(board_t brd_in, board_t brd_out, int zero_index, int neighbor_index, int rows) {
    memcpy(brd_out, brd_in, sizeof(board_t));

    int zero_row = zero_index / rows;
    int zero_col = zero_index % rows;

    int row_offset = NEIGHBOR_OFFSETS[neighbor_index][0];
    int col_offset = NEIGHBOR_OFFSETS[neighbor_index][1];
    int swap_row = zero_row + row_offset;
    int swap_col = zero_col + col_offset;
    int swap_index = swap_col + rows * swap_row;

    if (swap_row < 0 || swap_row >= rows || swap_col < 0 || swap_col >= rows) {
        return 1;
    }

    tile_t temp = brd_out[zero_index];
    brd_out[zero_index] = brd_out[swap_index];
    brd_out[swap_index] = temp;
    return 0;
}

int find_zero(const board_t brd, int size) {
    for (int i = 0; i < size; i++) {
        if (brd[i] == 0) {
            return i;
        }
    }
    printf("board_t doesn't contain 0");
    exit(1);
}

int heuristic(const board_t brd, int rows) {
    int h = 0;
    int size = rows * rows;
    for (int i = 0; i < size; i++) {
        tile_t tile = brd[i];
        if (tile != 0) {
            int row1 = i / rows;
            int col1 = i % rows;
            int row2 = tile / rows;
            int col2 = tile % rows;
            int manhattan_distance = abs(row2 - row1) + abs(col2 - col1);
            h += manhattan_distance;
       }
    }
    return h;
}

typedef struct action_t {
    board_t board;
    move_t move;
} action_t;

static const char* MOVE_STRINGS[] = {"Start", "Up", "Down", "Left", "Right"};

void print_board(const board_t board, int rows, FILE* file) {
    for (int i = 0; i < rows * rows; i++) {
        if (board[i] != 0) {
            fprintf(file, "%d ", board[i]);
        } else {
            fprintf(file, "  ");
        }
        if ((i + 1) % rows == 0) {
            fprintf(file, "\n");
        }
    }
}

void print_action(const action_t* action, int rows, FILE* file) {
    fprintf(file, "%s\n", MOVE_STRINGS[action->move]);
}

typedef struct result_t {
    action_t solution[LONGEST_SOL];
    int steps;
    double time;
    int nodes;
} result_t;

void print_solution(result_t result, int rows) {
    for (int i = result.steps - 1; i >= 0; i--) {
        action_t* a = &result.solution[i];
        print_action(a, rows, stdout);
    }
    printf("Solved in %d steps\n\n", result.steps - 1);
}

void reconstruct_path(puzzle_t* arena, int leaf_offset, result_t* result) {
    int i;
    for(i = 0; leaf_offset != -1; i++) {
        if (i >= LONGEST_SOL) {
            printf("An optimal solution should be no longer than %d steps\n", LONGEST_SOL);
            exit(1);
        }
        puzzle_t* puzzle = &arena[leaf_offset];
        memcpy(result->solution[i].board, puzzle->board, sizeof(board_t));
        result->solution[i].move = puzzle->move;
        leaf_offset = puzzle->parent_offset;
    }
    result->steps = i;
}

typedef struct board_input_t {
    board_t initial_brd;
    board_t goal_brd;
    int rows;
} board_input_t;

static const move_t NEIGHBOR_MOVES[NEIGHBOR_CNT] = {RIGHT, DOWN, LEFT, UP};

result_t solve(const board_input_t in) {
    result_t result = {.nodes = 0};
    int size = in.rows * in.rows;

    unsigned long long goal_hash = hash_board(in.goal_brd, size);

    int arena_size = INITIAL_ARENA_SIZE;
    int arena_offset = 0;
    puzzle_t* arena = (puzzle_t*) malloc(sizeof(puzzle_t) * arena_size);

    priorityq_t* open_set = new_pq();
    hash_table_t* closed_set = new_ht();

    puzzle_t* root = &arena[arena_offset];
    memcpy(root->board, in.initial_brd, sizeof(board_t));
    root->move = NONE;
    root->parent_offset = -1;
    root->f = 0;
    root->g = 0;

    push_pq(open_set, arena, arena_offset);
    arena_offset++;

    while(open_set->size > 0) {
        int cpuz_offset = pop_pq(open_set, arena);
        puzzle_t* cpuz = &arena[cpuz_offset];

        unsigned long long current_hash = hash_board(cpuz->board, size);
        insert_into_ht(closed_set, current_hash);

        result.nodes += 1;

        if (current_hash == goal_hash) {
            reconstruct_path(arena, cpuz_offset, &result);
            break;
        }

        int zero_index = find_zero(cpuz->board, size);

        for (int i = 0; i < NEIGHBOR_CNT; i++) {
            board_t nboard = {0};
            if (move_board(cpuz->board, nboard, zero_index, i, in.rows) != 0) {
                continue;
            }

            unsigned long long hash = hash_board(nboard, size);

            int not_visited = !ht_has_key(closed_set, hash);
            if (not_visited) {
                if (arena_offset >= arena_size) {
                    arena_size *= 2;
                    puzzle_t* next_arena = (puzzle_t*) realloc(arena, sizeof(puzzle_t) * arena_size);
                    if (arena == NULL) {
                        printf("arena reallocation failed\n");
                        exit(1);
                    }
                    arena = next_arena;
                    cpuz = &arena[cpuz_offset]; // WE NEED TO RECALCULATE THIS POINTER DUE TO REALLOC
                }

                puzzle_t* npuz = &arena[arena_offset];
                memcpy(npuz->board, nboard, sizeof(board_t));
                npuz->parent_offset = cpuz_offset;
                npuz->g = cpuz->g + 1;
                npuz->f = npuz->g + heuristic(nboard, in.rows);
                npuz->move = NEIGHBOR_MOVES[i];

                push_pq(open_set, arena, arena_offset);
                arena_offset++;
            }
        }
    }

    free(closed_set->table);
    free(closed_set);
    free(open_set->min_heap);
    free(open_set);
    free(arena);

    return result;
}