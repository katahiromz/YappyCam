// ring.hpp --- katahiromz's ring buffer
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT

#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_    8   // Version 8

#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include <cstring>
#include <cassert>

namespace katahiromz {

////////////////////////////////////////////////////////////////////////////
// katahiromz::ring<T_VALUE, t_size>

template <typename T_VALUE, std::size_t t_size>
class ring
{
public:
    typedef ring<T_VALUE, t_size> self_type;
    typedef T_VALUE value_type;
    typedef       T_VALUE& reference;
    typedef const T_VALUE& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    ring()
        : m_front_index(0)
        , m_back_index(0)
        , m_full(false)
    {
        assert(empty());
    }

    ring(const self_type& other)
        : m_front_index(other.m_front_index)
        , m_back_index(other.m_back_index)
        , m_full(other.m_full)
    {
        for (size_type i = 0; i < t_size; ++i)
        {
            m_data[i] = other.m_data[i];
        }
        assert(empty() == other.empty());
        assert(size() == other.size());
    }

    ring(self_type&& other)
        : m_front_index(other.m_front_index)
        , m_back_index(other.m_back_index)
        , m_full(other.m_full)
    {
        for (size_type i = 0; i < t_size; ++i)
        {
            m_data[i] = std::move(other.m_data[i]);
        }
        assert(empty() == other.empty());
        assert(size() == other.size());
    }

    bool empty() const
    {
        return !m_full && m_front_index == m_back_index;
    }
    bool full() const
    {
        return m_full;
    }

    size_type size() const
    {
        if (m_full)
            return t_size;

        if (m_front_index >= m_back_index)
        {
            return m_front_index - m_back_index;
        }
        else
        {
            return m_front_index + t_size - m_back_index;
        }
    }
    size_type capacity() const
    {
        return t_size;
    }

    value_type& operator[](size_type index)
    {
        return m_data[index];
    }
    const value_type& operator[](size_type index) const
    {
        return m_data[index];
    }
    value_type& at(size_type index)
    {
        assert(valid_index(index));
        if (!valid_index(index))
            throw std::out_of_range("katahiromz::ring");
        return m_data[index];
    }
    const value_type& at(size_type index) const
    {
        assert(valid_index(index));
        if (!valid_index(index))
            throw std::out_of_range("katahiromz::ring");
        return m_data[index];
    }

    void reset()
    {
        m_front_index = m_back_index = 0;
        m_full = false;
    }
    void clear()
    {
        reset();

        for (size_type i = 0; i < t_size; ++i)
        {
            m_data[i] = value_type();
        }
    }

    size_type front_index() const
    {
        return m_front_index;
    }
    value_type& front()
    {
        return (*this)[m_front_index];
    }
    const value_type& front() const
    {
        return (*this)[m_front_index];
    }

    size_type back_index() const
    {
        return m_back_index;
    }
    value_type& back()
    {
        return (*this)[m_back_index];
    }
    const value_type& back() const
    {
        return (*this)[m_back_index];
    }

    bool valid_index(size_type index) const
    {
        if (index >= t_size)
            return false;

        if (m_full)
            return true;

        if (m_back_index <= m_front_index)
        {
            return m_back_index <= index && index < m_front_index;
        }
        else
        {
            return index < m_front_index || m_back_index <= index;
        }
    }

    static size_type next_index(size_type index)
    {
        ++index;
        index %= t_size;
        return index;
    }
    static size_type prev_index(size_type index)
    {
        if (index == 0)
        {
            index = t_size;
        }
        return --index;
    }

    void push_front()
    {
        m_front_index = next_index(m_front_index);
        m_full = (m_front_index == m_back_index);
    }
    void push_front(const value_type& item)
    {
        front() = item;
        push_front();
    }
    void push_front(value_type&& item)
    {
        front() = std::move(item);
        push_front();
    }
    template <class... Args>
    void emplace_front(Args&&... args)
    {
        std::allocator<value_type> alloc;
        using traits = std::allocator_traits<decltype(alloc)>;
        traits::destroy(alloc, &front());
        traits::construct(alloc, &front(), std::forward<Args>(args)...);
        push_front();
    }
    void pop_front()
    {
        m_front_index = prev_index(m_front_index);
        m_full = false;
    }

