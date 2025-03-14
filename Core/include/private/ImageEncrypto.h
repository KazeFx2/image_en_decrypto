//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGE_ENCRYPTO_H
#define IMAGE_ENCRYPTO_H

#include "includes.h"
#include "ThreadPool.h"

void EncryptoImage(__IN_OUT cv::Mat &Image, __IN_OUT cv::Size &Size,__IN const Keys &Key,
                   __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool);

threadReturn **EncryptoImage(__IN_OUT cv::Mat &Image, __IN_OUT cv::Size &Size,__IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool);

void EncryptoImage(__IN_OUT void *Image, __IN u32 width, __IN u32 height, __IN const Keys &Key,
                   __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool);

threadReturn **EncryptoImage(__IN_OUT void *Image, __IN u32 width, __IN u32 height, __IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool);

#endif //IMAGE_ENCRYPTO_H
