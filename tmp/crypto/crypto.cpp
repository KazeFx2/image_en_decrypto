//
// Created by Fx Kaze on 25-3-27.
//

#include "private/crypto/crypto.h"

#include <cstdlib>
#include <cstring>

#include "Logger.h"

Crypto::Crypto(size_t en_in_size, size_t en_out_size, size_t de_in_size,
               size_t de_out_size) : encrypt_input_size(en_in_size), encrypt_in_fifo(encrypt_input_size),
                                     encrypt_output_size(en_out_size), encrypt_out_fifo(encrypt_output_size),
                                     decrypt_input_size(de_in_size), decrypt_in_fifo(decrypt_input_size),
                                     decrypt_output_size(de_out_size), decrypt_out_fifo(decrypt_output_size) {
}

void Crypto::setEncryptInputSize(size_t size) {
    encrypt_input_size = size;
    encrypt_in_fifo.set_blk_size(encrypt_input_size);
}

void Crypto::setEncryptOutputSize(size_t size) {
    encrypt_output_size = size;
    encrypt_out_fifo.set_blk_size(encrypt_output_size);
}

void Crypto::setDecryptInputSize(size_t size) {
    decrypt_input_size = size;
    decrypt_in_fifo.set_blk_size(decrypt_input_size);
}

void Crypto::setDecryptOutputSize(size_t size) {
    decrypt_output_size = size;
    decrypt_out_fifo.set_blk_size(decrypt_output_size);
}

size_t Crypto::encrypt(__OUT void *_dst, __IN size_t _dst_size,__IN const void *_src,__IN size_t _src_size) {
    FIFOBlocks &in = encrypt_in_fifo;
    FIFOBlocks &out = encrypt_out_fifo;
    size_t in_size = encrypt_input_size;
    size_t out_size = encrypt_output_size;

    if (in_size == 0 || out_size == 0) {
        if (in.size() == 0 && out.size() == 0)
            return __decrypt(
                _dst, _dst_size, _src, _src_size
            );
        in_size = _src_size + in.size();
        out_size = _dst_size + out.size();
    }
    void *tmp_in = malloc(in_size);
    void *tmp_out = malloc(out_size);
    if (!_src && in.size() % in_size) {
        size_t remain = in_size - (in.size() % in_size);
        memset(tmp_in, 0, remain);
        in.write(tmp_in, remain);
    } else
        in.write(_src, _src_size);
    while (in.size() / in_size) {
        in.read(tmp_in, in_size);
        size_t r_out = __encrypt(tmp_out, out_size, tmp_in, in_size);
        out.write(tmp_out, r_out);
    }
    free(tmp_in);
    free(tmp_out);
    return out.read(_dst, _dst_size);
}

size_t Crypto::decrypt(__OUT void *_dst, __IN size_t _dst_size,__IN const void *_src,__IN size_t _src_size) {
    FIFOBlocks &in = decrypt_in_fifo;
    FIFOBlocks &out = decrypt_out_fifo;
    size_t in_size = decrypt_input_size;
    size_t out_size = decrypt_output_size;

    if (in_size == 0 || out_size == 0) {
        if (in.size() == 0 && out.size() == 0)
            return __decrypt(
                _dst, _dst_size, _src, _src_size
            );
        in_size = _src_size + in.size();
        out_size = _dst_size + out.size();
    }
    void *tmp_in = malloc(in_size);
    void *tmp_out = malloc(out_size);
    if (!_src && in.size() % in_size) {
        size_t remain = in_size - (in.size() % in_size);
        memset(tmp_in, 0, remain);
        in.write(tmp_in, remain);
    } else
        in.write(_src, _src_size);
    while (in.size() / in_size) {
        in.read(tmp_in, in_size);
        size_t r_out = __encrypt(tmp_out, out_size, tmp_in, in_size);
        out.write(tmp_out, r_out);
    }
    free(tmp_in);
    free(tmp_out);
    return out.read(_dst, _dst_size);
}
