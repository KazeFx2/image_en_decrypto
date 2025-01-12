//
// Created by Fx Kaze on 25-1-3.
//

#ifndef BITMAP_H
#define BITMAP_H

#include <vector>
#include "private/types.h"

#define BITMAP_NOT_FOUND (~static_cast<count_t>(0x0))

class FastBitmap {
public:
    class element {
    public:
        element(uptr &data, const u8 offset) : data(&data), offset(offset) {
        };

        element(const element &other) = delete;

        ~element() {
        };

        operator bool() const {
            return static_cast<bool>(*data & static_cast<uptr>(0x1) << offset);
        }

        bool operator=(const bool val) const {
            if (val) {
                *data |= static_cast<uptr>(0x1) << offset;
            } else {
                *data &= ~(static_cast<uptr>(0x1) << offset);
            }
            return val;
        }

    private:
        uptr *data;
        u8 offset;
    };

    using u_size_t = u64;

    FastBitmap(u_size_t initBits = 128, bool defaultV = false);

    ~FastBitmap();

    element operator[](u_size_t index);

    count_t findNext(const count_t start = 0, const count_t end = ~static_cast<count_t>(0x0)) const {
        if (defaultValue) return findNextFalse(start, end);
        return findNextTrue(start, end);
    }

    count_t findNextTrue(const count_t start = 0, const count_t end = ~static_cast<count_t>(0x0)) const {
        return _findNext(true, start, end);
    }

    count_t findNextFalse(const count_t start = 0, const count_t end = ~static_cast<count_t>(0x0)) const {
        return _findNext(false, start, end);
    }

private:
    bool defaultValue;
    u_size_t max;
    std::vector<uptr> bitmap;

    count_t _findNext(bool value = true, count_t start = 0, count_t end = ~static_cast<count_t>(0x0)) const;
};

#endif //BITMAP_H
