//
// Created by Fx Kaze on 24-12-30.
//
#ifdef _MSC_VER
#pragma warning(disable:4576)
#endif

#include "private/Util.h"
#include "private/Random.h"
#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#endif
#include <cmath>
#include <filesystem>
#include <private/ImageEncrypto.h>
#include <private/ImageDecrypto.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/error.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avassert.h>
}

#undef av_err2str
static char* av_err2str(int errnum)
{
    char tmp[AV_ERROR_MAX_STRING_SIZE] = {0};
    return av_make_error_string(tmp, AV_ERROR_MAX_STRING_SIZE, errnum);
}

#define CALC_ITERATIONS_WH(width, height) const u32 iterations = static_cast<int>(Config.nChannel * ((width) + nThread) * ((height) + nThread) * Config.\
diffusionConfusionIterations \
/ (nThread * Config.byteReserve))

#define CALC_ITERATIONS CALC_ITERATIONS_WH(Size.width, Size.height)

f64 PLCM(__IN const f64 initialCondition, __IN const f64 controlCondition)
{
    assert(initialCondition >= 0 && initialCondition <= 1 && controlCondition > 0 && controlCondition < 0.5);
    if (initialCondition < controlCondition)
    {
        return initialCondition / controlCondition;
    }
    if (initialCondition <= 0.5)
    {
        return (initialCondition - controlCondition) / (0.5 - controlCondition);
    }
    return PLCM(1 - initialCondition, controlCondition);
}

f64 IteratePLCM(__IN const f64 initialCondition, __IN const f64 controlCondition, __IN const u32 iterations,
                __OUT f64* iterationResultArray)
{
    if (iterations == 0) return initialCondition;
    iterationResultArray[0] = PLCM(initialCondition, controlCondition);
    for (u32 i = 1; i < iterations; i++)
        iterationResultArray[i] = PLCM(iterationResultArray[i - 1], controlCondition);
    return iterationResultArray[iterations - 1];
}

void CvtF64toBytes(__IN const f64* iterationResultArray, __OUT u8* bytes, __IN const u32 length,
                   __IN const u8 bytesReserve)
{
    for (u32 i = 0; i < length; i++)
    {
        memcpy(bytes, &iterationResultArray[i], bytesReserve);
        bytes += bytesReserve;
    }
}

void XorByteSequence(__IN_OUT u8* bytesA, __IN const u8* bytesB, __IN const u32 length, __OUT u8* bytesOut)
{
    for (u32 i = 0; i < length; i++)
    {
        if (bytesOut != nullptr)
            bytesOut[i] = bytesA[i] ^ bytesB[i];
        else
            bytesA[i] ^= bytesB[i];
    }
}

void GenDiffusionSeeds(__IN f64 initialCondition1, __IN const f64 controlCondition1, __IN f64 initialCondition2,
                       __IN const f64 controlCondition2, __OUT u8* diffusionSeedArray,
                       __IN const u32 diffusionIteration, __IN const u8 nChannel)
{
    for (u32 i = 0; i < nChannel * diffusionIteration; i++)
    {
        initialCondition1 = PLCM(initialCondition1, controlCondition1);
        initialCondition2 = PLCM(initialCondition2, controlCondition2);
        diffusionSeedArray[i] = *reinterpret_cast<u8*>(&initialCondition1) ^ *reinterpret_cast<u8*>(&
            initialCondition2);
    }
}

void ConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const u32 width, __IN const u32 height,
                   __IN const u32 confusionSeed, __OUT u32& newRow,
                   __OUT u32& newCol)
{
    newRow = (row + col) % height;
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * newRow / height))) % width;
    newCol = (col + tmp) % width;
}

void InvertConfusionFunc(__IN const u32 row, __IN const u32 col, __IN const u32 width, __IN const u32 height,
                         __IN const u32 confusionSeed,
                         __OUT u32& newRow,
                         __OUT u32& newCol)
{
    const u32 tmp = static_cast<u32>(round(confusionSeed * sin(2 * M_PI * row / height))) % width;
    newCol = (col + width - tmp) % width;
    newRow = (row + height - newCol % height) % height;
}

void Confusion(__OUT u8* dstImage, __IN const u8* srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u32 width, __IN const u32 height, __IN const u32 confusionSeed, __IN const u8 nChannel)
{
    if (startRow >= endRow || startCol >= endCol) return;
    for (u32 i = startRow; i < endRow; i++)
    {
        auto src = srcImage + (i * width + startCol) * nChannel;
        for (u32 j = startCol; j < endCol; j++)
        {
            u32 newRow, newCol;
            ConfusionFunc(i, j, width, height, confusionSeed, newRow, newCol);
            auto dst = dstImage + (newRow * width + newCol) * nChannel;
            u8 n = nChannel;
            while (n--)
            {
                *dst++ = *src++;
            }
        }
    }
}

void InvertConfusion(__OUT u8* dstImage, __IN const u8* srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const u32 width, __IN const u32 height, __IN const u32 confusionSeed,
                     __IN const u8 nChannel)
{
    if (startRow >= endRow || startCol >= endCol) return;
    for (u32 i = startRow; i < endRow; i++)
    {
        auto src = srcImage + (i * width + startCol) * nChannel;
        for (u32 j = startCol; j < endCol; j++)
        {
            u32 newRow, newCol;
            InvertConfusionFunc(i, j, width, height, confusionSeed, newRow, newCol);
            auto dst = dstImage + (newRow * width + newCol) * nChannel;
            u8 n = nChannel;
            while (n--)
            {
                *dst++ = *src++;
            }
        }
    }
}

class iterator
{
public:
    iterator(cv::Mat& Mat, const u32 startRow, const u32 endRow, const u32 startCol, const u32 endCol,
             u8* diffusionSeed, const u8 nChannel, const u8 offset = 0,
             const bool front = false): mat(Mat), startRow(startRow), endRow(endRow), startCol(startCol),
                                        endCol(endCol),
                                        diffusionSeed(diffusionSeed), nChannel(nChannel), front(front),
                                        nCt(offset % nChannel)
    {
        if (front)
        {
            nowPtr = Mat.ptr(static_cast<i32>(startRow), static_cast<i32>(startCol)) + nCt;
            nowRow = startRow, nowCol = startCol;
        }
        else
        {
            nowPtr = Mat.ptr(static_cast<i32>(endRow - 1), static_cast<i32>(endCol - 1)) + nCt;
            nowRow = endRow - 1, nowCol = endCol - 1;
        }
    }

    iterator(cv::Mat& Mat, const u32 startRow, const u32 endRow, const u32 startCol, const u32 endCol,
             u8* diffusionSeed, const u8 nChannel, const u32 nowRow,
             const u32 nowCol, const u8 offset = 0): mat(Mat), startRow(startRow), endRow(endRow), startCol(startCol),
                                                     endCol(endCol),
                                                     diffusionSeed(diffusionSeed), nChannel(nChannel), front(false),
                                                     nCt(offset % nChannel)
    {
        nowPtr = Mat.ptr(static_cast<i32>(nowRow), static_cast<i32>(nowCol)) + nCt;
        this->nowRow = nowRow, this->nowCol = nowCol;
    }

    operator u8*() const
    {
        if (front)
        {
            return diffusionSeed + nCt;
        }
        return nowPtr;
    }

