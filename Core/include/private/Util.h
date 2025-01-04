//
// Created by Fx Kaze on 24-12-30.
//

#ifndef UTIL_H
#define UTIL_H

#include "includes.h"
#include "ThreadPool.h"

f64 PLCM(__IN f64 initialCondition, __IN f64 controlCondition);

f64 IteratePLCM(__IN f64 initialCondition, __IN f64 controlCondition, __IN u32 iterations,
                __OUT f64 *iterationResultArray);

void CvtF64toBytes(__IN const f64 *iterationResultArray, __OUT u8 *bytes, __IN u32 length, __IN u8 bytesReserve);

void XorByteSequence(__IN const u8 *bytesA, __IN const u8 *bytesB, __OUT u8 *bytesOut, __IN u32 length);

void GenDiffusionSeeds(__IN f64 initialCondition1, __IN f64 controlCondition1, __IN f64 initialCondition2,
                       __IN f64 controlCondition2, __OUT u8 *diffusionSeedArray, __IN u32 diffusionIteration);

void ConfusionFunc(__IN u32 row, __IN u32 col, __IN const cv::Size &size, __IN u32 confusionSeed, __OUT u32 &newRow,
                   __OUT u32 &newCol);

void InvertConfusionFunc(__IN u32 row, __IN u32 col, __IN const cv::Size &size, __IN u32 confusionSeed,
                         __OUT u32 &newRow,
                         __OUT u32 &newCol);

void Confusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN u32 startRow, __IN u32 endRow,
               __IN u32 startCol, __IN u32 endCol,
               __IN const cv::Size &size, __IN u32 confusionSeed);

void InvertConfusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
                     __IN u32 startRow, __IN u32 endRow,
                     __IN u32 startCol, __IN u32 endCol,
                     __IN const cv::Size &size, __IN u32 confusionSeed);

void Diffusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN u32 startRow, __IN u32 endRow,
               __IN u32 startCol, __IN u32 endCol,
               __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx);

void InvertDiffusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
                     __IN u32 startRow, __IN u32 endRow,
                     __IN u32 startCol, __IN u32 endCol,
                     __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx);

void PreGenerate(__IN_OUT cv::Mat &Image, __IN_OUT cv::Mat &tmpImage, __IN_OUT cv::Size &ImageSize,
                 __IN_OUT cv::Mat *&dst, __IN_OUT cv::Mat *&src, __IN_OUT u32 *threads, __IN_OUT threadParams *params,
                 __IN const ::ImageSize &Size,
                 __IN const Keys &Keys,
                 __IN const ParamControl &Config, __IN u32 nThread, __IN ThreadPool &pool,
                 __IN void *(*func)(void *));

void PreAssist(__IN_OUT u32 &rowStart, __IN_OUT u32 &rowEnd, __IN_OUT u32 &colStart, __IN_OUT u32 &colEnd,
               __IN_OUT threadParams &params, __IN_OUT u8 * &byteSeq, __IN_OUT u8 * &diffusionSeedArray);

void DestroyReturn(__IN threadReturn **ret, __IN const ParamControl &config);

void DumpBytes(__IN FILE *fd, __IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpBytes(__IN void *buf, __IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpBytes(__IN const char *name, __IN const u8 *array, __IN u32 size);

void DumpMat(__IN FILE *fd, __IN const char *name, __IN const cv::Mat &mat);

#endif //UTIL_H
