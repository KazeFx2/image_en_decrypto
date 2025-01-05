//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Util.h"
#include "private/Random.h"
#include <cmath>

f64 PLCM(__IN const f64 initialCondition, __IN const f64 controlCondition) {
    assert(initialCondition >= 0 && initialCondition <= 1 && controlCondition > 0 && controlCondition < 0.5);
    if (initialCondition < controlCondition) {
        return initialCondition / controlCondition;
    }
    if (initialCondition <= 0.5) {
        return (initialCondition - controlCondition) / (0.5 - controlCondition);
    }
    return PLCM(1 - initialCondition, controlCondition);
}

f64 IteratePLCM(__IN const f64 initialCondition, __IN const f64 controlCondition, __IN const u32 iterations,
                __OUT f64 *iterationResultArray) {
    if (iterations == 0) return initialCondition;
    iterationResultArray[0] = PLCM(initialCondition, controlCondition);
    for (u32 i = 1; i < iterations; i++)
        iterationResultArray[i] = PLCM(iterationResultArray[i - 1], controlCondition);
    return iterationResultArray[iterations - 1];
}

void CvtF64toBytes(__IN const f64 *iterationResultArray, __OUT u8 *bytes, __IN const u32 length,
                   __IN const u8 bytesReserve) {
    for (u32 i = 0; i < length; i++) {
        memcpy(bytes, &iterationResultArray[i], bytesReserve);
        bytes += bytesReserve;
    }
}

void XorByteSequence(__IN_OUT u8 *bytesA, __IN const u8 *bytesB, __IN const u32 length, __OUT u8 *bytesOut) {
    for (u32 i = 0; i < length; i++) {
        if (bytesOut != nullptr)
            bytesOut[i] = bytesA[i] ^ bytesB[i];
        else
            bytesA[i] ^= bytesB[i];
    }
}

void GenDiffusionSeeds(__IN f64 initialCondition1, __IN const f64 controlCondition1, __IN f64 initialCondition2,
                       __IN const f64 controlCondition2, __OUT u8 *diffusionSeedArray,
                       __IN const u32 diffusionIteration) {
    for (u32 i = 0; i < 3 * diffusionIteration; i++) {
        initialCondition1 = PLCM(initialCondition1, controlCondition1);
        initialCondition2 = PLCM(initialCondition2, controlCondition2);
        diffusionSeedArray[i] = *reinterpret_cast<u8 *>(&initialCondition1) ^ *reinterpret_cast<u8 *>(&
                                    initialCondition2);
    }
}

void ConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const cv::Size &size, __IN const u32 confusionSeed,
                   __OUT u32 &newRow,
                   __OUT u32 &newCol) {
    newRow = (row + col) % size.height;
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * newRow / size.height))) % size.height;
    newCol = (col + tmp) % size.width;
}

void InvertConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const cv::Size &size,
                         __IN const u32 confusionSeed,
                         __OUT u32 &newRow,
                         __OUT u32 &newCol) {
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * row / size.height))) % size.height;
    newCol = (col + size.width - tmp) % size.width;
    newRow = (row + size.height - newCol) % size.height;
}

void Confusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const cv::Size &size, __IN const u32 confusionSeed) {
    for (u32 i = startRow; i < endRow; i++) {
        for (u32 j = startCol; j < endCol; j++) {
            u32 newRow, newCol;
            ConfusionFunc(i, j, size, confusionSeed, newRow, newCol);
            dstImage.at<cv::Vec3b>(newRow, newCol)[0] = srcImage.at<cv::Vec3b>(i, j)[0];
            dstImage.at<cv::Vec3b>(newRow, newCol)[1] = srcImage.at<cv::Vec3b>(i, j)[1];
            dstImage.at<cv::Vec3b>(newRow, newCol)[2] = srcImage.at<cv::Vec3b>(i, j)[2];
        }
    }
}

void InvertConfusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const cv::Size &size, __IN const u32 confusionSeed) {
    for (u32 i = startRow; i < endRow; i++) {
        for (u32 j = startCol; j < endCol; j++) {
            u32 newRow, newCol;
            InvertConfusionFunc(i, j, size, confusionSeed, newRow, newCol);
            dstImage.at<cv::Vec3b>(newRow, newCol)[0] = srcImage.at<cv::Vec3b>(i, j)[0];
            dstImage.at<cv::Vec3b>(newRow, newCol)[1] = srcImage.at<cv::Vec3b>(i, j)[1];
            dstImage.at<cv::Vec3b>(newRow, newCol)[2] = srcImage.at<cv::Vec3b>(i, j)[2];
        }
    }
}

