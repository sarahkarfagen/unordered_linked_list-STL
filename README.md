# Unrolled Linked List
STL - совместивый контейнер

Контейнер предоставляет из себя шаблон, праметрезируемый типом хранимых объетов, максимальным количеством элементов в ноде и аллокатором, а так же частично(см ниже) удовлетворяет следующим требованиям к stl - совместимым контейнерам:

  - [контейнера](https://en.cppreference.com/w/cpp/named_req/Container)
  - [последовательный контейнера](https://en.cppreference.com/w/cpp/named_req/SequenceContainer)
     - ~~emplace~~
     - ~~assign_range~~
     - ~~emplace_front~~
     - ~~emplace_back~~
     - ~~prepend_range~~
     - ~~operator[]~~
  - [контейнера с обратным итератором](https://en.cppreference.com/w/cpp/named_req/ReversibleContainer)
  - [контейнера поддерживающие аллокатор](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer)
  - [oбладать двунаправленным итератом](https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator)


Помимое этого обладает следующими методами 

| Метод     |  Алгоримическая сложность        | Гарантии исключений |
| --------  | -------                          | -------             |
| insert    |  O(1) для 1 элемента, O(M) для M |  strong             |
| erase     |  O(1) для 1 элемента, O(M) для M |  noexcept           |
| clear     |  O(N)                            |  noexcept           |
| push_back |  O(1)                            |  strong             |
| pop_back  |  O(1)                            |  noexcept           |
| push_front|  O(1)                            |  strong             |
| pop_front |  O(1)                            |  noexcept           |
