#include <iostream>
#include <cassert>
#include "function.h"
#include <functional>
#include <vector>
#include <set>
#include <unordered_set>


int foo() {
    return 1;
}

int bar() {
    return 2;
}

double pi() {
    // kak grubo
    return 3.14;
}

void test_defaultConstructor() {
    myns::function<int(void)> f(foo);
    assert(f() == 1);
}

void test_copyConstructor() {
    myns::function<int(void)> b(bar);
    myns::function<int(void)> second(b);
    assert(second() == 2);
}

void test_nullptrConstructor() {
    myns::function<void(void)> f(nullptr);
}

void test_moveConstructor() {
    myns::function<int(void)> b(bar);
    myns::function<int(void)> second(std::move(b));
    assert(second() == 2);
}

void test_operatorAssignment() {
    myns::function<int(void)> b(bar);
    myns::function<int(void)> second = b;
    assert(second() == 2);
}

void test_moveAssignment() {
    myns::function<int(void)> b(bar);
    myns::function<int(void)> second(foo);
    second = std::move(b);
    assert(second() == 2);
}

void test_explicitOperatorBool() {
    myns::function<int(void)> f(nullptr);
    assert(!f);
    f = foo;
    assert(f);
}

void test_lambda() {
    int a = 10;
    myns::function<int(int)> l = [a](int x) {
        return a + x;
    };
    assert(l(5) == 15);
}

void test_swap() {
    myns::function<int()> f(foo);
    myns::function<int()> b(bar);
    assert(f() == 1);
    assert(b() == 2);

    f.swap(b);

    assert(f() == 2);
    assert(b() == 1);
}

void test_diffTypes() {
    myns::function<int()> f = foo;
    assert(f() == 1);
    f = pi;
    assert(pi() == 3.14);
}

void test_copy() {
    std::vector<int> buffer(100, -1);
    myns::function<int()> g;
    {
        myns::function<int()> f = [buffer]() {
            return buffer[99];
        };
        g = f;
        myns::function<int()> h(f);
        assert(f() == -1);
        assert(g() == -1);
        assert(h() == -1);
    }
    assert(g() == -1);
}

void test_copy_small_object() {
    std::shared_ptr<std::vector<int>> buffer = std::make_shared<std::vector<int>>(100, -1);
    myns::function<int()> g;
    {
        myns::function<int()> f = [buffer]() {
            return (*buffer)[99];
        };
        g = f;
        myns::function<int()> h(f);
        assert(f() == -1);
        assert(g() == -1);
        assert(h() == -1);
    }
    assert(g() == -1);
}

void NIKITOZZZZ_test() {
    // тут хз, мб плохой тест (для решение нужна убрать const после invoke/call/etc)
    int foo = 1;
    double bar = 3;
    double bar2 = 3;
    double bar3 = 3;

    myns::function<int (std::ostream &)> f([=](std::ostream &os) mutable {
        os << "test " << foo << " " << bar << std::endl;
        os << "test " << bar2 << " " << bar3 << std::endl;
        foo *= 2;
        foo += 2;
        bar -= 0.1;
        os << "test " << foo << " " << bar << std::endl;
        return foo;
    });

    f(std::cout);
}

void ignat_test() {
    struct foo {
        int *ptr;
        int x;
        foo(int x) : x{x}
        {
            ptr = &this->x;
        }
        foo(const foo &other) : foo(other.x) { }

        int operator()(int arg) {
            return *ptr + arg;
        }
    };
    std::function<int(int)> f = foo(42);
    std::function<int(int)> f2 = foo(0);
    f.swap(f2);
    assert(f(228) == 228);
    assert(f2(228) == 270);
}


void all_test() {
    test_defaultConstructor();
    test_copyConstructor();
    test_nullptrConstructor();
    test_moveConstructor();
    test_operatorAssignment();
    test_moveAssignment();
    test_explicitOperatorBool();
    test_swap();
    test_lambda();
    test_diffTypes();
    test_copy_small_object();
    test_copy();
    NIKITOZZZZ_test();
    ignat_test();
    std::cout << "OK!";
}

struct test_t{
    test_t(const test_t &) {
        std::cout<<"Copy constructor\n";
    }

    test_t(test_t &&) {
        std::cout<<"Move constructor\n";
    }

    test_t() {
        std::cout<<"Default constructor\n";
    }

    ~test_t(){
        std::cout<<"Destructor\n";
    }
    void operator()() {
        std::cout<<"Call\n";
    }
};




int main() {
    all_test();
    myns::function<void()> f = test_t();
    f();
    return 0;
}