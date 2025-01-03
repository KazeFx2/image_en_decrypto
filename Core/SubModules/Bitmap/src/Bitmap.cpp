//
// Created by Fx Kaze on 25-1-3.
//

#include "Bitmap.h"


FastBitmap::FastBitmap(const u_size_t initBits): bitmap(
    initBits / (8 * sizeof(uptr)) + (initBits % (8 * sizeof(uptr))
                                         ? 1
                                         : 0),
    0x0) {
    max = bitmap.size() * 8 * sizeof(uptr);
}

FastBitmap::~FastBitmap() = default;

FastBitmap::element FastBitmap::operator[](u_size_t index) {
    const u_size_t idx = index / (8 * sizeof(uptr));
    const u_size_t offset = index % (8 * sizeof(uptr));
    if (index >= max) {
        const u_size_t nNew = idx - bitmap.size() + 1;
        bitmap.insert(bitmap.end(), nNew, 0x0);
        max += nNew * 8 * sizeof(uptr);
    }
    return element(bitmap[idx], offset);
}
