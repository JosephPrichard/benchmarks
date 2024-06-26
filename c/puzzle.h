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

#define MAX_SIZE (4 * 4)
#define NEIGHBOR_CNT 4
#define CHILD_CNT 2
#define LF_THRESHOLD 0.7f
#define INITIAL_ARENA_SIZE 1000

typedef char Tile;

typedef Tile Board[MAX_SIZE];

// 64 BIT HASH FOR AN ARRAY OF LENGTH 16 WHERE EACH ELEMENT IS 4 BITS
unsigned long long hash_board(const Board brd, int size) {
    unsigned long long hash = 0;
    for (int i = 0; i < size; i++) {
        Tile tile = brd[i];
        unsigned long long mask = ((unsigned long long) tile) << (i * 4);
        hash = (hash | mask);
    }
    return hash;
}

typedef struct {
    unsigned long long* table; // 64 BIT KEYS
    int size;
    int capacity;
} HashTable;

int is_prime(int n) {
    for(int i = 2; i <= (n / 2); i++)  {
        if(n % i == 0)
            return 0;
    }
    return 1;
}

int next_prime(int n) {
    for (int i = n;; i++) {
        if (is_prime(i)) {
            return i;
        }
    }
}

HashTable new_ht() {
    HashTable ht;
    ht.capacity = next_prime(10);
    // calloc will make sure this is 0 initialized
    ht.table = (unsigned long long*) calloc(ht.capacity, sizeof(unsigned long long));
    ht.size = 0;
    if (ht.table == NULL) {
        printf("Failed to allocate hash_table\n");
        exit(1);
    }
    return ht;
}

unsigned long long probe(HashTable* ht, unsigned long long h, int i) {
    return (h + (unsigned long long) i) % ((unsigned long long) ht->capacity); // linear probe
}

void probe_ht(HashTable* ht, unsigned long long key) {
    for (int i = 0;; i++) {
        unsigned long long p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            ht->table[p] = key;
            break;
        }
    }
}

void rehash(HashTable* ht) {
    int old_capacity = ht->capacity;
    unsigned long long* old_table = ht->table;

    ht->capacity = next_prime(ht->capacity * 2);
    ht->table = (unsigned long long*) calloc(ht->capacity, sizeof(unsigned long long));
    if (ht->table == NULL) {
        printf("hash_table allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i] != 0) {
            probe_ht(ht, old_table[i]);
        }
    }

    free(old_table);
}

void insert_into_ht(HashTable* ht, unsigned long long key) {
    if ((float) ht->size / (float) ht->capacity > LF_THRESHOLD) {
        rehash(ht);
    }
    probe_ht(ht, key);
    ht->size += 1;
}

int ht_has_key(HashTable* ht, unsigned long long key) {
    for (int i = 0;; i++) {
        unsigned long long p = probe(ht, key, i);
        if (ht->table[p] == 0) {
            return 0;
        } else if (ht->table[p] == key) {
            return 1;
        }
    }
}

typedef struct {
    int* min_heap;
    int size;
    int capacity;
} Heap;

Heap new_pq() {
    Heap heap;
    heap.size = 0;
    heap.capacity = 10;
    heap.min_heap = (int*) malloc(sizeof(int) * heap.capacity);
    if (heap.min_heap == NULL) {
        printf("Failed to allocate priority queue heap\n");
        exit(1);
    }
    return heap;
}

void ensure_capacity(Heap* heap) {
    if (heap->size >= heap->capacity) {
        heap->capacity = heap->capacity * 2;
        int* min_heap = (int*) realloc(heap->min_heap, sizeof(int) * heap->capacity);
        if (min_heap == NULL) {
            printf("priority queue reallocation failed\n");
            exit(1);
        }
        heap->min_heap = min_heap;
    }
}

typedef enum {
    NONE, UP, DOWN, LEFT, RIGHT
} Move;

typedef struct {
    int parent_offset;
    Board board;
    Move move;
    int g;
    int f;
} Puzzle;

