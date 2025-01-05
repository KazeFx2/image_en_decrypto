//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGE_DECRYPTO_H
#define IMAGE_DECRYPTO_H

#include "includes.h"
#include "ThreadPool.h"

void DecryptoImage(__IN_OUT cv::Mat &Image, __IN_OUT cv::Size &Size,__IN const Keys &Key,
                   __IN threadReturn **threadKeys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool);

threadReturn **DecryptoImage(__IN_OUT cv::Mat &Image, __IN_OUT cv::Size &Size,__IN const Keys &Keys,
                             __IN const ParamControl &Config, __IN ThreadPool &pool);

#endif //IMAGE_DECRYPTO_H
