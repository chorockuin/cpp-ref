#include <iostream>
#include <memory>

class Car {
public:
    ~Car() {std::cout << "~Car" << std::endl;}
    void Go() {std::cout << "Go" << std::endl;}
};

static void basic() {
    // std::unique_ptr<Car> p = new Car; // error. 복사 생성자는 막아놨다
    std::unique_ptr<Car> p(new Car); // explicit 생성자

    // 포인터와 유사하게 사용 가능
    p->Go();
    (*p).Go();

    // 복사할 수는 없지만 이동은 가능
    // std::unique_ptr<Car> p1 = p; // error
    std::unique_ptr<Car> p2 = std::move(p); // ok

    // 제공하는 멤버 함수들이 있음
    Car *cp = p2.get();
    p2.reset();
}

template<typename T>
struct default_delete {
    void operator ()(T* p) const {
        std::cout << "delete" << std::endl;
        delete p;
    }
};

template<typename T, typename D=default_delete<T>>
class unique_ptr {
    T* p;
    // D del;
public:
    explicit unique_ptr(T* p) : p(p) {}
    // c++20 이전 버전에서 소멸자를 람다로 구현하려면, 클래스 멤버에 D 타입의 del을 추가한다
    // 그리고 아래처럼 생성자를 만들면 d가 복사 생성되는데, 람다가 복사 생성자는 지원하므로 사용 가능하다
    // 그런데 쓸 데 없이 D 타입의 del을 추가해서 공간을 차지하기 때문에, 이전에 구현했던 compressed_pair를 활용하면 좋다
    // unique_ptr(T* p, const D& d): p(p), del(d) {}
    // move도 지원하려면 아래와 같이
    // unique_ptr(T* p, D& d): p(p), del(std::move(d)) {} 
    ~unique_ptr() {
        if (p) {
            D del;
            del(p);
        }
    }
    // unique_ptr은 복사 생성자를 지원하지 않아야 하므로
    unique_ptr(const unique_ptr&) = delete; 
    unique_ptr& operator =(const unique_ptr&) = delete;

    // unique_ptr은 move 생성자를 지원해야 하므로
    unique_ptr(unique_ptr&& up) {}

    unique_ptr& operator =(unique_ptr&& up) {
        if (this != std::addressof(up)) {
        }
        return *this;
    }

    // 참조(&)로 리턴하는 것 참고
    T& operator *() const {return *p;}
    // unique_ptr을 const로 생성해서 사용할 수도 있기 때문에
    // 연산자들의 리턴값도 모두 const를 붙인다
    T* operator ->() const {return p;}
};

struct Freer {
    void operator ()(void *p) const {
        std::cout << "free" << std::endl;
        free(p);
    }
};

static void making_unique_ptr1() {
    // unique_ptr<Car> p1 = new Car; // error
    unique_ptr<Car> p2(new Car);

    p2->Go();
    (*p2).Go();

    unique_ptr<int> p3(new int);
    // 삭제자를 지정해야 하는 경우가 생김
    // 삭제자 타입을 template 인자로 받아서 unique_ptr 소멸자에서 그 타입에 맞게 처리할 수 있도록 함
    unique_ptr<int, Freer> p4(static_cast<int*>(malloc(sizeof(int))));
}

static void making_unique_ptr2() {
    // 삭제자에 람다를 넘기고 싶은데, 람다는 타입이 아닌 함수 객체이므로 아래 코드는 에러난다
    // unique_ptr<int, [](int* p) {free(p);}> p1(static_cast<int*>(malloc(sizeof(int))));

    // 아래처럼 decltype()으로 타입을 넘기면 될까?
    // c++20 이전에는 아래처럼 하면 될 것 같은데,
    // 그래도 안되는 이유는 람다는 디폴트 생성자가 없기 때문에 
    // ~unique_ptr() {
    //     if (p) {
    //         D del; // 이것이 실패하고, 그래서 안된다
    //         del(p);
    //     }
    // }
    // 그래서 결국 삭제자를 인자로 받는 unique_ptr의 생성자를 따로 하나 더 만들 수 밖엔 없다
    // auto del = [](int* p) {free(p);};
    // unique_ptr<int, decltype(del)> p1(static_cast<int*>(malloc(sizeof(int))), del);
    // move를 지원하려면 동일하게 unique_ptr의 생성자를 따로 하나 더 만든다
    // unique_ptr<int, decltype(del)> p2(static_cast<int*>(malloc(sizeof(int))), std::move(del));

    // c++20 이전까지 람다는 평가되지 않는 표현식에 넣을 수 없었기 때문에 안되었었다. c++20 부터 된다
    unique_ptr<int, decltype([](int* p) {free(p);})> p1(static_cast<int*>(malloc(sizeof(int))));
}

