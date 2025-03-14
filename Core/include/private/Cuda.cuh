//
// Created by kazefx on 25-2-17.
//

#ifndef CUDA_CUH
#define CUDA_CUH

#endif //CUDA_CUH

#include "includes.h"

void ConfusionCuda(__OUT void *dstImage, __IN const void *srcImage,
                   __IN u32 width, __IN u32 height, __IN u32 confusionSeed, __IN u8 nChannel);

void InvertConfusionCuda(__OUT void *dstImage, __IN const void *srcImage,
                         __IN u32 width, __IN u32 height, __IN u32 confusionSeed, __IN u8 nChannel);

void ConfusionCuda(__OUT void *dstImage, __IN const void *srcImage,
                   __IN const cv::Size &size, __IN u32 confusionSeed, __IN u8 nChannel);

void InvertConfusionCuda(__OUT void *dstImage, __IN const void *srcImage,
                         __IN const cv::Size &size, __IN u32 confusionSeed, __IN u8 nChannel);

void *AllocCopyMemToCuda(__IN const void *srcImage, __IN u32 size);

void *AllocCopyMatToCuda(__IN const cv::Mat &srcImage);

void CopyMemToCuda(__OUT void *dstImage, __IN const void *srcImage, __IN u32 size);

void CopyCudaToMem(__OUT void *dstImage, __IN const void *srcImage, __IN u32 size);

void CopyMatToCuda(__OUT void *dstImage, __IN const cv::Mat &srcImage);

void CopyCudaToMat(__OUT const cv::Mat &dstImage, __IN const void *srcImage);

void *MallocCuda(__IN u32 size);

void FreeCuda(__IN void *ptr);

bool CudaAvailable();
