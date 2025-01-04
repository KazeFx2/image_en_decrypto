//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGE_ENCRYPTO_H
#define IMAGE_ENCRYPTO_H

#include "includes.h"
#include "ThreadPool.h"
#include <string>

void EncryptoImage(__IN_OUT cv::Mat &Image, __IN const ImageSize &Size,__IN const Keys &Keys,
                   __IN const ParamControl &Config, __IN ThreadPool &pool);

#endif //IMAGE_ENCRYPTO_H