#include <utility>
#include "compressed_pair.hpp"
namespace using_compressed_pair {
    // 디폴트 삭제자도 템플릿으로 만든다
    template<typename T> struct default_delete {
        default_delete() = default;
        template<typename U> default_delete(const default_delete<U>&) {}
        void operator ()(T* p) const {
            std::cout << "delete" << std::endl;
            delete p;
        }
    };

    template <typename T, typename D = default_delete<T> > class unique_ptr
    {
    public:
        using pointer = T*;
        using element_type = T;
        using deleter_type = D;

        unique_ptr() : cpair(zero_and_variadic_arg_t{}) {}
        unique_ptr(std::nullptr_t) 			: cpair(zero_and_variadic_arg_t{}) {}
        explicit unique_ptr(pointer p) 		: cpair(zero_and_variadic_arg_t{}, p) {}
        unique_ptr(pointer p, const D& d) 	: cpair(one_and_variadic_arg_t{}, d, p) {}
        unique_ptr(pointer p, D&& d) 		: cpair(one_and_variadic_arg_t{}, std::move(d), p) {}

        ~unique_ptr() { if (cpair.getSecond()) cpair.getFirst()(cpair.getSecond()); }

        T& operator*()       const { return *cpair.getSecond(); }
        pointer operator->() const { return cpair.getSecond(); }

        // 멤버 함수 추가
        pointer get() const { return cpair.getSecond(); }

        D& get_deleter()  { return cpair.getFirst(); }
        const D& get_deleter() const  { return cpair.getFirst(); }
        explicit operator bool() const { return static_cast<bool>(cpair.getSecond()); }
        // https://github.com/doxygen/doxygen/issues/8909
        pointer release()  { return std::exchange(cpair.getSecond(), nullptr); }
        void reset(pointer ptr = nullptr)
        {
            pointer old = std::exchange(cpair.getSecond(), ptr);
            if (old) {
                cpair.getFirst()(old);
            }
        }

        void swap(unique_ptr& up)
        {
            std::swap(cpair.getFirst(),  up.cpair.getFirst());
            std::swap(cpair.getSecond(), up.cpair.getSecond());
        }

        // 복사 생성자는 금지시키고
        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;

        // move 생성자는 지원한다
        template<typename T2, typename D2>
        unique_ptr(unique_ptr<T2, D2>&& up)
            : cpair(one_and_variadic_arg_t{}, std::forward<D2>(up.get_deleter()), up.release()) {}

        template<typename T2, typename D2>
        unique_ptr& operator=(unique_ptr<T2, D2>&& up)
        {
            if (this != std::addressof(up))
            {
                reset(up.release());   // pointer
                cpair.getFirst()() = std::forward<D>(up.cpair.getFirst()); // deleter
            }
            return *this;
        }

    private:
        compressed_pair<D, pointer> cpair;
    };
}

static void making_unique_ptr3() {
    using_compressed_pair::unique_ptr<int> up1(new int);
    // using_compressed_pair::unique_ptr<int> up2 = up1; // unique_ptr에서는 복사 생성자는 지원하지 않아야 하므로, 지원하지 않도록 구현해야 함
    using_compressed_pair::unique_ptr<int> up3 = std::move(up1); // move는 지원해야 하므로 지원하도록 구현해야 함
}

class Animal {};
class Dog : public Animal {};

static void making_unique_ptr4() {
    using_compressed_pair::unique_ptr<int> up1(new int);
    using_compressed_pair::unique_ptr<int> up2 = std::move(up1);

    using_compressed_pair::unique_ptr<Dog> up3(new Dog);
    // Animal type과 Dog type이 다르기 때문에 move 생성자가 안먹힌다
    // move 생성자도 템플릿으로 만들어줘야 한다
    // 이런 기법을 coercion by member template 이라고 한다
    // 또한 삭제자도 type에 따라 달라야하기 때문에
    // default 삭제자도 템플릿으로 만들어줘야 한다

    using_compressed_pair::unique_ptr<Animal> up4 = std::move(up3); // default_delete<Animal>
}

void making_unique_ptr() {
    basic();
    making_unique_ptr1();
    making_unique_ptr2();
    making_unique_ptr3();
    making_unique_ptr4();
}
