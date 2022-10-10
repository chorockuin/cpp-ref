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
public:
    explicit unique_ptr(T* p) : p(p) {}
    ~unique_ptr() {
        if (p) {
            D del;
            del(p);
        }
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

void making_unique_ptr1() {
    // unique_ptr<Car> p1 = new Car; // error
    unique_ptr<Car> p2(new Car);

    p2->Go();
    (*p2).Go();

    unique_ptr<int> p3(new int);
    // 삭제자를 지정해야 하는 경우가 생김
    // 삭제자 타입을 template 인자로 받아서 unique_ptr 소멸자에서 그 타입에 맞게 처리할 수 있도록 함
    unique_ptr<int, Freer> p4(static_cast<int*>(malloc(sizeof(int))));
}

void making_unique_ptr() {
    basic();
    making_unique_ptr1();
}
