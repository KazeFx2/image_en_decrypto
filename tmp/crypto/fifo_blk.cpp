//
// Created by Fx Kaze on 25-3-27.
//

#include "private/crypto/fifo_blk.h"
#include "Logger.h"
#include "private/crypto/macro.h"
#include <cstdlib>
#include <cstring>

typedef struct {
    node_t node;
    size_t size;
    size_t remain;
    void *start;
} BlkNode;

#define DATA_SIZE (sizeof(BlkNode) - sizeof(node_t))

static BlkNode *mallocNode(size_t blk_size) {
    BlkNode *ret = static_cast<BlkNode *>(malloc(sizeof(BlkNode)));
    if (!ret) {
        LOG_NAMED_FATAL(__func__, "FIFO block node allocation failed");
        return nullptr;
    }
    ret->start = malloc(blk_size);
    if (!(ret->start)) {
        LOG_NAMED_FATAL(__func__, "FIFO block allocation failed");
        free(ret);
        return nullptr;
    }
    ret->size = blk_size;
    ret->remain = blk_size;
    return ret;
}

FIFOBlocks::FIFOBlocks(size_t _blk_size): blk_size(_blk_size), total_size(0) {
    blk_list = init_list_default();
    if (!blk_list)
        LOG_NAMED_FATAL(__func__, "FIFO block initialization failed");
}

FIFOBlocks::FIFOBlocks(const FIFOBlocks &other): blk_size(other.blk_size), total_size(other.total_size) {
    blk_list = init_list_default();
    if (!blk_list)
        LOG_NAMED_ERROR(__func__, "FIFO block initialization failed");
    FOREACH(BlkNode, i, other.blk_list) {
        if (i->remain == i->size)
            continue;
        BlkNode *new_blk = mallocNode(i->size);
        if (!new_blk) {
            LOG_NAMED_FATAL(__func__, "FIFO block node allocation failed");
            return;
        }
        memcpy(new_blk->start, i->start, i->size - i->remain);
        push_exist_end(blk_list, reinterpret_cast<node_t *>(new_blk));
    }
}

FIFOBlocks &FIFOBlocks::operator=(const FIFOBlocks &other) {
    this->~FIFOBlocks();
    blk_list = init_list_default();
    blk_size = other.blk_size;
    total_size = other.total_size;
    if (!blk_list)
        LOG_NAMED_ERROR(__func__, "FIFO block initialization failed");
    FOREACH(BlkNode, i, other.blk_list) {
        if (i->remain == i->size)
            continue;
        BlkNode *new_blk = mallocNode(i->size);
        if (!new_blk) {
            LOG_NAMED_FATAL(__func__, "FIFO block node allocation failed");
            return *this;
        }
        memcpy(new_blk->start, i->start, i->size - i->remain);
        push_exist_end(blk_list, reinterpret_cast<node_t *>(new_blk));
    }
    return *this;
}

FIFOBlocks::FIFOBlocks(FIFOBlocks &&other): blk_size(other.blk_size), total_size(other.total_size) {
    blk_list = other.blk_list;
    other.blk_list = nullptr;
}

FIFOBlocks &FIFOBlocks::operator=(FIFOBlocks &&other) {
    this->~FIFOBlocks();
    blk_list = other.blk_list;
    blk_size = other.blk_size;
    total_size = other.total_size;
    other.blk_list = nullptr;
    return *this;
}

FIFOBlocks::~FIFOBlocks() {
    if (!blk_list) return;
    FOREACH(BlkNode, i, blk_list) {
        if (i->start)
            free(i->start);
    }
    destroy_list(blk_list);
}

size_t FIFOBlocks::size() const {
    return total_size;
}

void FIFOBlocks::set_blk_size(size_t size) {
    blk_size = size;
}

size_t FIFOBlocks::write(const void *src, size_t len) {
    size_t written = 0;
    BlkNode *new_node;

    if (!blk_list) {
        LOG_NAMED_ERROR(__func__, "FIFO block list not initialized");
        return 0;
    }
    FOREACH(BlkNode, i, blk_list) {
        if (i->remain) {
            memcpy(i->start + i->size - i->remain, src, MIN(i->remain, len));
            written += MIN(i->remain, len);
            src += MIN(i->remain, len);
            len -= MIN(i->remain, len);
            i->remain -= MIN(i->remain, len);
        }
    }
    BlkNode *now = reinterpret_cast<BlkNode *>(get_last(blk_list));
    while (len) {
        new_node = mallocNode(blk_size);
        if (!new_node) {
            LOG_NAMED_ERROR(__func__, "FIFO block node allocation failed");
            total_size += written;
            return written;
        }
        memcpy(new_node->start, src, MIN(new_node->size, len));
        written += MIN(new_node->size, len);
        src += MIN(new_node->size, len);
        len -= MIN(new_node->size, len);
        new_node->remain -= MIN(new_node->size, len);
        push_exist_node_before2(blk_list, reinterpret_cast<void **>(&(now->node.next)),
                                reinterpret_cast<node_t *>(new_node));
        now = reinterpret_cast<BlkNode *>(now->node.next);
    }
    total_size += written;
    return written;
}

size_t FIFOBlocks::read(void *dst, size_t len) {
    size_t written = 0;
    size_t idx = 0, n_blk = list_size(blk_list);

    FOREACH(BlkNode, i, blk_list) {
        if (i->remain == i->size) {
            break;
        }
        memcpy(dst, i->start, MIN(i->size - i->remain, len));
        written += MIN(i->size - i->remain, len);
        dst += MIN(i->size - i->remain, len);
        len -= MIN(i->size - i->remain, len);
        i->remain += MIN(i->size - i->remain, len);
        if (i->remain == i->size) {
            move_from_to(blk_list, idx, n_blk);
        } else {
            idx++;
        }
        if (len == 0)
            break;
    }
    total_size -= written;
    return written;
}
