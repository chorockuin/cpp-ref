/*
empty class
- instance(non-static) member data 없음
- 가상 함수 없음
- 가상 상속 없음
- class 또는 struct
- union은 empty class 아님
- tag dispatching에 활용됨
*/

#include <iostream>
#include <type_traits>

class Empty {
};

class NoEmpty {
    int data;
};

void basic() {
    Empty e;
    
    std::cout << sizeof(Empty) << std::endl; // 1
    std::cout << sizeof(NoEmpty) << std::endl; // 4

    std::cout << std::boolalpha;
    std::cout << std::is_empty<Empty>::value << std::endl; // true
    std::cout << std::is_empty_v<Empty> << std::endl; // true
    std::cout << std::is_empty_v<NoEmpty> << std::endl; // false
}

class E1 {
};

struct E2 {
};

class E3 {
    static int data;
public:
    E3() {
    }

    void func() {
    }
};
int E3::data = 0;

union NE1 {
};

class NE2 {
    int data;
};

class NE3 {
    virtual void func() {
    }
};

class NE4 : virtual public E2 {
};

class NE5 { // 참고
    E1 e1;
};

void empty_class1() {
    std::cout << std::boolalpha;
    std::cout << std::is_empty_v<E1> << std::endl;
    std::cout << std::is_empty_v<E2> << std::endl;
    std::cout << std::is_empty_v<E3> << std::endl;
    
    std::cout << std::is_empty_v<NE1> << std::endl;
    std::cout << std::is_empty_v<NE2> << std::endl;
    std::cout << std::is_empty_v<NE3> << std::endl;
    std::cout << std::is_empty_v<NE4> << std::endl;
    std::cout << std::is_empty_v<NE5> << std::endl;
}

#include <functional>
#include <type_traits>

void empty_class2() {
    std::plus<int> f1; // 함수 객체로써 ()연산자만 정의되어 있음. empty
    std::modulus<int> f2; // 함수 객체로써 ()연산자만 정의되어 있음. empty

    int v=0;
    auto f3 = [](int a, int b) {return a+b;}; // 캡쳐하지 않은 람다는 함수 객체. empty
    auto f4 = [v](int a, int b) {return a+b+v;}; // 캡쳐한 람다는 캡쳐된 파라미터가 공간을 차지. no empty

    std::allocator<int> ax;

    std::cout << std::is_empty_v<decltype(f1)> << std::endl;
    std::cout << std::is_empty_v<decltype(f2)> << std::endl;
    std::cout << std::is_empty_v<decltype(f3)> << std::endl;
    std::cout << std::is_empty_v<decltype(f4)> << std::endl;
    std::cout << std::is_empty_v<decltype(ax)> << std::endl;
}

#include <mutex>

// empty struct
struct adopt_lock_t {
    explicit adopt_lock_t() = default; // lock_guard g(m, {}); 이렇게 쓰는 것을 막기 위해
};
constexpr adopt_lock_t adopt_lock; // constexpr을 써서 compile time에 확인하자

// RAII(Resource Acquisition Is Initialization)
template <class Mutex>
class lock_guard {
public:
    using mutex_type = Mutex;
    // compile time이 아닌, run time에 autolock 조건이 체크됨. 초큼 구림
    explicit lock_guard(Mutex& mtx, bool autolock=true) : mtx(mtx) {
        if (autolock) {
            mtx.lock();
        }
    }
    // 그래서 empty struct를 인자로 받고, mtx.lock();을 하지 않는 생성자를 만듬
    // compile time에 autolock 조건을 체크할 수 있으며, 이런 트릭을 tag dispatching 이라고 함
    // empty struct를 사용하지 않고, 그냥 int와 같은 type을 적어도 되지만
    // int라는 type만으로 autolock의 의미를 나타내긴 어려우므로
    // 의미를 나타내는 adopt_lock_t라는 type(그래서 tag type이라고 부름)을 정의하는 것이 가독성면에서 좋다
    explicit lock_guard(Mutex& mtx, adopt_lock_t) : mtx(mtx) {
    }
    ~lock_guard() noexcept {mtx.unlock();}

    lock_guard(const lock_guard&) = delete; // 복사 금지
    lock_guard& operator =(const lock_guard&) = delete; // 대입 금지
private:
    Mutex& mtx;
};

void tag_dispatching() {
    {
        std::mutex m;
        lock_guard g(m);
    }
    {
        std::mutex m;
        m.lock();
        lock_guard g(m, false);
    }
    {
        std::mutex m;
        m.lock();
        lock_guard g(m, adopt_lock);
        // 아래처럼 써도 c++11 부터 가능함
        // 그러나 본 목적인 가독성을 떨어뜨리는 사용법이므로
        // 이런 사용을 막기 위해 adopt_lock_t에 explicit 생성자를 추가한다
        // lock_guard g(m, {});
    }
}

void tag_dispatching_in_std() {
    std::mutex m;
    std::unique_lock u1(m, std::adopt_lock);
    std::unique_lock u2(m, std::defer_lock);
    std::unique_lock u3(m, std::try_to_lock);

    int* p1 = new int[10]; // 실패시 std::bad_alloc 예외 발생
    delete[] p1;

    // c++98 시절부터 존재하던 tag dispatching
    int* p2 = new(std::nothrow) int[10]; // 실패시 0 반환
    delete[] p2;
}

void empty_class() {
    basic();
    empty_class1();
    empty_class2();
    tag_dispatching();
    tag_dispatching_in_std();
}
