#pragma once
#include <memory>

template <typename T, size_t NodeMaxSize = 10,
          typename Allocator = std::allocator<T>>
class unrolled_list {
   public:
    using value_type = T;
    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;

   private:
    struct Node {
        T* data;
        size_type count;
        Node* next;
        Node* prev;
        allocator_type& allocator;
        explicit Node(allocator_type& allocator)
            : count(0), allocator(allocator), next(nullptr), prev(nullptr) {
            data = std::allocator_traits<Allocator>::allocate(allocator,
                                                              NodeMaxSize);
        }
        ~Node() {
            for (size_t i = 0; i < NodeMaxSize; ++i) {
                std::allocator_traits<Allocator>::destroy(allocator, data + i);
            }
            std::allocator_traits<Allocator>::deallocate(allocator, data,
                                                         NodeMaxSize);
        }
    };

   public:
    // Итератор
    class iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;

        iterator(Node* node, size_type index)
            : current_node(node), index_in_node(index) {}
        reference operator*() const { return current_node[index_in_node]; }
        pointer operator->() const { return &current_node[index_in_node]; }
        iterator& operator++() {
            if (index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                index_in_node++;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator it = this;
            if (index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                index_in_node++;
            }
            return it;
        }
        iterator& operator--() {
            if (index_in_node == 0) {
                current_node = current_node->prev;
                index_in_node = current_node->next->count - 1;
            } else {
                index_in_node++;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator it = this;
            if (index_in_node == 0) {
                current_node = current_node->prev;
                index_in_node = current_node->next->count - 1;
            } else {
                index_in_node++;
            }
            return it;
        }
        bool operator==(const iterator& other) const {
            return this->current_node == other->current_node &&
                   this->index_in_node == other->index_in_node;
        }
        bool operator!=(const iterator& other) const {
            return !(this == other);
        }

       private:
        Node* current_node;
        size_type index_in_node;
    };

    // Константный итератор
    class const_iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;

        const_iterator(const Node* node, size_type index)
            : current_node(node), index_in_node(index) {}
        reference operator*() const { return current_node[index_in_node]; }
        pointer operator->() const { return &current_node[index_in_node]; }
        const_iterator& operator++() {
            if (index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                index_in_node++;
            }
            return *this;
        }
        const_iterator operator++(int) {
            iterator it = *this;
            if (index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                index_in_node++;
            }
            return it;
        }
        const_iterator& operator--() {
            if (index_in_node == 0) {
                current_node = current_node->prev;
                index_in_node = current_node->next->count - 1;
            } else {
                index_in_node++;
            }
            return *this;
        }
        const_iterator operator--(int) {
            iterator it = *this;
            if (index_in_node == 0) {
                current_node = current_node->prev;
                index_in_node = current_node->next->count - 1;
            } else {
                index_in_node++;
            }
            return it;
        }
        bool operator==(const const_iterator& other) const {
            return this->current_node == other->current_node &&
                   this->index_in_node == other->index_in_node;
        }
        bool operator!=(const const_iterator& other) const {
            return !(this == other);
        }

       private:
        const Node* current_node;
        size_type index_in_node;
    };

