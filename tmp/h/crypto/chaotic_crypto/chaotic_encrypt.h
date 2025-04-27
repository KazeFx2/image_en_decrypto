//
// Created by Fx Kaze on 25-3-27.
//

#ifndef CHAOTIC_ENCRYPT_H
#define CHAOTIC_ENCRYPT_H

#include <private/crypto/crypto.h>

#include "chaotic_types.h"

void chaotic_encrypt(
    __OUT void *_dst,
    __IN const void *_src,
    __IN const ChaoticControl &ctrl,
    __IN const ChaoticKeys &keys,
    __IN const CryptoShape &shape,
    __IN_OUT ChaoticReturns **returns
    );

#endif //CHAOTIC_ENCRYPT_H
