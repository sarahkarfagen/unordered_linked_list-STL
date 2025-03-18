#pragma once
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
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
        size_type count;
        Node* next;
        Node* prev;
        allocator_type& allocator;
        alignas(T) std::byte data[sizeof(T) * NodeMaxSize];

        explicit Node(allocator_type& alloc)
            : count(0), next(nullptr), prev(nullptr), allocator(alloc) {}

        T* elem(size_type index) { return &reinterpret_cast<T*>(data)[index]; }
        const T* elem(size_type index) const {
            return &reinterpret_cast<const T*>(data)[index];
        }

        ~Node() {
            for (size_type i = 0; i < count; ++i) {
                allocator_traits::destroy(allocator, elem(i));
            }
        }
    };

   public:
    using node_allocator =
        typename allocator_traits::template rebind_alloc<Node>;
    using node_allocator_traits = std::allocator_traits<node_allocator>;

    class const_iterator;
    class iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;

        iterator(Node* node, size_type index, const unrolled_list* parent)
            : current_node(node), index_in_node(index), parent(parent) {}

        reference operator*() const {
            return *current_node->elem(index_in_node);
        }
        pointer operator->() const { return current_node->elem(index_in_node); }
        iterator& operator++() {
            if (!current_node) return *this;
            if (index_in_node < current_node->count - 1)
                ++index_in_node;
            else {
                current_node = current_node->next;
                index_in_node = 0;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }
        iterator& operator--() {
            if (!current_node) {
                current_node = parent->tail;
                if (current_node) index_in_node = current_node->count - 1;
            } else if (index_in_node > 0)
                --index_in_node;
            else {
                current_node = current_node->prev;
                if (current_node) index_in_node = current_node->count - 1;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp(*this);
            --(*this);
            return tmp;
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
        const unrolled_list* parent;
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

        const_iterator(Node* node, size_type index, const unrolled_list* parent)
            : current_node(node), index_in_node(index), parent(parent) {}
        const_iterator(const iterator& it)
            : current_node(it.current_node),
              index_in_node(it.index_in_node),
              parent(it.parent) {}

        reference operator*() const {
            return *current_node->elem(index_in_node);
        }
        pointer operator->() const { return current_node->elem(index_in_node); }
        const_iterator& operator++() {
            if (!current_node) return *this;
            if (index_in_node < current_node->count - 1)
                ++index_in_node;
            else {
                current_node = current_node->next;
                index_in_node = 0;
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp(*this);
            ++(*this);
            return tmp;
        }
        const_iterator& operator--() {
            if (!current_node) {
                current_node = parent->tail;
                if (current_node) index_in_node = current_node->count - 1;
            } else if (index_in_node > 0)
                --index_in_node;
            else {
                current_node = current_node->prev;
                if (current_node) index_in_node = current_node->count - 1;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp(*this);
            --(*this);
            return tmp;
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
        const unrolled_list* parent;
        friend class unrolled_list;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    unrolled_list()
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(Allocator()),
          node_alloc(allocator) {}
    unrolled_list(const allocator_type& alloc)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(alloc),
          node_alloc(allocator) {}
    template <typename InputIt>
    unrolled_list(InputIt first, InputIt last, const allocator_type& alloc)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(alloc),
          node_alloc(allocator) {
        try {
            for (; first != last; ++first) {
                push_back(*first);
            }
        } catch (...) {
            clear();
            throw;
        }
    }
    unrolled_list(size_type count, const T& value,
                  const Allocator& alloc = Allocator())
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(alloc),
          node_alloc(allocator) {
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
          node_alloc(allocator) {
        other.head = other.tail = nullptr;
        other.total_size = 0;
    }
    unrolled_list(const unrolled_list& other)
        : head(nullptr),
          tail(nullptr),
          total_size(0),
          allocator(allocator_traits::select_on_container_copy_construction(
              other.allocator)),
          node_alloc(allocator) {
        for (Node* cur = other.head; cur != nullptr; cur = cur->next) {
            Node* new_node = create_node();
            for (size_type i = 0; i < cur->count; ++i) {
                allocator_traits::construct(allocator, new_node->elem(i),
                                            *cur->elem(i));
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
          allocator(std::move(other.allocator)),
          node_alloc(allocator) {
        other.head = other.tail = nullptr;
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
            allocator = std::move(other.allocator);
            node_alloc = node_allocator(allocator);
            other.head = other.tail = nullptr;
            other.total_size = 0;
        }
        return *this;
    }
    bool operator==(const unrolled_list& other) const {
        if (this == &other) return true;
        if (size() != other.size()) return false;
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
        return *node->elem(index - cnt);
    }
    const_reference operator[](size_type index) const {
        size_type cnt = 0;
        Node* node = head;
        while (node && cnt + node->count <= index) {
            cnt += node->count;
            node = node->next;
        }
        return *node->elem(index - cnt);
    }
    reference front() {
        if (!head) throw std::out_of_range("List is empty");
        return *head->elem(0);
    }
    const_reference front() const {
        if (!head) throw std::out_of_range("List is empty");
        return *head->elem(0);
    }
    reference back() {
        if (!tail) throw std::out_of_range("List is empty");
        return *tail->elem(tail->count - 1);
    }
    const_reference back() const {
        if (!tail) throw std::out_of_range("List is empty");
        return *tail->elem(tail->count - 1);
    }

    iterator begin() { return iterator(head, 0, this); }
    const_iterator begin() const { return cbegin(); }
    const_iterator cbegin() const { return const_iterator(head, 0, this); }
    iterator end() { return iterator(nullptr, 0, this); }
    const_iterator end() const { return cend(); }
    const_iterator cend() const { return const_iterator(nullptr, 0, this); }
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
    allocator_type get_allocator() const { return allocator; }

    void clear() noexcept {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            destroy_node(cur);
            cur = next;
        }
        head = tail = nullptr;
        total_size = 0;
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() { erase(--end()); }
    void push_front(const T& value) { emplace(begin(), value); }
    void push_front(T&& value) { emplace(begin(), std::move(value)); }
    void pop_front() { erase(begin()); }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }
    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }
    iterator insert(const_iterator pos, size_type count, const T& value) {
        iterator it = iterator(pos.current_node, pos.index_in_node, this);
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
        if (!head) {
            head = create_node();
            tail = head;
        }
        if (pos.current_node == nullptr)
            return emplace_into_node_(tail, tail->count,
                                      std::forward<Args>(args)...);
        else {
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
    void assign_range(InputIt first, InputIt last) {
        clear();
        for (; first != last; ++first) push_back(*first);
    }
    template <typename InputIt>
    void prepend_range(InputIt first, InputIt last) {
        for (; first != last; ++first) push_front(*first);
    }

    iterator erase(const_iterator pos) {
        const_iterator pos_end = pos;
        ++pos_end;
        return erase(pos, pos_end);
    }

    iterator remove_node(Node* node) {
        iterator ret(nullptr, 0, this);
        if (head == tail) {
            destroy_node(node);
            head = tail = nullptr;
            ret = iterator(nullptr, 0, this);
        } else if (node == head) {
            head = node->next;
            head->prev = nullptr;
            ret = iterator(head, 0, this);
            destroy_node(node);
        } else if (node == tail) {
            tail = node->prev;
            tail->next = nullptr;
            ret = iterator(nullptr, 0, this);
            destroy_node(node);
        } else {
            Node* next_node = node->next;
            node->prev->next = node->next;
            node->next->prev = node->prev;
            ret = iterator(next_node, 0, this);
            destroy_node(node);
        }
        return ret;
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first == last || total_size == 0) {
            return iterator(first.current_node, first.index_in_node, this);
        }

        Node* end_node = last.current_node;
        size_type end_index = last.index_in_node;
        if (!end_node) {
            end_node = tail;
            end_index = tail->count;
        }

        Node* start_node = first.current_node;
        size_type start_index = first.index_in_node;
        if (!start_node) {
            start_node = head;
            start_index = 0;
        }

        size_type erasedCount = 0;

        if (start_node == end_node) {
            for (size_type i = start_index; i < end_index; ++i) {
                allocator_traits::destroy(allocator, start_node->elem(i));
                ++erasedCount;
            }
            normalize_node(end_node, end_index, erasedCount);
            start_node->count -= erasedCount;
            total_size -= erasedCount;

            if (start_node->count == 0) {
                return remove_node(start_node);
            }

            return iterator(start_node, start_index, this);
        }

        for (size_type i = start_index; i < start_node->count; ++i) {
            allocator_traits::destroy(allocator, start_node->elem(i));
            ++erasedCount;
        }
        start_node->count = start_index;

        Node* cur = start_node->next;
        if (start_node->count == 0) {
            remove_node(start_node);
        }

        while (cur && cur != end_node) {
            for (size_type i = 0; i < cur->count; ++i) {
                allocator_traits::destroy(allocator, cur->elem(i));
                ++erasedCount;
            }
            Node* next = cur->next;
            remove_node(cur);
            cur = next;
        }

        for (size_type i = 0; i < end_index; ++i) {
            allocator_traits::destroy(allocator, end_node->elem(i));
            ++erasedCount;
        }
        normalize_node(end_node, end_index, end_index);
        end_node->count -= end_index;
        total_size -= erasedCount;

        if (end_node->count == 0) {
            return remove_node(end_node);
        }

        return iterator(end_node, 0, this);
    }

   private:
    Node* head = nullptr;
    Node* tail = nullptr;
    size_type total_size = 0;
    allocator_type allocator = Allocator();
    node_allocator node_alloc;

    Node* create_node() {
        Node* p = node_allocator_traits::allocate(node_alloc, 1);
        node_allocator_traits::construct(node_alloc, p, allocator);
        return p;
    }

    void destroy_node(Node* p) {
        node_allocator_traits::destroy(node_alloc, p);
        node_allocator_traits::deallocate(node_alloc, p, 1);
    }
    void normalize_node(Node* p, const size_type from, const size_type shift) {
        if (shift == 0) {
            return;
        }

        for (size_type i = from; i < p->count; ++i) {
            allocator_traits::construct(allocator, p->elem(i - shift),
                                        std::move(*p->elem(i)));
            allocator_traits::destroy(allocator, p->elem(i));
        }
    }

    template <typename... Args>
    iterator emplace_into_node_(Node* node, size_type ptr, Args&&... args) {
        assert(node != nullptr);
        if (node->count == NodeMaxSize) {
            Node* newNode = create_node();
            size_type dataToMove = NodeMaxSize / 2;
            size_type startIndex = NodeMaxSize - dataToMove;
            for (size_type i = 0; i < dataToMove; i++) {
                allocator_traits::construct(
                    allocator, newNode->elem(i),
                    std::move(*node->elem(startIndex + i)));
            }
            node->count = startIndex;

            newNode->count = dataToMove;
            newNode->next = node->next;
            newNode->prev = node;
            node->next = newNode;
            if (newNode->next) newNode->next->prev = newNode;
            if (node == tail) tail = newNode;
            if (ptr > node->count) {
                ptr -= node->count;
                node = newNode;
            }
        }

        for (size_type i = node->count; i > ptr; i--) {
            if (i == node->count) {
                allocator_traits::construct(allocator, node->elem(i),
                                            std::move(*node->elem(i - 1)));
                allocator_traits::destroy(allocator, node->elem(i - 1));
            } else {
                T temp = std::move(*node->elem(i - 1));
                allocator_traits::destroy(allocator, node->elem(i - 1));
                allocator_traits::construct(allocator, node->elem(i),
                                            std::move(temp));
            }
        }
        allocator_traits::construct(allocator, node->elem(ptr),
                                    std::forward<Args>(args)...);
        ++node->count;
        ++total_size;
        return iterator(node, ptr, this);
    }

    void swap(unrolled_list& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(total_size, other.total_size);
        std::swap(allocator, other.allocator);
        std::swap(node_alloc, other.node_alloc);
    }
};
