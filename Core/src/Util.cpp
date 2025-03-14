//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Util.h"
#include "private/Random.h"
#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif
#include <cmath>
#include <filesystem>

#define CALC_ITERATIONS_WH(width, height) const u32 iterations = static_cast<int>(Config.nChannel * ((width) + nThread) * ((height) + nThread) * Config.\
diffusionConfusionIterations \
/ (nThread * Config.byteReserve))

#define CALC_ITERATIONS CALC_ITERATIONS_WH(Size.width, Size.height)

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
                       __IN const u32 diffusionIteration, __IN const u8 nChannel) {
    for (u32 i = 0; i < nChannel * diffusionIteration; i++) {
        initialCondition1 = PLCM(initialCondition1, controlCondition1);
        initialCondition2 = PLCM(initialCondition2, controlCondition2);
        diffusionSeedArray[i] = *reinterpret_cast<u8 *>(&initialCondition1) ^ *reinterpret_cast<u8 *>(&
                                    initialCondition2);
    }
}

void ConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const u32 width, __IN const u32 height,
                   __IN const u32 confusionSeed, __OUT u32 &newRow,
                   __OUT u32 &newCol) {
    newRow = (row + col) % height;
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * newRow / height))) % width;
    newCol = (col + tmp) % width;
}

void InvertConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const u32 width, __IN const u32 height,
                         __IN const u32 confusionSeed,
                         __OUT u32 &newRow,
                         __OUT u32 &newCol) {
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * row / height))) % width;
    newCol = (col + width - tmp) % width;
    newRow = (row + height - newCol % height) % height;
}

void Confusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u32 width, __IN const u32 height, __IN const u32 confusionSeed, __IN const u8 nChannel) {
    if (startRow >= endRow || startCol >= endCol) return;
    for (u32 i = startRow; i < endRow; i++) {
        auto src = srcImage + (i * width + startCol) * nChannel;
        for (u32 j = startCol; j < endCol; j++) {
            u32 newRow, newCol;
            ConfusionFunc(i, j, width, height, confusionSeed, newRow, newCol);
            auto dst = dstImage + (newRow * width + newCol) * nChannel;
            u8 n = nChannel;
            while (n--) {
                *dst++ = *src++;
            }
        }
    }
}

void InvertConfusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const u32 width, __IN const u32 height, __IN const u32 confusionSeed,
                     __IN const u8 nChannel) {
    if (startRow >= endRow || startCol >= endCol) return;
    for (u32 i = startRow; i < endRow; i++) {
        auto src = srcImage + (i * width + startCol) * nChannel;
        for (u32 j = startCol; j < endCol; j++) {
            u32 newRow, newCol;
            InvertConfusionFunc(i, j, width, height, confusionSeed, newRow, newCol);
            auto dst = dstImage + (newRow * width + newCol) * nChannel;
            u8 n = nChannel;
            while (n--) {
                *dst++ = *src++;
            }
        }
    }
}

class iterator {
public:
    iterator(cv::Mat &Mat, const u32 startRow, const u32 endRow, const u32 startCol, const u32 endCol,
             u8 *diffusionSeed, const u8 nChannel, const u8 offset = 0,
             const bool front = false): mat(Mat), startRow(startRow), endRow(endRow), startCol(startCol),
                                        endCol(endCol),
                                        diffusionSeed(diffusionSeed), nChannel(nChannel), front(front),
                                        nCt(offset % nChannel) {
        if (front) {
            nowPtr = Mat.ptr(static_cast<i32>(startRow), static_cast<i32>(startCol)) + nCt;
            nowRow = startRow, nowCol = startCol;
        } else {
            nowPtr = Mat.ptr(static_cast<i32>(endRow - 1), static_cast<i32>(endCol - 1)) + nCt;
            nowRow = endRow - 1, nowCol = endCol - 1;
        }
    }

