//
// Created by Fx Kaze on 25-1-3.
//

#include "CudaCalc.h"

#include <iostream>

__global__ void kernel()
{
    int a = 1 * 1;
    int b = a * 2;
    printf("Hello from CUDA kernel!\n");
}
