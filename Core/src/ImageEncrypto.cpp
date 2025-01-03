//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageEncrypto.h"

#include <private/Util.h>

using namespace cv;

typedef struct {
    __OUT Mat **dst;
    __IN Mat **src;
    __IN Size *size;
    __IN size_t nThreads;
    __IN size_t threadId;
    __IN u32 iterations;
    __IN Keys keys;
    __IN const ParamControl *config;
    __IN Semaphore Start;
    __IN Semaphore Finish;
} encryptoParams;

typedef struct {
    __OUT u8 *byteSeq;
    __OUT u8 *diffusionSeedArray;
} encryptoReturn;

void *encryptoAssistant(__IN void *param);

void EncryptoImage(__IN_OUT Mat &Image,__IN const std::string &QuantumBitsFile,__IN const ImageSize &Size) {
}

#define DOLOOP \
{\
    for (u32 j = 0; j < nThread; j++)\
        params[j].Start.post();\
    for (u32 j = 0; j < nThread; j++)\
        params[j].Finish.wait();\
}

void EncryptoImage(__IN_OUT Mat &Image, __IN const ImageSize &Size,__IN const Keys &Keys,
                   __IN const ParamControl &Config, __IN const u32 nThread, __IN ThreadPool &pool) {
    // resize image if needed
    if (Size.width != 0 && Size.height != 0) {
        resize(Image, Image, ::Size(Size.width, Size.height));
    }
    ::Size ImageSize = Image.size();
    Mat tmpImage = Image.clone();
    Mat *dst = &tmpImage, *src = &Image;
    u32 *threads = new u32[nThread];
    encryptoParams *params = new encryptoParams[nThread];
    encryptoReturn **ret = new encryptoReturn *[nThread];
    const u32 iterations = static_cast<int>(3 * ImageSize.width * ImageSize.height * Config.diffusionConfusionIterations
                                            / (
                                                nThread * Config.byteReserve)) + 1;
    ::Keys iterated_keys = Keys;
    // pre-iterate
    for (u32 i = 0; i < Config.preIterations; i++) {
        iterated_keys.gParam1.initCondition = PLCM(iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        iterated_keys.gParam2.initCondition = PLCM(iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
    }
    ::Keys tmpKeys = iterated_keys;

    for (u32 i = 0; i < nThread; i++) {
        params[i].dst = &dst;
        params[i].src = &src;
        params[i].size = &ImageSize;
        params[i].nThreads = nThread;
        params[i].threadId = i;
        params[i].iterations = iterations;
        params[i].keys.confusionSeed = Keys.confusionSeed;
        // generate params
        params[i].keys.gParam1.initCondition = tmpKeys.gParam1.initCondition = PLCM(
                                                   tmpKeys.gParam1.initCondition,
                                                   tmpKeys.gParam1.ctrlCondition);
        params[i].keys.gParam1.ctrlCondition = tmpKeys.gParam2.initCondition = PLCM(
                                                   tmpKeys.gParam2.initCondition,
                                                   tmpKeys.gParam2.ctrlCondition);
        params[i].keys.gParam2.initCondition = tmpKeys.gParam1.initCondition = PLCM(
                                                   tmpKeys.gParam1.initCondition,
                                                   tmpKeys.gParam1.ctrlCondition);
        params[i].keys.gParam2.ctrlCondition = tmpKeys.gParam2.initCondition = PLCM(
                                                   tmpKeys.gParam2.initCondition,
                                                   tmpKeys.gParam2.ctrlCondition);
        // params[i].keys = Keys;
        params[i].config = &Config;
        params[i].threadId = i;
        threads[i] = pool.addThread(encryptoAssistant, &params[i]);
    }
    // Encrypto confusion
    for (u32 i = 0; i < Config.confusionIterations; i++) {
        DOLOOP;
        swap(src, dst);
    }
    // Diffusion & Confusion
    for (u32 i = 0; i < Config.diffusionConfusionIterations; i++) {
        DOLOOP;
        swap(src, dst);
        DOLOOP;
        swap(src, dst);
    }
    for (u32 i = 0; i < nThread; i++) {
        ret[i] = static_cast<encryptoReturn *>(pool.waitThread(threads[i]));
    }
    delete[] threads;
    delete[] params;
    Image = src->clone();
    // returns
    for (u32 i = 0; i < nThread; i++) {
        delete []ret[i]->byteSeq;
        delete []ret[i]->diffusionSeedArray;
    }
    delete[] ret;
}

void *encryptoAssistant(__IN void *param) {
    encryptoParams &params = *static_cast<encryptoParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    rowStart = (params.size->height * params.threadId / params.nThreads), rowEnd = (
        params.size->height * (params.threadId + 1) / params.nThreads);
    colStart = 0, colEnd = params.size->width;
    f64 *resultArray1 = new f64[params.iterations];
    f64 *resultArray2 = new f64[params.iterations];
    u8 *bytes1 = new u8[params.iterations * params.config->byteReserve];
    u8 *bytes2 = new u8[params.iterations * params.config->byteReserve];
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[3 * params.config->diffusionConfusionIterations];

    // const Keys keysBackup = params.keys;
    // assert(params.keys.gParam1.ctrlCondition <= 0.5 &&
    //     params.keys.gParam2.ctrlCondition <= 0.5);
    if (params.keys.gParam1.ctrlCondition > 0.5)
        params.keys.gParam1.ctrlCondition = 1 - params.keys.gParam1.ctrlCondition;
    if (params.keys.gParam2.ctrlCondition > 0.5)
        params.keys.gParam2.ctrlCondition = 1 - params.keys.gParam2.ctrlCondition;
    for (u32 i = 0; i < params.config->preIterations; i++) {
        params.keys.gParam1.initCondition = PLCM(
            params.keys.gParam1.initCondition,
            params.keys.gParam1.ctrlCondition);
        params.keys.gParam2.initCondition = PLCM(
            params.keys.gParam2.initCondition,
            params.keys.gParam2.ctrlCondition);
    }

    const f64 initCondition1 = IteratePLCM(params.keys.gParam1.initCondition, params.keys.gParam1.ctrlCondition,
                                           params.iterations, resultArray1);
    const f64 initCondition2 = IteratePLCM(params.keys.gParam2.initCondition, params.keys.gParam2.ctrlCondition,
                                           params.iterations, resultArray2);
    CvtF64toBytes(resultArray1, bytes1, params.iterations, params.config->byteReserve);
    CvtF64toBytes(resultArray2, bytes2, params.iterations, params.config->byteReserve);
    XorByteSequence(bytes1, bytes2, byteSeq, params.config->byteReserve);

    GenDiffusionSeeds(initCondition1, params.keys.gParam1.ctrlCondition,
                      initCondition2, params.keys.gParam2.ctrlCondition,
                      diffusionSeedArray, params.config->diffusionConfusionIterations);
    delete [] resultArray1;
    delete [] resultArray2;
    delete [] bytes1;
    delete [] bytes2;
    // Encrypto
    // Confusion
    for (u32 i = 0; i < params.config->confusionIterations; i++) {
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
    }
    // Diffusion
    u32 seqIdx = 0;
    for (u32 i = 0; i < params.config->diffusionConfusionIterations; i++) {
        u8 diffusionSeed[3];
        params.Start.wait();
        memcpy(diffusionSeed, diffusionSeedArray + i * 3, 3);
        Diffusion(**params.dst, **params.src,
                  rowStart, rowEnd, colStart, colEnd,
                  diffusionSeed, byteSeq, seqIdx);
        params.Finish.post();
        params.Start.wait();
        Confusion(**params.dst,
                  **params.src,
                  rowStart, rowEnd, colStart, colEnd, *params.size, params.keys.confusionSeed);
        params.Finish.post();
    }
    return new encryptoReturn{byteSeq, diffusionSeedArray};
}
