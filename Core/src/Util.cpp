//
// Created by Fx Kaze on 24-12-30.
//

#include "private/Util.h"
#include "private/Random.h"
#include "math.h"

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

void XorByteSequence(__IN const u8 *bytesA, __IN const u8 *bytesB, __OUT u8 *bytesOut, __IN const u32 length) {
    for (u32 i = 0; i < length; i++) {
        bytesOut[i] = bytesA[i] ^ bytesB[i];
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
    newCol = ((col + static_cast<u32>(round(confusionSeed * sin(2 * M_PI * newRow / size.height)))) % size.width + size.
              width) % size.
             width;
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

#define DIFFUSION(Prev) {\
dstImage.at<cv::Vec3b>(startRow, startCol)[0] = byteSequence[seqIdx] ^ (srcImage.at<cv::Vec3b>(startRow, startCol)[0] + byteSequence[seqIdx]) % 256 ^ Prev[0];\
seqIdx++;\
dstImage.at<cv::Vec3b>(startRow, startCol)[1] = byteSequence[seqIdx] ^ (srcImage.at<cv::Vec3b>(startRow, startCol)[1] + byteSequence[seqIdx]) % 256 ^ Prev[1];\
seqIdx++;\
dstImage.at<cv::Vec3b>(startRow, startCol)[2] = byteSequence[seqIdx] ^ (srcImage.at<cv::Vec3b>(startRow, startCol)[2] + byteSequence[seqIdx]) % 256 ^ Prev[2];\
seqIdx++;\
}

void Diffusion(__OUT cv::Mat &dstImage, __IN const cv::Mat &srcImage,
               __IN const u32 startRow, __IN const u32 endRow,
               __IN const u32 startCol, __IN const u32 endCol,
               __IN const u8 *diffusionSeed, __IN const u8 *byteSequence, __IN_OUT u32 &seqIdx) {
    DIFFUSION(diffusionSeed);
    u32 prevI = startRow, prevJ = startCol;
    for (u32 i = startRow; i < endRow; i++) {
        for (u32 j = startCol + 1; j < endCol; j++) {
            DIFFUSION(dstImage.at<cv::Vec3b>(prevI, prevJ));
            prevI = i, prevJ = j;
        }
    }
}

