//
// Created by Fx Kaze on 25-1-18.
//

#include "AutoMemoryPool.h"

AutoMemoryPool::AutoMemoryPool(const u32 addition): addition(addition), size(addition) {
    list = initList();
    if (list == nullptr)
        throw std::bad_alloc();
    addNode();
}

AutoMemoryPool::~AutoMemoryPool() {
    FOREACH(AutoMemoryNode, i, list) {
        free(i->data.start);
    }
    destroyList(list);
}


void AutoMemoryPool::addNode() const {
    const auto node = static_cast<AutoMemoryNode *>(malloc(sizeof(AutoMemoryNode)));
    if (node == nullptr) {
        throw std::bad_alloc();
    }
    node->data.start = malloc(addition);
#ifdef __DEBUG
    printf("[addNode]malloc: 0x%p\n", node->data.start);
#endif
    node->data.size = addition;
    pushExistEnd(list, reinterpret_cast<node_t *>(node));
}

AutoMemoryPool::AutoMemoryPointer<u8> AutoMemoryPool::operator&() {
    return AutoMemoryPointer<u8>(this);
}
