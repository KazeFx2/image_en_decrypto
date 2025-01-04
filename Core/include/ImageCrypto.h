//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGECRYPTO_H
#define IMAGECRYPTO_H

#include "ThreadPool.h"
#include "private/includes.h"
#include "private/ImageEncrypto.h"
#include "private/ImageDecrypto.h"
#include "private/Random.h"
#include "private/Util.h"

class ImageCrypto {
public:
    ImageCrypto(ThreadPool *threadPool): threadPool(threadPool), config(DEFAULT_CONFIG), keys(RANDOM_KEYS) {
    };

    ImageCrypto(ThreadPool *threadPool, const ParamControl &config): threadPool(threadPool), config(config),
                                                                     keys(RANDOM_KEYS) {
    };

    ImageCrypto(ThreadPool *threadPool, const ParamControl &config, const Keys &keys): threadPool(threadPool),
        config(config),
        keys(keys) {
    };

    ParamControl getConfig() const {
        return config;
    }

    Keys getKeys() const {
        return keys;
    }

    void setKeys(const Keys &keys) {
        this->keys = keys;
    }

    void setKeys(const ParamControl &config) {
        this->config = config;
    }

    ThreadPool *getThreadPool() const {
        return threadPool;
    }

    void setThreadPool(ThreadPool *threadPool) {
        this->threadPool = threadPool;
    }

    cv::Mat encrypt(const cv::Mat &image, const ImageSize size = ORIGINAL_SIZE) const {
        cv::Mat imageMat = image.clone();
        DestroyReturn(EncryptoImage(imageMat, size, keys, config, *threadPool), config);
        return imageMat;
    }

    cv::Mat decrypt(const cv::Mat &image, const ImageSize size = ORIGINAL_SIZE) const {
        cv::Mat imageMat = image.clone();
        DestroyReturn(DecryptoImage(imageMat, size, keys, config, *threadPool), config);
        return imageMat;
    }

    ~ImageCrypto() {
    }

private:
    ThreadPool *threadPool;
    ParamControl config;
    Keys keys;
};

#endif //IMAGECRYPTO_H