#define DIFFUSION(i, j, Prev) {\
dstImage.at<cv::Vec3b>(i, j)[0] = byteSequence[seqIdx] ^ ((srcImage.at<cv::Vec3b>(i, j)[0] + byteSequence[seqIdx]) % 256) ^ Prev[0];\
seqIdx++;\
dstImage.at<cv::Vec3b>(i, j)[1] = byteSequence[seqIdx] ^ ((srcImage.at<cv::Vec3b>(i, j)[1] + byteSequence[seqIdx]) % 256) ^ Prev[1];\
seqIdx++;\
dstImage.at<cv::Vec3b>(i, j)[2] = byteSequence[seqIdx] ^ ((srcImage.at<cv::Vec3b>(i, j)[2] + byteSequence[seqIdx]) % 256) ^ Prev[2];\
seqIdx++;\
}

void Diffusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx) {
    DIFFUSION(startRow, startCol, diffusionSeed);
    u32 i = startRow, j = startCol;
    if (j + 1 == endCol)
        i++;
    else
        j++;
    u32 prevI = startRow, prevJ = startCol;
    for (; i < endRow; i++) {
        for (; j < endCol; j++) {
            DIFFUSION(i, j, dstImage.at<cv::Vec3b>(prevI, prevJ));
            prevI = i, prevJ = j;
        }
        j = startCol;
    }
}

#define INV_DIFFUSION(i, j, Prev) {\
seqIdx--;\
dstImage.at<cv::Vec3b>(i, j)[2] = \
    ((srcImage.at<cv::Vec3b>(i, j)[2] ^ byteSequence[seqIdx] ^ Prev[2]) + 256 - (byteSequence[seqIdx]));\
seqIdx--;\
dstImage.at<cv::Vec3b>(i, j)[1] = \
    ((srcImage.at<cv::Vec3b>(i, j)[1] ^ byteSequence[seqIdx] ^ Prev[1]) + 256 - (byteSequence[seqIdx]));\
seqIdx--;\
dstImage.at<cv::Vec3b>(i, j)[0] = \
    ((srcImage.at<cv::Vec3b>(i, j)[0] ^ byteSequence[seqIdx] ^ Prev[0]) + 256 - (byteSequence[seqIdx]));\
}

void InvertDiffusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx) {
    u32 nextI = endRow - 1, nextJ = endCol - 1;
    u32 i = endRow - 1, j = endCol - 1;
    if (j == startCol)
        i--;
    else
        j--;
    for (; ; i--) {
        for (; ; j--) {
            INV_DIFFUSION(nextI, nextJ, srcImage.at<cv::Vec3b>(i, j));
            nextI = i, nextJ = j;
            if (j == startCol)
                break;
        }
        if (i == startRow)
            break;
        j = endCol - 1;
    }
    INV_DIFFUSION(nextI, nextJ, diffusionSeed);
}

void PreGenerate(__IN_OUT cv::Mat &Image, __IN_OUT cv::Mat &tmpImage, __IN_OUT cv::Size &Size,
                 __IN_OUT cv::Mat *&dst, __IN_OUT cv::Mat *&src, __IN_OUT u32 *threads, __IN_OUT threadParams *params,
                 __IN const Keys &Keys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *)) {
    const u32 nThread = Config.nThread;
    // resize image if needed
    if (Size.width != 0 && Size.height != 0) {
        const u32 H = std::min(Size.height, Size.width);
        resize(Image, Image, cv::Size(H, H));
    } else {
        const u32 H = std::min(Image.size().height, Image.size().width);
        resize(Image, Image, cv::Size(H, H));
    }
    tmpImage = Image.clone();
    Size = Image.size();
    dst = &tmpImage, src = &Image;
    const u32 iterations = static_cast<int>(3 * (Size.width + nThread) * (Size.height) * Config.
                                            diffusionConfusionIterations
                                            / (nThread * Config.byteReserve));
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
        params[i].size = &Size;
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
        threads[i] = pool.addThread(func, &params[i]);
    }
}

