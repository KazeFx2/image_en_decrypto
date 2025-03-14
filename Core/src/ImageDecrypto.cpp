//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageDecrypto.h"

#include "private/Util.h"
#ifdef __USE_CUDA
#include "private/Cuda.cuh"
#endif

using namespace cv;

#ifdef __DEBUG
static FILE *gfd;
static ::Mutex mu;
#endif

void *decryptoAssistant(__IN void *param);

void *decryptoAssistantWithKeys(__IN void *param);

#ifdef __DEBUG
#define DUMP_MAT {\
    snprintf(name, sizeof(name), "%d_de_dump_mat.txt", dumpId--);\
    snprintf(path, sizeof(path), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/%s", name);\
    FILE *fd = fopen(path, "w+");\
    DumpMat(fd, name, *dst);\
    fclose(fd);\
}
#else
#define DUMP_MAT
#endif

void DecryptoImage(__IN_OUT Mat &Image, __IN_OUT Size &Size,__IN const Keys &Key, __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool) {
    // resize image if needed
    if (Size.width != 0 && Size.height != 0 && Size != Image.size()) {
        resize(Image, Image, Size);
    }
    Size = Image.size();
    DecryptoImage(
        Image.ptr(), Size.width, Size.height, Key, threadKeys, Config, pool
    );
}

void DecryptoImage(__IN_OUT void *Image, __IN const u32 width, __IN const u32 height, __IN const Keys &Key,
                   __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool) {
    u8 *tmpImage;
    u8 *dst, *src;
    u32 *threads = new u32[Config.nThread];
    auto *params = new threadParamsWithKeyMem[Config.nThread];

    PreGenerate(static_cast<u8 *>(Image), tmpImage, width, height, dst, src, threads, params, Key, threadKeys, Config,
                pool,
                decryptoAssistantWithKeys);

    // Diffusion & Confusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP_KEY;
        swap(src, dst);
        DOLOOP_KEY;
        swap(src, dst);
    }
    // Encrypto confusion
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        DOLOOP_KEY;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        pool.waitThread(threads[i]);
    }
    delete[] threads;
    delete[] params;
    if (Image != src)
        memcpy(Image, src, width * height * Config.nChannel);
    delete[] tmpImage;
}

threadReturn **DecryptoImage(__IN_OUT Mat &Image, __IN_OUT Size &Size,__IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool) {
    // resize image if needed
    if (Size.width != 0 && Size.height != 0 && Size != Image.size()) {
        resize(Image, Image, Size);
    }
    Size = Image.size();
    return DecryptoImage(
        Image.ptr(), Size.width, Size.height, Keys, Config, pool
    );
}

threadReturn **DecryptoImage(__IN_OUT void *Image, __IN const u32 width, __IN const u32 height, __IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool) {
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Decrypto_Procedure.txt");
    gfd = fopen(name, "w+");
#endif

    u8 *tmpImage;
    u8 *dst, *src;
    u32 *threads = new u32[Config.nThread];
    auto *params = new threadParamsMem[Config.nThread];
    auto **ret = new threadReturn *[Config.nThread];

    PreGenerate(static_cast<u8 *>(Image), tmpImage, width, height, dst, src, threads, params, Keys, Config, pool,
                decryptoAssistant);

    // Decrypto inv_confusion & inv_diffusion
#ifdef __DEBUG
    u32 dumpId = 13;
    char path[256];
#endif
    DUMP_MAT;
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
    }
    // Inv_confusion
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        ret[i] = static_cast<threadReturn *>(pool.waitThread(threads[i]));
    }
    delete[] threads;
    delete[] params;
    if (Image != src)
        memcpy(Image, src, width * height * Config.nChannel);
    delete[] tmpImage;

#ifdef __DEBUG
    fclose(gfd);
#endif
    // returns
    return ret;
}