    void push_back()
    {
        m_back_index = prev_index(m_back_index);
        m_full = (m_front_index == m_back_index);
    }
    void push_back(const value_type& item)
    {
        back() = item;
        push_back();
    }
    void push_back(value_type&& item)
    {
        back() = std::move(item);
        push_back();
    }
    template <class... Args>
    void emplace_back(Args&&... args)
    {
        std::allocator<value_type> alloc;
        using traits = std::allocator_traits<decltype(alloc)>;
        traits::destroy(&back());
        traits::construct(alloc, &back(), std::forward<Args>(args)...);
        push_back();
    }
    void pop_back()
    {
        m_back_index = next_index(m_back_index);
        m_full = false;
    }

    void push()
    {
        push_front();
    }
    void push(const value_type& item)
    {
        push_front(item);
    }
    void push(value_type&& item)
    {
        push_front(std::move(item));
    }
    template <class... Args>
    void emplace(Args&&... args)
    {
        emplace_front(std::forward<Args>(args)...);
    }
    void pop()
    {
        pop_back();
    }

    void peek_front(T_VALUE *values, size_type count) const
    {
        if (count > size())
            count = size();

        size_type i = 0, k = prev_index(m_front_index);
        while (count-- > 0)
        {
            values[i++] = m_data[k];
            k = prev_index(k);
        }
    }
    void peek_back(T_VALUE *values, size_type count) const
    {
        if (count > size())
            count = size();

        size_type i = 0, k = m_back_index;
        while (count-- > 0)
        {
            values[i++] = m_data[k];
            k = next_index(k);
        }
    }

    void skip_front(size_type count)
    {
        assert(count <= t_size);
        m_front_index += t_size - count;
        m_front_index %= t_size;
        if (count > 0)
            m_full = false;
    }
    void skip_back(size_type count)
    {
        assert(count <= t_size);
        m_back_index += count;
        m_back_index %= t_size;
        if (count > 0)
            m_full = false;
    }

    struct iterator
    {
        self_type& m_self;
        size_type m_index;
        size_type m_count;

        explicit iterator(self_type& self)
            : m_self(self)
            , m_index(self.front_index())
            , m_count(self.size())
        {
        }
        iterator& operator=(const iterator& other)
        {
            assert(&m_self == &other.m_self);
            m_index = other.m_index;
            m_count = other.m_count;
            return *this;
        }

        iterator(self_type& self, size_type index, size_type count)
            : m_self(self)
            , m_index(index)
            , m_count(count)
        {
        }

        reference operator*()
        {
            return m_self[prev_index(m_index)];
        }
        const_reference operator*() const
        {
            return m_self[prev_index(m_index)];
        }

        const iterator& operator++()
        {
            if (m_count > 0)
            {
                m_index = m_self.prev_index(m_index);
                --m_count;
            }
            return *this;
        }
        iterator operator++(int)
        {
            iterator it = *this;
            ++*this;
            return it;
        }
        const iterator& operator--()
        {
            if (m_count < m_self.size())
            {
                m_index = next_index(m_index);
                ++m_count;
            }
            return *this;
        }
        iterator operator--(int)
        {
            iterator it = *this;
            --*this;
            return it;
        }

        bool operator==(const iterator& it) const
        {
            return &m_self == &it.m_self && m_index == it.m_index && m_count == it.m_count;
        }
        bool operator!=(const iterator& it) const
        {
            return !(*this == it);
        }
    };

    struct reverse_iterator
    {
        self_type& m_self;
        size_type m_index;
        size_type m_count;

