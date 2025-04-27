//
// Created by Fx Kaze on 25-3-27.
//

#include "private/crypto/chaotic_crypto/chaotic_util.h"

void DestroyReturn(__IN ChaoticReturns *ret) {
    if (!ret) return;
    for (u32 i = 0; i < ret->nThread; i++)
        delete [] ret->threadReturn[i]->byteSeq, delete [] ret->threadReturn[i]->diffusionSeedArray, delete ret->
                threadReturn[i];
    delete [] ret->threadReturn;
    delete [] ret;
}

