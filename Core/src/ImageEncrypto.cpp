//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageEncrypto.h"

#include "private/Util.h"

#ifdef __USE_CUDA
#include "private/Cuda.cuh"
#endif

using namespace cv;

#ifdef __DEBUG
static FILE *gfd;
static ::Mutex mu;
#endif

void *encryptoAssistant(__IN void *param);

void *encryptoAssistantWithKeys(__IN void *param);

#ifdef __DEBUG
#define DUMP_MAT {\
    snprintf(name, sizeof(name), "%d_en_dump_mat.txt", dumpId++);\
    snprintf(path, sizeof(path), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/%s", name);\
    FILE *fd = fopen(path, "w+");\
    DumpMat(fd, name, *dst);\
    fclose(fd);\
}
#else
#define DUMP_MAT
#endif

void EncryptoImage(__IN_OUT Mat &Image, __IN_OUT Size &Size,__IN const Keys &Key, __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool) {
    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    auto *params = new threadParamsWithKey[Config.nThread];

    PreGenerate(Image, tmpImage, Size, dst, src, threads, params, Key, threadKeys, Config, pool,
                encryptoAssistantWithKeys);

    // Encrypto confusion
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        {
            for (u32 j = 0; j < Config.nThread; j++)
                params[j].params.Start.post();
            for (u32 j = 0; j < Config.nThread; j++)
                params[j].params.Finish.wait();
        }
        swap(src, dst);
    }
    // Diffusion & Confusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP_KEY;
        swap(src, dst);
        DOLOOP_KEY;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        pool.waitThread(threads[i]);
    }
    delete[] threads;
    delete[] params;
    Image = src->clone();
}

threadReturn **EncryptoImage(__IN_OUT Mat &Image, __IN_OUT Size &Size,__IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool) {
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Encrypto_Procedure.txt");
    gfd = fopen(name, "w+");
#endif

    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    auto *params = new threadParams[Config.nThread];
    auto **ret = new threadReturn *[Config.nThread];

    PreGenerate(Image, tmpImage, Size, dst, src, threads, params, Keys, Config, pool,
                encryptoAssistant);

    // Encrypto confusion
#ifdef __DEBUG
    u32 dumpId = 0;
    char path[256];
#endif
    DUMP_MAT;
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
    }
    // Diffusion & Confusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        ret[i] = static_cast<threadReturn *>(pool.waitThread(threads[i]));
    }
    delete[] threads;
    delete[] params;
    Image = src->clone();

#ifdef __DEBUG
    fclose(gfd);
#endif
    // returns
    return ret;
}

