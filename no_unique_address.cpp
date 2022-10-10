#include <iostream>

struct Empty {
};

struct Data {
    Empty e;
    int data;
};

struct Data1 {
    [[no_unique_address]] Empty e;
    int data;
};

static void basic() {
    Data d;
    std::cout << sizeof(d) << std::endl; // e(1) + padding(3) + int(4) = 8
    std::cout << &(d.e) << " != " << &(d.data) << std::endl;

    Data1 d1;
    std::cout << sizeof(d1) << std::endl; // int(4)
    std::cout << &(d1.e) << " == " << &(d1.data) << std::endl;
}

struct E1 {};
struct E2 {};

struct D1 { // size: 4
    [[no_unique_address]] E1 e1;
    [[no_unique_address]] E2 e2;
    int data;
};

struct D2 { // size: 1
    [[no_unique_address]] E1 e1;
};

struct D3 { // size: 1
    [[no_unique_address]] E1 e1;
    [[no_unique_address]] E2 e2;
};

struct D4 { // 타입이 같을 경우, 구분해야 하기 때문에 각각 1byte를 갖는다. size: 2
    [[no_unique_address]] E1 e1_1;
    [[no_unique_address]] E1 e1_2;
};

static void basic1() {
    std::cout << sizeof(D1) << std::endl;
    std::cout << sizeof(D2) << std::endl;
    std::cout << sizeof(D3) << std::endl;
    std::cout << sizeof(D4) << std::endl;
    D3 d3;
    std::cout << &(d3.e1) << " == " << &(d3.e2) << std::endl;
    D4 d4;
    std::cout << &(d4.e1_1) << " != " << &(d4.e1_2) << std::endl;
}

// [[no_unique_address]]를 사용하면, 기존의 compressed_pair를 더 쉽게 만들 수 있다
struct one_and_variadic_arg_t {};
struct zero_and_variadic_arg_t {};

template<typename T1, typename T2>
struct compressed_pair {
    [[no_unique_address]] T1 first;
    [[no_unique_address]] T2 second;

    constexpr T1& getFirst() noexcept {return first;}
    constexpr T2& getSecond() noexcept {return second;}
    constexpr const T1& getFirst() const noexcept {return first;}
    constexpr const T2& getSecond() const noexcept {return second;}

    template<typename F, typename ... S>
    constexpr compressed_pair(one_and_variadic_arg_t, F&& f, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_constructible<T1, F>, std::is_nothrow_constructible<T2, S...>>)
    : first(std::forward<F>(f)), second(std::forward<S>(s)...) {}

    template<typename ... S>
    constexpr compressed_pair(zero_and_variadic_arg_t, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_default_constructible<T1>, std::is_nothrow_constructible<T2, S...>>)
    : first(), second(std::forward<S>(s)...) {}
};

static void no_unique_address_compressed_pair() {
    compressed_pair<int, int> cp1(one_and_variadic_arg_t{}, 1, 1); // 8
    compressed_pair<Empty, int> cp2(zero_and_variadic_arg_t{}, 1); // 4
    compressed_pair<int, Empty> cp3(zero_and_variadic_arg_t{}); // 4
    compressed_pair<Empty, Empty> cp4(zero_and_variadic_arg_t{}); // 2

    std::cout << sizeof(cp1) << "," << sizeof(cp2) << "," << sizeof(cp3) << "," << sizeof(cp4) << std::endl;
}

void no_unique_address() {
    basic();
    basic1();
    no_unique_address_compressed_pair();
}
