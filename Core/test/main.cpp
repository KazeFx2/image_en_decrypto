//
// Created by Fx Kaze on 24-12-30.
//

#include "ImageCrypto.h"
#include "private/Random.h"
#include "private/ImageEncrypto.h"
#include "unistd.h"
#include "vars.h"
#include "private/ThreadPool.h"

Mutex mu;

void *threadFunc(void *arg) {
    mu.lock();
    mu.lock();
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
    auto img = cv::imread("inputs/1.jpeg");
    EncryptoImage(img, RANDOM_KEYS, ORIGINAL_SIZE);
    int ct = 0;
    ThreadPool sp(10);
    while (true) {
        int n = 64;
        size_t handles[n];
        for (int i = 0; i < n; i++) {
            handles[i] = sp.addThread(threadFunc, (void *) (i + n));
        }
        for (int i = 0; i < n; i++) {
            // std::cout << sp.waitThread(handles[i]) << std::endl;
            sp.waitThread(handles[i]);
        }
        for (int i = 0; i < n; i++) {
            handles[i] = sp.addThread(threadFunc, (void *) i, false);
        }
        system("clear");
        std::cout << ct++ << std::endl;
        std::cout << "current threads: " << sp.getNumThreads() << std::endl;
        std::cout << "current working: " << sp.getNumThreads() - sp.getIdlyThreads() << std::endl;
        // sp.reduceTo(10, true);
        sp.waitFinish();
    }
    return 0;
}