    iterator(cv::Mat &Mat, const u32 startRow, const u32 endRow, const u32 startCol, const u32 endCol,
             u8 *diffusionSeed, const u8 nChannel, const u32 nowRow,
             const u32 nowCol, const u8 offset = 0): mat(Mat), startRow(startRow), endRow(endRow), startCol(startCol),
                                                     endCol(endCol),
                                                     diffusionSeed(diffusionSeed), nChannel(nChannel), front(false),
                                                     nCt(offset % nChannel) {
        nowPtr = Mat.ptr(static_cast<i32>(nowRow), static_cast<i32>(nowCol)) + nCt;
        this->nowRow = nowRow, this->nowCol = nowCol;
    }

    operator u8 *() const {
        if (front) {
            return diffusionSeed + nCt;
        }
        return nowPtr;
    }

    u8 *operator++() {
        if (front) {
            nCt++;
            const auto tmp = nCt;
            if (nCt == nChannel) {
                nCt = 0;
                front = false;
            }
            return diffusionSeed + tmp;
        }
        nCt++;
        if (nCt == nChannel) {
            nCt = 0;
            nowCol++;
            if (nowCol == endCol) {
                nowCol = startCol;
                nowRow++;
                if (nowRow == endRow) {
                    nowRow = endRow - 1;
                    nowCol = endCol - 1;
                }
                nowPtr = mat.ptr(static_cast<i32>(nowRow), static_cast<i32>(nowCol));
                return nowPtr;
            }
            return ++nowPtr;
        }
        return ++nowPtr;
    }

    u8 *operator++(int) {
        u8 *tmp = *this;
        operator++();
        return tmp;
    }

    u8 *operator--() {
        if (front) {
            if (nCt == 0) {
                return diffusionSeed;
            }
            const auto tmp = nCt;
            nCt--;
            return diffusionSeed + tmp;
        }
        if (nCt == 0) {
            nCt = nChannel - 1;
            if (nowCol == startCol) {
                if (nowRow == startRow) {
                    front = true;
                    return diffusionSeed + nChannel;
                }
                nowCol = endCol - 1;
                nowRow--;
                nowPtr = mat.ptr(static_cast<i32>(nowRow), static_cast<i32>(nowCol)) + nCt;
                return nowPtr;
            }
            nowCol--;
            return --nowPtr;
        }
        nCt--;
        return --nowPtr;
    }

    u8 *operator--(int) {
        u8 *tmp = *this;
        operator--();
        return tmp;
    }

private:
    cv::Mat &mat;
    u32 startRow, endRow, startCol, endCol;
    u8 *diffusionSeed;
    u32 nowRow, nowCol;
    u8 nChannel;
    bool front;
    u8 nCt;
    u8 *nowPtr;
};

#define DIFFUSION(dst, src, Prev) {\
    auto n = nChannel;\
    while (n--) {\
        *(dst)++ = byteSequence[seqIdx] ^ ((*(src)++ + byteSequence[seqIdx]) % 256) ^ *(Prev)++;\
        seqIdx++;\
    }\
}

void Diffusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u32 width, __IN const u32 height,
               __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx,
               __IN const u8 nChannel) {
    if (startRow >= endRow || startCol >= endCol) return;
    auto dst = dstImage + (startRow * width + startCol) * nChannel;
    auto src = srcImage + (startRow * width + startCol) * nChannel;
    auto nPrev = dst;
    DIFFUSION(dst, src, diffusionSeed);
    auto prev = nPrev;
    u32 i = startRow, j = startCol + 1;
    while (true) {
        for (; j < endCol; j++) {
            DIFFUSION(dst, src, prev);
        }
        i++;
        if (i == endRow) {
            break;
        }
        j = startCol + 1;
        dst = dstImage + (i * width + startCol) * nChannel;
        src = srcImage + (i * width + startCol) * nChannel;
        nPrev = dst;
        DIFFUSION(dst, src, prev);
        prev = nPrev;
    }
}

#define INV_DIFFUSION(dst, src, Prev) {\
    auto n = nChannel;\
    while (n--) {\
        seqIdx--;\
        *(dst)-- = (*(src)-- ^ byteSequence[seqIdx] ^ *(Prev)--) + 256 - byteSequence[seqIdx];\
    }\
}

void InvertDiffusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const u32 width, __IN const u32 height,
                     __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx,
                     __IN const u8 nChannel) {
    if (startRow >= endRow || startCol >= endCol) return;
    u32 i = endRow - 1, j = endCol - 1;
    auto nextDst = dstImage + (i * width + j) * nChannel + nChannel - 1;
    auto nextSrc = srcImage + (i * width + j) * nChannel + nChannel - 1;
    // auto dst = nextDst - nChannel;
    u8 *dst = nullptr;
    auto src = nextSrc - nChannel;
    goto INNER;
    while (true) {
        for (;; j--) {
            INV_DIFFUSION(nextDst, nextSrc, src)
        INNER:
            if (j == startCol) {
                break;
            }
        }
        if (i == startRow) {
            break;
        }
        i--;
        j = endCol - 1;
        dst = dstImage + (i * width + j) * nChannel + nChannel - 1;
        src = srcImage + (i * width + j) * nChannel + nChannel - 1;
        const auto srcBak = src;
        INV_DIFFUSION(nextDst, nextSrc, src)
        nextDst = dst;
        nextSrc = srcBak;
        goto INNER;
    }
    diffusionSeed += nChannel - 1;
    INV_DIFFUSION(nextDst, nextSrc, diffusionSeed)
}

void PreGenerate(__IN_OUT u8 *Image, __IN_OUT u8 *&tmpImage, __IN const u32 width, __IN_OUT const u32 height,
                 __IN_OUT u8 *&dst, __IN_OUT u8 *&src, __IN_OUT u32 *threads, __IN_OUT threadParamsMem *params,
                 __IN const Keys &Keys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *)) {
    const u32 nThread = Config.nThread;
    tmpImage = new u8[width * height * Config.nChannel];
    memcpy(tmpImage, Image, width * height * Config.nChannel);
    dst = tmpImage, src = Image;
    CALC_ITERATIONS_WH(width, height);

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
        params[i].width = width;
        params[i].height = height;
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

void PreGenerate(__IN_OUT u8 *Image, __IN_OUT u8 *&tmpImage, __IN const u32 width, __IN_OUT const u32 height,
                 __IN_OUT u8 *&dst, __IN_OUT u8 *&src, __IN_OUT u32 *threads,
                 __IN_OUT threadParamsWithKeyMem *params,
                 __IN const Keys &Keys,
                 __IN threadReturn **threadKeys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *)) {
    const u32 nThread = Config.nThread;
    tmpImage = new u8[width * height * Config.nChannel];
    memcpy(tmpImage, Image, width * height * Config.nChannel);
    dst = tmpImage, src = Image;
    CALC_ITERATIONS_WH(width, height);

    for (u32 i = 0; i < nThread; i++) {
        params[i].params.dst = &dst;
        params[i].params.src = &src;
        params[i].params.width = width;
        params[i].params.height = height;
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

    auto *byteSeq = new u8[iterations * config->byteReserve];
    auto *diffusionSeedArray = new u8[config->nChannel * config->diffusionConfusionIterations];

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
                      diffusionSeedArray, config->diffusionConfusionIterations, config->nChannel);
    delete [] resultArray1;
    delete [] resultArray2;
    delete [] bytesTmp;

    return new threadReturn{byteSeq, diffusionSeedArray};
}

threadReturn **GenerateThreadKeys(__IN const cv::Size &Size,
                                  __IN const Keys &Keys,
                                  __IN const ParamControl &Config, __IN ThreadPool &pool) {
    const u32 nThread = Config.nThread;
    CALC_ITERATIONS;

    auto *params = new KeyAssParam[nThread];
    auto *threads = new ThreadPool::task_descriptor_t[nThread];
    auto **threadReturns = new threadReturn *[nThread];

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
                 __IN const threadParamsMem &params) {
    rowStart = (params.height * params.threadId / params.config->nThread), rowEnd = (
        params.height * (params.threadId + 1) / params.config->nThread);
    colStart = 0, colEnd = params.width;
}

