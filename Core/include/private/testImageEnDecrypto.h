//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGE_TEST_EN_DECRYPTO_H
#define IMAGE_TEST_EN_DECRYPTO_H

#include "includes.h"
#include "ThreadPool.h"

threadReturn **testEnDecryptoImage(__IN_OUT cv::Mat &Image, __IN const ImageSize &Size,__IN const Keys &Keys,
                                   __IN const ParamControl &Config, __IN ThreadPool &pool);

#endif //IMAGE_TEST_EN_DECRYPTO_H
