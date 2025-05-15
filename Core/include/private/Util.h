
//
// Created by Fx Kaze on 24-12-30.
//

#ifndef UTIL_H
#define UTIL_H

#include "includes.h"
#include "ThreadPool.h"
#include "Bitmap.h"
#include "../../SubModules/Mutex/include/private/Mutex.h"

f64 PLCM(__IN f64 initialCondition, __IN f64 controlCondition);

f64 IteratePLCM(__IN f64 initialCondition, __IN f64 controlCondition, __IN u32 iterations,
                __OUT f64 *iterationResultArray);

void CvtF64toBytes(__IN const f64 *iterationResultArray, __OUT u8 *bytes, __IN u32 length, __IN u8 bytesReserve);

void XorByteSequence(__IN_OUT u8 *bytesA, __IN const u8 *bytesB, __IN u32 length, __OUT u8 *bytesOut = nullptr);

void GenDiffusionSeeds(__IN f64 initialCondition1, __IN f64 controlCondition1, __IN f64 initialCondition2,
                       __IN f64 controlCondition2, __OUT u8 *diffusionSeedArray, __IN u32 diffusionIteration,
                       __IN u8 nChannel);

void ConfusionFunc(__IN u32 row, __IN u32 col, __IN u32 width, __IN u32 height, __IN u32 confusionSeed,
                   __OUT u32 &newRow,
                   __OUT u32 &newCol);

void InvertConfusionFunc(__IN u32 row, __IN u32 col, __IN u32 width, __IN u32 height, __IN u32 confusionSeed,
                         __OUT u32 &newRow,
                         __OUT u32 &newCol);

void Confusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
               __IN u32 startRow, __IN u32 endRow,
               __IN u32 startCol, __IN u32 endCol,
               __IN u32 width, __IN u32 height, __IN u32 confusionSeed, __IN u8 nChannel);

void InvertConfusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
                     __IN u32 startRow, __IN u32 endRow,
                     __IN u32 startCol, __IN u32 endCol,
                     __IN u32 width, __IN u32 height, __IN u32 confusionSeed, __IN u8 nChannel);

void Diffusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
               __IN u32 startRow, __IN u32 endRow,
               __IN u32 startCol, __IN u32 endCol,
               __IN u32 width, __IN u32 height,
               __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx, __IN u8 nChannel);

void InvertDiffusion(__OUT u8 *dstImage, __IN const u8 *srcImage,
                     __IN u32 startRow, __IN u32 endRow,
                     __IN u32 startCol, __IN u32 endCol,
                     __IN u32 width, __IN u32 height,
                     __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx, __IN u8 nChannel);

void PreGenerate(__IN_OUT u8 *Image, __IN_OUT u8 *&tmpImage, __IN u32 width, __IN u32 height,
                 __IN_OUT u8 *&dst, __IN_OUT u8 *&src, __IN_OUT u32 *threads, __IN_OUT threadParams *params,
                 __IN const Keys &Keys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *));

void PreGenerate(__IN_OUT u8 *Image, __IN_OUT u8 *&tmpImage, __IN u32 width, __IN u32 height,
                 __IN_OUT u8 *&dst, __IN_OUT u8 *&src, __IN_OUT u32 *threads,
                 __IN_OUT threadParamsWithKey *params,
                 __IN const Keys &Keys,
                 __IN threadReturn **threadKeys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool,
                 __IN void *(*func)(void *));

threadReturn **GenerateThreadKeys(__IN const cv::Size &Size,
                                  __IN const Keys &Keys,
                                  __IN const ParamControl &Config, __IN ThreadPool &pool);

void CalcRowCols(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
                 __IN const threadParams &params);

void PreAssist(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
               __IN_OUT threadParams &params, __IN_OUT u8 * &byteSeq, __IN_OUT u8 * &diffusionSeedArray);

void DestroyReturn(__IN threadReturn **ret, __IN const ParamControl &config);

void PrintReturn(__IN FILE *fp, __IN threadReturn **ret, __IN const ParamControl &config, __IN u32 width, __IN u32 height);

threadReturn **CopyReturn(__IN threadReturn **other, __IN const ParamControl &Config, __IN const cv::Size &Size);

void DumpBytes(__IN FILE *fd, __IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpBytes(__IN void *buf, __IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpBytes(__IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpMat(__IN FILE *fd, __IN const char *name, __IN const cv::Mat &mat);

void LoadConfKey(__OUT ParamControl &params, __OUT Keys &Keys, __IN const char *path);

void SaveConfKey(__IN const ParamControl &params, __IN const Keys &Keys, __IN const char *path);

bool StructureEqual(__IN const void *a,__IN const void *b, __IN u32 size);

bool FileExists(__IN const char *path);

std::string FileUnique(__IN const char *path, __IN const char *suffix);

std::string FileUniqueForceSuffix(__IN const char *path, __IN const char *suffix);

u64 getPutIdx(u64 &idx, Semaphore &semaphore, FastBitmap &bitmap, bool isRet, u64 oldIdx);

#ifdef _WIN32
std::string UTF8toGBK(__IN const std::string &str);

std::string GBKtoUTF8(__IN const std::string &str);
#endif

void audio_open(__IN const char *input_filenam, __OUT u8 *channels = nullptr, __OUT u32 *sample_size = nullptr,
                __OUT void **data = nullptr, __OUT int *sample_rate = nullptr, __OUT bool *is_float = nullptr, __OUT double *duration_msec = nullptr, __OUT u64 *duration_pts = nullptr);

void audio_open_out(__IN void *in_data, __IN const char *output_filename, __OUT u32 *sample_size, __OUT void **data);

typedef struct audio_control_s {
    Semaphore sem;

    Mutex mtx;

    void *ctx;

    bool term;
    bool fin;

    int type;
    Keys keys;
    ParamControl config;
    ThreadPool *pool;
    u32 width;
    u32 height;

    int new_type;
    bool update_type;

    double new_msec;
    bool update_msec;

    Keys new_keys;
    ParamControl new_config;
    bool update_keys;

    bool new_cuda;
    bool update_cuda;

    u32 new_width;
    u32 new_height;
    bool update_size;

    void (*send_samples)(const u8 *data, u32 size, void *ctx);
    void (*set_msec)(double msec, u32 pts, void *ctx);
    void (*cleanup)(void *ctx);
    void (*type_change)(void *ctx);
} audio_control_t;

int audio_crypto(__IN void *in_data, __IN void *out_data, __IN const Keys &Key,
                 __IN threadReturn **threadKeys,
                 __IN const ParamControl &Config, __IN ThreadPool &pool, __IN u32 width,
                 __IN u32 height,
                 __IN bool encrypt,__IN const bool *term, __IN_OUT void (*signal)(u64, u64) = nullptr);

int audio_crypto_realtime(__IN void *data,
                          __IN_OUT audio_control_t *ctrl);

double GetCPUSecond();

#endif //UTIL_H
