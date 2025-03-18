#include <unrolled_list.h>

#include <iostream>

int main(int argc, char** argv) {
    unrolled_list<int> a;
    for (int i = 1; i <= 14; ++i) {
        a.push_back(i);
    }
    for (int i = 0; i < 5; ++i) {
        a.pop_front();
    }
    std::cout << a.size() << '\n';
    for (auto i : a) {
        std::cout << i << ' ';
    }
    return 0;
}