        explicit reverse_iterator(self_type& self)
            : m_self(self)
            , m_index(self.back_index())
            , m_count(self.size())
        {
        }
        reverse_iterator& operator=(const reverse_iterator& other)
        {
            assert(&m_self == &other.m_self);
            m_index = other.m_index;
            m_count = other.m_count;
            return *this;
        }

        reverse_iterator(self_type& self, size_type index, size_type count)
            : m_self(self)
            , m_index(index)
            , m_count(count)
        {
        }

        reference operator*()
        {
            return m_self[m_index];
        }
        const_reference operator*() const
        {
            return m_self[m_index];
        }

        const reverse_iterator& operator++()
        {
            if (m_count > 0)
            {
                m_index = next_index(m_index);
                --m_count;
            }
            return *this;
        }
        reverse_iterator operator++(int)
        {
            reverse_iterator it = *this;
            ++*this;
            return it;
        }
        const reverse_iterator& operator--()
        {
            if (m_count < m_self.size())
            {
                m_index = m_self.prev_index(m_index);
                ++m_count;
            }
            return *this;
        }
        reverse_iterator operator--(int)
        {
            reverse_iterator it = *this;
            --*this;
            return it;
        }

        bool operator==(const reverse_iterator& it) const
        {
            return &m_self == &it.m_self && m_index == it.m_index && m_count == it.m_count;
        }
        bool operator!=(const reverse_iterator& it) const
        {
            return !(*this == it);
        }
    };

    struct const_iterator
    {
        const self_type& m_self;
        size_type m_index;
        size_type m_count;

        explicit const_iterator(const iterator& it)
            : m_self(it.m_self)
            , m_index(it.m_self.front_index())
            , m_count(it.m_self.size())
        {
        }
        const_iterator& operator=(const const_iterator& other)
        {
            assert(&m_self == &other.m_self);
            m_index = other.m_index;
            m_count = other.m_count;
            return *this;
        }

        explicit const_iterator(const self_type& self)
            : m_self(self)
            , m_index(self.front_index())
            , m_count(self.size())
        {
        }

        const_iterator(const self_type& self, size_type index, size_type count)
            : m_self(self)
            , m_index(index)
            , m_count(count)
        {
        }

        const_reference operator*() const
        {
            return m_self[prev_index(m_index)];
        }

        const const_iterator& operator++()
        {
            if (m_count > 0)
            {
                m_index = m_self.prev_index(m_index);
                --m_count;
            }
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator it = *this;
            ++*this;
            return it;
        }
        const const_iterator& operator--()
        {
            if (m_count < m_self.size())
            {
                m_index = next_index(m_index);
                ++m_count;
            }
            return *this;
        }
        const_iterator operator--(int)
        {
            const_iterator it = *this;
            --*this;
            return it;
        }

        bool operator==(const const_iterator& it) const
        {
            return &m_self == &it.m_self && m_index == it.m_index && m_count == it.m_count;
        }
        bool operator!=(const const_iterator& it) const
        {
            return !(*this == it);
        }
    };

    struct const_reverse_iterator
    {
        const self_type& m_self;
        size_type m_index;
        size_type m_count;

        explicit const_reverse_iterator(const self_type& self)
            : m_self(self)
            , m_index(self.back_index())
            , m_count(self.size())
        {
        }
        const_reverse_iterator& operator=(const const_reverse_iterator& other)
        {
            assert(&m_self == &other.m_self);
            m_index = other.m_index;
            m_count = other.m_count;
            return *this;
        }

        explicit const_reverse_iterator(const reverse_iterator& it)
            : m_self(it.m_self)
            , m_index(it.m_self.front_index())
            , m_count(it.m_self.size())
        {
        }

        const_reverse_iterator(const self_type& self, size_type index, size_type count)
            : m_self(self)
            , m_index(index)
            , m_count(count)
        {
        }

        const_reference operator*() const
        {
            return m_self[m_index];
        }

