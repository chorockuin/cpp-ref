#pragma once

struct one_and_variadic_arg_t {}; // 인자 1개 + 나머지 가변 인자
struct zero_and_variadic_arg_t {}; // 가변인자만

// 만약 Empty class가 상속을 할 수 없는 final class라고 하면, 에러 내지말고 그냥 이 버전을 사용하도록 하기 위해 is_final_v 조건을 넣는다
template<typename T1, typename T2, bool = std::is_empty_v<T1> && !std::is_final_v<T1>>
struct compressed_pair;

template<typename T1, typename T2>
struct compressed_pair<T1, T2, false> {
    T1 first;
    T2 second;

    // compile time에 활용하기 위해 constexpr 키워드를 붙인다
    // 예외가 없다면 noexcept를 붙이는 것이 좋다
    constexpr T1& getFirst() noexcept {return first;}
    constexpr T2& getSecond() noexcept {return second;}
    constexpr const T1& getFirst() const noexcept {return first;}
    constexpr const T2& getSecond() const noexcept {return second;}

    // 이렇게 생성자가 const ref로 인자를 받으면 move를 지원하지 못하므로
    // compressed_pair(const T1& f, const T2& s) : first(f), second(s) {}
    
    // 이렇게 바꿔준다
    // 그런데, 아래 one_and_variadic_arg_t, zero_and_variadic_arg_t가 추가된 생성자를 정의하면 생성자 정의가 중복 될 수 있으므로 체크해 두자
    template<typename F, typename S>
    constexpr compressed_pair(F&& f, S&& s) noexcept(std::conjunction_v<std::is_nothrow_constructible<T1, F>, std::is_nothrow_constructible<T2, S>>)
    : first(std::forward<F>(f)), second(std::forward<S>(s)) {}

    // second 인자에 가변인자 템플릿을 활용
    template<typename F, typename ... S>
    // noexcept 안에 조건을 넣을 수가 있는데 T1을 F로 생성할 때 예외가 없고, T2를 S로 생성할 때 예외가 없는 경우에만 실제 예외가 없는 것이므로, 아래와 같이 적어준다
    constexpr compressed_pair(one_and_variadic_arg_t, F&& f, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_constructible<T1, F>, std::is_nothrow_constructible<T2, S...>>)
    : first(std::forward<F>(f)), second(std::forward<S>(s)...) {}

    // first 인자를 생략하고, second 인자에만 가변인자 템플릿을 활용
    template<typename ... S>
    // noexcept 안에 조건을 넣을 수가 있는데 T1을 디폴트로 생성할 때 예외가 없고, T2를 S로 생성할 때 예외가 없는 경우에만 실제 예외가 없는 것이므로, 아래와 같이 적어준다
    constexpr compressed_pair(zero_and_variadic_arg_t, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_default_constructible<T1>, std::is_nothrow_constructible<T2, S...>>)
    : first(), second(std::forward<S>(s)...) {}
};

template<typename T1, typename T2>
struct compressed_pair<T1, T2, true> : public T1 {
    T2 second;

    constexpr T1& getFirst() noexcept {return *this;}
    constexpr T2& getSecond() noexcept {return second;}
    constexpr const T1& getFirst() const noexcept {return *this;}
    constexpr const T2& getSecond() const noexcept {return second;}

    // second 인자에 가변인자 템플릿을 활용
    template<typename F, typename ... S>
    constexpr compressed_pair(one_and_variadic_arg_t, F&& f, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_constructible<T1, F>, std::is_nothrow_constructible<T2, S...>>)
    : T1(std::forward<F>(f)), second(std::forward<S>(s)...) {}

    // first 인자를 생략하고, second 인자에만 가변인자 템플릿을 활용
    template<typename ... S>
    // noexcept 안에 조건을 넣을 수가 있는데 T1을 디폴트로 생성할 때 예외가 없고, T2를 S로 생성할 때 예외가 없는 경우에만 실제 예외가 없는 것이므로, 아래와 같이 적어준다
    constexpr compressed_pair(zero_and_variadic_arg_t, S&& ... s) noexcept(std::conjunction_v<std::is_nothrow_default_constructible<T1>, std::is_nothrow_constructible<T2, S...>>)
    : T1(), second(std::forward<S>(s)...) {}
};