void PreAssist(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
               __IN_OUT threadParamsMem &params, __IN_OUT u8 * &byteSeq, __IN_OUT u8 * &diffusionSeedArray) {
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
                      diffusionSeedArray, params.config->diffusionConfusionIterations, params.config->nChannel);
    delete [] resultArray1;
    delete [] resultArray2;
    delete [] bytesTmp;
}

void DestroyReturn(__IN threadReturn **ret, __IN const ParamControl &config) {
    if (!ret) return;
    for (u32 i = 0; i < config.nThread; i++)
        delete [] ret[i]->byteSeq, delete [] ret[i]->diffusionSeedArray, delete ret[i];
    delete [] ret;
}

threadReturn **CopyReturn(__IN threadReturn **other, __IN const ParamControl &Config, __IN const cv::Size &Size) {
    if (!other) return nullptr;
    auto nThread = Config.nThread;
    CALC_ITERATIONS;
    threadReturn **ret = new threadReturn *[Config.nThread];
    for (u32 i = 0; i < Config.nThread; i++) {
        ret[i] = new threadReturn{
            new u8[iterations * Config.byteReserve], new u8[Config.nChannel * Config.diffusionConfusionIterations]
        };
        memcpy(ret[i]->byteSeq, other[i]->byteSeq, iterations * Config.byteReserve);
        memcpy(ret[i]->diffusionSeedArray, other[i]->diffusionSeedArray,
               Config.nChannel * Config.diffusionConfusionIterations);
    }
    return ret;
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

#define PRT_ELEM(type, mat, fd, i, j) \
    {\
        switch ((type)) {\
            case CV_8SC1:\
                fprintf(fd, "(%d) ", (mat).at<cv::int8_t>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_8UC1:\
                fprintf(fd, "(%d) ", (mat).at<cv::uint8_t>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_8SC2:\
            case CV_8UC2:\
                fprintf(fd, "(%d, %d) ", (mat).at<cv::Vec2b>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2b>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_8SC3:\
            case CV_8UC3:\
                fprintf(fd, "(%d, %d, %d) ", (mat).at<cv::Vec3b>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3b>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3b>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
            case CV_16SC1:\
                fprintf(fd, "(%d) ", (mat).at<cv::int16_t>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_16UC1:\
                fprintf(fd, "(%d) ", (mat).at<cv::uint16_t>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_16SC2:\
                fprintf(fd, "(%d, %d) ", (mat).at<cv::Vec2w>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2w>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_16UC2:\
                fprintf(fd, "(%d, %d) ", (mat).at<cv::Vec2s>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2s>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_16SC3:\
                fprintf(fd, "(%d, %d, %d) ", (mat).at<cv::Vec3w>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3w>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3w>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
            case CV_16UC3:\
                fprintf(fd, "(%d, %d, %d) ", (mat).at<cv::Vec3s>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3s>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3s>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
            case CV_32SC1:\
                fprintf(fd, "(%d) ", (mat).at<cv::int32_t>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_32SC2:\
                fprintf(fd, "(%d, %d) ", (mat).at<cv::Vec2i>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2i>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_32SC3:\
                fprintf(fd, "(%d, %d, %d) ", (mat).at<cv::Vec3i>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3i>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3i>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
            case CV_32FC1:\
                fprintf(fd, "(%f) ", (mat).at<float>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_32FC2:\
                fprintf(fd, "(%f, %f) ", (mat).at<cv::Vec2f>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2f>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_32FC3:\
                fprintf(fd, "(%f, %f, %f) ", (mat).at<cv::Vec3f>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3f>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3f>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
            case CV_64FC1:\
                fprintf(fd, "(%lf) ", (mat).at<double>(static_cast<i32>(i), static_cast<i32>(j)));\
                break;\
            case CV_64FC2:\
                fprintf(fd, "(%lf, %lf) ", (mat).at<cv::Vec2d>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec2d>(static_cast<i32>(i), static_cast<i32>(j))[1]);\
                break;\
            case CV_64FC3:\
                fprintf(fd, "(%lf, %lf, %lf) ", (mat).at<cv::Vec3d>(static_cast<i32>(i), static_cast<i32>(j))[0], \
                    (mat).at<cv::Vec3d>(static_cast<i32>(i), static_cast<i32>(j))[1], \
                    (mat).at<cv::Vec3d>(static_cast<i32>(i), static_cast<i32>(j))[2]);\
                break;\
        }\
    }


void DumpMat(FILE *fd, const char *name, const cv::Mat &mat) {
    fprintf(fd, "DumpMat start\n");
    fprintf(fd, "Name: %s\n", name);
    fprintf(fd, "Mat size: row: %u, col: %u\n", mat.rows, mat.cols);
    fprintf(fd, "Data:\n[\n");
    const auto type = mat.type();
    for (u32 i = 0; i < mat.rows; i++) {
        for (u32 j = 0; j < mat.cols; j++) {
            PRT_ELEM(type, mat, fd, i, j);
        }
        fprintf(fd, "\n");
    }
    fprintf(fd, "]\n");
    fprintf(fd, "DumpMat end\n");
}

void LoadConfKey(ParamControl &params, Keys &Keys, const char *path) {
    FILE *fd = fopen(path, "r");
    if (fd) {
        fread(&params, sizeof(ParamControl), 1, fd);
        fread(&Keys, sizeof(Keys), 1, fd);
        fclose(fd);
    }
}

void SaveConfKey(const ParamControl &params, const Keys &Keys, const char *path) {
#ifdef _WIN32
    std::string tmp = path;
    tmp = UTF8toGBK(tmp);
#else
    std::string tmp = path;
#endif
    std::string file = FileUniqueForceSuffix(tmp.c_str(), "ikey");
    FILE *fd = fopen(file.c_str(), "w");
    if (fd) {
        fwrite(&params, sizeof(ParamControl), 1, fd);
        fwrite(&Keys, sizeof(Keys), 1, fd);
        fclose(fd);
    }
}

bool StructureEqual(__IN const void *a,__IN const void *b, __IN u32 size) {
    auto t_a = static_cast<const char *>(a);
    auto t_b = static_cast<const char *>(b);
    for (u32 i = 0; i < size; i++) {
        if (t_a[i] != t_b[i]) return false;
    }
    return true;
}

bool FileExists(__IN const char *path) {
    return std::filesystem::exists(path);
}

std::string FileUnique(__IN const char *path, __IN const char *suf) {
    std::string file = path;
    auto pos = file.rfind(".");
    std::string name, suffix;
    if (pos == std::string::npos) {
        name = file;
        suffix = suf;
    } else {
        name = file.substr(0, pos);
        suffix = file.substr(pos + 1);
    }
    if (std::filesystem::exists(file)) {
        int ct = 0;
        do {
            ct++;
            file = name + "(" + std::to_string(ct) + ")." + suffix;
        } while (std::filesystem::exists(file));
    }
    return file;
}

std::string FileUniqueForceSuffix(__IN const char *path, __IN const char *suf) {
    std::string file = path;
    auto pos = file.rfind(".");
    std::string name, suffix = suf;
    if (pos == std::string::npos) {
        name = file;
    } else {
        name = file.substr(0, pos);
    }
    file = name + "." + suffix;
    if (std::filesystem::exists(file)) {
        int ct = 0;
        do {
            ct++;
            file = name + "(" + std::to_string(ct) + ")." + suffix;
        } while (std::filesystem::exists(file));
    }
    return file;
}

#ifdef _WIN32
std::string UTF8toGBK(__IN const std::string &str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    wchar_t *wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, nullptr, 0, nullptr, nullptr);
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, nullptr, nullptr);
    std::string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}

std::string GBKtoUTF8(__IN const std::string &str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *szUTF8 = new char[len + 1];
    memset(szUTF8, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, szUTF8, len, NULL, NULL);
    std::string strTemp = szUTF8;
    if (wstr) delete[] wstr;
    if (szUTF8) delete[] szUTF8;
    return strTemp;
}
#endif
