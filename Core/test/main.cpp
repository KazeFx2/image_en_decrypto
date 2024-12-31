//
// Created by Fx Kaze on 24-12-30.
//

#include "ImageCrypto.h"
#include "private/ImageEncrypto.h"
#include "unistd.h"
#include "vars.h"
#include "private/ThreadPool.h"

std::mutex mu;

void *threadFunc(void *arg) {
    mu.lock();
    // int a = 1000;
    // while (a--) {}
    mu.unlock();
    return arg;
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
    auto img = cv::imread("inputs/1.jpeg");
    EncryptoImage(img, RANDOM_KEYS, ORIGINAL_SIZE);
    int ct = 0;
    while (true) {
        ThreadPool sp(10);
        int n = 100;
        size_t handles[n];
        for (int i = 0; i < n; i++) {
            handles[i] = sp.addThread(threadFunc, (void *)i, false);
        }
        for (int i = 0; i < n; i++) {
            handles[i] = sp.addThread(threadFunc, (void *)(i+n));
        }
        for (int i = 0; i < n; i++) {
            // std::cout << sp.waitThread(handles[i]) << std::endl;
            sp.waitThread(handles[i]);
        }
        sp.reduceTo(3, true);
        sp.waitReduce();
        std::cout << ct++ << std::endl;
    }
    return 0;
}
