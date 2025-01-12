//
// Created by Fx Kaze on 25-1-6.
//

#include "ThreadPool.h"

class Test {
public:
    int *a;

    Test() {
        a = new int[5]{1, 2, 3, 4, 5};
    }

    ~Test() {
        if (a != nullptr)
            delete[] a;
    }

    Test(const Test &other) {
        a = new int[5]{1, 2, 3, 4, 5};
    }

    Test &operator=(const Test &other) {
        if (a != other.a && a != nullptr) delete[] a;
        a = new int[5]{1, 2, 3, 4, 5};
        return *this;
    }

    Test(Test &&other) {
        a = other.a;
        other.a = nullptr;
    }

    Test &operator=(Test &&other) {
        if (a != other.a && a != nullptr) delete[] a;
        a = other.a;
        other.a = nullptr;
        return *this;
    }
};

Test testC(Test test) {
    return test;
}

Test &testB(Test &other) {
    return other;
}

Test &&testA(Test &&other) {
    return std::move(other);
}

Test *testD() {
    Test tmp;
    auto &&ret = testC(tmp);
    return new Test(std::move(ret));
}

auto testFunc() {
    ThreadPool sp(10);

    Test b;
    // printf("%p\n", &b);

    auto d = sp.addThreadEX([](Test a, Test &b)-> Test &{
        auto *t = new Test;
        *t = a;
        return *t;
    }, Test(), std::ref(b)).setWait().start();
    // sp.setMax(12);
    // sp.reduceTo(1);
    // sp.waitReduce();
    auto &&ret = d.wait();
    delete &ret;
    return sp.addThreadEX([&sp](int n)-> void {
        printf("%d\n", n);
        while (n--) {
            sp.addThreadEX([n]()-> int {
                printf("%d, Hello, World!\n", n);
                throw std::runtime_error("HHHHHHHHHHHHHHH!");
                return n;
            }).setNoWait().except([n](const std::exception &e) {
                printf("except: %s\n", e.what());
                return new int(n);
            }).then([](int n) {
                printf("%d,Rec, World!\n", n);
            }).start();
        }
    }, 10).setNoWait().start();
}

int main(int argc, const char *argv[]) {\
    while (true) {
        // auto nm =
        testFunc();
        // nm.wait();
    }
    return 0;
}
