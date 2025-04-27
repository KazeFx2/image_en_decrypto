//
// Created by Fx Kaze on 25-3-27.
//

#ifndef CHAOTIC_CRYPTO_H
#define CHAOTIC_CRYPTO_H

#include "../crypto.h"
#include "ThreadPool.h"
#include "chaotic_types.h"

class ChaoticCrypto : public Crypto {
    ThreadPool *threadPool;
    ChaoticControl control;
    ChaoticKeys keys;
    CryptoShape shape;
    ChaoticReturns *returns;

    size_t __encrypt(void *, size_t, const void *, size_t) override;

    size_t __decrypt(void *, size_t, const void *, size_t) override;

public:
    ChaoticCrypto(ThreadPool *);

    ~ChaoticCrypto() override;

    Crypto *clone() const override;

    Crypto *move() override;

    void setShape(CryptoShape) override;

};

#endif //CHAOTIC_CRYPTO_H