typedef struct {
    int size;
    int offset;
    Puzzle* mem;
} Arena;

Arena new_arena() {
    Arena arena;
    arena.size = INITIAL_ARENA_SIZE;
    arena.offset = 0;
    arena.mem = (Puzzle*) malloc(sizeof(Puzzle) * arena.size);
    if (arena.mem == NULL) {
        printf("Failed to allocate memory for arena\n");
        exit(1);
    }
    return arena;
}

void expand_arena(Arena* arena) {
    if (arena->offset >= arena->size) {
        arena->size *= 2;
        Puzzle* next_mem = (Puzzle*) realloc(arena->mem, sizeof(Puzzle) * arena->size);
        if (next_mem == NULL) {
            printf("arena reallocation failed\n");
            exit(1);
        }
        arena->mem = next_mem;
    }
}

void push_heap(Heap* heap, Arena* arena, int offset) {
    ensure_capacity(heap);

    heap->min_heap[heap->size] = offset;

    int pos = heap->size;
    int parent = (pos - 1) / CHILD_CNT;

    while (parent >= 0) {
        Puzzle* lpuz = &arena->mem[heap->min_heap[pos]];
        Puzzle* rpuz = &arena->mem[heap->min_heap[parent]];

        if (lpuz->f < rpuz->f) {
            int temp = heap->min_heap[pos];
            heap->min_heap[pos] = heap->min_heap[parent];
            heap->min_heap[parent] = temp;

            pos = parent;
            parent = (pos - 1) / CHILD_CNT;
        } else {
            parent = -1;
        }
    }
    heap->size += 1;
}

int pop_heap(Heap* heap, Arena* arena) {
    if (heap->size == 0) {
        printf("Can't pop an empty priority queue\n");
        exit(1);
    }

    int top = heap->min_heap[0];
    heap->min_heap[0] = heap->min_heap[heap->size - 1];

    int pos = 0;
    for (;;) {
        int first_child = CHILD_CNT * pos + 1;
        int child = first_child;
        if (child >= heap->size) {
            break;
        }

        for (int i = 1; i < CHILD_CNT; i++) {
            int new_child = first_child + i;
            if (new_child >= heap->size) {
                break;
            }

            Puzzle* lpuz = &arena->mem[heap->min_heap[new_child]];
            Puzzle* rpuz = &arena->mem[heap->min_heap[child]];
            if(lpuz->f < rpuz->f) {
                child = new_child;
            }
        }

        Puzzle* lpuz = &arena->mem[heap->min_heap[pos]];
        Puzzle* rpuz = &arena->mem[heap->min_heap[child]];
        if (lpuz->f > rpuz->f) {
            int temp = heap->min_heap[pos];
            heap->min_heap[pos] = heap->min_heap[child];
            heap->min_heap[child] = temp;

            pos = child;
        } else {
            break;
        }
    }
    heap->size -= 1;
    return top;
}

