#ifndef __K_CFC_
#define __K_CFC_
// #define __K_NO_WIN_

#ifndef _IOSTREAM_
#include <iostream>
#endif

#ifndef _CSTRING_
#include <cstring>
#endif

#ifndef _CSTDDEF_
#include <cstddef>
#endif

#ifndef _IOMANIP_
#include <iomanip>
#endif

#ifndef __K_NO_WIN_

#ifdef _WIN32
#ifndef _WINDOWS_
#include <windows.h>
#undef ERROR
#endif

HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
// COORD CurSorPos = {0, 0};
inline void setCursor(int r, int c) {
    SetConsoleCursorPosition(hOut, COORD{(SHORT) r, (SHORT) c});
}
#else
inline void setCursor(int r, int c) {
  std::cout << "\033[" << r + 1 << ";" << c + 1 << "H";
}
#endif

#define RESET_CURSOR setCursor(0, 0)

#ifndef _APISETCONSOLE_
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#define ENABLE_LVB_GRID_WORLDWIDE 0x0010
#endif

#endif

#ifndef _WIN32
#define Proc1 "▏"
#define Proc2 "▎"
#define Proc3 "▍"
#define Proc4 "▌"
#define Proc5 "▋"
#define Proc6 "▊"
#define Proc7 "▉"
#define Proc8 "█"
#else
#define Proc1 "\xa8\x87"
#define Proc2 "\xa8\x86"
#define Proc3 "\xa8\x85"
#define Proc4 "\xa8\x84"
#define Proc5 "\xa8\x83"
#define Proc6 "\xa8\x82"
#define Proc7 "\xa8\x81"
#define Proc8 "\xa8\x80"
#endif

#define _K_ESC "\x1b"
#define _K_CSI "\x1b["
#define _K_CSI_(x) _K_CSI #x "m"
#define _K_CFC_F_CLOR(r, g, b) _K_CSI "38;2;" #r ";" #g ";" #b "m"
#define _K_CFC_B_CLOR(r, g, b) _K_CSI "48;2;" #r ";" #g ";" #b "m"
#define _K_CFC_F_BLACK _K_CSI_(30)
#define _K_CFC_F_RED _K_CSI_(31)
#define _K_CFC_F_GREEN _K_CSI_(32)
#define _K_CFC_F_YELLOW _K_CSI_(33)
#define _K_CFC_F_BLUE _K_CSI_(34)
#define _K_CFC_F_MAGENTA _K_CSI_(35)
#define _K_CFC_F_CYAN _K_CSI_(36)
#define _K_CFC_F_WHITE _K_CSI_(37)
#define _K_CFC_F_DEFAULT _K_CSI_(39)
#define _K_CFC_B_BLACK _K_CSI_(40)
#define _K_CFC_B_RED _K_CSI_(41)
#define _K_CFC_B_GREEN _K_CSI_(42)
#define _K_CFC_B_YELLOW _K_CSI_(43)
#define _K_CFC_B_BLUE _K_CSI_(44)
#define _K_CFC_B_MAGENTA _K_CSI_(45)
#define _K_CFC_B_CYAN _K_CSI_(46)
#define _K_CFC_B_WHITE _K_CSI_(47)
#define _K_CFC_B_DEFAULT _K_CSI_(49)
#define _K_CFC_BF_BLACK _K_CSI_(90)
#define _K_CFC_BF_RED _K_CSI_(91)
#define _K_CFC_BF_GREEN _K_CSI_(92)
#define _K_CFC_BF_YELLOW _K_CSI_(93)
#define _K_CFC_BF_BLUE _K_CSI_(94)
#define _K_CFC_BF_MAGENTA _K_CSI_(95)
#define _K_CFC_BF_CYAN _K_CSI_(96)
#define _K_CFC_BF_WHITE _K_CSI_(97)
#define _K_CFC_BB_BLACK _K_CSI_(100)
#define _K_CFC_BB_RED _K_CSI_(101)
#define _K_CFC_BB_GREEN _K_CSI_(102)
#define _K_CFC_BB_YELLOW _K_CSI_(103)
#define _K_CFC_BB_BLUE _K_CSI_(104)
#define _K_CFC_BB_MAGENTA _K_CSI_(105)
#define _K_CFC_BB_CYAN _K_CSI_(106)
#define _K_CFC_BB_WHITE _K_CSI_(107)
#define _K_CFC_DEFAULT _K_CSI_(0)
#define _K_CFC_BB _K_CSI_(1)
#define _K_CFC_NO_BB _K_CSI_(22)
#define _K_CFC_UNDERLINE _K_CSI_(4)
#define _K_CFC_NO_UNDERLINE _K_CSI_(24)
#define _K_CFC_NEGATIVE _K_CSI_(7)
#define _K_CFC_POSITIVE _K_CSI_(27)

