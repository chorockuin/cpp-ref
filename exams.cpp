/*  */

#include <memory>
#include <iostream>
#include <cstring>
void exam1() {
    auto deleter = [](int* p) {free(p); std::cout << "deleted pointer" << std::endl;};
    std::unique_ptr<int, decltype(deleter)> up{static_cast<int*>(malloc(sizeof(int)*20)), deleter};
}

class Label {
    char* text;
    std::size_t size;
    int* ref;
public:
    Label(const char* s) {
        size = strlen(s);
        text = new char[size+1];
        strcpy(text, s);
        ref = new int(1);
    }
    Label(const Label& other) : text(other.text), size(other.size), ref(other.ref) {
        ++(*ref);
    }
    ~Label() {
        if (--(*ref) == 0) {
            std::cout << "deleted " << text << " ref: " << *ref << std::endl;
            delete ref;
            delete text;
        }
    }

    struct temporary_proxy {
        Label *lb;
        int idx;

        temporary_proxy(Label *lb, int idx) : lb(lb), idx(idx) {}

        temporary_proxy& operator =(char value) {
            --*(lb->ref);
            char *text = new char[lb->size+1];
            int *ref = new int(1);
            strcpy(text, lb->text);
            lb->text = text;
            lb->ref = ref;
            lb->text[idx] = value;
            return *this;
        }

        operator char() {
            return lb->text[idx];
        }
    };

    temporary_proxy operator [](int idx) {
        return temporary_proxy(this, idx);
    }

    void print() const {
        std::cout << text << " ref: " << *ref << std::endl;
    }
};

void exam2() {
    Label lb1("hello");
    Label lb2 = lb1;

    char c = lb1[0];
    std::cout << c << std::endl;
    lb1.print();
    lb2.print();
    
    lb1[0] = 'A';

    lb1.print();
    lb2.print();
}

#include <vector>
#include <ranges>
template<typename T> class drop_view : public std::ranges::view_interface<drop_view<T>> {
    T ranges;
    std::size_t count;
public:
    drop_view() = default;
    drop_view(T& ranges, std::size_t count) : ranges(ranges), count(count) {}

    auto begin() {return ranges.begin() + count;}
    auto end() {return ranges.end();}
};

template<typename T>
drop_view(T&& t, std::size_t)->drop_view<std::views::all_t<T>>;

void exam3() {
    std::vector v = {1,2,3,4,5,6,7,8,9,10};
    std::ranges::reverse_view rv(v);
    drop_view dv(rv, 3);

    for (auto e : dv) {
        std::cout << e << ", ";
    }
    std::cout << std::endl;
}

void exams() {
    exam1();
    exam2();
    exam3();
}