static const int NEIGHBOR_OFFSETS[NEIGHBOR_CNT][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

int move_board(Board brd_in, Board brd_out, int zero_index, int neighbor_index, int rows) {
    memcpy(brd_out, brd_in, sizeof(Board));

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

    Tile temp = brd_out[zero_index];
    brd_out[zero_index] = brd_out[swap_index];
    brd_out[swap_index] = temp;
    return 0;
}

int find_zero(const Board brd, int size) {
    for (int i = 0; i < size; i++) {
        if (brd[i] == 0) {
            return i;
        }
    }
    printf("board_t doesn't contain 0");
    exit(1);
}

int heuristic(const Board brd, int rows) {
    int h = 0;
    int size = rows * rows;
    for (int i = 0; i < size; i++) {
        Tile tile = brd[i];
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

typedef struct {
    Board board;
    Move move;
} Action;

static const char* MOVE_STRINGS[] = {"Start", "Up", "Down", "Left", "Right"};

void print_board(const Board board, int rows, FILE* file) {
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

void print_action(const Action* action, int rows, FILE* file) {
    fprintf(file, "%s\n", MOVE_STRINGS[action->move]);
}

typedef struct {
    Board initial_brd;
    int rows;
    Action* solution;
    int steps;
    double time;
    int nodes;
} Run;

void print_solution(Run* run) {
    for (int i = run->steps - 1; i >= 0; i--) {
        Action* a = &run->solution[i];
        print_action(a, run->rows, stdout);
    }
    printf("Solved in %d steps\n\n", run->steps - 1);
}

void reconstruct_path(Arena* arena, int leaf_offset, Run* run) {
    int capacity = 10;
    run->solution = (Action*) malloc(sizeof(Action) * capacity);
    if (run->solution == NULL) {
        printf("Failed to allocate solution buffer");
        exit(1);
    }

    int i;
    for(i = 0; leaf_offset != -1; i++) {
        if (i >= capacity) {
            capacity *= 2;
            Action* next_solution = (Action*) realloc(run->solution, sizeof(Action) * capacity);
            if (next_solution == NULL) {
                printf("Failed to reallocate solution buffer");
                exit(1);
            }
            run->solution = next_solution;
        }

        Puzzle* puzzle = &arena->mem[leaf_offset];
        Action* solution = &run->solution[i];

        memcpy(solution->board, puzzle->board, sizeof(Board));
        solution->move = puzzle->move;
        leaf_offset = puzzle->parent_offset;
    }
    run->steps = i;
}

static const Move NEIGHBOR_MOVES[NEIGHBOR_CNT] = {RIGHT, DOWN, LEFT, UP};

void solve(Run* run) {
    run->nodes = 0;

    int rows = run->rows;
    int size = rows * rows;

    Board goal_brd = {0};
    for (int i = 0; i < size; i++) {
        goal_brd[i] = (Tile) i;
    }
    unsigned long long goal_hash = hash_board(goal_brd, size);

    Arena arena = new_arena();

    Heap open_set = new_pq();
    HashTable closed_set = new_ht();

    Puzzle* root = &arena.mem[arena.offset];
    memcpy(root->board, run->initial_brd, sizeof(Board));
    root->move = NONE;
    root->parent_offset = -1;
    root->f = 0;
    root->g = 0;

    push_heap(&open_set, &arena, arena.offset);
    arena.offset += 1;

    while(open_set.size > 0) {
        int cpuz_offset = pop_heap(&open_set, &arena);
        Puzzle* cpuz = &arena.mem[cpuz_offset];
        run->nodes += 1;

        unsigned long long current_hash = hash_board(cpuz->board, size);
        insert_into_ht(&closed_set, current_hash);

        if (current_hash == goal_hash) {
            reconstruct_path(&arena, cpuz_offset, run);
            break;
        }

        int zero_index = find_zero(cpuz->board, size);

        for (int i = 0; i < NEIGHBOR_CNT; i++) {
            Board nboard = {0};
            if (move_board(cpuz->board, nboard, zero_index, i, rows) != 0) {
                continue;
            }

            unsigned long long hash = hash_board(nboard, size);

            int not_visited = !ht_has_key(&closed_set, hash);
            if (not_visited) {
                expand_arena(&arena); // ensures our arena has enough space before we attempt to allocate
                cpuz = &arena.mem[cpuz_offset]; // we need to recalculate the pointer due to realloc

                Puzzle* npuz = &arena.mem[arena.offset];
                memcpy(npuz->board, nboard, sizeof(Board));
                npuz->parent_offset = cpuz_offset;
                npuz->g = cpuz->g + 1;
                npuz->f = npuz->g + heuristic(nboard, rows);
                npuz->move = NEIGHBOR_MOVES[i];

                push_heap(&open_set, &arena, arena.offset);
                arena.offset += 1;
            }
        }
    }

    free(closed_set.table);
    free(open_set.min_heap);
    free(arena.mem);
}