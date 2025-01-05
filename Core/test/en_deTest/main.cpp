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
#include "ImageCrypto.h"

typedef struct {
    u32 r;
    u32 c;
} Item;

int main(int argc, const char *argv[]) {
    const u32 H = 300;
    chdir(homePath);
    // auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    // if (img.data == nullptr) {
    //     std::cout << "Read image failed." << std::endl;
    //     return 1;
    // }
    // resize(img, img, cv::Size(H, H));
    // auto en = img.clone();
    // ThreadPool sp(10);
    // cv::Size size = {H, H};
    // Keys k = RANDOM_KEYS;
    // ParamControl conf = DEFAULT_CONFIG;
    // DestroyReturn(testEnDecryptoImage(
    //                   en, size, k, conf, sp
    //               ), conf);
    auto img = imread("inputs/test.png", cv::IMREAD_UNCHANGED);
    ThreadPool sp(8);
    ImageCrypto cy(&sp);
    auto en = cy.encrypt(img);
    imwrite("outputs/test_en.png", en);
    imwrite("outputs/test_en.jpg", en);
    auto read = imread("outputs/test_en.png", cv::IMREAD_UNCHANGED);
    imwrite("outputs/test_de.png", cy.decrypt(read));
    // imshow("de", cy.decrypt(en));
    // cv::waitKey(0);
    return 0;
}
