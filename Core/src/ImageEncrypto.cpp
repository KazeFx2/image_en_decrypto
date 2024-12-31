//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageEncrypto.h"

#include <private/Util.h>

using namespace cv;

typedef struct {
    __IN Mat &img;
    __OUT Mat &dst;
    __IN Size &size;
    __IN size_t nThreads;
    __IN size_t threadId;
    __IN Keys &keys;
    __IN ParamControl &config;
} encryptoParams;

void EncryptoImage(__IN_OUT Mat &Image,__IN const std::string &QuantumBitsFile,__IN const ImageSize &Size) {
}

void EncryptoImage(__IN_OUT Mat &Image, __IN const Keys &Keys, __IN const ImageSize &Size) {
    // resize image if needed
    if (Size.width != 0 && Size.height != 0) {
        resize(Image, Image, ::Size(Size.width, Size.height));
    }
}

void *encryptoAssistant(__IN void *param) {
    encryptoParams *params = static_cast<encryptoParams *>(param);
    u32 rowStart, rowEnd, colStart, colEnd;
    rowStart = (params->size.height * params->threadId / params->nThreads), rowEnd = (
        params->size.height * (params->threadId + 1) / params->nThreads);
    colStart = 0, colEnd = params->size.width;
    f64 *resultArray1 = new f64[params->config.iterations];
    f64 *resultArray2 = new f64[params->config.iterations];
    u8 *bytes1 = new u8[params->config.iterations * params->config.byteReserve];
    u8 *bytes2 = new u8[params->config.iterations * params->config.byteReserve];
    u8 *bytesRes = new u8[params->config.iterations * params->config.byteReserve];
    u8 *diffusionSeedArray = new u8[3 * params->config.diffusionIterations];

    const f64 initCondition1 = IteratePLCM(params->keys.gParam1.initCondition, params->keys.gParam1.ctrlCondition,
                                           params->config.iterations, resultArray1);
    const f64 initCondition2 = IteratePLCM(params->keys.gParam2.initCondition, params->keys.gParam2.ctrlCondition,
                                           params->config.iterations, resultArray2);
    CvtF64toBytes(resultArray1, bytes1, params->config.iterations, params->config.byteReserve);
    CvtF64toBytes(resultArray2, bytes2, params->config.iterations, params->config.byteReserve);
    XorByteSequence(bytes1, bytes2, bytesRes, params->config.byteReserve);

    GenDiffusionSeeds(initCondition1, params->keys.gParam1.ctrlCondition,
                      initCondition2, params->keys.gParam2.ctrlCondition,
                      diffusionSeedArray, params->config.diffusionIterations);


}
