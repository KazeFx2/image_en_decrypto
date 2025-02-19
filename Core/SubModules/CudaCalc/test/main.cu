//
// Created by Fx Kaze on 25-1-6.
//

#include <cuda_runtime_api.h>

#include "CudaCalc.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include "private/vars.h"


// #include "cuda_runtime.h"
// #include <highgui.hpp>

using namespace cv;

#define DIM 600   //图像长宽

__global__ void kernel(unsigned char* ptr)
{
	// map from blockIdx to pixel position
	int bx = blockIdx.x;
	int by = blockIdx.y;
	int boffset = bx + by * gridDim.x;
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int toffset = tx + ty * blockDim.x;
	int offset = toffset + boffset * blockDim.x * blockDim.y;
	// printf("offset: %d\n", offset);

	//BGR设置
	ptr[offset * 3 + 0] = 999 * tx * ty % 255;
	ptr[offset * 3 + 1] = 99 * tx * tx * ty * ty % 255;
	ptr[offset * 3 + 2] = 9 * offset * offset % 255;
}

int main()
{
	cudaError_t error;
	Mat image = Mat(DIM, DIM, CV_8UC3, Scalar::all(0));
	u8* dev_bitmap;
	error = cudaMalloc(&dev_bitmap, 3 * image.cols * image.rows);
	if (error != cudaSuccess)
		printf("cudaMalloc failed\n");

	dim3 block(60, 60);
	dim3 thread(10, 10);
	//DIM*DIM个线程块
	kernel<<<block, thread>>>(dev_bitmap);
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess)
	{
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
		// Possibly: exit(-1) if program cannot continue....
	}
	cudaDeviceSynchronize();

	error = cudaMemcpy(image.data, dev_bitmap,
	                   3 * image.cols * image.rows,
	                   cudaMemcpyDeviceToHost);
	if (error != cudaSuccess)
		printf("cudaMemcpyDeviceToHost failed\n");

	cudaFree(dev_bitmap);

	imshow("CUDA Grid/Block/Thread)", image);
	waitKey();
}

int __main()
{
	chdir(homePath);
	int devCount = 0;
	cudaGetDeviceCount(&devCount);
	printf("Device count: %d\n", devCount);
	int devId = devCount - 1;
	cudaSetDevice(devId);
	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, devId);
	printf("Device name: %s\n", prop.name);
	printf("Device major: %d\n", prop.major);
	printf("Device minor: %d\n", prop.minor);
	printf("Total memory size: %lu\n", prop.totalGlobalMem);
	printf("Warp size: %d\n", prop.warpSize);
	printf("Max thread per block: %d\n", prop.maxThreadsPerBlock);
	printf("Max grid size: %d, %d, %d\n", prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]);
	printf("Max thread dim: %d, %d, %d\n", prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2]);
	auto img = cv::imread("./inputs/1.jpeg");
	printf("Image size: %dx%d\n", img.cols, img.rows);
	printf("Data type: %d, data size: %ld, warpSize: \n", img.type(), img.elemSize());
	u8* img_cuda = nullptr;
	cudaMalloc(&img_cuda, img.cols * img.rows * img.elemSize());
	auto ret = cudaMemcpy(img_cuda, img.ptr(), img.cols * img.rows * img.elemSize(), cudaMemcpyHostToDevice);
	if (ret == cudaSuccess)
	{
		printf("Image data copy success\n");
	}
	// kernel<<<1, 1>>>();
	cudaDeviceSynchronize();
	printf("Hello from CUDA kernel!\n");
	float *d_A, *d_B, *d_C;
	cudaMalloc(&d_A, 1024 * 1024 * 1024);
	cudaMalloc(&d_B, 512);
	cudaMalloc(&d_C, 128);
	return 0;
}

