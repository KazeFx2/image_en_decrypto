//
// Created by Fx Kaze on 25-1-4.
//
#include <private/ImageEncrypto.h>
#include <private/testImageEnDecrypto.h>

#include "private/Util.h"
#include "Bitmap.h"
#include "../vars.h"
#include "private/Random.h"
#include "private/testImageEnDecrypto.h"
#include "private/Util.h"
#include "ThreadPool.h"

typedef struct {
    u32 r;
    u32 c;
} Item;

int main(int argc, const char *argv[]) {
    const u32 H = 300;
    chdir(homePath);
    auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    resize(img, img, cv::Size(H, H));
    auto en = img.clone();
    ThreadPool sp(10);
    cv::Size size = {H, H};
    Keys k = RANDOM_KEYS;
    ParamControl conf = DEFAULT_CONFIG;
    DestroyReturn(testEnDecryptoImage(
                      en, size, k, conf, sp
                  ), conf);
    return 0;
}
