//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageDecrypto.h"

#include "private/Util.h"

using namespace cv;

#ifdef __DEBUG
static FILE *gfd;
static ::Mutex mu;
#endif

void *decryptoAssistantWithKeys(__IN void *param);

void *decryptoAssistant(__IN void *param);

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
    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    threadParamsWithKey *params = new threadParamsWithKey[Config.nThread];

    PreGenerate(Image, tmpImage, Size, dst, src, threads, params, Key, threadKeys, Config, pool,
                decryptoAssistantWithKeys);

    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP_KEY;
        swap(src, dst);
        DOLOOP_KEY;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.confusionIterations; i++) {
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

threadReturn **DecryptoImage(__IN_OUT Mat &Image, __IN_OUT Size &Size,__IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool) {
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Decrypto_Procedure.txt");
    gfd = fopen(name, "w+");
#endif

    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    threadParams *params = new threadParams[Config.nThread];
    threadReturn **ret = new threadReturn *[Config.nThread];

    PreGenerate(Image, tmpImage, Size, dst, src, threads, params, Keys, Config, pool,
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
    Image = src->clone();
#ifdef __DEBUG
    fclose(gfd);
#endif
    // returns
    return ret;
}

void *decryptoAssistant(__IN_OUT void *param) {
    threadParams &params = *static_cast<threadParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[params.config->nChannel * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

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

    for (u32 i = params.config->diffusionConfusionIterations - 1; ; i--) {
        u8 diffusionSeed[3];
        memcpy(diffusionSeed, diffusionSeedArray + i * 3, 3);
        params.Start.wait();
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
        params.Start.wait();
#ifdef __DEBUG
        mu.lock();
        fprintf(gfd, "[DiffusionSeed]id: %lu, Round: %u, %02X, %02X, %02X, idx: %u, ", params.threadId,
                i,
                diffusionSeed[0],
                diffusionSeed[1], diffusionSeed[2], seqIdx);
#endif
        InvertDiffusion(**params.dst, **params.src,
                        rowStart, rowEnd, colStart, colEnd,
                        diffusionSeed, byteSeq, seqIdx);
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
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
    }
    return new threadReturn{byteSeq, diffusionSeedArray};
}

void *decryptoAssistantWithKeys(__IN_OUT void *param) {
    auto &[params, ret] = *static_cast<threadParamsWithKey *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    CalcRowCols(rowStart, rowEnd, colStart, colEnd, params);
    u8 *byteSeq = ret->byteSeq;
    u8 *diffusionSeedArray = ret->diffusionSeedArray;

    u32 seqIdx = params.config->nChannel * (rowEnd - rowStart) * (colEnd - colStart) * params.config->
                 diffusionConfusionIterations;
#ifdef __DEBUG
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Decrypto_%lu.txt", params.threadId);
    FILE *fd = fopen(name, "w+");
    fprintf(fd, "[DECRYPT]id: %lu, seqIdx: %d, confSeed: %d\n", params.threadId, seqIdx, params.keys.confusionSeed);
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
    for (u32 i = params.config->diffusionConfusionIterations - 1; ; i--) {
        u8 diffusionSeed[3];
        memcpy(diffusionSeed, diffusionSeedArray + i * 3, 3);
        params.Start.wait();
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
        params.Start.wait();
        InvertDiffusion(**params.dst, **params.src,
                        rowStart, rowEnd, colStart, colEnd,
                        diffusionSeed, byteSeq, seqIdx);
        params.Finish.post();
        if (i == 0)
            break;
    }
    for (u32 i = 0; i < params.config->confusionIterations; i++) {
        params.Start.wait();
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
    }
    return nullptr;
}
