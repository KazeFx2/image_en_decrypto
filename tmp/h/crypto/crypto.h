//
// Created by Fx Kaze on 25-3-27.
//

#ifndef CRYPTO_H
#define CRYPTO_H

#include "macro.h"
#include "fifo_blk.h"
#include "types.h"

typedef struct {
    u32 width;
    u32 height;
    u8 channels;
} CryptoShape;

class Crypto {
protected:
    size_t encrypt_input_size;
    size_t encrypt_output_size;
    size_t decrypt_input_size;
    size_t decrypt_output_size;
    FIFOBlocks encrypt_in_fifo;
    FIFOBlocks encrypt_out_fifo;
    FIFOBlocks decrypt_in_fifo;
    FIFOBlocks decrypt_out_fifo;

    virtual size_t __encrypt(void *, size_t, const void *, size_t) = 0;

    virtual size_t __decrypt(void *, size_t, const void *, size_t) = 0;

    void setEncryptInputSize(size_t size);

    void setEncryptOutputSize(size_t size);

    void setDecryptInputSize(size_t size);

    void setDecryptOutputSize(size_t size);

public:
    Crypto(size_t, size_t, size_t, size_t);

    Crypto(const Crypto &) = delete;

    Crypto &operator=(const Crypto &) = delete;

    Crypto(Crypto &&) = delete;

    Crypto &operator=(Crypto &&) = delete;

    virtual ~Crypto() = 0;

    virtual Crypto *move() = 0;

    virtual Crypto *clone() const = 0;

    virtual void setShape(CryptoShape) = 0;

    size_t encrypt(__OUT void *,__IN size_t,__IN const void *,__IN size_t);

    size_t decrypt(__OUT void *,__IN size_t,__IN const void *,__IN size_t);
};

#endif //CRYPTO_H
