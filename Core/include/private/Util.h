//
// Created by Fx Kaze on 24-12-30.
//

#ifndef UTIL_H
#define UTIL_H

#include "includes.h"

f64 PLCM(__IN f64 initialCondition, __IN f64 controlCondition);

f64 IteratePLCM(__IN f64 initialCondition, __IN f64 controlCondition, __IN u32 iterations,
                __OUT f64 *iterationResultArray);

void CvtF64toBytes(__IN const f64 *iterationResultArray, __OUT u8 *bytes, __IN u32 length, __IN u8 bytesReserve);

void XorByteSequence(__IN const u8 *bytesA, __IN const u8 *bytesB, __OUT u8 *bytesOut, __IN u32 length);

void GenDiffusionSeeds(__IN f64 initialCondition1, __IN f64 controlCondition1, __IN f64 initialCondition2,
                       __IN f64 controlCondition2, __OUT u8 *diffusionSeedArray, __IN u32 diffusionIteration);

void ConfusionFunc(__IN u32 row, __IN u32 col, __IN const cv::Size &size, __IN u32 confusionSeed, __OUT u32 &newRow,
                   __OUT u32 &newCol);

void Confusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN u32 startRow, __IN u32 endRow,
               __IN u32 startCol, __IN u32 endCol,
               __IN const cv::Size &size, __IN u32 confusionSeed);

#endif //UTIL_H
