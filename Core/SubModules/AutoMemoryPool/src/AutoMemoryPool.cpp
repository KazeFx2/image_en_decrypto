//
// Created by Fx Kaze on 25-1-18.
//

#include "AutoMemoryPool.h"

AutoMemoryPool::AutoMemoryPool(const u32 addition): addition(addition), size(addition) {
    list = init_list_default();
    if (list == nullptr)
        throw std::bad_alloc();
    addNode();
}

AutoMemoryPool::AutoMemoryPool(const AutoMemoryPool &other) {
    addition = other.addition;
    size = other.size;
    list = init_list_default();
    AutoMemoryNode **node = reinterpret_cast<AutoMemoryNode **>(&list->next);
    FOREACH(AutoMemoryNode, i, list) {
        addNode(i->data.size);
        memcpy((*node)->data.start, i->data.start, i->data.size);
        node = reinterpret_cast<AutoMemoryNode **>(&(*node)->node.next);
    }
}

AutoMemoryPool &AutoMemoryPool::operator=(const AutoMemoryPool &other) {
    this->~AutoMemoryPool();
    addition = other.addition;
    size = other.size;
    list = init_list_default();
    AutoMemoryNode **node = reinterpret_cast<AutoMemoryNode **>(&list->next);
    FOREACH(AutoMemoryNode, i, list) {
        addNode(i->data.size);
        memcpy((*node)->data.start, i->data.start, i->data.size);
        node = reinterpret_cast<AutoMemoryNode **>(&(*node)->node.next);
    }
}

AutoMemoryPool::AutoMemoryPool(AutoMemoryPool &&other) {
    addition = other.addition;
    size = other.size;
    list = other.list;
    other.list = nullptr;
}

AutoMemoryPool &AutoMemoryPool::operator=(AutoMemoryPool &&other) {
    this->~AutoMemoryPool();
    addition = other.addition;
    size = other.size;
    list = other.list;
    other.list = nullptr;
}


AutoMemoryPool::~AutoMemoryPool() {
    if (!list)
        return;
    FOREACH(AutoMemoryNode, i, list) {
        free(i->data.start);
    }
    destroy_list(list);
    list = nullptr;
}

void AutoMemoryPool::addNode(const u32 size) const {
    const auto node = static_cast<AutoMemoryNode *>(malloc(sizeof(AutoMemoryNode)));
    if (node == nullptr) {
        throw std::bad_alloc();
    }
    node->data.start = malloc(size);
#ifdef __DEBUG
    printf("[addNode]malloc: 0x%p\n", node->data.start);
#endif
    node->data.size = size;
    push_exist_end(list, reinterpret_cast<node_t *>(node));
}

void AutoMemoryPool::addNode() const {
    addNode(addition);
}

AutoMemoryPool::AutoMemoryPointer<u8> AutoMemoryPool::operator&() {
    return AutoMemoryPointer<u8>(this);
}
