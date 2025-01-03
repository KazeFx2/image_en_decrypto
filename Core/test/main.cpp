//
// Created by Fx Kaze on 24-12-30.
//

#include "ImageCrypto.h"
#include "private/Random.h"
#include "private/ImageEncrypto.h"
#include "unistd.h"
#include "vars.h"
#include "ThreadPool.h"
#include "Mutex.h"

Mutex mu;

void *threadFunc(void *arg) {
    mu.lock();
    // mu.lock();
    u64 ret = 0;
    u64 a = 1e10 * rand();
    while (a--) {
        ret += a;
    }
    std::cout << ret << std::endl;
    mu.unlock();
    return (void *) ret;
}

int main() {
    chdir(homePath);

    int times = 100;
    double tmp;
    while (times--) {
        if ((tmp = GenDoubleFloatFrom1To0Random()) > 1 || tmp < 0) {
            std::cout << tmp << std::endl;
        }
    }
    std::cout << getcwd(nullptr, 0) << std::endl;
    auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    imshow("original", img);
    // cv::waitKey(0);
    ThreadPool sp(10);
    EncryptoImage(img, ORIGINAL_SIZE, RANDOM_KEYS, DEFAULT_CONFIG, 64, sp);
    imshow("encrypted", img);
    cv::waitKey(0);
    sp.reduceTo(0);
    sp.waitReduce();
    return 0;
}