        const const_reverse_iterator& operator++()
        {
            if (m_count > 0)
            {
                m_index = next_index(m_index);
                --m_count;
            }
            return *this;
        }
        const_reverse_iterator operator++(int)
        {
            const_reverse_iterator it = *this;
            ++*this;
            return it;
        }
        const const_reverse_iterator& operator--()
        {
            if (m_count < m_self.size())
            {
                m_index = m_self.prev_index(m_index);
                --m_count;
            }
            return *this;
        }
        const_reverse_iterator operator--(int)
        {
            const_reverse_iterator it = *this;
            --*this;
            return it;
        }

        bool operator==(const const_reverse_iterator& it) const
        {
            return &m_self == &it.m_self && m_index == it.m_index && m_count == it.m_count;
        }
        bool operator!=(const const_reverse_iterator& it) const
        {
            return !(*this == it);
        }
    };

    static_assert(std::is_copy_constructible<iterator>::value);
    static_assert(std::is_copy_assignable<iterator>::value);
    static_assert(std::is_destructible<iterator>::value);

    static_assert(std::is_copy_constructible<reverse_iterator>::value);
    static_assert(std::is_copy_assignable<reverse_iterator>::value);
    static_assert(std::is_destructible<reverse_iterator>::value);

    static_assert(std::is_copy_constructible<const_iterator>::value);
    static_assert(std::is_copy_assignable<const_iterator>::value);
    static_assert(std::is_destructible<const_iterator>::value);

    static_assert(std::is_copy_constructible<const_reverse_iterator>::value);
    static_assert(std::is_copy_assignable<const_reverse_iterator>::value);
    static_assert(std::is_destructible<const_reverse_iterator>::value);

    iterator begin()
    {
        return iterator(*this);
    }
    const_iterator cbegin() const
    {
        return const_iterator(*this);
    }
    const_iterator begin() const
    {
        return cbegin();
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(*this);
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(*this);
    }
    const_iterator rbegin() const
    {
        return crbegin();
    }

    iterator end()
    {
        return iterator(*this, back_index(), 0);
    }
    const_iterator cend() const
    {
        return const_iterator(*this, back_index(), 0);
    }
    const_iterator end() const
    {
        return cend();
    }

    reverse_iterator rend()
    {
        return reverse_iterator(*this, front_index(), 0);
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(*this, front_index(), 0);
    }
    const_reverse_iterator rend() const
    {
        return crend();
    }

protected:
    value_type m_data[t_size];
    size_type m_front_index;
    size_type m_back_index;
    bool m_full;
};

////////////////////////////////////////////////////////////////////////////
// katahiromz::detail

namespace detail
{
    struct test_struct
    {
        int m_a;
        int m_b;
        static int g;
        static int f;
        static int h;

        test_struct(): m_a(0), m_b(0)
        {
            ++g;
        }

        test_struct(int a, int b) : m_a(a), m_b(b)
        {
            ++f;
            ++g;
        }
        ~test_struct()
        {
            ++h;
            m_a = 0;
            m_b = 0;
            --g;
        }
    };