    u8* operator++()
    {
        if (front)
        {
            nCt++;
            const auto tmp = nCt;
            if (nCt == nChannel)
            {
                nCt = 0;
                front = false;
            }
            return diffusionSeed + tmp;
        }
        nCt++;
        if (nCt == nChannel)
        {
            nCt = 0;
            nowCol++;
            if (nowCol == endCol)
            {
                nowCol = startCol;
                nowRow++;
                if (nowRow == endRow)
                {
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

    u8* operator++(int)
    {
        u8* tmp = *this;
        operator++();
        return tmp;
    }

    u8* operator--()
    {
        if (front)
        {
            if (nCt == 0)
            {
                return diffusionSeed;
            }
            const auto tmp = nCt;
            nCt--;
            return diffusionSeed + tmp;
        }
        if (nCt == 0)
        {
            nCt = nChannel - 1;
            if (nowCol == startCol)
            {
                if (nowRow == startRow)
                {
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

    u8* operator--(int)
    {
        u8* tmp = *this;
        operator--();
        return tmp;
    }

private:
    cv::Mat& mat;
    u32 startRow, endRow, startCol, endCol;
    u8* diffusionSeed;
    u32 nowRow, nowCol;
    u8 nChannel;
    bool front;
    u8 nCt;
    u8* nowPtr;
};

#define DIFFUSION(dst, src, Prev) {\
    auto n = nChannel;\
    while (n--) {\
        *(dst)++ = byteSequence[seqIdx] ^ ((*(src)++ + byteSequence[seqIdx]) % 256) ^ *(Prev)++;\
        seqIdx++;\
    }\
}

void Diffusion(__OUT u8* dstImage, __IN const u8* srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u32 width, __IN const u32 height,
               __IN const u8* diffusionSeed, __IN const u8* byteSequence, __IN_OUT u32& seqIdx,
               __IN const u8 nChannel)
{
    if (startRow >= endRow || startCol >= endCol) return;
    auto dst = dstImage + (startRow * width + startCol) * nChannel;
    auto src = srcImage + (startRow * width + startCol) * nChannel;
    auto nPrev = dst;
    DIFFUSION(dst, src, diffusionSeed);
    auto prev = nPrev;
    u32 i = startRow, j = startCol + 1;
    while (true)
    {
        for (; j < endCol; j++)
        {
            DIFFUSION(dst, src, prev);
        }
        i++;
        if (i == endRow)
        {
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

void InvertDiffusion(__OUT u8* dstImage, __IN const u8* srcImage,
                     __IN const u32 startRow, __IN const u32 endRow,
                     __IN const u32 startCol, __IN const u32 endCol,
                     __IN const u32 width, __IN const u32 height,
                     __IN const u8* diffusionSeed, __IN const u8* byteSequence, __IN_OUT u32& seqIdx,
                     __IN const u8 nChannel)
{
    if (startRow >= endRow || startCol >= endCol) return;
    u32 i = endRow - 1, j = endCol - 1;
    auto nextDst = dstImage + (i * width + j) * nChannel + nChannel - 1;
    auto nextSrc = srcImage + (i * width + j) * nChannel + nChannel - 1;
    // auto dst = nextDst - nChannel;
    u8* dst = nullptr;
    auto src = nextSrc - nChannel;
    goto INNER;
    while (true)
    {
        for (;; j--)
        {
            INV_DIFFUSION(nextDst, nextSrc, src)
        INNER:
            if (j == startCol)
            {
                break;
            }
        }
        if (i == startRow)
        {
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

void PreGenerate(__IN_OUT u8* Image, __IN_OUT u8*& tmpImage, __IN const u32 width, __IN_OUT const u32 height,
                 __IN_OUT u8*& dst, __IN_OUT u8*& src, __IN_OUT u32* threads, __IN_OUT threadParams* params,
                 __IN const Keys& Keys,
                 __IN const ParamControl& Config, __IN ThreadPool& pool,
                 __IN void*(*func)(void*))
{
    const u32 nThread = Config.nThread;
    tmpImage = new u8[width * height * Config.nChannel];
    memcpy(tmpImage, Image, width * height * Config.nChannel);
    dst = tmpImage, src = Image;
    CALC_ITERATIONS_WH(width, height);

    ::Keys iterated_keys = Keys;
    // pre-iterate
    for (u32 i = 0; i < Config.preIterations; i++)
    {
        iterated_keys.gParam1.initCondition = PLCM(iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        iterated_keys.gParam2.initCondition = PLCM(iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
    }
    ::Keys tmpKeys = iterated_keys;

    for (u32 i = 0; i < nThread; i++)
    {
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

void PreGenerate(__IN_OUT u8* Image, __IN_OUT u8*& tmpImage, __IN const u32 width, __IN_OUT const u32 height,
                 __IN_OUT u8*& dst, __IN_OUT u8*& src, __IN_OUT u32* threads,
                 __IN_OUT threadParamsWithKey* params,
                 __IN const Keys& Keys,
                 __IN threadReturn** threadKeys,
                 __IN const ParamControl& Config, __IN ThreadPool& pool,
                 __IN void*(*func)(void*))
{
    const u32 nThread = Config.nThread;
    tmpImage = new u8[width * height * Config.nChannel];
    memcpy(tmpImage, Image, width * height * Config.nChannel);
    dst = tmpImage, src = Image;
    CALC_ITERATIONS_WH(width, height);

    for (u32 i = 0; i < nThread; i++)
    {
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

typedef struct
{
    Keys keys;
    const ParamControl* config;
    u32 iterations;
} KeyAssParam;

static void* KeyAssist(void* param)
{
    auto& [keys, config, iterations] = *static_cast<KeyAssParam*>(param);

    auto* byteSeq = new u8[iterations * config->byteReserve];
    auto* diffusionSeedArray = new u8[config->nChannel * config->diffusionConfusionIterations];

    f64* resultArray1 = new f64[iterations];
    f64* resultArray2 = new f64[iterations];
    u8* bytesTmp = new u8[iterations * config->byteReserve];

    // const Keys keysBackup = params.keys;
    if (keys.gParam1.ctrlCondition > 0.5)
        keys.gParam1.ctrlCondition = 1 - keys.gParam1.ctrlCondition;
    if (keys.gParam2.ctrlCondition > 0.5)
        keys.gParam2.ctrlCondition = 1 - keys.gParam2.ctrlCondition;
    for (u32 i = 0; i < config->preIterations; i++)
    {
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

threadReturn** GenerateThreadKeys(__IN const cv::Size& Size,
                                  __IN const Keys& Keys,
                                  __IN const ParamControl& Config, __IN ThreadPool& pool)
{
    const u32 nThread = Config.nThread;
    CALC_ITERATIONS;

    auto* params = new KeyAssParam[nThread];
    auto* threads = new ThreadPool::task_descriptor_t[nThread];
    auto** threadReturns = new threadReturn*[nThread];

    ::Keys iterated_keys = Keys;
    // pre-iterate
    for (u32 i = 0; i < Config.preIterations; i++)
    {
        iterated_keys.gParam1.initCondition = PLCM(iterated_keys.gParam1.initCondition,
                                                   iterated_keys.gParam1.ctrlCondition);
        iterated_keys.gParam2.initCondition = PLCM(iterated_keys.gParam2.initCondition,
                                                   iterated_keys.gParam2.ctrlCondition);
    }
    for (u32 i = 0; i < nThread; i++)
    {
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
    for (u32 i = 0; i < nThread; i++)
    {
        threadReturns[i] = static_cast<threadReturn*>(pool.waitThread(threads[i]));
    }
    delete[] params;
    delete[] threads;

    return threadReturns;
}

void CalcRowCols(__IN_OUT u32& rowStart, __IN_OUT u32& rowEnd, __IN_OUT u32& colStart, __IN_OUT u32& colEnd,
                 __IN const threadParams& params)
{
    rowStart = (params.height * params.threadId / params.config->nThread), rowEnd = (
        params.height * (params.threadId + 1) / params.config->nThread);
    colStart = 0, colEnd = params.width;
}

void PreAssist(__IN_OUT u32& rowStart, __IN_OUT u32& rowEnd, __IN_OUT u32& colStart, __IN_OUT u32& colEnd,
               __IN_OUT threadParams& params, __IN_OUT u8* & byteSeq, __IN_OUT u8* & diffusionSeedArray)
{
    CalcRowCols(rowStart, rowEnd, colStart, colEnd, params);
    f64* resultArray1 = new f64[params.iterations];
    f64* resultArray2 = new f64[params.iterations];
    u8* bytesTmp = new u8[params.iterations * params.config->byteReserve];

    // const Keys keysBackup = params.keys;
    if (params.keys.gParam1.ctrlCondition > 0.5)
        params.keys.gParam1.ctrlCondition = 1 - params.keys.gParam1.ctrlCondition;
    if (params.keys.gParam2.ctrlCondition > 0.5)
        params.keys.gParam2.ctrlCondition = 1 - params.keys.gParam2.ctrlCondition;
    for (u32 i = 0; i < params.config->preIterations; i++)
    {
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

void DestroyReturn(__IN threadReturn** ret, __IN const ParamControl& config)
{
    if (!ret) return;
    for (u32 i = 0; i < config.nThread; i++)
        delete [] ret[i]->byteSeq, delete [] ret[i]->diffusionSeedArray, delete ret[i];
    delete [] ret;
}

void PrintReturn(__IN FILE* fp, __IN threadReturn** ret, __IN const ParamControl& Config, __IN const u32 width,
                 __IN const u32 height)
{
    u32 nThread = Config.nThread;
    CALC_ITERATIONS_WH(width, height);;
    for (u32 i = 0; i < Config.nThread; i++)
    {
        fprintf(fp, "thread: %d\nbyteSeq: \n", i);
        for (u32 j = 0; j < iterations * Config.byteReserve; j++)
        {
            fprintf(fp, "%02X", ret[i]->byteSeq[j]);
        }
        fprintf(fp, "\ndiffusionSeedArray: \n");
        for (u32 j = 0; j < Config.nChannel * Config.diffusionConfusionIterations; j++)
        {
            fprintf(fp, "%02X", ret[i]->diffusionSeedArray[j]);
        }
        fprintf(fp, "\n");
    }
}

threadReturn** CopyReturn(__IN threadReturn** other, __IN const ParamControl& Config, __IN const cv::Size& Size)
{
    if (!other) return nullptr;
    auto nThread = Config.nThread;
    CALC_ITERATIONS;
    threadReturn** ret = new threadReturn*[Config.nThread];
    for (u32 i = 0; i < Config.nThread; i++)
    {
        ret[i] = new threadReturn{
            new u8[iterations * Config.byteReserve], new u8[Config.nChannel * Config.diffusionConfusionIterations]
        };
        memcpy(ret[i]->byteSeq, other[i]->byteSeq, iterations * Config.byteReserve);
        memcpy(ret[i]->diffusionSeedArray, other[i]->diffusionSeedArray,
               Config.nChannel * Config.diffusionConfusionIterations);
    }
    return ret;
}

void DumpBytes(__IN FILE* fd, __IN const char* name, __IN const u8* array, __IN const u32 size)
{
    fprintf(fd, "DumpBytes start\n");
    fprintf(fd, "Name: %s\n", name);
    fprintf(fd, "Bytes size: %u\n", size);
    fprintf(fd, "Data:\n");
    for (u32 i = 0; i < size; i++)
    {
        fprintf(fd, "%02X ", array[i]);
    }
    fprintf(fd, "\n");
    fprintf(fd, "DumpBytes end\n");
}

#define sprintf(a, ...) snprintf(a, 1024, ##__VA_ARGS__)

void DumpBytes(__IN void* buf, __IN const char* name, __IN const u8* array, __IN const u32 size)
{
    char* p = static_cast<char*>(buf);
    p += sprintf(p, "DumpBytes start\n");
    p += sprintf(p, "Name: %s\n", name);
    p += sprintf(p, "Bytes size: %u\n", size);
    p += sprintf(p, "Data:\n");
    for (u32 i = 0; i < size; i++)
    {
        p += sprintf(p, "%02X ", array[i]);
    }
    p += sprintf(p, "\n");
    p += sprintf(p, "DumpBytes end\n");
}

void DumpBytes(__IN const char* name, __IN const u8* array, __IN const u32 size)
{
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


void DumpMat(FILE* fd, const char* name, const cv::Mat& mat)
{
    fprintf(fd, "DumpMat start\n");
    fprintf(fd, "Name: %s\n", name);
    fprintf(fd, "Mat size: row: %u, col: %u\n", mat.rows, mat.cols);
    fprintf(fd, "Data:\n[\n");
    const auto type = mat.type();
    for (u32 i = 0; i < mat.rows; i++)
    {
        for (u32 j = 0; j < mat.cols; j++)
        {
            PRT_ELEM(type, mat, fd, i, j);
        }
        fprintf(fd, "\n");
    }
    fprintf(fd, "]\n");
    fprintf(fd, "DumpMat end\n");
}

void LoadConfKey(ParamControl& params, Keys& Keys, const char* path)
{
    FILE* fd = fopen(path, "r");
    if (fd)
    {
        fread(&params, sizeof(ParamControl), 1, fd);
        fread(&Keys, sizeof(Keys), 1, fd);
        fclose(fd);
    }
}

void SaveConfKey(const ParamControl& params, const Keys& Keys, const char* path)
{
#ifdef _WIN32
    std::string tmp = path;
    tmp = UTF8toGBK(tmp);
#else
    std::string tmp = path;
#endif
    std::string file = FileUniqueForceSuffix(tmp.c_str(), "ikey");
    FILE* fd = fopen(file.c_str(), "w");
    if (fd)
    {
        fwrite(&params, sizeof(ParamControl), 1, fd);
        fwrite(&Keys, sizeof(Keys), 1, fd);
        fclose(fd);
    }
}

bool StructureEqual(__IN const void* a,__IN const void* b, __IN u32 size)
{
    auto t_a = static_cast<const char*>(a);
    auto t_b = static_cast<const char*>(b);
    for (u32 i = 0; i < size; i++)
    {
        if (t_a[i] != t_b[i]) return false;
    }
    return true;
}

bool FileExists(__IN const char* path)
{
    return std::filesystem::exists(path);
}

std::string FileUnique(__IN const char* path, __IN const char* suf)
{
    std::string file = path;
    auto pos = file.rfind(".");
    std::string name, suffix;
    if (pos == std::string::npos)
    {
        name = file;
        suffix = suf;
    }
    else
    {
        name = file.substr(0, pos);
        suffix = file.substr(pos + 1);
    }
    if (std::filesystem::exists(file))
    {
        int ct = 0;
        do
        {
            ct++;
            file = name + "(" + std::to_string(ct) + ")." + suffix;
        }
        while (std::filesystem::exists(file));
    }
    return file;
}

std::string FileUniqueForceSuffix(__IN const char* path, __IN const char* suf)
{
    std::string file = path;
    auto pos = file.rfind(".");
    std::string name, suffix = suf;
    if (pos == std::string::npos)
    {
        name = file;
    }
    else
    {
        name = file.substr(0, pos);
    }
    file = name + "." + suffix;
    if (std::filesystem::exists(file))
    {
        int ct = 0;
        do
        {
            ct++;
            file = name + "(" + std::to_string(ct) + ")." + suffix;
        }
        while (std::filesystem::exists(file));
    }
    return file;
}

u64 getPutIdx(u64& idx, Semaphore& semaphore, FastBitmap& bitmap, const bool isRet, const u64 oldIdx)
{
    semaphore.wait();
    if (isRet == true)
    {
        bitmap[oldIdx] = false;
        semaphore.post();
        return oldIdx;
    }
    auto id = bitmap.findNextFalse(0, idx);
    if (id != BITMAP_NOT_FOUND)
    {
        bitmap[id] = true;
        semaphore.post();
        return id;
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    semaphore.post();
    return ret;
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

static
int open_input_file(const char* filename, AVFormatContext** input_format_ctx, AVCodecContext** input_codec_ctx,
                    int* stream_idx)
{
    int error = AVERROR_EXIT, i = 0;
    const AVStream* input_stream = nullptr;
    const AVCodec* input_codec = nullptr;
    *stream_idx = -1;

    error = avformat_open_input(input_format_ctx, filename, nullptr, nullptr);
    if (error < 0)
    {
        fprintf(stderr, "Could not open input file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    error = avformat_find_stream_info(*input_format_ctx, nullptr);
    if (error < 0)
    {
        fprintf(stderr, "Could not find stream info in input file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    for (i = 0; i < (*input_format_ctx)->nb_streams; i++)
    {
        if ((*input_format_ctx)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            input_stream = (*input_format_ctx)->streams[i];
            *stream_idx = i;
            break;
        }
    }
    if (!input_stream)
    {
        fprintf(stderr, "Could not find an audio stream in input file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    input_codec = avcodec_find_decoder(input_stream->codecpar->codec_id);
    if (!input_codec)
    {
        fprintf(stderr, "Could not find an audio codec in input file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    *input_codec_ctx = avcodec_alloc_context3(input_codec);
    if (!*input_codec_ctx)
    {
        fprintf(stderr, "Could not allocate audio codec context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    error = avcodec_parameters_to_context(*input_codec_ctx, input_stream->codecpar);
    if (error < 0)
    {
        fprintf(stderr, "Could not apply params to audio codec context.\n");
        goto open_output_file_clean_up;
    }
    error = avcodec_open2(*input_codec_ctx, input_codec, nullptr);
    if (error < 0)
    {
        fprintf(stderr, "Could not open audio codec context.\n");
        goto open_output_file_clean_up;
    }
    (*input_codec_ctx)->pkt_timebase = input_stream->time_base;
    return 0;
open_output_file_clean_up:
    if (*input_codec_ctx)
    {
        avcodec_free_context(input_codec_ctx);
        *input_codec_ctx = nullptr;
    }
    if (*input_format_ctx)
    {
        avformat_close_input(input_format_ctx);
        *input_format_ctx = nullptr;
    }
    return error;
}

static int open_output_file(const char* filename, AVFormatContext** output_format_ctx,
                            const AVCodecContext* input_codec_ctx,
                            AVCodecContext** output_codec_ctx)
{
    int error = AVERROR_EXIT, n_format = 0;
    AVIOContext* output_io_ctx = nullptr;
    const AVCodec* output_codec = nullptr;
    AVStream* output_stream = nullptr;
    const AVSampleFormat* sample_formats = nullptr;
    error = avio_open(&output_io_ctx, filename, AVIO_FLAG_WRITE);
    if (error < 0)
    {
        fprintf(stderr, "Could not open output file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    *output_format_ctx = avformat_alloc_context();
    if (!*output_format_ctx)
    {
        fprintf(stderr, "Could not allocate output format context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_format_ctx)->pb = output_io_ctx;
    (*output_format_ctx)->oformat = av_guess_format(nullptr, filename, nullptr);
    if (!(*output_format_ctx)->oformat)
    {
        fprintf(stderr, "Could not guess output format.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_format_ctx)->url = av_strdup(filename);
    output_codec = avcodec_find_encoder((*output_format_ctx)->oformat->audio_codec);
    if (!output_codec)
    {
        fprintf(stderr, "Could not find an audio codec in output file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    output_stream = avformat_new_stream(*output_format_ctx, nullptr);
    if (!output_stream)
    {
        fprintf(stderr, "Could not allocate an audio stream in output file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    *output_codec_ctx = avcodec_alloc_context3(output_codec);
    if (!*output_codec_ctx)
    {
        fprintf(stderr, "Could not allocate an output audio codec context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    av_channel_layout_default(&(*output_codec_ctx)->ch_layout, input_codec_ctx->ch_layout.nb_channels);
    (*output_codec_ctx)->sample_rate = input_codec_ctx->sample_rate;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 19, 100)
    error = avcodec_get_supported_config(*output_codec_ctx, output_codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0,
                                         reinterpret_cast<const void**>(&sample_formats),
                                         &n_format);
    if (error < 0)
    {
        fprintf(stderr, "Could not find sample format. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    if (n_format < 1)
    {
        fprintf(stderr, "Could not find sample format.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_codec_ctx)->sample_fmt = sample_formats[0];
#else
    (*output_codec_ctx)->sample_fmt = output_codec->sample_fmts[0];
#endif
    (*output_codec_ctx)->bit_rate = input_codec_ctx->bit_rate;
    output_stream->time_base.den = input_codec_ctx->sample_rate;
    output_stream->time_base.num = 1;
    if ((*output_format_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
    {
        (*output_codec_ctx)->flags2 |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    error = avcodec_open2(*output_codec_ctx, output_codec, nullptr);
    if ((*output_codec_ctx)->frame_size == 0)
        (*output_codec_ctx)->frame_size = input_codec_ctx->frame_size > 0 ? input_codec_ctx->frame_size : 1152;
    if (error < 0)
    {
        fprintf(stderr, "Could not open an audio codec. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    error = avcodec_parameters_from_context(output_stream->codecpar, *output_codec_ctx);
    if (error < 0)
    {
        fprintf(stderr, "Could not get params from context. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    return 0;
open_output_file_clean_up:
    if (output_io_ctx)
        avio_close(output_io_ctx);
    if (*output_format_ctx)
    {
        avformat_free_context(*output_format_ctx);
        *output_format_ctx = nullptr;
    }
    if (*output_codec_ctx)
    {
        avcodec_free_context(output_codec_ctx);
        *output_codec_ctx = nullptr;
    }
    return error;
}

static int read_decode_cvt_store(AVAudioFifo* fifo, AVFormatContext* input_format_ctx, AVCodecContext* input_codec_ctx,
                                 const AVCodecContext* output_codec_ctx, SwrContext* swr_ctx, int* finished,
                                 const int stream_idx, const int cvt = 1, void (*signal)(u64, u64) = nullptr)
{
    int error = AVERROR_EXIT, n_out = 0;
    AVFrame* input_frame = nullptr;
    AVPacket* input_pkt = nullptr;
    u8** converted_input_samples = nullptr;
    *finished = 0;
    input_frame = av_frame_alloc();
    if (!input_frame)
    {
        fprintf(stderr, "Could not allocate input frame.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    input_pkt = av_packet_alloc();
    if (!input_pkt)
    {
        fprintf(stderr, "Could not allocate input packet.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    error = av_read_frame(input_format_ctx, input_pkt);
    if (error < 0)
    {
        if (error == AVERROR_EOF)
        {
            *finished = 1;
            error = 0;
            goto read_decode_cvt_store_clean_up;
        }
        fprintf(stderr, "Could not read an audio frame. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (input_pkt->stream_index != stream_idx)
    {
        error = 0;
        goto read_decode_cvt_store_clean_up;
    }
    error = avcodec_send_packet(input_codec_ctx, input_pkt);
    if (error < 0)
    {
        fprintf(stderr, "Could not send a packet to decoder. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    error = avcodec_receive_frame(input_codec_ctx, input_frame);
    if (error < 0)
    {
        if (error == AVERROR(EAGAIN))
        {
            error = 0;
            goto read_decode_cvt_store_clean_up;
        }
        av_assert0(error != AVERROR_EOF);
        fprintf(stderr, "Could not receive an audio frame from decoder. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (signal)
    {
        signal(input_frame->pts, input_format_ctx->streams[stream_idx]->duration);
    }
    if (cvt)
    {
        error = av_samples_alloc_array_and_samples(
            &converted_input_samples, nullptr,
            output_codec_ctx->ch_layout.nb_channels,
            input_frame->nb_samples,
            output_codec_ctx->sample_fmt, 0);
        if (error < 0)
        {
            fprintf(stderr, "Could not alloc input samples. (error: '%s')\n", av_err2str(error));
            goto read_decode_cvt_store_clean_up;
        }
        error = swr_convert(
            swr_ctx,
            converted_input_samples,
            input_frame->nb_samples,
            const_cast<const uint8_t**>(input_frame->data),
            input_frame->nb_samples
        );
        if (error < 0)
        {
            fprintf(stderr, "Could not convert input samples. (error: '%s')\n", av_err2str(error));
            goto read_decode_cvt_store_clean_up;
        }
        n_out = error;
    }
    else
    {
        converted_input_samples = input_frame->data;
        n_out = input_frame->nb_samples;
    }
    error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + n_out);
    if (error != 0)
    {
        fprintf(stderr, "Could not realloc fifo with size '%d'. (error: '%s')\n",
                av_audio_fifo_size(fifo) + n_out, av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (av_audio_fifo_write(
        fifo,
        reinterpret_cast<void**>(converted_input_samples),
        n_out) < n_out)
    {
        fprintf(stderr, "Could not write to FIFO.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    error = 0;
read_decode_cvt_store_clean_up:
    if (cvt && converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        av_freep(&converted_input_samples);
    }
    if (input_frame)
    {
        av_frame_free(&input_frame);
    }
    if (input_pkt)
    {
        av_packet_free(&input_pkt);
    }
    return error;
}

#include <cstdio>

int load_crypto_store(AVAudioFifo* fifo_in, AVAudioFifo* fifo_out, SwrContext* swr_ctx, const Keys& Key,
                      threadReturn** threadKeys,
                      const ParamControl& Config, ThreadPool& pool,
                      bool encrypt, int crypto_block_size, const u32 width, const u32 height, const int sample_size,
                      AVSampleFormat out_fmt,
                      const int n_channels)
{
    const int read_size = FFMIN(av_audio_fifo_size(fifo_in), crypto_block_size);
    const int byte_size_per_ch = crypto_block_size * sample_size;
    const int byte_size_total = byte_size_per_ch * n_channels;
    int i = 0, error = AVERROR_EXIT;

    u8** crypto_ptrs = nullptr;
    u8 *crypto_data = nullptr, *crypto_data_out = nullptr;
    u8** converted_input_samples = nullptr;
    crypto_ptrs = new u8*[n_channels];
    if (!crypto_ptrs)
    {
        fprintf(stderr, "Could not allocate crypto_ptrs.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    crypto_data = new u8[byte_size_total];
    if (!crypto_data)
    {
        fprintf(stderr, "Could not allocate crypto_data.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    memset(crypto_data, 0, byte_size_total);
    for (i = 0; i < n_channels; i++)
    {
        crypto_ptrs[i] = &crypto_data[i * byte_size_per_ch];
    }
    if (av_audio_fifo_read(fifo_in, reinterpret_cast<void* const *>(crypto_ptrs), read_size) < read_size)
    {
        fprintf(stderr, "Could not read input data from fifo.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    crypto_data_out = static_cast<u8*>(malloc(width * height * sample_size * n_channels));
    if (crypto_data_out == nullptr)
    {
        fprintf(stderr, "Could not malloc crypto memory.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    memcpy(crypto_data_out, crypto_data, width * height * sample_size * n_channels);
    if (encrypt)
    {
        EncryptoImage(
            crypto_data_out, width * sample_size, height, Key, threadKeys, Config, pool
        );
    }
    else
    {
        DecryptoImage(
            crypto_data_out, width * sample_size, height, Key, threadKeys, Config, pool
        );
    }
    if (!crypto_data_out)
    {
        fprintf(stderr, "Could not decrypt crypto_data.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    for (i = 0; i < n_channels; i++)
    {
        crypto_ptrs[i] = &crypto_data_out[i * byte_size_per_ch];
    }
    if (!encrypt)
    {
        error = av_samples_alloc_array_and_samples(
            &converted_input_samples, nullptr,
            n_channels,
            crypto_block_size,
            out_fmt, 0);
        if (error < 0)
        {
            fprintf(stderr, "Could not alloc output samples. (error: '%s')\n", av_err2str(error));
            goto load_crypto_store_clean_up;
        }
        error = swr_convert(
            swr_ctx,
            converted_input_samples,
            crypto_block_size,
            const_cast<const uint8_t**>(crypto_ptrs),
            crypto_block_size
        );
        if (error < 0)
        {
            fprintf(stderr, "Could not convert output samples. (error: '%s')\n", av_err2str(error));
            goto load_crypto_store_clean_up;
        }
        crypto_block_size = error;
    }
    else
    {
        converted_input_samples = crypto_ptrs;
    }
    error = av_audio_fifo_realloc(fifo_out, av_audio_fifo_size(fifo_out) + crypto_block_size);
    if (error != 0)
    {
        fprintf(stderr, "Could not re-allocate output fifo. (error: '%s')\n", av_err2str(error));
        goto load_crypto_store_clean_up;
    }
    if (av_audio_fifo_write(fifo_out, reinterpret_cast<void* const *>(converted_input_samples), crypto_block_size) <
        crypto_block_size)
    {
        fprintf(stderr, "Could not write output data to fifo.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    error = 0;
load_crypto_store_clean_up:
    if (crypto_ptrs)
    {
        delete[] crypto_ptrs;
    }
    if (crypto_data)
    {
        delete[] crypto_data;
    }
    if (crypto_data_out)
    {
        delete[] crypto_data_out;
    }
    if (!encrypt && converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        av_freep(&converted_input_samples);
    }
    return error;
}

static
int load_encode_write(AVAudioFifo* fifo, AVFormatContext* output_format_ctx,
                      AVCodecContext* output_codec_ctx, i64* global_pts)
{
    int error = AVERROR_EXIT, frame_size = 0;
    AVFrame* output_frame = nullptr;
    AVPacket* out_pkt = nullptr;

    frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_ctx->frame_size);
    output_frame = av_frame_alloc();
    if (!output_frame)
    {
        fprintf(stderr, "Could not allocate output frame.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    output_frame->nb_samples = frame_size;
    output_frame->format = output_codec_ctx->sample_fmt;
    output_frame->sample_rate = output_codec_ctx->sample_rate;
    error = av_channel_layout_copy(&output_frame->ch_layout, &output_codec_ctx->ch_layout);
    if (error < 0)
    {
        fprintf(stderr, "Could not copy output channel layout. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = av_frame_get_buffer(output_frame, 0);
    if (error < 0)
    {
        fprintf(stderr, "Could not get output frame buffer. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    if (av_audio_fifo_read(fifo,
                           reinterpret_cast<void* const *>(output_frame->data),
                           frame_size) < frame_size)
    {
        printf("Could not read output frame from FIFO.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    out_pkt = av_packet_alloc();
    if (!out_pkt)
    {
        fprintf(stderr, "Could not allocate output packet.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    output_frame->pts = *global_pts;
    *global_pts += output_frame->nb_samples;
    error = avcodec_send_frame(output_codec_ctx, output_frame);
    if (error < 0)
    {
        fprintf(stderr, "Could not send packet for encoding. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = avcodec_receive_packet(output_codec_ctx, out_pkt);
    if (error < 0)
    {
        if (error == AVERROR(EAGAIN))
        {
            error = 0;
            goto load_encode_and_write_clean_up;
        }
        if (error == AVERROR_EOF)
        {
            error = 0;
            goto load_encode_and_write_clean_up;
        }
        fprintf(stderr, "Could not encode frame. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = av_write_frame(output_format_ctx, out_pkt);
    if (error < 0)
    {
        fprintf(stderr, "Could not write output packet. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = 0;
load_encode_and_write_clean_up:
    if (output_frame)
    {
        av_frame_free(&output_frame);
    }
    if (out_pkt)
    {
        av_packet_free(&out_pkt);
    }
    return error;
}

static
int flush_audio_frame(AVFormatContext* output_format_ctx, AVCodecContext* output_codec_ctx,
                      int* data_writen)
{
    int error = AVERROR_EXIT;
    AVPacket* out_pkt = nullptr;

    out_pkt = av_packet_alloc();
    *data_writen = 0;
    if (!out_pkt)
    {
        fprintf(stderr, "Could not allocate output packet.\n");
        error = AVERROR_EXIT;
        goto encode_audio_frame_clean_up;
    }
    error = avcodec_send_frame(output_codec_ctx, nullptr);
    if (error < 0)
    {
        if (error == AVERROR_EOF)
        {
            error = 0;
            goto encode_audio_frame_clean_up;
        }
        fprintf(stderr, "Could not send frame to output encoder. (error: '%s')\n", av_err2str(error));
        goto encode_audio_frame_clean_up;
    }
    // *data_exists = 0;
    do
    {
        error = avcodec_receive_packet(output_codec_ctx, out_pkt);
        if (error < 0)
        {
            if (error == AVERROR(EAGAIN))
            {
                error = 0;
                goto encode_audio_frame_clean_up;
            }
            if (error == AVERROR_EOF)
            {
                error = 0;
                *data_writen = 1;
                goto encode_audio_frame_clean_up;
            }
            fprintf(stderr, "Could not encode frame. (error: '%s')\n", av_err2str(error));
            goto encode_audio_frame_clean_up;
        }
        error = av_write_frame(output_format_ctx, out_pkt);
        if (error < 0)
        {
            fprintf(stderr, "Could not write output packet. (error: '%s')\n", av_err2str(error));
            goto encode_audio_frame_clean_up;
        }
    }
    while (true);
encode_audio_frame_clean_up:
    if (out_pkt)
    {
        av_packet_free(&out_pkt);
    }
    return error;
}

typedef struct audio_tmp_data_s
{
    AVFormatContext* input_format_ctx;
    AVCodecContext* input_codec_ctx;
    int stream_idx;
} audio_tmp_data_t;

void audio_open(__IN const char* input_filename, __OUT u8* channels, __OUT u32* sample_size, __OUT void** data,
                __OUT int* sample_rate, __OUT bool* is_float, __OUT double* duration_msec, __OUT u64* duration_pts)
{
    int stream_idx = -1;

    AVFormatContext* input_format_ctx = nullptr;
    AVCodecContext* input_codec_ctx = nullptr;

    if (channels)
        *channels = 0;
    if (sample_size)
        *sample_size = 0;
    if (sample_rate)
        *sample_rate = 0;
    if (is_float)
        *is_float = false;
    if (duration_msec)
        *duration_msec = 0;
    if (duration_pts)
        *duration_pts = 0;

    if (open_input_file(input_filename, &input_format_ctx,
                        &input_codec_ctx, &stream_idx))
        goto cleanup;
    if (channels)
        *channels = input_codec_ctx->ch_layout.nb_channels;
    if (sample_size)
        *sample_size = av_get_bytes_per_sample(input_codec_ctx->sample_fmt);
    if (sample_rate)
        *sample_rate = input_codec_ctx->sample_rate;
    if (is_float && (input_codec_ctx->sample_fmt == AV_SAMPLE_FMT_FLT || input_codec_ctx->sample_fmt ==
        AV_SAMPLE_FMT_DBL
        || input_codec_ctx->sample_fmt == AV_SAMPLE_FMT_FLTP || input_codec_ctx->sample_fmt ==
        AV_SAMPLE_FMT_DBLP))
        *is_float = true;
    if (duration_pts)
        *duration_pts = input_format_ctx->streams[
            stream_idx]->duration;
    if (duration_msec)
    {
        *duration_msec = av_q2d(input_format_ctx->streams[stream_idx]->time_base) * input_format_ctx->streams[
            stream_idx]->duration * 1000;
    }
    if (data)
    {
        auto* t_data = static_cast<audio_tmp_data_t*>(malloc(sizeof(audio_tmp_data_t)));
        if (!t_data)
            goto cleanup;
        t_data->input_codec_ctx = input_codec_ctx;
        t_data->input_format_ctx = input_format_ctx;
        t_data->stream_idx = stream_idx;
        *data = t_data;
        return;
    }
cleanup:
    if (input_codec_ctx)
        avcodec_free_context(&input_codec_ctx);
    if (input_format_ctx)
        avformat_close_input(&input_format_ctx);
}


typedef struct audio_tmp_out_data_s
{
    AVFormatContext* output_format_ctx;
    AVCodecContext* output_codec_ctx;
} audio_tmp_out_data_t;

void audio_open_out(__IN void* in_data, __IN const char* output_filename, __OUT u32* sample_size, __OUT void** data)
{
    auto* t_data = static_cast<audio_tmp_data_t*>(in_data);
    AVFormatContext* output_format_ctx = nullptr;
    AVCodecContext *input_codec_ctx = nullptr, *output_codec_ctx = nullptr;

    if (sample_size)
        *sample_size = 0;

    if (!t_data) return;
    input_codec_ctx = t_data->input_codec_ctx;


    if (open_output_file(output_filename,
                         &output_format_ctx, input_codec_ctx, &output_codec_ctx))
        goto cleanup;
    if (sample_size)
    {
        *sample_size = av_get_bytes_per_sample(output_codec_ctx->sample_fmt);
    }
    if (data)
    {
        auto* o_data = static_cast<audio_tmp_out_data_t*>(malloc(sizeof(audio_tmp_out_data_t)));
        if (!o_data)
            goto cleanup;
        o_data->output_codec_ctx = output_codec_ctx;
        o_data->output_format_ctx = output_format_ctx;
        *data = o_data;
        return;
    }
cleanup:
    if (output_codec_ctx)
        avcodec_free_context(&output_codec_ctx);
    if (output_format_ctx)
        avformat_close_input(&output_format_ctx);
}

int audio_crypto(__IN void* in_data, __IN void* out_data, __IN const Keys& Key,
                 __IN threadReturn** threadKeys,
                 __IN const ParamControl& Config, __IN ThreadPool& pool, __IN const u32 width,
                 __IN const u32 height,
                 __IN const bool encrypt,__IN const bool* term = nullptr, __IN_OUT void (*signal)(u64, u64))
{
    audio_tmp_data_t* t_data = static_cast<audio_tmp_data_t*>(in_data);
    audio_tmp_out_data_t* o_data = static_cast<audio_tmp_out_data_t*>(out_data);
    int error = AVERROR_EXIT, stream_idx = -1, finished = 0, output_size = 0, n_channels = 0;
    u32 crypto_block_size = width * height;
    i64 global_pts = 0;
    AVFormatContext *input_format_ctx = nullptr, *output_format_ctx = nullptr;
    AVCodecContext *input_codec_ctx = nullptr, *output_codec_ctx = nullptr;
    SwrContext* swr_ctx = nullptr;
    AVAudioFifo* fifo_1 = nullptr;
    AVAudioFifo* fifo_2 = nullptr;

    if (!t_data || !o_data)
        goto cleanup;

    stream_idx = t_data->stream_idx;
    input_format_ctx = t_data->input_format_ctx;
    input_codec_ctx = t_data->input_codec_ctx;
    free(t_data);
    t_data = nullptr;

    output_codec_ctx = o_data->output_codec_ctx;
    output_format_ctx = o_data->output_format_ctx;
    free(o_data);
    o_data = nullptr;

    // 分配重采样器（SwrContext）并初始化参数 <---3--->
    n_channels = input_codec_ctx->ch_layout.nb_channels;
    // 确认采样率要一样，否则处理方式不一样
    av_assert0(output_codec_ctx->sample_rate == input_codec_ctx->sample_rate);
    error = swr_alloc_set_opts2(
        &swr_ctx,
        &output_codec_ctx->ch_layout, output_codec_ctx->sample_fmt,
        output_codec_ctx->sample_rate,
        &input_codec_ctx->ch_layout, input_codec_ctx->sample_fmt,
        input_codec_ctx->sample_rate,
        0, nullptr);
    if (error < 0)
    {
        fprintf(stderr, "Could not set input to crypto swr options. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    //打开重采样器，调用 swr_init 之前必须用 swr_alloc_set_opts2 设置参数
    error = swr_init(swr_ctx);
    if (error < 0)
    {
        fprintf(stderr, "Could not init input to crypto swr context. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    // 根据指定的输出样本格式创建FIFO缓冲区 <---4--->
    output_size = output_codec_ctx->frame_size;
    if (encrypt)
    {
        fifo_1 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     crypto_block_size);
        fifo_2 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     output_size);
    }
    else
    {
        fifo_1 = av_audio_fifo_alloc(input_codec_ctx->sample_fmt, n_channels,
                                     crypto_block_size);
        fifo_2 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     output_size);
    }
    if (!fifo_1)
    {
        fprintf(stderr, "Could not allocate input to crypto audio fifo.\n");
        goto cleanup;
    }
    if (!fifo_2)
    {
        fprintf(stderr, "Could not allocate crypto to output audio fifo.\n");
        goto cleanup;
    }
    // 写入输出文件头 <---5--->
    error = avformat_write_header(output_format_ctx, nullptr);
    if (error < 0)
    {
        fprintf(stderr, "Could not write output file header. (error: '%s')\n",
                av_err2str(error));
        goto cleanup;
    }
    // 只要有输入样本要读取或输出样本要写入，就会循环;一旦都没有，就中止
    while (true)
    {
        while (av_audio_fifo_size(fifo_1) < crypto_block_size)
        {
            // 解码一帧音频样本，将其转换为输出样本格式并将其放入FIFO缓冲区
            if (read_decode_cvt_store(fifo_1, input_format_ctx, input_codec_ctx,
                                      output_codec_ctx, swr_ctx, &finished, stream_idx, encrypt, signal) || *term)
            {
                goto cleanup;
            }
            if (finished)
                break;
        }
        if (av_audio_fifo_size(fifo_1) >= crypto_block_size || (
            finished && av_audio_fifo_size(fifo_1) > 0))
        {
            if (load_crypto_store(fifo_1, fifo_2, swr_ctx, Key, threadKeys, Config, pool, encrypt, crypto_block_size,
                                  width, height,
                                  av_get_bytes_per_sample(
                                      encrypt ? output_codec_ctx->sample_fmt : input_codec_ctx->sample_fmt),
                                  output_codec_ctx->sample_fmt,
                                  n_channels) || *term)
                goto cleanup;
        }
        while (av_audio_fifo_size(fifo_2) >= output_size ||
            (finished && av_audio_fifo_size(fifo_2) > 0))
        {
            // 从FIFO缓冲区中获取一帧的音频样本，对其进行编码并将其写入输出文件
            if (load_encode_write(fifo_2, output_format_ctx, output_codec_ctx, &global_pts) || *term)
                goto cleanup;
        }
        /**
        * 如果我们在输入文件的末尾并已对所有剩余样本进行编码，则可以退出此循环并完成
        **/
        if (finished)
        {
            int data_written = 0;
            // 刷新编码器，因为它可能有延迟帧
            do
            {
                flush_audio_frame(output_format_ctx, output_codec_ctx, &data_written);
            }
            while (!data_written);
            break;
        }
        if (*term)
            goto cleanup;
    }
    // 写入输出文件容器的尾部 <---11--->
    error = av_write_trailer(output_format_ctx);
    if (error < 0 || *term)
        goto cleanup;
    error = 0;
cleanup:
    // 释放资源 <--- 12--->
    if (t_data)
        free(t_data);
    if (o_data)
        free(o_data);
    if (fifo_1)
        av_audio_fifo_free(fifo_1);
    if (fifo_2)
        av_audio_fifo_free(fifo_2);
    if (swr_ctx)
        swr_free(&swr_ctx);
    if (output_codec_ctx)
        avcodec_free_context(&output_codec_ctx);
    if (output_format_ctx)
    {
        avio_closep(&output_format_ctx->pb);
        avformat_free_context(output_format_ctx);
    }
    if (input_codec_ctx)
        avcodec_free_context(&input_codec_ctx);
    if (input_format_ctx)
        avformat_close_input(&input_format_ctx);
    if (*term)
        return AVERROR_EXIT;
    return error;
}

int audio_crypto_realtime(__IN void* data,
                          __IN_OUT audio_control_t* ctrl)
{
    auto* t_data = static_cast<audio_tmp_data_t*>(data);
    int i = 0, error = AVERROR_EXIT, stream_idx = -1, n_channels = 0, n_out = 0,
        sample_size =
            0, i_sample_size = 0, o_sample_size = 0, m_sample_size = 0;
    u32 crypto_block_size = 0, read_size = 0, current_pts = 0, sample_pos = 0,
        offset = 0, n_samples = 0;
    bool seek_frame = false, sem_rec = false;
    AVFormatContext* input_format_ctx = nullptr;
    AVCodecContext* input_codec_ctx = nullptr;
    SwrContext* swr_ctx = nullptr;
    AVAudioFifo *o_fifo = nullptr, *i_fifo = nullptr, *fifo = nullptr;
    threadReturn** threadKeys = nullptr;
    AVFrame* input_frame = nullptr;
    AVPacket* input_pkt = nullptr;
    u8 **converted_input_samples = nullptr, **input_samples = nullptr, **crypto_ptr = nullptr, **crypto_cvt_ptr =
           nullptr, **out_samples = nullptr;
    AVSampleFormat ofmt = AV_SAMPLE_FMT_S16, current_ifmt = AV_SAMPLE_FMT_NONE;
    u8 *crypto_buf = nullptr, *crypto_cvt_buf = nullptr;

    if (!ctrl || !t_data)
    {
        error = AVERROR_EXIT;
        goto cleanup;
    }

    crypto_block_size = ctrl->width * ctrl->height;

    stream_idx = t_data->stream_idx;
    input_format_ctx = t_data->input_format_ctx;
    input_codec_ctx = t_data->input_codec_ctx;
    free(t_data);
    n_channels = input_codec_ctx->ch_layout.nb_channels;
    o_sample_size = av_get_bytes_per_sample(ofmt);
    i_sample_size = av_get_bytes_per_sample(input_codec_ctx->sample_fmt);
    m_sample_size = MAX(i_sample_size, o_sample_size);

    if (ctrl->type != 0)
    {
        if (ctrl->type == 1)
        {
            threadKeys = GenerateThreadKeys({
                                                static_cast<int>(ctrl->width * o_sample_size),
                                                static_cast<int>(ctrl->height)
                                            },
                                            ctrl->keys,
                                            ctrl->config, *(ctrl->pool));
        }
        else
        {
            threadKeys = GenerateThreadKeys({
                                                static_cast<int>(ctrl->width * i_sample_size),
                                                static_cast<int>(ctrl->height)
                                            },
                                            ctrl->keys,
                                            ctrl->config, *(ctrl->pool));
        }
        if (!threadKeys)
        {
            error = AVERROR_EXIT;
            goto cleanup;
        }
    }

    error = swr_alloc_set_opts2(
        &swr_ctx,
        &input_codec_ctx->ch_layout, ofmt, input_codec_ctx->sample_rate,
        &input_codec_ctx->ch_layout, input_codec_ctx->sample_fmt,
        input_codec_ctx->sample_rate,
        0, nullptr
    );
    if (error < 0)
    {
        fprintf(stderr, "Could not set input to crypto swr options. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    error = swr_init(swr_ctx);
    if (error < 0)
    {
        fprintf(stderr, "Could not init input to crypto swr context. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }

    // block_size;
    i_fifo = av_audio_fifo_alloc(input_codec_ctx->sample_fmt, n_channels,
                                 static_cast<int>(crypto_block_size));
    o_fifo = av_audio_fifo_alloc(ofmt, n_channels,
                                 static_cast<int>(crypto_block_size));
    if (!i_fifo || !o_fifo)
    {
        fprintf(stderr, "Could not allocate crypto audio fifo.\n");
        error = AVERROR_EXIT;
        goto cleanup;
    }

    input_frame = av_frame_alloc();
    if (!input_frame)
    {
        fprintf(stderr, "Could not allocate input frame.\n");
        error = AVERROR_EXIT;
        goto cleanup;
    }
    input_pkt = av_packet_alloc();
    if (!input_pkt)
    {
        fprintf(stderr, "Could not allocate input packet.\n");
        error = AVERROR_EXIT;
        goto cleanup;
    }

    // block_size;
    crypto_buf = static_cast<u8*>(
        malloc(crypto_block_size * n_channels * m_sample_size)
    );
    crypto_cvt_buf = static_cast<u8*>(
        malloc(crypto_block_size * n_channels * m_sample_size)
    );
    if (!crypto_buf || !crypto_cvt_buf)
    {
        fprintf(stderr, "Could not malloc crypto memory.\n");
        error = AVERROR_EXIT;
        goto cleanup;
    }
    crypto_ptr = static_cast<u8**>(
        malloc(sizeof(u8*) * n_channels)
    );
    crypto_cvt_ptr = static_cast<u8**>(
        malloc(sizeof(u8*) * n_channels)
    );
    if (!crypto_ptr || !crypto_cvt_ptr)
    {
        fprintf(stderr, "Could not malloc pointers for crypto memory.\n");
        error = AVERROR_EXIT;
        goto cleanup;
    }
    for (i = 0; i < n_channels; i++)
    {
        crypto_ptr[i] = crypto_buf + i * crypto_block_size * m_sample_size;
        crypto_cvt_ptr[i] = crypto_cvt_buf + i * crypto_block_size * m_sample_size;
    }

    while (true)
    {
        // read pkt
        error = av_read_frame(input_format_ctx, input_pkt);
        if (error < 0)
        {
            if (error == AVERROR_EOF)
            {
                // end of file
                goto solve_fifo;
            }
            else
            {
                fprintf(stderr, "Could not read an audio frame. (error: '%s')\n", av_err2str(error));
                goto cleanup;
            }
        }
        if (input_pkt->stream_index != stream_idx)
            continue;
        error = avcodec_send_packet(input_codec_ctx, input_pkt);
        if (error < 0)
        {
            fprintf(stderr, "Could not send a packet to decoder. (error: '%s')\n", av_err2str(error));
            goto cleanup;
        }
        // read frame
        error = avcodec_receive_frame(input_codec_ctx, input_frame);
        if (error < 0)
        {
            if (error == AVERROR(EAGAIN))
            {
                continue;
            }
            av_assert0(error != AVERROR_EOF);
            fprintf(stderr, "Could not receive an audio frame from decoder. (error: '%s')\n", av_err2str(error));
            goto cleanup;
        }
        error = av_samples_alloc_array_and_samples(
            &converted_input_samples, nullptr,
            input_codec_ctx->ch_layout.nb_channels,
            input_frame->nb_samples,
            ofmt, 0);
        if (error < 0)
        {
            fprintf(stderr, "Could not alloc input samples. (error: '%s')\n", av_err2str(error));
            goto cleanup;
        }

        // if raw or encrypt
        if (ctrl->type == 0 || ctrl->type == 1)
        {
            // cvt to s16
            error = swr_convert(
                swr_ctx,
                converted_input_samples,
                input_frame->nb_samples,
                const_cast<const uint8_t**>(input_frame->data),
                input_frame->nb_samples
            );
            if (error < 0)
            {
                fprintf(stderr, "Could not convert input samples. (error: '%s')\n", av_err2str(error));
                goto cleanup;
            }
            n_out = error;
            av_assert0(n_out > 0);
            input_samples = converted_input_samples;
            fifo = o_fifo;
            current_ifmt = ofmt;
            sample_size = o_sample_size;
        }
        else
        {
            // decrypt
            n_out = input_frame->nb_samples;
            input_samples = input_frame->data;
            fifo = i_fifo;
            current_ifmt = input_codec_ctx->sample_fmt;
            sample_size = i_sample_size;
        }

        /// input_samples:
        /// ofmt, if raw or encrypt
        /// ifmt, if decrypt

        // do crypto
        if (ctrl->type != 0)
        {
            // put fifo
            error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + n_out);
            if (error != 0)
            {
                fprintf(stderr, "Could not realloc fifo with size '%d'. (error: '%s')\n",
                        av_audio_fifo_size(fifo) + n_out, av_err2str(error));
                goto cleanup;
            }
            if (av_audio_fifo_write(
                fifo,
                reinterpret_cast<void**>(input_samples),
                n_out) < n_out)
            {
                fprintf(stderr, "Could not write to FIFO.\n");
                error = AVERROR_EXIT;
                goto cleanup;
            }
            if (seek_frame)
            {
                offset = input_frame->pts - input_frame->pts / crypto_block_size * crypto_block_size;
                if (offset != 0)
                {
                    offset = crypto_block_size - offset;
                    if (av_audio_fifo_read(fifo, reinterpret_cast<void* const *>(crypto_ptr),
                                           static_cast<int>(offset)) <
                        offset)
                    {
                        fprintf(stderr, "Could not drop data from fifo.\n");
                        error = AVERROR_EXIT;
                        goto cleanup;
                    }
                }
                offset += input_frame->pts;
                if (ctrl->set_msec)
                {
                    ctrl->set_msec(static_cast<double>(offset) * 1000 / input_codec_ctx->sample_rate, offset,
                                   ctrl->ctx);
                }
                seek_frame = false;
            }
            error = 0;
            if (av_audio_fifo_size(fifo) < crypto_block_size)
                continue;

            // fifo enough or eof
        solve_fifo:
            read_size = MIN(av_audio_fifo_size(fifo), crypto_block_size);
            // clear
            if (read_size == 0)
            {
                goto sema_slp;
            }
            // read frm fifo
            if (av_audio_fifo_read(fifo, reinterpret_cast<void* const *>(crypto_ptr), static_cast<int>(read_size)) <
                read_size)
            {
                fprintf(stderr, "Could not read input data from fifo.\n");
                error = AVERROR_EXIT;
                goto cleanup;
            }
            // fill zeros if need
            if (read_size < crypto_block_size)
            {
                // planar
                if (av_sample_fmt_is_planar(current_ifmt))
                {
                    for (i = 0; i < n_channels; i++)
                    {
                        memset(crypto_ptr[i] + read_size * sample_size, 0,
                               (crypto_block_size - read_size) * sample_size);
                    }
                    // interleave (packed)
                }
                else
                {
                    memset(crypto_buf + read_size * n_channels * sample_size, 0,
                           (crypto_block_size - read_size) * n_channels * sample_size);
                }
            }

            // do crypt
            switch (ctrl->type)
            {
            case 1:
                // encrypt
                if (ctrl->pool->getMaxThreads() < ctrl->config.nThread) ctrl->pool->setMax(ctrl->config.nThread);
                else ctrl->pool->reduceTo(static_cast<int>(ctrl->config.nThread));
                EncryptoImage(
                    crypto_buf, ctrl->width * sample_size, ctrl->height, ctrl->keys, threadKeys, ctrl->config,
                    *(ctrl->pool)
                );
                break;
            case 2:
                // decrypt
                if (ctrl->pool->getMaxThreads() < ctrl->config.nThread) ctrl->pool->setMax(ctrl->config.nThread);
                else ctrl->pool->reduceTo(static_cast<int>(ctrl->config.nThread));
                DecryptoImage(
                    crypto_buf, ctrl->width * sample_size, ctrl->height, ctrl->keys, threadKeys, ctrl->config,
                    *(ctrl->pool)
                );
                break;
            default:
                fprintf(stderr, "Invalid Parameter 'type': '%d'\n", ctrl->type);
                error = AVERROR_EXIT;
                goto cleanup;
                break;
            }
            // if decrypt, cvt to s16
            if (ctrl->type == 2)
            {
                error = swr_convert(
                    swr_ctx,
                    crypto_cvt_ptr,
                    static_cast<int>(crypto_block_size),
                    const_cast<const uint8_t**>(crypto_ptr),
                    static_cast<int>(crypto_block_size)
                );
                if (error < 0)
                {
                    fprintf(stderr, "Could not convert output samples. (error: '%s')\n", av_err2str(error));
                    goto cleanup;
                }
                n_out = error;
                av_assert0(n_out > 0);
                out_samples = crypto_cvt_ptr;
            }
            else
            {
                // encrypt
                out_samples = crypto_ptr;
            }
            n_samples = crypto_block_size;
        }
        else
        {
            // raw
            out_samples = input_samples;
            if (seek_frame)
            {
                offset = input_frame->pts;
                if (ctrl->set_msec)
                {
                    ctrl->set_msec(static_cast<double>(offset) * 1000 / input_codec_ctx->sample_rate, offset,
                                   ctrl->ctx);
                }
                seek_frame = false;
            }
            n_samples = input_frame->nb_samples;
        }

        // sample call back
        if (ctrl->send_samples)
        {
            ctrl->send_samples(out_samples[0], n_samples * n_channels * o_sample_size, ctrl->ctx);
        }

        if (converted_input_samples)
        {
            av_freep(&converted_input_samples[0]);
            av_freep(&converted_input_samples);
        }

        current_pts = input_frame->pts;
    chk:
        ctrl->mtx.lock();

        if (ctrl->term)
        {
            if (!sem_rec)
            {
                ctrl->sem.wait();
                sem_rec = true;
            }
            error = 0;
            ctrl->mtx.unlock();
            goto cleanup;
        }

        if (ctrl->update_msec)
        {
            if (!sem_rec)
            {
                ctrl->sem.wait();
                sem_rec = true;
            }
            ctrl->update_msec = false;
            current_pts = av_rescale_q(static_cast<i64>(ctrl->new_msec), AVRational{1, 1000},
                                       input_format_ctx->streams[stream_idx]->time_base);
            seek_frame = true;
        }

        if (ctrl->update_type)
        {
            if (!sem_rec)
            {
                ctrl->sem.wait();
                sem_rec = true;
            }
            ctrl->update_type = false;
            if (ctrl->new_type != ctrl->type)
            {
                if (ctrl->type == 0 || ctrl->new_type == 0)
                {
                    seek_frame = true;
                }
                else if (i_sample_size != o_sample_size)
                {
                    if (threadKeys)
                        DestroyReturn(threadKeys, ctrl->config);
                    threadKeys = nullptr;
                }
                ctrl->type = ctrl->new_type;
                if (ctrl->type_change)
                    ctrl->type_change(ctrl->ctx);
                if (ctrl->type != 0 && threadKeys == nullptr)
                {
                    if (ctrl->type == 1)
                    {
                        threadKeys = GenerateThreadKeys({
                                                            static_cast<int>(ctrl->width * o_sample_size),
                                                            static_cast<int>(ctrl->height)
                                                        },
                                                        ctrl->keys,
                                                        ctrl->config, *(ctrl->pool));
                    }
                    else
                    {
                        threadKeys = GenerateThreadKeys({
                                                            static_cast<int>(ctrl->width * i_sample_size),
                                                            static_cast<int>(ctrl->height)
                                                        },
                                                        ctrl->keys,
                                                        ctrl->config, *(ctrl->pool));
                    }
                    if (!threadKeys)
                    {
                        error = AVERROR_EXIT;
                        goto cleanup;
                    }
                }
            }
        }

        if (ctrl->update_keys || ctrl->update_size)
        {
            if (!sem_rec)
            {
                ctrl->sem.wait();
                sem_rec = true;
            }
            if (threadKeys)
                DestroyReturn(threadKeys, ctrl->config);
            if (ctrl->update_keys)
            {
                ctrl->keys = ctrl->new_keys;
                ctrl->config = ctrl->new_config;
            }
            if (ctrl->update_size)
            {
                ctrl->width = ctrl->new_width;
                ctrl->height = ctrl->new_height;
            }
            if (ctrl->type != 0)
            {
                if (ctrl->type == 1)
                {
                    threadKeys = GenerateThreadKeys({
                                                        static_cast<int>(ctrl->width * o_sample_size),
                                                        static_cast<int>(ctrl->height)
                                                    },
                                                    ctrl->keys,
                                                    ctrl->config, *(ctrl->pool));
                }
                else
                {
                    threadKeys = GenerateThreadKeys({
                                                        static_cast<int>(ctrl->width * i_sample_size),
                                                        static_cast<int>(ctrl->height)
                                                    },
                                                    ctrl->keys,
                                                    ctrl->config, *(ctrl->pool));
                }
                if (!threadKeys)
                {
                    error = AVERROR_EXIT;
                    goto cleanup;
                }
            }

            if (ctrl->update_size)
            {
                crypto_block_size = ctrl->width * ctrl->height;
                av_audio_fifo_free(i_fifo);
                av_audio_fifo_free(o_fifo);
                i_fifo = av_audio_fifo_alloc(input_codec_ctx->sample_fmt, n_channels,
                                             static_cast<int>(crypto_block_size));
                o_fifo = av_audio_fifo_alloc(ofmt, n_channels,
                                             static_cast<int>(crypto_block_size));
                if (!i_fifo || !o_fifo)
                {
                    fprintf(stderr, "Could not allocate crypto audio fifo.\n");
                    error = AVERROR_EXIT;
                    goto cleanup;
                }
                free(crypto_buf);
                free(crypto_cvt_buf);
                free(crypto_ptr);
                free(crypto_cvt_ptr);
                crypto_buf = static_cast<u8*>(
                    malloc(crypto_block_size * n_channels * sample_size)
                );
                crypto_cvt_buf = static_cast<u8*>(
                    malloc(crypto_block_size * n_channels * sample_size)
                );
                if (!crypto_buf || !crypto_cvt_buf)
                {
                    fprintf(stderr, "Could not malloc crypto memory.\n");
                    error = AVERROR_EXIT;
                    goto cleanup;
                }
                crypto_ptr = static_cast<u8**>(
                    malloc(sizeof(u8*) * n_channels)
                );
                crypto_cvt_ptr = static_cast<u8**>(
                    malloc(sizeof(u8*) * n_channels)
                );
                if (!crypto_ptr || !crypto_cvt_ptr)
                {
                    fprintf(stderr, "Could not malloc pointers for crypto memory.\n");
                    error = AVERROR_EXIT;
                    goto cleanup;
                }
                for (i = 0; i < n_channels; i++)
                {
                    crypto_ptr[i] = crypto_buf + i * crypto_block_size * sample_size;
                    crypto_cvt_ptr[i] = crypto_cvt_buf + i * crypto_block_size * sample_size;
                }
                if (ctrl->type != 0)
                {
                    // encrypt or decrypt
                    seek_frame = true;
                }
            }
            ctrl->update_keys = false;
            ctrl->update_size = false;
        }

        if (ctrl->update_cuda)
        {
            ctrl->config.cuda = ctrl->new_cuda;
            ctrl->update_cuda = false;
        }

        if (seek_frame)
        {
            // seek frame, then calc offset
            sample_pos = current_pts / crypto_block_size * crypto_block_size;
            // seek frame
            av_seek_frame(input_format_ctx, stream_idx,
                          sample_pos,
                          AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(input_codec_ctx);
            // clear fifo
            av_audio_fifo_reset(i_fifo);
            av_audio_fifo_reset(o_fifo);
            if (ctrl->type == 0)
            {
                offset = 0;
            }
            // seek_frame = false;
        }

        sem_rec = false;

        ctrl->mtx.unlock();

        continue;
    sema_slp:
        sem_rec = true;
        ctrl->fin = true;
        ctrl->sem.wait();
        current_pts = 0;
        seek_frame = true;
        goto chk;
    }
cleanup:
    ctrl->term = true;
    if (swr_ctx)
    {
        swr_free(&swr_ctx);
    }
    if (i_fifo)
    {
        av_audio_fifo_free(i_fifo);
        i_fifo = nullptr;
    }
    if (o_fifo)
    {
        av_audio_fifo_free(o_fifo);
        o_fifo = nullptr;
    }
    if (threadKeys)
    {
        DestroyReturn(threadKeys, ctrl->config);
        threadKeys = nullptr;
    }
    if (input_frame)
    {
        av_frame_free(&input_frame);
    }
    if (input_pkt)
    {
        av_packet_free(&input_pkt);
    }
    if (converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        av_freep(&converted_input_samples);
    }
    if (crypto_buf)
    {
        free(crypto_buf);
        crypto_buf = nullptr;
    }
    if (crypto_ptr)
    {
        free(crypto_ptr);
        crypto_ptr = nullptr;
    }
    if (crypto_cvt_buf)
    {
        free(crypto_cvt_buf);
        crypto_cvt_buf = nullptr;
    }
    if (crypto_cvt_ptr)
    {
        free(crypto_cvt_ptr);
        crypto_cvt_ptr = nullptr;
    }
    if (ctrl->cleanup)
    {
        ctrl->cleanup(ctrl->ctx);
    }
    return error;
}

#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>

void gettimeofday(struct timeval *tv, void *tz) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    const uint64_t EPOCH_OFFSET = 116444736000000000ULL;
    uli.QuadPart -= EPOCH_OFFSET;
    uli.QuadPart /= 10;
    tv->tv_sec = uli.QuadPart / 1000000ULL;
    tv->tv_usec = uli.QuadPart % 1000000ULL;
}
#endif

double GetCPUSecond()
{
    timeval tp;
    gettimeofday(&tp, nullptr);
    return (static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_usec) * 1.e-6);
}