namespace _Kaze {
#define c_CFC(x)                                                               \
  case F_##x:                                                                  \
    if (Bk_C == B_##x) {                                                       \
      if (Bk_C != B_BLACK && Bk_C != BB_BLACK)                                 \
        return _K_CFC_F_BLACK;                                                 \
      else                                                                     \
        return _K_CFC_F_WHITE;                                                 \
    } else                                                                     \
      return _K_CFC_F_##x;                                                     \
    break;                                                                     \
  case B_##x:                                                                  \
    Bk_C = B_##x;                                                              \
    return _K_CFC_B_##x;                                                       \
    break;                                                                     \
  case BF_##x:                                                                 \
    if (Bk_C == BB_##x) {                                                      \
      if (Bk_C != B_BLACK && Bk_C != BB_BLACK)                                 \
        return _K_CFC_BF_BLACK;                                                \
      else                                                                     \
        return _K_CFC_BF_WHITE;                                                \
    } else                                                                     \
      return _K_CFC_BF_##x;                                                    \
    break;                                                                     \
  case BB_##x:                                                                 \
    Bk_C = BB_##x;                                                             \
    return _K_CFC_BB_##x;                                                      \
    break

#define ForceF(x)                                                              \
  case F_##x:                                                                  \
    std::cout << _K_CFC_F_##x;                                                 \
    break;                                                                     \
  case BF_##x:                                                                 \
    std::cout << _K_CFC_BF_##x;                                                \
    break

#define F2B(x)                                                                 \
  case F_##x:                                                                  \
    std::cout << Cor(B_##x);                                                   \
    break;                                                                     \
  case BF_##x:                                                                 \
    std::cout << Cor(BB_##x);                                                  \
    break

    enum _K_CFC {
        D = 0,
        F_BLACK,
        F_RED,
        F_GREEN,
        F_YELLOW,
        F_BLUE,
        F_MAGENTA,
        F_CYAN,
        F_WHITE,
        F_D,
        B_BLACK,
        B_RED,
        B_GREEN,
        B_YELLOW,
        B_BLUE,
        B_MAGENTA,
        B_CYAN,
        B_WHITE,
        B_D,
        BF_BLACK,
        BF_RED,
        BF_GREEN,
        BF_YELLOW,
        BF_BLUE,
        BF_MAGENTA,
        BF_CYAN,
        BF_WHITE,
        BB_BLACK,
        BB_RED,
        BB_GREEN,
        BB_YELLOW,
        BB_BLUE,
        BB_MAGENTA,
        BB_CYAN,
        BB_WHITE,
        BB,
        NO_BB,
        UNDER,
        NO_UNDER,
        NEGA,
        POSI
    };

    _K_CFC Bk_C = B_BLACK;

    const char *Cor(_K_CFC in) {
        switch (in) {
            case D:
                Bk_C = B_BLACK;
                return _K_CFC_DEFAULT;
                break;
            case F_D:
                return _K_CFC_F_DEFAULT;
                break;
            case B_D:
                Bk_C = B_BLACK;
                return _K_CFC_B_DEFAULT;
                break;
            case BB:
                return _K_CFC_BB;
                break;
            case NO_BB:
                return _K_CFC_NO_BB;
                break;
            case UNDER:
                return _K_CFC_UNDERLINE;
                break;
            case NO_UNDER:
                return _K_CFC_NO_UNDERLINE;
                break;
            case NEGA:
                return _K_CFC_NEGATIVE;
                break;
            case POSI:
                return _K_CFC_POSITIVE;
                break;
            c_CFC(BLACK);
            c_CFC(RED);
            c_CFC(GREEN);
            c_CFC(YELLOW);
            c_CFC(BLUE);
            c_CFC(MAGENTA);
            c_CFC(CYAN);
            c_CFC(WHITE);
            default:
                return NULL;
                break;
        }
    }

    void ProcessBar(int now, int total, _K_CFC cor_a = F_WHITE,
                    _K_CFC cor_b = F_BLACK) {
        float proc = now * 100.0 / total;
        int left = (proc - (int) proc) * 7, i = 0;
        std::cout << "[" << Cor(cor_a);
        switch (cor_b) {
            F2B(BLACK);
            F2B(RED);
            F2B(GREEN);
            F2B(YELLOW);
            F2B(BLUE);
            F2B(MAGENTA);
            F2B(CYAN);
            F2B(WHITE);
            default:
                std::cout << Cor(B_BLACK);
                break;
        }
        for (; i < (int) proc; i++)
            std::cout << Proc8;
        switch (left) {
            case 0:
                std::cout << Proc1;
                break;
            case 1:
                std::cout << Proc2;
                break;
            case 2:
                std::cout << Proc3;
                break;
            case 3:
                std::cout << Proc4;
                break;
            case 4:
                std::cout << Proc5;
                break;
            case 5:
                std::cout << Proc6;
                break;
            case 6:
                std::cout << Proc7;
                break;
            default:
                i--;
                break;
        }
        i++;
        switch (cor_b) {
            ForceF(BLACK);
            ForceF(RED);
            ForceF(GREEN);
            ForceF(YELLOW);
            ForceF(BLUE);
            ForceF(MAGENTA);
            ForceF(CYAN);
            ForceF(WHITE);
            default:
                std::cout << _K_CFC_F_BLACK;
                break;
        }
        for (; i < 100; i++)
            std::cout << Proc8;
        std::cout << Cor(D) << "]" << proc << "%      \n";
    }

    void ProcessBar_(_K_CFC *cor_l, int64_t total, _K_CFC cor_a = F_WHITE,
                     _K_CFC cor_b = F_BLACK) {
        int now = 0;
        for (int64_t i = 0; i < total; i++)
            if (cor_l[i] == B_GREEN)
                now++;
        float proc = now * 100.0 / total;
        int left = (proc - (int) proc) * 7, i = 0;
        std::cout << "[" << Cor(cor_a);
        switch (cor_b) {
            F2B(BLACK);
            F2B(RED);
            F2B(GREEN);
            F2B(YELLOW);
            F2B(BLUE);
            F2B(MAGENTA);
            F2B(CYAN);
            F2B(WHITE);
            default:
                std::cout << Cor(B_BLACK);
                break;
        }
        for (; i < (int) proc; i++)
            std::cout << Proc8;
        switch (left) {
            case 0:
                std::cout << Proc1;
                break;
            case 1:
                std::cout << Proc2;
                break;
            case 2:
                std::cout << Proc3;
                break;
            case 3:
                std::cout << Proc4;
                break;
            case 4:
                std::cout << Proc5;
                break;
            case 5:
                std::cout << Proc6;
                break;
            case 6:
                std::cout << Proc7;
                break;
            default:
                i--;
                break;
        }
        i++;
        switch (cor_b) {
            ForceF(BLACK);
            ForceF(RED);
            ForceF(GREEN);
            ForceF(YELLOW);
            ForceF(BLUE);
            ForceF(MAGENTA);
            ForceF(CYAN);
            ForceF(WHITE);
            default:
                std::cout << _K_CFC_F_BLACK;
                break;
        }
        for (; i < 100; i++)
            std::cout << Proc8;
        std::cout << Cor(D) << "]" << proc << "%      \n";
    }
} // namespace _Kaze

#endif
