//
// Created by Joseph on 3/10/2024.
//

#ifndef CPP_ARENA_H
#define CPP_ARENA_H

#include <vector>
#include <memory>

template<class T, int BLK_SZ = 40960>
class Arena {
public:
    Arena() {
        auto block = new char[BLK_SZ];
        blocks.push_back(block);
    }

    T* alloc() {
        if (offset >= BLK_SZ) {
            auto block = new char[BLK_SZ];
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
    int offset = 0;
};

#endif //CPP_ARENA_H