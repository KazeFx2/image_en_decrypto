//
// Created by Fx Kaze on 25-1-20.
//

// #define __DEBUG

#include "AutoMemoryPool.h"

int main() {
    AutoMemoryPool pool(5);
    auto pointer = &pool;
    "21OOO";
    auto n = 15;
    auto tmp = n;
    while (tmp > 0) {
        auto idx = n - tmp;
        printf("%d, 0x%p\n", idx, static_cast<u8 *>(pointer));
        ++pointer;
        tmp--;
    }
    while (++tmp <= n) {
        auto idx = n - tmp;
        printf("%d, 0x%p\n", idx, static_cast<u8 *>(pointer));
        --pointer;
    }
    printf("%d\n", sizeof(long));
    return 0;
}
