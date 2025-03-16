#include <unrolled_list.h>

#include <iostream>

int main(int argc, char** argv) {
    unrolled_list<int> a;
    a.push_back(10);
    std::cout << a.back() << std::endl;
    return 0;
}
