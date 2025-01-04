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
    const u32 H = 300;
    FastBitmap bitmap(H * H);
    Item map[H * H];
    for (u32 i = 0; i < H; i++) {
        for (u32 j = 0; j < H; j++) {
            u32 nr, nc;
            cv::Size size(H, H);
            ConfusionFunc(i, j, size, 0x1234, nr, nc);
            if (bitmap[nr * H + nc]) {
                printf("[Conflict]row: %d, col: %d, -> row: %d, col: %d (old row: %d, old col: %d)\n", i, j, nr, nc,
                       map[nr * H + nc].r, map[nr * H + nc].c);
            } else {
                map[nr * H + nc] = {i, j};
            }
            bitmap[nr * H + nc] = true;
        }
    }
    for (u32 i = 0; i < H; i++) {
        for (u32 j = 0; j < H; j++) {
            if (!bitmap[i * H + j]) {
                printf("[Lost]row: %d, col: %d\n", i, j);
            }
        }
    }
    chdir(homePath);
    auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    resize(img, img, cv::Size(H, H));
    auto en = img.clone();
    u32 k = Rand32();
    Confusion(en, img, 0, H, 0, H, {H, H}, k);
    imwrite("outputs/1_en_fu.jpeg", en);
    auto de = en.clone();
    InvertConfusion(de, en, 0, H, 0, H, {H, H}, k);
    imwrite("outputs/1_de_fu.jpeg", de);
    return 0;
}
