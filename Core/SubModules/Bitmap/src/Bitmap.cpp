//
// Created by Fx Kaze on 25-1-3.
//

#include "Bitmap.h"


FastBitmap::FastBitmap(const u_size_t initBits, const bool defaultV): defaultValue(defaultV), bitmap(
                                                                          initBits / (8 * sizeof(uptr)) + (
                                                                              initBits % (8 * sizeof(uptr))
                                                                                  ? 1
                                                                                  : 0),
                                                                          defaultV ? ~static_cast<uptr>(0x0) : 0x0) {
    max = bitmap.size() * 8 * sizeof(uptr);
}

FastBitmap::~FastBitmap() = default;

FastBitmap::element FastBitmap::operator[](const u_size_t index) {
    const u_size_t idx = index / (8 * sizeof(uptr));
    const u_size_t offset = index % (8 * sizeof(uptr));
    if (index >= max) {
        const u_size_t nNew = idx - bitmap.size() + 1;
        bitmap.insert(bitmap.end(), nNew, defaultValue ? ~static_cast<uptr>(0x0) : 0x0);
        max += nNew * 8 * sizeof(uptr);
    }
    return element(bitmap[idx], offset);
}

#define HIGH_BIT_MASK(reserve) (static_cast<uptr>((static_cast<iptr>(static_cast<uptr>(0x1) << (sizeof(uptr) * 8 - 1)) * -1) >> (reserve)))

static u8 findFirstNonZeroBit(uptr value) {
    if (value == 0) { return 8 * sizeof(uptr); }
    if (value == 1) { return 0; }
    if (value > (static_cast<uptr>(0x1) << (8 * sizeof(uptr) / 2))) {
        auto b = 0;
        while (value) {
            value <<= 1;
            ++b;
        }
        return 8 * sizeof(uptr) - b;
    }
    u8 start = 0, end = 8 * sizeof(uptr);
    while (start + 1 != end) {
        const u8 mid = (end + start) / 2;
        const uptr t_value = static_cast<uptr>(0x1) << mid;
        if (value == t_value) return mid;
        if (value < t_value) {
            end = mid;
        } else {
            if ((value & ~HIGH_BIT_MASK(8 * sizeof(uptr) - 1 - mid)) == 0) {
                start = mid;
            } else {
                if (mid + 1 != end)
                    end = mid + 1;
                else return mid - 1;
            }
        }
    }
    return start - 1;
}

count_t FastBitmap::_findNext(const bool value, const count_t start, const count_t end) const {
    if (end <= start) return BITMAP_NOT_FOUND;
    u_size_t idx = start / (8 * sizeof(uptr));
    const u_size_t end_idx = (end - 1) / (8 * sizeof(uptr)) + 1;
    u8 offset = start % (8 * sizeof(uptr));
    uptr tmp = bitmap[idx];
    const u8 reserve = 8 * sizeof(uptr) - 1 - offset;
    if (!value) {
        tmp = ~tmp;
    }
    tmp &= HIGH_BIT_MASK(reserve);
    while (!tmp) {
        if (++idx >= bitmap.size() || idx == end_idx) { return BITMAP_NOT_FOUND; }
        tmp = bitmap[idx];
        if (!value) {
            tmp = ~tmp;
        }
    }
    offset = findFirstNonZeroBit(tmp);
    const count_t res = idx * 8 * sizeof(uptr) + offset;
    if (res < end) return res;
    return BITMAP_NOT_FOUND;
}
