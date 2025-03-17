//
// Created by Fx Kaze on 25-1-18.
//

#ifndef AUTO_MEMORY_POOL_H
#define AUTO_MEMORY_POOL_H

#include <iostream>
#include "private/types.h"

extern "C" {
#include "list/CList.h"
}

#define POINTER_OPERATE(ptr, op) (ptr = reinterpret_cast<decltype(ptr)>(reinterpret_cast<uptr>(ptr) op))

class AutoMemoryPool {
public:
    AutoMemoryPool(u32 addition = 512);

    AutoMemoryPool(const AutoMemoryPool &);

    AutoMemoryPool &operator=(const AutoMemoryPool &);

    AutoMemoryPool(AutoMemoryPool &&);

    AutoMemoryPool &operator=(AutoMemoryPool &&);

    ~AutoMemoryPool();

    typedef struct {
        void *start;
        u32 size;
    } AutoMemoryData;

    typedef struct {
        node_t node;
        AutoMemoryData data;
    } AutoMemoryNode;

    template<typename T>
    class AutoMemoryPointer {
    public:
        typedef enum {
            AUTO_MEMORY_POINTER_END = 0x0,
        } AutoMemoryPointerType;

        AutoMemoryPointer(AutoMemoryPool *parent): typeSize(sizeof(T)), parent(parent) {
            node = reinterpret_cast<AutoMemoryNode *>(parent->list->next);
            data = node->data.start;
            size = node->data.size;
            offset = 0;
            // printf("%d\n", typeSize);
        }

        ~AutoMemoryPointer() {
        }

        operator T *() {
            return static_cast<T *>(data);
        }

        T &operator*() {
            return *static_cast<T *>(data);
        }

        T *operator+=(i32 n) {
            n *= typeSize;
            const u32 remain = size - offset;
            if (remain <= n) {
                u32 left = n - remain;
                goNext();
                while (left >= size) {
                    left -= size;
                    goNext();
                }
                offset = left;
                POINTER_OPERATE(data, +offset);
            } else {
                offset += n;
                POINTER_OPERATE(data, +n);
            }
            return static_cast<T *>(data);
        }

        T *operator-=(i32 n) {
            n *= typeSize;
            if (offset < n) {
                u32 left = n - offset;
                if (!goPrev())
                    return static_cast<T *>(data);
                while (size < left) {
                    left -= size;
                    if (!goPrev())
                        return static_cast<T *>(data);
                }
                offset = size - left;
                POINTER_OPERATE(data, +offset);
            } else {
                offset -= n;
                POINTER_OPERATE(data, -n);
            }
            return static_cast<T *>(data);
        }

        T *operator++() {
            if constexpr (!std::is_same_v<T, void>) {
                if (typeSize > 1)
                    return operator+=(1);
            }
            ++offset;
            if (offset == size) {
                offset = 0;
                if (node->node.next == nullptr) {
                    parent->addNode();
                    if (node->node.next == nullptr) {
                        data = reinterpret_cast<void *>(AUTO_MEMORY_POINTER_END);
                        return static_cast<T *>(data);
                    }
                }
                node = reinterpret_cast<AutoMemoryNode *>(node->node.next);
                data = node->data.start;
                size = node->data.size;
            } else {
                POINTER_OPERATE(data, +1);
                // ++data;
            }
            return static_cast<T *>(data);
        }

        T *operator--() {
            if constexpr (!std::is_same_v<T, void>) {
                if (typeSize > 1)
                    return operator-=(1);
            }
            if (offset == 0) {
                if (node->node.prev != parent->list) {
                    node = reinterpret_cast<AutoMemoryNode *>(node->node.prev);
                    size = node->data.size;
                    offset = size - 1;
                    data = node->data.start;
                    POINTER_OPERATE(data, +offset);
                }
            } else {
                --offset;
                POINTER_OPERATE(data, -1);
                // --data;
            }
            return static_cast<T *>(data);
        }

        T *operator++(int) {
            auto old = data;
            operator++();
            return old;
        }

        T *operator--(int) {
            auto old = data;
            operator--();
            return old;
        }

    private:
        u8 typeSize;
        void *data;
        u32 size, offset;
        AutoMemoryPool *parent;
        AutoMemoryNode *node;

        bool nextStart() {
            goNext();
            offset = 0;
            return true;
        }

        bool nextEnd() {
            goNext();
            offset = size - 1;
            data += offset;
            return true;
        }

        /// @return return `true` if `node` is not the beginning of memory list,
        /// else `false`
        bool prevStart() {
            if (!goPrev()) return false;
            offset = 0;
            return true;
        }


        bool prevEnd() {
            if (!goPrev()) return false;
            offset = size - 1;
            data += offset;
            return true;
        }

        bool checkBegin() const {
            if (node->node.prev == parent->list)
                return true;
            return false;
        }

        void goNext() {
            if (node->node.next == nullptr) {
                parent->addNode();
                if (node->node.next == nullptr) {
                    throw std::bad_alloc();
                }
            }
            node = reinterpret_cast<AutoMemoryNode *>(node->node.next);
            size = node->data.size;
            data = node->data.start;
        }

        bool goPrev() {
            if (checkBegin()) return false;
            node = reinterpret_cast<AutoMemoryNode *>(node->node.prev);
            size = node->data.size;
            data = node->data.start;
            return true;
        }
    };

    AutoMemoryPointer<u8> operator &();

private:
    u32 addition;
    u64 size;
    list_t list;

    void addNode() const;

    void addNode(u32 size) const;
};

#endif //AUTO_MEMORY_POOL_H
