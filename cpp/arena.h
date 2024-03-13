//
// Created by Joseph on 3/10/2024.
//

#ifndef CPP_ARENA_H
#define CPP_ARENA_H

#include <vector>
#include <memory>

template<class T, int BLK_SZ = 4096>
class Arena {
public:
    T* alloc(T);
private:
    std::vector<T>& last_block();

    void next_block();

    std::vector<std::unique_ptr<std::vector<T>>> blocks;
    int elem_cnt = 0;
};

template<class T, int BLK_SZ>
void Arena<T, BLK_SZ>::next_block() {
    auto block = std::make_unique<std::vector<T>>(std::vector<T>(BLK_SZ));
    blocks.push_back(std::move(block));
    elem_cnt = 0;
}

template<class T, int BLK_SZ>
std::vector<T>& Arena<T, BLK_SZ>::last_block() {
    if (blocks.size() <= 0 || elem_cnt >= BLK_SZ) {
        next_block();
    }
    return *blocks.at(blocks.size() - 1);
}

template<class T, int BLK_SZ>
T* Arena<T, BLK_SZ>::alloc(T elem) {
    auto& block = last_block();
    T* ptr = block.data() + elem_cnt;
    *ptr = elem;
    elem_cnt++;
    return ptr;
}

#endif //CPP_ARENA_H
