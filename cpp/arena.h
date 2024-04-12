//
// Created by Joseph on 3/10/2024.
//

#ifndef CPP_ARENA_H
#define CPP_ARENA_H

#include <vector>
#include <memory>

template<class T>
class Arena {
public:
    Arena() {
        block_size = 40960;
        offset = 0;
        auto block = new char[block_size];
        blocks.push_back(block);
    }

    T* alloc() {
        if (offset >= block_size) {
            block_size *= 2;
            auto block = new char[block_size];
            blocks.push_back(block);
            offset = 0;
        }
        char* last_block = blocks.at(blocks.size() - 1);
        char* ptr = last_block + offset;
        offset += sizeof(T);
        return reinterpret_cast<T*>(ptr);
    }

    ~Arena() {
        for (auto block : blocks) {
            delete block;
        }
    }
private:
    std::vector<char*> blocks;
    int offset;
    int block_size;
};

#endif //CPP_ARENA_H