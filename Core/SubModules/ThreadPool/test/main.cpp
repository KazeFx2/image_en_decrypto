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

int main(int argc, const char *argv[]) {\
    while (true) {
        ThreadPool sp(10);

        Test b;
        // printf("%p\n", &b);

        auto d = sp.addThreadEX([](Test a, Test &b)-> Test &{
            auto *t = new Test;
            *t = a;
            return *t;
        }, Test(), std::ref(b)).setWait().start();
        auto &ret = d.wait();
        delete &ret;

        auto f = sp.addThreadEX([]()-> Test &{
            auto *t = new Test;
            throw std::runtime_error("test");
            return *t;
        }).except([](std::exception &e, Test &t) {
            t.a[0] = 0;
        }).then([](Test &t) {
            printf("%d\n", t.a[0]);
            delete &t;
        }).start();
    }
    return 0;
}
