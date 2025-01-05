//
// Created by Fx Kaze on 25-1-4.
//
#include <private/ImageEncrypto.h>

#include "private/Util.h"
#include "Bitmap.h"
#include "../vars.h"
#include "private/Random.h"

typedef struct {
    u32 r;
    u32 c;
} Item;

int main(int argc, const char *argv[]) {
    cv::Mat test(10, 10, CV_8UC3);

    printf("%p, %p\n", test.ptr(1, 2), &test.at<cv::Vec3b>(1, 2)[0]);
    printf("%p, %p\n", test.ptr(1, 2) + 1, &test.at<cv::Vec3b>(1, 2)[1]);
    printf("%p, %p\n", test.ptr(1, 2) + 2, &test.at<cv::Vec3b>(1, 2)[2]);


    const u32 W = 600, H = 300;
    FastBitmap bitmap(W * H);
    Item map[W * H];
    for (u32 i = 0; i < H; i++) {
        for (u32 j = 0; j < W; j++) {
            u32 nr, nc;
            cv::Size size(W, H);
            ConfusionFunc(i, j, size, 0x134, nr, nc);
            if (bitmap[nr * W + nc]) {
                printf("[Conflict]row: %d, col: %d, -> row: %d, col: %d (old row: %d, old col: %d)\n", i, j, nr, nc,
                       map[nr * W + nc].r, map[nr * W + nc].c);
            } else {
                map[nr * W + nc] = {i, j};
            }
            bitmap[nr * W + nc] = true;
        }
    }
    for (u32 i = 0; i < H; i++) {
        for (u32 j = 0; j < W; j++) {
            if (!bitmap[i * W + j]) {
                printf("[Lost]row: %d, col: %d\n", i, j);
            }
        }
    }
    // return 0;
    chdir(homePath);
    auto img = imread("inputs/test2.png", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    // resize(img, img, cv::Size(H, H));
    auto en = img.clone();
    auto s = img.size();
    u32 k = Rand32();
    Confusion(en, img, 0, s.height, 0, s.width, s, k, 3);
    imwrite("outputs/test_en_fu.jpeg", en);
    auto de = en.clone();
    InvertConfusion(de, en, 0, s.height, 0, s.width, s, k, 3);
    imwrite("outputs/test_de_fu.jpeg", de);
    return 0;
}