inline void decryptoBody(__IN_OUT threadParamsMem &params,
                         __IN const u32 rowStart,__IN const u32 rowEnd,
                         __IN const u32 colStart,__IN const u32 colEnd,
                         __IN const u8 *byteSeq, __IN const u8 *diffusionSeedArray) {
#ifdef __USE_CUDA
    void *cudaDst = nullptr;
    void *cudaSrc = nullptr;
    if (params.threadId == 0 && params.config->cuda) {
        cudaDst = MallocCuda(params.width * params.height * params.config->nChannel);
        cudaSrc = AllocCopyMemToCuda(*params.src, params.width * params.height * params.config->nChannel);
    }
#endif


    // Decrypto
    u32 seqIdx = params.config->nChannel * (rowEnd - rowStart) * (colEnd - colStart) * params.config->
                 diffusionConfusionIterations;
    // Inv-confusion & inv-diffusion
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Decrypto_%lu.txt", params.threadId);
    FILE *fd = fopen(name, "w+");
    fprintf(fd, "[DECRYPT]id: %lu, seqIdx: %d\n", params.threadId, seqIdx);
    fprintf(fd, "startRow: %u, endRow: %u, startCol: %u, endCol: %u\n", rowStart, rowEnd, colStart, colEnd);
    DumpBytes(
        fd, "byteSeq", byteSeq, params.iterations * params.config->byteReserve
    );
    DumpBytes(
        fd, "diffusionSeedArray", diffusionSeedArray,
        params.config->nChannel * params.config->diffusionConfusionIterations
    );
    fprintf(fd, "[END DECRYPT]\n");
    fclose(fd);
#endif

    u8 *diffusionSeed = new u8[params.config->nChannel];
    if (params.config->diffusionConfusionIterations > 0)
        for (u32 i = params.config->diffusionConfusionIterations - 1; ; i--) {
            memcpy(diffusionSeed, diffusionSeedArray + i * params.config->nChannel, params.config->nChannel);
            params.Start.wait();
#ifdef __USE_CUDA
            if (params.config->cuda) {
                if (params.threadId == 0) {
                    CopyMemToCuda(cudaSrc, *params.src, params.width * params.height * params.config->nChannel);
                    InvertConfusionCuda(cudaDst, cudaSrc, params.width, params.height, params.keys.confusionSeed,
                                        params.config->nChannel);
                    CopyCudaToMem(*params.dst, cudaDst, params.width * params.height * params.config->nChannel);
                }
            } else
                InvertConfusion(*params.dst,
                                *params.src,
                                rowStart, rowEnd, colStart, colEnd, params.width, params.height,
                                params.keys.confusionSeed,
                                params.config->nChannel);
#else
            InvertConfusion(*params.dst,
                            *params.src,
                            rowStart, rowEnd, colStart, colEnd, params.width, params.height, params.keys.confusionSeed,
                            params.config->nChannel);
#endif
            params.Finish.post();
            params.Start.wait();
#ifdef __DEBUG
        mu.lock();
        fprintf(gfd, "[DiffusionSeed]id: %lu, Round: %u, %02X, %02X, %02X, idx: %u, ", params.threadId,
                i,
                diffusionSeed[0],
                diffusionSeed[1], diffusionSeed[2], seqIdx);
#endif
            InvertDiffusion(*params.dst, *params.src,
                            rowStart, rowEnd, colStart, colEnd, params.width, params.height,
                            diffusionSeed, byteSeq, seqIdx, params.config->nChannel);
#ifdef __DEBUG
        fprintf(gfd, "ori_data: ");
        for (u32 _i = rowStart; _i < rowEnd; _i++) {
            for (u32 j = colStart; j < colEnd; j++) {
                fprintf(gfd, "%02X%02X%02X", (*params.dst)->at<Vec3b>(_i, j)[0], (*params.dst)->at<Vec3b>(_i, j)[1],
                        (*params.dst)->at<Vec3b>(_i, j)[2]);
            }
        }
        fprintf(gfd, ", enc_data: ");
        for (u32 _i = rowStart; _i < rowEnd; _i++) {
            for (u32 j = colStart; j < colEnd; j++) {
                fprintf(gfd, "%02X%02X%02X", (*params.src)->at<Vec3b>(_i, j)[0], (*params.src)->at<Vec3b>(_i, j)[1],
                        (*params.src)->at<Vec3b>(_i, j)[2]);
            }
        }
        fprintf(gfd, "\n");
        mu.unlock();
#endif
            params.Finish.post();
            if (i == 0)
                break;
        }
    // Inv-confusion
    for (u32 i = 0; i < params.config->confusionIterations; i++) {
        params.Start.wait();
#ifdef __USE_CUDA
        if (params.config->cuda) {
            if (params.threadId == 0) {
                if (i == 0) {
                    CopyMemToCuda(cudaSrc, *params.src, params.width * params.height * params.config->nChannel);
                }
                InvertConfusionCuda(cudaDst, cudaSrc, params.width, params.height, params.keys.confusionSeed,
                                    params.config->nChannel);
                if (i + 1 == params.config->confusionIterations) {
                    CopyCudaToMem(*params.dst, cudaDst, params.width * params.height * params.config->nChannel);
                } else
                    swap(cudaDst, cudaSrc);
            }
        } else
            InvertConfusion(*params.dst,
                            *params.src,
                            rowStart, rowEnd, colStart, colEnd, params.width, params.height, params.keys.confusionSeed,
                            params.config->nChannel);
#else
        InvertConfusion(*params.dst,
                        *params.src,
                        rowStart, rowEnd, colStart, colEnd, params.width, params.height, params.keys.confusionSeed,
                        params.config->nChannel);
#endif
        params.Finish.post();
    }
#ifdef __USE_CUDA
    if (params.threadId == 0 && params.config->cuda) {
        FreeCuda(cudaDst);
        FreeCuda(cudaSrc);
    }
#endif
    delete []diffusionSeed;
}

void *decryptoAssistant(__IN_OUT void *param) {
    threadParamsMem &params = *static_cast<threadParamsMem *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[params.config->nChannel * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

    decryptoBody(params, rowStart, rowEnd, colStart, colEnd, byteSeq, diffusionSeedArray);

    return new threadReturn{byteSeq, diffusionSeedArray};
}

void *decryptoAssistantWithKeys(__IN_OUT void *param) {
    auto &[params, ret] = *static_cast<threadParamsWithKeyMem *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    CalcRowCols(rowStart, rowEnd, colStart, colEnd, params);
    u8 *byteSeq = ret->byteSeq;
    u8 *diffusionSeedArray = ret->diffusionSeedArray;

    decryptoBody(params, rowStart, rowEnd, colStart, colEnd, byteSeq, diffusionSeedArray);

    return nullptr;
}
