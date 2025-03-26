//
// Created by Fx Kaze on 25-3-25.
//

#include "include/private/RWLock.h"
#include "print_size.h"

int main() {
    PRT_SIZE(RWLock);
    PRT_SIZE(RWLock::SubLock);
    return 0;
}
