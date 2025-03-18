#pragma once
#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

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
    using difference_type = std::ptrdiff_t;

   private:
    struct Node {
        T* data;
        size_type count;
        Node* next;
        Node* prev;
        allocator_type& allocator;
        explicit Node(allocator_type& alloc)
            : count(0), allocator(alloc), next(nullptr), prev(nullptr) {
            data = allocator_traits::allocate(allocator, NodeMaxSize);
        }
        ~Node() {
            for (size_t i = 0; i < count; ++i) {
                allocator_traits::destroy(allocator, data + i);
            }
            allocator_traits::deallocate(allocator, data, NodeMaxSize);
        }
    };

   public:
    class const_iterator;

    class iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;

        iterator(Node* node, size_type index)
            : current_node(node), index_in_node(index) {}
        reference operator*() const {
            return current_node->data[index_in_node];
        }
        pointer operator->() const {
            return &current_node->data[index_in_node];
        }
        iterator& operator++() {
            if (current_node && current_node->next &&
                index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                ++index_in_node;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator it(*this);
            if (current_node && current_node->next &&
                index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                ++index_in_node;
            }
            return it;
        }
        iterator& operator--() {
            if (current_node) {
                if (current_node->prev) {
                    if (current_node->next == nullptr) {
                        current_node = current_node->prev;
                        index_in_node = current_node->count - 1;
                    } else if (index_in_node == 0) {
                        current_node = current_node->prev;
                        index_in_node = current_node->count - 1;
                    } else {
                        --index_in_node;
                    }
                }
            }
            return *this;
        }
        iterator operator--(int) {
            iterator it(*this);
            if (current_node && current_node->prev) {
                if (current_node->next == nullptr || index_in_node == 0) {
                    current_node = current_node->prev;
                    index_in_node = current_node->count - 1;
                } else {
                    --index_in_node;
                }
            }
            return it;
        }
        bool operator==(const iterator& other) const {
            return current_node == other.current_node &&
                   index_in_node == other.index_in_node;
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

       private:
        Node* current_node;
        size_type index_in_node;

        friend class const_iterator;
        friend class unrolled_list;
    };

    class const_iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;

        const_iterator(const Node* node, size_type index)
            : current_node(const_cast<Node*>(node)), index_in_node(index) {}
        const_iterator(const iterator& it)
            : current_node(it.current_node), index_in_node(it.index_in_node) {}
        reference operator*() const {
            return current_node->data[index_in_node];
        }
        pointer operator->() const {
            return &(current_node->data[index_in_node]);
        }
        const_iterator& operator++() {
            if (current_node && current_node->next &&
                index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                ++index_in_node;
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator it(*this);
            if (current_node && current_node->next &&
                index_in_node == current_node->count - 1) {
                current_node = current_node->next;
                index_in_node = 0;
            } else {
                ++index_in_node;
            }
            return it;
        }
        const_iterator& operator--() {
            if (current_node && current_node->prev) {
                if (current_node->next == nullptr || index_in_node == 0) {
                    current_node = current_node->prev;
                    index_in_node = current_node->count - 1;
                } else {
                    --index_in_node;
                }
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator it(*this);
            if (current_node && current_node->prev) {
                if (current_node->next == nullptr || index_in_node == 0) {
                    current_node = current_node->prev;
                    index_in_node = current_node->count - 1;
                } else {
                    --index_in_node;
                }
            }
            return it;
        }
        bool operator==(const const_iterator& other) const {
            return current_node == other.current_node &&
                   index_in_node == other.index_in_node;
        }
        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

       private:
        Node* current_node;
        size_type index_in_node;

        friend class unrolled_list;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    unrolled_list()
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(Allocator()),
          dummy(new Node(allocator)) {}
    unrolled_list(const allocator_type& allocator_)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(allocator_),
          dummy(new Node(allocator)) {}
    template <typename InputIt>
    unrolled_list(InputIt first, InputIt last, const allocator_type& alloc)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(alloc),
          dummy(new Node(allocator)) {
        for (; first != last; ++first) {
            push_back(*first);
        }
    }
    unrolled_list(size_type count, const T& value,
                  const Allocator& alloc = Allocator())
        : head(nullptr), tail(nullptr), total_size(0), allocator(alloc) {
        dummy = new Node(allocator);
        for (size_type i = 0; i < count; i++) push_back(value);
    }
    unrolled_list(std::initializer_list<T> ilist,
                  const Allocator& alloc = Allocator())
        : unrolled_list(ilist.begin(), ilist.end(), alloc) {}

    unrolled_list(unrolled_list&& other, const Allocator& alloc)
        : head(other.head),
          tail(other.tail),
          total_size(other.total_size),
          allocator(alloc),
          dummy(other.dummy) {
        other.head = other.tail = nullptr;
        other.total_size = 0;
        other.dummy = new Node(other.allocator);
    }
    unrolled_list(const unrolled_list& other)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(allocator_traits::select_on_container_copy_construction(
              other.allocator)),
          dummy(new Node(allocator)) {
        for (Node* cur = other.head; cur != nullptr; cur = cur->next) {
            Node* new_node = new Node(allocator);
            for (size_type i = 0; i < cur->count; ++i) {
                allocator_traits::construct(allocator, &new_node->data[i],
                                            cur->data[i]);
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
        if (tail) {
            tail->next = dummy;
            dummy->prev = tail;
        }
    }
    unrolled_list(unrolled_list&& other) noexcept
        : head(other.head),
          tail(other.tail),
          total_size(other.total_size),
          allocator(std::move(other.allocator)),
          dummy(other.dummy) {
        other.head = nullptr;
        other.tail = nullptr;
        other.total_size = 0;
        other.dummy = nullptr;
    }
    ~unrolled_list() {
        clear();
        delete dummy;
    }
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
            allocator = std::move(other.allocator);
            delete dummy;
            dummy = other.dummy;
            other.head = nullptr;
            other.tail = nullptr;
            other.total_size = 0;
            other.dummy = nullptr;
        }
        return *this;
    }
    bool operator==(const unrolled_list& other) const {
        if (this == &other) {
            return true;
        }
        if (size() != other.size()) {
            return false;
        }
        return std::equal(begin(), end(), other.begin());
    }
    bool operator!=(const unrolled_list& other) const {
        return !(*this == other);
    }

    reference operator[](size_type index) {
        size_type cnt = 0;
        Node* node = head;
        while (node && cnt + node->count <= index) {
            cnt += node->count;
            node = node->next;
        }
        return node->data[index - cnt];
    }
    const_reference operator[](size_type index) const {
        size_type cnt = 0;
        Node* node = head;
        while (node && cnt + node->count <= index) {
            cnt += node->count;
            node = node->next;
        }
        return node->data[index - cnt];
    }
    reference front() { return head->data[0]; }
    const_reference front() const { return head->data[0]; }
    reference back() { return tail->data[tail->count - 1]; }
    const_reference back() const { return tail->data[tail->count - 1]; }

    iterator begin() { return head ? iterator(head, 0) : end(); }
    const_iterator begin() const { return cbegin(); }
    const_iterator cbegin() const {
        return head ? const_iterator(head, 0) : cend();
    }
    iterator end() { return iterator(dummy, 0); }
    const_iterator end() const { return cend(); }
    const_iterator cend() const { return const_iterator(dummy, 0); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crbegin() const { return rbegin(); }
    const_reverse_iterator crend() const { return rend(); }

    bool empty() const { return total_size == 0; }
    size_type size() const { return total_size; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    allocator_type get_allocator() { return allocator; }

    void clear() noexcept {
        Node* cur = head;
        while (cur && cur != dummy) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        total_size = 0;
        if (dummy) {
            dummy->prev = nullptr;
        }
    }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() noexcept { erase(--end()); }

    void push_front(const T& value) { emplace(begin(), value); }
    void push_front(T&& value) { emplace(begin(), std::move(value)); }
    void pop_front() noexcept { erase(begin()); }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }
    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }
    iterator insert(const_iterator pos, size_type count, const T& value) {
        iterator it = iterator(pos.current_node, pos.index_in_node);
        iterator firstInserted = it;
        for (size_type i = 0; i < count; i++) {
            it = insert(it, value);
            if (i == 0) firstInserted = it;
            ++it;
        }
        return firstInserted;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        if (head == nullptr) {
            head = new Node(allocator);
            tail = head;
            head->next = dummy;
            dummy->prev = head;
        }
        if (pos == cend()) {
            return emplace_into_node_(tail, tail->count,
                                      std::forward<Args>(args)...);
        } else {
            Node* node = pos.current_node;
            size_type idx = pos.index_in_node;
            return emplace_into_node_(node, idx, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        emplace(cend(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        emplace(cbegin(), std::forward<Args>(args)...);
    }

    template <typename InputIt>
    void assign_range(InputIt first, InputIt last);
    template <typename InputIt>
    void prepend_range(InputIt first, InputIt last);

    iterator erase(const_iterator pos) noexcept {
        const_iterator pos_end = pos;
        ++pos_end;
        return erase(pos, pos_end);
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        if (first == last)
            return iterator(first.current_node, first.index_in_node);

        Node* start_node = first.current_node;
        size_type start_index = first.index_in_node;
        Node* end_node = last.current_node;
        size_type end_index = last.index_in_node;

        size_type erasedCount = 0;

        if (start_node == end_node) {
            size_type countToErase = end_index - start_index;
            for (size_type i = start_index; i < end_index; ++i) {
                allocator_traits::destroy(allocator, &start_node->data[i]);
            }
            for (size_type i = end_index; i < start_node->count; ++i) {
                start_node->data[i - countToErase] =
                    std::move(start_node->data[i]);
            }
            start_node->count -= countToErase;
            total_size -= countToErase;

            if (start_node->count == 0) {
                if (head == tail) {
                    head = tail = nullptr;
                    dummy->prev = nullptr;
                    return iterator(dummy, 0);
                } else {
                    if (start_node == head) {
                        head = start_node->next;
                        head->prev = nullptr;
                    } else if (start_node == tail) {
                        tail = start_node->prev;
                        tail->next = dummy;
                        dummy->prev = tail;
                    } else {
                        start_node->prev->next = start_node->next;
                        start_node->next->prev = start_node->prev;
                    }
                    delete start_node;
                    return iterator(dummy, 0);
                }
            }
            return iterator(start_node, start_index);
        }

        for (size_type i = start_index; i < start_node->count; ++i) {
            allocator_traits::destroy(allocator, &start_node->data[i]);
            ++erasedCount;
        }
        start_node->count = start_index;

        Node* cur = start_node->next;
        while (cur != end_node) {
            for (size_type i = 0; i < cur->count; ++i) {
                allocator_traits::destroy(allocator, &cur->data[i]);
                ++erasedCount;
            }
            Node* next = cur->next;

            if (cur->prev) cur->prev->next = cur->next;
            if (cur->next) cur->next->prev = cur->prev;
            if (cur == tail) tail = cur->prev;
            delete cur;
            cur = next;
        }

        for (size_type i = 0; i < end_index; ++i) {
            allocator_traits::destroy(allocator, &end_node->data[i]);
            ++erasedCount;
        }
        size_type remaining = end_node->count - end_index;
        for (size_type i = end_index; i < end_node->count; ++i) {
            end_node->data[i - end_index] = std::move(end_node->data[i]);
        }
        end_node->count = remaining;
        total_size -= erasedCount;

        if (end_node->count == 0 && head != end_node) {
            if (end_node == tail) {
                tail = end_node->prev;
                tail->next = dummy;
                dummy->prev = tail;
            } else {
                end_node->prev->next = end_node->next;
                end_node->next->prev = end_node->prev;
            }
            delete end_node;
            return iterator(dummy, 0);
        }
        return iterator(end_node, 0);
    }

   private:
    Node* head = nullptr;
    Node* tail = nullptr;
    size_type total_size = 0;
    allocator_type allocator = Allocator();
    Node* dummy = nullptr;

    template <typename... Args>
    iterator emplace_into_node_(Node* node, size_type ptr, Args&&... args) {
        assert(node != nullptr);
        if (node->count == NodeMaxSize) {
            Node* newNode = new Node(allocator);
            size_type dataToMove = NodeMaxSize / 2;
            size_type startIndex = NodeMaxSize - dataToMove;
            for (size_type i = 0; i < dataToMove; i++) {
                allocator_traits::construct(
                    allocator, &newNode->data[i],
                    std::move(node->data[startIndex + i]));
                allocator_traits::destroy(allocator,
                                          &node->data[startIndex + i]);
            }
            node->count -= dataToMove;
            newNode->count = dataToMove;
            newNode->next = node->next;
            newNode->prev = node;
            node->next = newNode;
            if (newNode->next) {
                newNode->next->prev = newNode;
            }
            if (node == tail) {
                tail = newNode;
                tail->next = dummy;
                dummy->prev = tail;
            }
            if (ptr > node->count) {
                ptr -= node->count;
                node = newNode;
            }
        }
        for (size_type i = node->count; i > ptr; i--) {
            if (i == node->count) {
                allocator_traits::construct(allocator, &node->data[i],
                                            std::move(node->data[i - 1]));
                allocator_traits::destroy(allocator, &node->data[i - 1]);
            } else {
                T temp = std::move(node->data[i - 1]);
                allocator_traits::destroy(allocator, &node->data[i - 1]);
                allocator_traits::construct(allocator, &node->data[i],
                                            std::move(temp));
            }
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
        std::swap(dummy, other.dummy);
    }
};
