//
// Created by Fx Kaze on 25-3-27.
//

#include "private/crypto/chaotic_crypto/chaotic_crypto.h"

static void encrypt(
void *_dst,
const void *_src,
CryptoShape shape,
const ChaoticControl &control,

) {

}

ChaoticCrypto::ChaoticCrypto(ThreadPool *pool): Crypto(0, 0, 0, 0),
                                                threadPool(pool),
                                                control({CHAOTIC_DEFAULT_CTRL}),
                                                keys({CHAOTIC_RAND_KEYS}),
                                                shape({CHAOTIC_NO_SHAPE}),
                                                returns(nullptr) {
}

ChaoticCrypto::~ChaoticCrypto() {
}

void ChaoticCrypto::setShape(const CryptoShape shape) {
    setEncryptInputSize(shape.width * shape.height * shape.channels);
    setEncryptOutputSize(shape.width * shape.height * shape.channels);
    setDecryptInputSize(shape.width * shape.height * shape.channels);
    setDecryptOutputSize(shape.width * shape.height * shape.channels);
    this->shape = shape;
}

size_t ChaoticCrypto::__encrypt(void *_dst, size_t _dst_size, const void *_src, size_t _src_size) {
    size_t writen = 0;
    return writen;
}

size_t ChaoticCrypto::__decrypt(void *_dst, size_t _dst_size, const void *_src, size_t _src_size) {
    size_t writen = 0;
    return writen;
}