void PreGenerate(__IN_OUT cv::Mat &Image, __IN_OUT cv::Mat &tmpImage, __IN_OUT cv::Size &Size,
                 __IN_OUT cv::Mat *&dst, __IN_OUT cv::Mat *&src, __IN_OUT u32 *threads,
                 __IN_OUT threadParamsWithKey *params,
                 __IN const Keys &Keys,
                 __IN threadReturn **threadKeys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *)) {
    const u32 nThread = Config.nThread;
    // resize image if needed
    if (Size.width != 0 && Size.height != 0) {
        const u32 H = std::min(Size.height, Size.width);
        resize(Image, Image, cv::Size(H, H));
    } else {
        const u32 H = std::min(Image.size().height, Image.size().width);
        resize(Image, Image, cv::Size(H, H));
    }
    tmpImage = Image.clone();
    Size = Image.size();
    dst = &tmpImage, src = &Image;
    const u32 iterations = static_cast<int>(3 * (Size.width + nThread) * (Size.height) * Config.
                                            diffusionConfusionIterations
                                            / (nThread * Config.byteReserve));

    for (u32 i = 0; i < nThread; i++) {
        params[i].params.dst = &dst;
        params[i].params.src = &src;
        params[i].params.size = &Size;
        params[i].params.threadId = i;
        params[i].params.iterations = iterations;
        params[i].params.keys.confusionSeed = Keys.confusionSeed;
        params[i].params.config = &Config;
        params[i].params.threadId = i;
        params[i].ret = threadKeys[i];
        threads[i] = pool.addThread(func, &params[i]);
    }
}

typedef struct {
    Keys keys;
    const ParamControl *config;
    u32 iterations;
} KeyAssParam;

static void *KeyAssist(void *param) {
    auto &[keys, config, iterations] = *static_cast<KeyAssParam *>(param);

    u8 *byteSeq = new u8[iterations * config->byteReserve];
    u8 *diffusionSeedArray = new u8[3 * config->diffusionConfusionIterations];

    f64 *resultArray1 = new f64[iterations];
    f64 *resultArray2 = new f64[iterations];
    u8 *bytesTmp = new u8[iterations * config->byteReserve];

    // const Keys keysBackup = params.keys;
    if (keys.gParam1.ctrlCondition > 0.5)
        keys.gParam1.ctrlCondition = 1 - keys.gParam1.ctrlCondition;
    if (keys.gParam2.ctrlCondition > 0.5)
        keys.gParam2.ctrlCondition = 1 - keys.gParam2.ctrlCondition;
    for (u32 i = 0; i < config->preIterations; i++) {
        keys.gParam1.initCondition = PLCM(
            keys.gParam1.initCondition,
            keys.gParam1.ctrlCondition);
        keys.gParam2.initCondition = PLCM(
            keys.gParam2.initCondition,
            keys.gParam2.ctrlCondition);
    }

    const f64 initCondition1 = IteratePLCM(keys.gParam1.initCondition, keys.gParam1.ctrlCondition,
                                           iterations, resultArray1);
    const f64 initCondition2 = IteratePLCM(keys.gParam2.initCondition, keys.gParam2.ctrlCondition,
                                           iterations, resultArray2);
    CvtF64toBytes(resultArray1, bytesTmp, iterations, config->byteReserve);
    CvtF64toBytes(resultArray2, byteSeq, iterations, config->byteReserve);
    XorByteSequence(byteSeq, bytesTmp, iterations * config->byteReserve);

    GenDiffusionSeeds(initCondition1, keys.gParam1.ctrlCondition,
                      initCondition2, keys.gParam2.ctrlCondition,
                      diffusionSeedArray, config->diffusionConfusionIterations);
    delete [] resultArray1;
    delete [] resultArray2;
    delete [] bytesTmp;

    return new threadReturn{byteSeq, diffusionSeedArray};
}

