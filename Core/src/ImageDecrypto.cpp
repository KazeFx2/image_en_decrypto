//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageDecrypto.h"

#include "private/Util.h"

using namespace cv;

void *decryptoAssistant(__IN void *param);

void DecryptoImage(__IN_OUT Mat &Image, __IN const ImageSize &Size,__IN const Keys &Keys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool) {
    cv::Size ImageSize;
    Mat tmpImage;
    Mat *dst, *src;
    u32 *threads = new u32[Config.nThread];
    threadParams *params = new threadParams[Config.nThread];
    threadReturn **ret = new threadReturn *[Config.nThread];

    PreGenerate(Image, tmpImage, ImageSize, dst, src, threads, params, Size, Keys, Config, Config.nThread, pool,
                decryptoAssistant);

    // Decrypto inv_confusion & inv_diffusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP;
        swap(src, dst);
        DOLOOP;
        swap(src, dst);
    }
    // Inv_confusion
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        DOLOOP;
        swap(src, dst);
    }
    for (u32 i = 0; i < Config.nThread; i++) {
        ret[i] = static_cast<threadReturn *>(pool.waitThread(threads[i]));
    }
    delete[] threads;
    delete[] params;
    Image = src->clone();
    // returns
    for (u32 i = 0; i < Config.nThread; i++) {
        delete []ret[i]->byteSeq;
        delete []ret[i]->diffusionSeedArray;
    }
    delete[] ret;
}

void *decryptoAssistant(__IN_OUT void *param) {
    threadParams &params = *static_cast<threadParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[3 * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

    // Decrypto
    u32 seqIdx = 3 * (rowEnd - rowStart) * (colEnd - colStart) * params.config->diffusionConfusionIterations;
    // Inv-confusion & inv-diffusion
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