    // Конструкторы, деструктор и операторы присваивания
    unrolled_list() = default;
    unrolled_list(const unrolled_list& other)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(
              allocator_traits<Allocator>::
                  select_on_container_copy_construction(other.allocator)) {
        for (Node* cur = other.head; cur != nullptr; cur = cur->next) {
            Node* new_node = new Node(allocator);
            for (size_type i = 0; i < cur->count; ++i) {
                allocator_traits<Allocator>::construct(
                    allocator, &new_node->data[i], cur->data[i]);
            }
            new_node->count = cur->count;
            if (tail) {
                tail->next = new_node;
                new_node->prev = tail;
                tail = new_node;
            } else {
                head = tail = new_node;
            }
            total_size += new_node->count;
        }
    }
    unrolled_list(unrolled_list&& other) noexcept
        : head(other.head),
          tail(other.tail),
          total_size(other.total_size),
          allocator(std::move(other.allocator)) {
        other.head = nullptr;
        other.tail = nullptr;
        other.total_size = 0;
    }
    ~unrolled_list() { clear(); }
    unrolled_list& operator=(const unrolled_list& other) {
        if (this != &other) {
            unrolled_list tmp(other);
            swap(tmp);
        }
        return *this;
    }
    unrolled_list& operator=(unrolled_list&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            total_size = other.total_size;
            other.head = nullptr;
            other.tail = nullptr;
            other.total_size = 0;
        }
        return *this;
    }

    // Элементы доступа
    reference operator[](size_type index) {
        size_type cnt = 0;
        Node* node = head;
        while (node && node->count + cnt <= index) {
            cnt += node->count;
            node = node->next;
        }
        return node->data[index - cnt];
    }
    const_reference operator[](size_type index) const {
        size_type cnt = 0;
        Node* node = head;
        while (node && node->count + cnt <= index) {
            cnt += node->count;
            node = node->next;
        }
        return node->data[index - cnt];
    }
    reference front() { return head->data[0]; }
    const_reference cfront() const { return head->data[0]; }
    reference back() { return tail->data[tail->count - 1]; }
    const_reference cback() const { return tail->data[tail->count - 1]; }

    // Итераторы
    iterator begin() {
        iterator it(head, 0);
        return it;
    }
    const_iterator cbegin() const {
        const_iterator it(head, 0);
        return it;
    }
    iterator end() {
        iterator it(tail, total_size);
        return it;
    }
    const_iterator cend() const {
        const_iterator it(tail, total_size);
        return it;
    }
    std::reverse_iterator<iterator> rbegin() {
        return std::reverse_iterator<iterator>(end());
    }
    std::reverse_iterator<iterator> rend() {
        return std::reverse_iterator<iterator>(begin());
    }
    std::reverse_iterator<const_iterator> crbegin() const {
        return std::reverse_iterator<const_iterator>(cend());
    }
    std::reverse_iterator<const_iterator> crend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }

    // Методы для размера
    bool empty() const { return total_size == 0; }
    size_type size() const { return total_size; }

    // Модификаторы
    void clear() noexcept {
        Node* newNode = head;
        while (newNode) {
            Node* next = newNode->next;
            delete newNode;
            newNode = next;
        }
        head = tail = nullptr;
        total_size = 0;
    }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() noexcept;

    void push_front(const T& value) { emplace(begin(), value); }
    void push_front(T&& value) { emplace(begin(), std::move(value)); }
    void pop_front() noexcept;

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }
    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        if (pos == end()) {
            return emplace_into_node_(tail, tail->count, args);
        } else {
            Node* node = pos->current_node;
            size_type idx = pos->index_in_node;
            return emplace_into_node_(node, idx, args);
        }
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        emplace(end(), args);
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        emplace(begin(), args);
    }

    template <typename InputIt>
    void assign_range(InputIt first, InputIt last);

    template <typename InputIt>
    void prepend_range(InputIt first, InputIt last);

    iterator erase(const_iterator pos) noexcept;
    iterator erase(const_iterator first, const_iterator last) noexcept;

   private:
    Node* head = nullptr;
    Node* tail = nullptr;
    size_type total_size = 0;
    allocator_type allocator = Allocator();

    template <typename... Args>
    void emplace_into_node_(Node* node, size_type ptr, Args&&... args) {
        assert(node != nullptr);
        if (node->count == NodeMaxSize) {
            Node* newNode = new Node();
            size_type dataToMove = NodeMaxSize / 2;
            size_type startIndex = NodeMaxSize - dataToMove;
            for (size_type i = 0; i < dataToMove; i++) {
                newNode->data[i] = node->data[startIndex + i];
                node->data[startIndex + i] = null;
            }
            node->count -= dataToMove;
            newNode->count = dataToMove;
            newNode->next = node.next;
            newNode->prev = node;
            if (node->next) {
                node->next->prev = newNode;
            }
            node->next = newNode;

            if (node == tail) {
                tail = newNode;
            }
            if (ptr > node->count) {
                node = newNode;
                ptr -= node->count;
            }
        }
        for (size_type i = node->count; i > ptr; i--) {
            node->data[i] = node->data[i - 1];
        }
        allocator_traits::construct(allocator, &node->data[ptr],
                                    std::forward<Args>(args)...);
        ++node->count;
        ++total_size;
        return iterator(node, ptr);
    }

    void swap(unrolled_list& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(total_size, other.total_size);
        std::swap(allocator, other.allocator);
    }
};