threadReturn **GenerateThreadKeys(__IN const cv::Size &Size,
                                  __IN const Keys &Keys,
                                  __IN const ParamControl &Config, __IN ThreadPool &pool) {
    const u32 nThread = Config.nThread;
    const u32 iterations = static_cast<int>(3 * (Size.width + nThread) * Size.height * Config.
                                            diffusionConfusionIterations
                                            / (nThread * Config.byteReserve));
    KeyAssParam *params = new KeyAssParam[nThread];
    ThreadPool::thread_descriptor_t *threads = new ThreadPool::thread_descriptor_t[nThread];
    threadReturn **threadReturns = new threadReturn *[nThread];

    ::Keys iterated_keys = Keys;
    // pre-iterate
    for (u32 i = 0; i < Config.preIterations; i++) {
        iterated_keys.gParam1.initCondition = PLCM(iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        iterated_keys.gParam2.initCondition = PLCM(iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
    }
    for (u32 i = 0; i < nThread; i++) {
        params[i].iterations = iterations;
        params[i].keys.confusionSeed = Keys.confusionSeed;
        // generate params
        params[i].keys.gParam1.initCondition = iterated_keys.gParam1.initCondition = PLCM(
                                                   iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        params[i].keys.gParam1.ctrlCondition = iterated_keys.gParam2.initCondition = PLCM(
                                                   iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
        params[i].keys.gParam2.initCondition = iterated_keys.gParam1.initCondition = PLCM(
                                                   iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        params[i].keys.gParam2.ctrlCondition = iterated_keys.gParam2.initCondition = PLCM(
                                                   iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
        params[i].config = &Config;
        threads[i] = pool.addThread(KeyAssist, &params[i]);
    }
    for (u32 i = 0; i < nThread; i++) {
        threadReturns[i] = static_cast<threadReturn *>(pool.waitThread(threads[i]));
    }
    delete[] params;
    delete[] threads;

    return threadReturns;
}

void CalcRowCols(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
                 __IN const threadParams &params) {
    rowStart = (params.size->height * params.threadId / params.config->nThread), rowEnd = (
        params.size->height * (params.threadId + 1) / params.config->nThread);
    colStart = 0, colEnd = params.size->width;
}

void PreAssist(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
               __IN_OUT threadParams &params, __IN_OUT u8 * &byteSeq, __IN_OUT u8 * &diffusionSeedArray) {
    CalcRowCols(rowStart, rowEnd, colStart, colEnd, params);
    f64 *resultArray1 = new f64[params.iterations];
    f64 *resultArray2 = new f64[params.iterations];
    u8 *bytesTmp = new u8[params.iterations * params.config->byteReserve];

    // const Keys keysBackup = params.keys;
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
    CvtF64toBytes(resultArray1, bytesTmp, params.iterations, params.config->byteReserve);
    CvtF64toBytes(resultArray2, byteSeq, params.iterations, params.config->byteReserve);
    XorByteSequence(byteSeq, bytesTmp, params.iterations * params.config->byteReserve, byteSeq);

    GenDiffusionSeeds(initCondition1, params.keys.gParam1.ctrlCondition,
                      initCondition2, params.keys.gParam2.ctrlCondition,
                      diffusionSeedArray, params.config->diffusionConfusionIterations);
    delete [] resultArray1;
    delete [] resultArray2;
    delete [] bytesTmp;
}

void DestroyReturn(__IN threadReturn **ret, __IN const ParamControl &config) {
    for (u32 i = 0; i < config.nThread; i++)
        delete [] ret[i]->byteSeq, delete [] ret[i]->diffusionSeedArray;
    delete [] ret;
}

void DumpBytes(__IN FILE *fd, __IN const char *name, __IN const u8 *array, __IN const u32 size) {
    fprintf(fd, "DumpBytes start\n");
    fprintf(fd, "Name: %s\n", name);
    fprintf(fd, "Bytes size: %u\n", size);
    fprintf(fd, "Data:\n");
    for (u32 i = 0; i < size; i++) {
        fprintf(fd, "%02X ", array[i]);
    }
    fprintf(fd, "\n");
    fprintf(fd, "DumpBytes end\n");
}

#define sprintf(a, ...) snprintf(a, 1024, ##__VA_ARGS__)

void DumpBytes(__IN void *buf, __IN const char *name, __IN const u8 *array, __IN const u32 size) {
    char *p = static_cast<char *>(buf);
    p += sprintf(p, "DumpBytes start\n");
    p += sprintf(p, "Name: %s\n", name);
    p += sprintf(p, "Bytes size: %u\n", size);
    p += sprintf(p, "Data:\n");
    for (u32 i = 0; i < size; i++) {
        p += sprintf(p, "%02X ", array[i]);
    }
    p += sprintf(p, "\n");
    p += sprintf(p, "DumpBytes end\n");
}

void DumpBytes(__IN const char *name, __IN const u8 *array, __IN const u32 size) {
    DumpBytes(stdout, name, array, size);
}

void DumpMat(FILE *fd, const char *name, const cv::Mat &mat) {
    fprintf(fd, "DumpMat start\n");
    fprintf(fd, "Name: %s\n", name);
    fprintf(fd, "Mat size: row: %u, col: %u\n", mat.rows, mat.cols);
    fprintf(fd, "Data:\n[\n");
    // auto type = mat.type();
    // auto depth = mat.depth();
    for (u32 i = 0; i < mat.rows; i++) {
        for (u32 j = 0; j < mat.cols; j++) {
            fprintf(fd, "(%d, %d, %d) ", mat.at<cv::Vec3b>(i, j)[0], mat.at<cv::Vec3b>(i, j)[1],
                    mat.at<cv::Vec3b>(i, j)[2]);
        }
        fprintf(fd, "\n");
    }
    fprintf(fd, "]\n");
    fprintf(fd, "DumpMat end\n");
}
