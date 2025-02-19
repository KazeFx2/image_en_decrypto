//
// Created by Fx Kaze on 24-12-30.
//

#include "private/testImageEnDecrypto.h"

#include "private/Util.h"

using namespace cv;

void *en_decryptoAssistant(__IN void *param);

#define DUMP_MAT {\
    snprintf(name, sizeof(name), "test_%d_en_de_dump_mat.txt", dumpId++);\
    snprintf(path, sizeof(path), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/%s", name);\
    FILE *fd = fopen(path, "w+");\
    DumpMat(fd, name, *dst);\
    fclose(fd);\
}

threadReturn **testEnDecryptoImage(__IN_OUT Mat &Image, __IN_OUT cv::Size &Size,__IN const Keys &Keys,
                                   __IN const ParamControl &Config, __IN ThreadPool &pool) {
    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    auto *params = new threadParams[Config.nThread];
    auto **ret = new threadReturn *[Config.nThread];

    PreGenerate(Image, tmpImage, Size, dst, src, threads, params, Keys, Config, pool,
                en_decryptoAssistant);

    // Encrypto confusion
    u32 dumpId = 0;
    char name[64];
    char path[256];
    // row
    DUMP_MAT;
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        // en
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // de
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // en
        DOLOOP;
        // DUMP_MAT;
        swap(src, dst);
    }
    // Diffusion & Confusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        // en
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // de
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // en
        DOLOOP;
        // DUMP_MAT;
        swap(src, dst);
        // en
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // de
        DOLOOP;
        DUMP_MAT;
        swap(src, dst);
        // en
        DOLOOP;
        // DUMP_MAT;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        ret[i] = static_cast<threadReturn *>(pool.waitThread(threads[i]));
    }
    delete[] threads;
    delete[] params;
    Image = src->clone();
    // returns
    return ret;
}

void *en_decryptoAssistant(__IN_OUT void *param) {
    threadParams &params = *static_cast<threadParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[3 * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

    // Encrypto
    u32 seqIdx = 0;
    // Confusion
    char name[256];
    snprintf(name, sizeof(name), "/Users/kazefx/毕设/Code/ImageEn_Decrypto/outputs/Encrypto_%lu.txt", params.threadId);
    FILE *fd = fopen(name, "w+");
    fprintf(fd, "[ENCRYPT]id: %lu, seqIdx: %d\n", params.threadId, seqIdx);
    fprintf(fd, "startRow: %u, endRow: %u, startCol: %u, endCol: %u\n", rowStart, rowEnd, colStart, colEnd);
    DumpBytes(
        fd, "byteSeq", byteSeq, params.iterations * params.config->byteReserve
    );
    DumpBytes(
        fd, "diffusionSeedArray", diffusionSeedArray, 3 * params.config->diffusionConfusionIterations
    );
    fprintf(fd, "[END ENCRYPT]\n");
    fclose(fd);

    for (u32 i = 0; i < params.config->confusionIterations; i++) {
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed,
                        params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
        params.Finish.post();
    }
    // Diffusion & confusion
    for (u32 i = 0; i < params.config->diffusionConfusionIterations; i++) {
        u8 diffusionSeed[params.config->nChannel];
        memcpy(diffusionSeed, diffusionSeedArray + i * params.config->nChannel, params.config->nChannel);
        params.Start.wait();
        Diffusion(**params.dst, **params.src,
                  rowStart, rowEnd, colStart, colEnd,
                  diffusionSeed, byteSeq, seqIdx, params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        InvertDiffusion(**params.dst, **params.src,
                        rowStart, rowEnd, colStart, colEnd,
                        diffusionSeed, byteSeq, seqIdx, params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        Diffusion(**params.dst, **params.src,
                  rowStart, rowEnd, colStart, colEnd,
                  diffusionSeed, byteSeq, seqIdx, params.config->nChannel);
        params.Finish.post();
        ///
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        InvertConfusion(**params.dst,
                        **params.src,
                        rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed,
                        params.config->nChannel);
        params.Finish.post();
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed, params.config->nChannel);
        params.Finish.post();
    }
    return new threadReturn{byteSeq, diffusionSeedArray};
}