    int test_struct::g = 0;
    int test_struct::f = 0;
    int test_struct::h = 0;
}

////////////////////////////////////////////////////////////////////////////
// katahiromz::ring_unittest

inline void ring_unittest()
{
    ring<int, 4> r;
    assert(r.empty());
    assert(r.size() == 0);
    assert(r.capacity() == 4);
    assert(r.front_index() == 0);
    assert(r.back_index() == 0);

    r.push(1);
    assert(!r.empty());
    assert(r.size() == 1);
    assert(r.front_index() == 1);
    assert(r.back_index() == 0);
    assert(r.back() == 1);
    assert(r[0] == 1);

    r.push(2);
    assert(!r.empty());
    assert(r.size() == 2);
    assert(r.front_index() == 2);
    assert(r.back_index() == 0);
    assert(r.back() == 1);
    assert(r[0] == 1);
    assert(r[1] == 2);

    r.push(3);
    assert(!r.empty());
    assert(r.size() == 3);
    assert(r.front_index() == 3);
    assert(r.back_index() == 0);
    assert(r.back() == 1);
    assert(r[0] == 1);
    assert(r[1] == 2);
    assert(r[2] == 3);

    r.pop();
    assert(!r.empty());
    assert(r.size() == 2);
    assert(r.front_index() == 3);
    assert(r.back_index() == 1);
    assert(r.back() == 2);
    assert(r[1] == 2);
    assert(r[2] == 3);

    {
        auto it = r.begin();
        assert(it.m_index == r.front_index());
        assert(it.m_count == r.size());
    }
    {
        auto it = r.end();
        assert(it.m_index == r.back_index());
        assert(it.m_count == 0);
    }

    {
        auto it = r.cbegin();
        assert(it.m_index == r.front_index());
        assert(it.m_count == r.size());
    }
    {
        auto it = r.cend();
        assert(it.m_index == r.back_index());
        assert(it.m_count == 0);
    }

    {
        auto it = r.rbegin();
        assert(it.m_index == r.back_index());
        assert(it.m_count == r.size());
    }
    {
        auto it = r.rend();
        assert(it.m_index == r.front_index());
        assert(it.m_count == 0);
    }

    {
        auto it = r.crbegin();
        assert(it.m_index == r.back_index());
        assert(it.m_count == r.size());
    }
    {
        auto it = r.crend();
        assert(it.m_index == r.front_index());
        assert(it.m_count == 0);
    }

    size_t i;

    i = 0;
    for (auto it = r.begin(); it != r.end(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 3);
            break;
        case 1:
            assert(*it == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    i = 0;
    for (auto it = r.cbegin(); it != r.cend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 3);
            break;
        case 1:
            assert(*it == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    i = 0;
    for (auto it = r.rbegin(); it != r.rend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 2);
            break;
        case 1:
            assert(*it == 3);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    i = 0;
    for (auto it = r.crbegin(); it != r.crend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 2);
            break;
        case 1:
            assert(*it == 3);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    i = 0;
    for (auto& item : r)
    {
        switch (i)
        {
        case 0:
            assert(item == 3);
            break;
        case 1:
            assert(item == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    i = 0;
    for (const auto& item : r)
    {
        switch (i)
        {
        case 0:
            assert(item == 3);
            break;
        case 1:
            assert(item == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 2);

    r.push(4);
    r.push(5);

    assert(r.front_index() == 1);
    assert(r.back_index() == 1);
    assert(r[0] == 5);
    assert(r[1] == 2);
    assert(r[2] == 3);
    assert(r[3] == 4);
    assert(r.full());

    int data[4] = { 0 };
    r.peek_back(data, 2);
    assert(data[0] == 2);
    assert(data[1] == 3);
    assert(data[2] == 0);
    assert(data[3] == 0);

    r.peek_back(data, 4);
    assert(data[0] == 2);
    assert(data[1] == 3);
    assert(data[2] == 4);
    assert(data[3] == 5);

    std::memset(data, 0, sizeof(data));
    r.peek_front(data, 2);
    assert(data[0] == 5);
    assert(data[1] == 4);
    assert(data[2] == 0);
    assert(data[3] == 0);

    r.peek_front(data, 4);
    assert(data[0] == 5);
    assert(data[1] == 4);
    assert(data[2] == 3);
    assert(data[3] == 2);

    i = 0;
    for (const auto& item : r)
    {
        switch (i)
        {
        case 0:
            assert(item == 5);
            break;
        case 1:
            assert(item == 4);
            break;
        case 2:
            assert(item == 3);
            break;
        case 3:
            assert(item == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 4);

    i = 0;
    for (auto it = r.begin(); it != r.end(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 5);
            break;
        case 1:
            assert(*it == 4);
            break;
        case 2:
            assert(*it == 3);
            break;
        case 3:
            assert(*it == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 4);

    i = 0;
    for (auto it = r.cbegin(); it != r.cend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 5);
            break;
        case 1:
            assert(*it == 4);
            break;
        case 2:
            assert(*it == 3);
            break;
        case 3:
            assert(*it == 2);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 4);

    i = 0;
    for (auto it = r.rbegin(); it != r.rend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 2);
            break;
        case 1:
            assert(*it == 3);
            break;
        case 2:
            assert(*it == 4);
            break;
        case 3:
            assert(*it == 5);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 4);

    i = 0;
    for (auto it = r.crbegin(); it != r.crend(); ++it)
    {
        switch (i)
        {
        case 0:
            assert(*it == 2);
            break;
        case 1:
            assert(*it == 3);
            break;
        case 2:
            assert(*it == 4);
            break;
        case 3:
            assert(*it == 5);
            break;
        default:
            assert(false);
            break;
        }
        ++i;
    }
    assert(i == 4);

    assert(r.full());
    assert(!r.empty());
    r.pop();
    assert(!r.full());
    assert(!r.empty());

    assert(r.front_index() == 1);
    assert(r.back_index() == 2);

    r.reset();
    assert(r.front_index() == 0);
    assert(r.back_index() == 0);
    assert(r.size() == 0);
    assert(r.empty());
    assert(!r.full());

    //
    // constructor and destructor tests
    //
    using test_struct = katahiromz::detail::test_struct;

    assert(test_struct::g == 0);
    ring<test_struct, 3> rA;
    assert(test_struct::g == 3);

    assert(rA[0].m_a == 0);
    assert(rA[0].m_b == 0);
    assert(rA[1].m_a == 0);
    assert(rA[1].m_b == 0);
    assert(rA[2].m_a == 0);
    assert(rA[2].m_b == 0);
    assert(test_struct::f == 0);
    assert(test_struct::h == 0);

    rA.emplace_front(1, 2);
    assert(test_struct::g == 3);
    assert(rA[0].m_a == 1);
    assert(rA[0].m_b == 2);
    assert(rA[1].m_a == 0);
    assert(rA[1].m_b == 0);
    assert(rA[2].m_a == 0);
    assert(rA[2].m_b == 0);
    assert(test_struct::f == 1);
    assert(test_struct::h == 1);

    rA.emplace_front(3, 4);
    assert(rA[0].m_a == 1);
    assert(rA[0].m_b == 2);
    assert(rA[1].m_a == 3);
    assert(rA[1].m_b == 4);
    assert(rA[2].m_a == 0);
    assert(rA[2].m_b == 0);
    assert(test_struct::g == 3);
    assert(test_struct::f == 2);
    assert(test_struct::h == 2);

    rA.pop_back();
    assert(test_struct::g == 3);
    assert(test_struct::f == 2);
    assert(test_struct::h == 2);

    rA.emplace_front(5, 6);
    assert(rA[0].m_a == 1);
    assert(rA[0].m_b == 2);
    assert(rA[1].m_a == 3);
    assert(rA[1].m_b == 4);
    assert(rA[2].m_a == 5);
    assert(rA[2].m_b == 6);
    assert(test_struct::g == 3);
    assert(test_struct::f == 3);
    assert(test_struct::h == 3);

    rA.emplace_front(7, 8);
    assert(rA[0].m_a == 7);
    assert(rA[0].m_b == 8);
    assert(rA[1].m_a == 3);
    assert(rA[1].m_b == 4);
    assert(rA[2].m_a == 5);
    assert(rA[2].m_b == 6);
    assert(test_struct::g == 3);
    assert(test_struct::f == 4);
    assert(test_struct::h == 4);
    assert(rA.full());
} // ring_unittest

////////////////////////////////////////////////////////////////////////////

} // namespace katahiromz

#endif  // ndef RING_BUFFER_HPP_
