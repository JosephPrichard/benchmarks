//
// Created by Joseph on 3/10/2024.
//

#ifndef CPP_ARENA_H
#define CPP_ARENA_H

#include <vector>

template<class T, int BLK_SZ = 10000>
class Arena {
public:
    T* alloc();

    std::array<T, BLK_SZ>* last_block();

    void next_block();
private:
    std::vector<std::array<T, BLK_SZ>> blocks;
    int blocks_cnt = 0;
    int elem_cnt = 0;
};

template<class T, int BLK_SZ>
void Arena<T, BLK_SZ>::next_block() {
    blocks.push_back(std::array<T, BLK_SZ>());
    blocks_cnt++;
    elem_cnt = 0;
}

template<class T, int BLK_SZ>
std::array<T, BLK_SZ>* Arena<T, BLK_SZ>::last_block() {
    return &blocks[blocks.size() - 1];
}

template<class T, int BLK_SZ>
T* Arena<T, BLK_SZ>::alloc() {
    if (blocks_cnt == 0) {
        next_block();
    }
    std::array<T, BLK_SZ>* block = last_block();
    if (elem_cnt >= BLK_SZ) {
        next_block();
        block = last_block();
    }
    T* ptr = &(*block)[elem_cnt];
    elem_cnt++;
    return ptr;
}

#endif //CPP_ARENA_H
