//
// Created by kazefx on 25-2-17.
//

#include "private/Cuda.cuh"
#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#define THREADS_PER_BLOCK 32

void* AllocCopyMatToCuda(__IN const cv::Mat& srcImage)
{
    void* p = MallocCuda(srcImage.rows * srcImage.cols * srcImage.elemSize());
    if (p != nullptr)
        cudaMemcpy(p, srcImage.data, srcImage.rows * srcImage.cols * srcImage.elemSize(), cudaMemcpyHostToDevice);
    return p;
}

void CopyMatToCuda(void* dstImage, const cv::Mat& srcImage)
{
    if (dstImage != nullptr)
        cudaMemcpy(dstImage, srcImage.data, srcImage.rows * srcImage.cols * srcImage.elemSize(),
                   cudaMemcpyHostToDevice);
}

void CopyCudaToMat(const cv::Mat& dstImage, const void* srcImage)
{
    cudaMemcpy(dstImage.data, srcImage, dstImage.rows * dstImage.cols * dstImage.elemSize(), cudaMemcpyDeviceToHost);
}


void* MallocCuda(__IN const u32 size)
{
    void* p = nullptr;
    cudaError_t err = cudaMalloc(&p, size);
    if (err != cudaSuccess)
    {
        printf("CudaMalloc failed, %s\n", cudaGetErrorString(err));
    }
    return p;
}

void FreeCuda(__IN void* ptr)
{
    if (ptr != nullptr)
        cudaFree(ptr);
}

__device__ void ConfusionFuncCuda(__IN const u32 row, __IN const u32 col, __IN const u32 width, __IN const u32 height,
                                  __IN const u32 confusionSeed,
                                  __OUT u32& newRow,
                                  __OUT u32& newCol)
{
    newRow = (row + col) % height;
    const u32 tmp = static_cast<u32>(static_cast<i32>(rint(confusionSeed * sin(2 * M_PI * newRow / height)))) % width;
    newCol = (col + tmp) % width;
}

__global__ void DoConfusionCuda(__IN u8* dst, __IN const u8* src, __IN const u32 confusionSeed, __IN const u32 width,
                                __IN const u32 height, __IN const u8 nChannel)
{
    const u32 blockId = blockIdx.x + blockIdx.y * gridDim.x;
    u32 idx = threadIdx.x + blockId * blockDim.x;
    const u32 r = idx / width;
    const u32 c = idx % width;
    if (r < height && c < width)
    {
        u32 nr, nc;
        ConfusionFuncCuda(r, c, width, height, confusionSeed, nr, nc);
        u32 newIdx = (nr * width + nc) * nChannel;
        idx *= nChannel;
        u8 times = nChannel;
        while (times--)
            dst[newIdx++] = src[idx++];
    }
}

void ConfusionCuda(__OUT void* dstImage, __IN const void* srcImage,
                   __IN const cv::Size& size, __IN const u32 confusionSeed, __IN const u8 nChannel)
{
    const u32 fa = static_cast<u32>(sqrt(THREADS_PER_BLOCK));
    DoConfusionCuda<<<dim3((size.width + fa - 1) / fa,
                           (size.height) / fa
    ), THREADS_PER_BLOCK>>>(static_cast<u8*>(dstImage), static_cast<const u8*>(srcImage),
                            confusionSeed, size.width,
                            size.height, nChannel);
}


__device__ void InvertConfusionFuncCuda(__IN const u32 row, __IN const u32 col, __IN const u32 width,
                                        __IN const u32 height,
                                        __IN const u32 confusionSeed,
                                        __OUT u32& newRow,
                                        __OUT u32& newCol)
{
    const u32 tmp = static_cast<u32>(static_cast<i32>(rint(confusionSeed * sin(2 * M_PI * row / height)))) % width;
    newCol = (col + width - tmp) % width;
    newRow = (row + height - newCol % height) % height;
}

__global__ void DoInvertConfusionCuda(__IN u8* dst, __IN const u8* src, __IN const u32 confusionSeed,
                                      __IN const u32 width,
                                      __IN const u32 height, __IN const u8 nChannel)
{
    const u32 blockId = blockIdx.x + blockIdx.y * gridDim.x;
    u32 idx = threadIdx.x + blockId * blockDim.x;
    const u32 r = idx / width;
    const u32 c = idx % width;
    if (r < height && c < width)
    {
        u32 nr, nc;
        InvertConfusionFuncCuda(r, c, width, height, confusionSeed, nr, nc);
        u32 newIdx = (nr * width + nc) * nChannel;
        idx *= nChannel;
        u8 times = nChannel;
        while (times--)
            dst[newIdx++] = src[idx++];
    }
}

void InvertConfusionCuda(__OUT void* dstImage, __IN const void* srcImage,
                         __IN const cv::Size& size, __IN const u32 confusionSeed, __IN const u8 nChannel)
{
    const u32 fa = static_cast<u32>(sqrt(THREADS_PER_BLOCK));
    DoInvertConfusionCuda<<<dim3((size.width + fa - 1) / fa,
                                 (size.height + fa - 1) / fa
    ), THREADS_PER_BLOCK>>>(static_cast<u8*>(dstImage), static_cast<const u8*>(srcImage),
                            confusionSeed, size.width,
                            size.height, nChannel);
}

bool CudaAvailable()
{
    int deviceCount = 0;
    const cudaError_t err = cudaGetDeviceCount(&deviceCount);
    if (err != cudaSuccess || deviceCount <= 0)
    {
        return false;
    }
    return true;
}