inline void encryptoBody(__IN_OUT threadParams &params,
                         __IN const u32 rowStart,__IN const u32 rowEnd,
                         __IN const u32 colStart,__IN const u32 colEnd,
                         __IN const u8 *byteSeq, __IN const u8 *diffusionSeedArray) {
#ifdef __USE_CUDA
    void *cudaDst = nullptr;
    void *cudaSrc = nullptr;
    if (params.threadId == 0 && params.config->cuda) {
        cudaDst = MallocCuda((*params.src)->cols * (*params.src)->rows * (*params.src)->elemSize());
        cudaSrc = AllocCopyMatToCuda(**params.src);
    }
#endif

    // Encrypto
    u32 seqIdx = 0;
    // Confusion
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Encrypto_%lu.txt", params.threadId);
    FILE *fd = fopen(name, "w+");
    fprintf(fd, "[ENCRYPT]id: %lu, seqIdx: %d\n", params.threadId, seqIdx);
    fprintf(fd, "startRow: %u, endRow: %u, startCol: %u, endCol: %u\n", rowStart, rowEnd, colStart, colEnd);
    DumpBytes(
        fd, "byteSeq", byteSeq, params.iterations * params.config->byteReserve
    );
    DumpBytes(
        fd, "diffusionSeedArray", diffusionSeedArray,
        params.config->nChannel * params.config->diffusionConfusionIterations
    );
    fprintf(fd, "[END ENCRYPT]\n");
    fclose(fd);
#endif
    for (u32 i = 0; i < params.config->confusionIterations; i++) {
        params.Start.wait();
#ifdef __USE_CUDA
        if (params.config->cuda) {
            if (params.threadId == 0) {
                ConfusionCuda(cudaDst, cudaSrc, *params.size, params.keys.confusionSeed, params.config->nChannel);
                if (i + 1 == params.config->confusionIterations) {
                    CopyCudaToMat(**params.dst, cudaDst);
                } else
                    swap(cudaDst, cudaSrc);
            }
        } else
            Confusion(**params.dst,
                      **params.src,
                      rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed,
                      params.config->nChannel);
#else
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
#endif
        params.Finish.post();
    }
    // Diffusion & confusion
    u8 *diffusionSeed = new u8[params.config->nChannel];
    for (u32 i = 0; i < params.config->diffusionConfusionIterations; i++) {
        memcpy(diffusionSeed, diffusionSeedArray + i * params.config->nChannel, params.config->nChannel);
        params.Start.wait();

        Diffusion(**params.dst, **params.src,
                  rowStart, rowEnd, colStart, colEnd,
                  diffusionSeed, byteSeq, seqIdx, params.config->nChannel);
#ifdef __DEBUG
        mu.lock();
        fprintf(gfd, "[DiffusionSeed]id: %lu, Round: %u, %02X, %02X, %02X, idx: %u, ", params.threadId,
                i,
                diffusionSeed[0],
                diffusionSeed[1], diffusionSeed[2], seqIdx);
        fprintf(gfd, "ori_data: ");
        for (u32 _i = rowStart; _i < rowEnd; _i++) {
            for (u32 j = colStart; j < colEnd; j++) {
                fprintf(gfd, "%02X%02X%02X", (*params.src)->at<Vec3b>(_i, j)[0], (*params.src)->at<Vec3b>(_i, j)[1],
                        (*params.src)->at<Vec3b>(_i, j)[2]);
            }
        }
        fprintf(gfd, ", enc_data: ");
        for (u32 _i = rowStart; _i < rowEnd; _i++) {
            for (u32 j = colStart; j < colEnd; j++) {
                fprintf(gfd, "%02X%02X%02X", (*params.dst)->at<Vec3b>(_i, j)[0], (*params.dst)->at<Vec3b>(_i, j)[1],
                        (*params.dst)->at<Vec3b>(_i, j)[2]);
            }
        }
        fprintf(gfd, "\n");
        mu.unlock();
#endif
        params.Finish.post();
        params.Start.wait();
#ifdef __USE_CUDA
        if (params.config->cuda) {
            if (params.threadId == 0) {
                CopyMatToCuda(cudaSrc, **params.src);
                ConfusionCuda(cudaDst, cudaSrc, *params.size, params.keys.confusionSeed, params.config->nChannel);
                CopyCudaToMat(**params.dst, cudaDst);
            }
        } else
            Confusion(**params.dst,
                      **params.src,
                      rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed,
                      params.config->nChannel);
#else
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
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

void *encryptoAssistant(__IN_OUT void *param) {
    threadParams &params = *static_cast<threadParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[params.config->nChannel * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

    encryptoBody(params, rowStart, rowEnd, colStart, colEnd, byteSeq, diffusionSeedArray);

    return new threadReturn{byteSeq, diffusionSeedArray};
}

void *encryptoAssistantWithKeys(__IN_OUT void *param) {
    auto &[params, ret] = *static_cast<threadParamsWithKey *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    CalcRowCols(rowStart, rowEnd, colStart, colEnd, params);
    u8 *byteSeq = ret->byteSeq;
    u8 *diffusionSeedArray = ret->diffusionSeedArray;

    encryptoBody(params, rowStart, rowEnd, colStart, colEnd, byteSeq, diffusionSeedArray);

    return nullptr;
}
