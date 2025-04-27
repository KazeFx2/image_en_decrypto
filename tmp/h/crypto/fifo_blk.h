//
// Created by Fx Kaze on 25-3-27.
//

#ifndef FIFO_BLK_H
#define FIFO_BLK_H

extern "C" {
#include "c_list.h"
}

class FIFOBlocks {
    list_t blk_list;
    size_t blk_size;
    size_t total_size;

public:
    FIFOBlocks(size_t);

    FIFOBlocks(const FIFOBlocks &);

    FIFOBlocks &operator=(const FIFOBlocks &);

    FIFOBlocks(FIFOBlocks &&);

    FIFOBlocks &operator=(FIFOBlocks &&);

    ~FIFOBlocks();

    size_t size() const;

    void set_blk_size(size_t);

    size_t write(const void *, size_t);

    size_t read(void *, size_t);
};

#endif //FIFO_BLK_H
