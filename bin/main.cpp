#include <unrolled_list.h>

#include <iostream>

int main(int argc, char** argv) {
    unrolled_list<int> a;
    a.push_back(10);
    a.push_front(5);
    std::cout << a.back() << ' ' << a.front() << std::endl;
    a.push_back(15);
    auto it = a.begin();
    it++;
    a.erase(it);
    it = a.begin();
    for (; it != a.end(); it++) std::cout << *it << ' ';
    return 0;
}
