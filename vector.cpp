#include <cassert>
#include <initializer_list>
#include <iostream>
#include <memory>

template <class T, class Allocator = std::allocator<T>>
class Vector
{
private:
    size_t    m_size{};
    size_t    m_capacity{};
    T*        m_data{ nullptr };
    Allocator m_allocator{};

    void deallocate()
    {
        using Traits = std::allocator_traits<Allocator>;
        for (size_t i = 0; i < m_size; ++i)
        {
            Traits::destroy(m_allocator, m_data + i);
        }
        Traits::deallocate(m_allocator, m_data, m_capacity);
    }

    void try_increase_capacity()
    {
        if (m_size == m_capacity)
            reserve(m_capacity == 0 ? 1 : m_capacity * 2);
    }

public:
    explicit Vector(size_t size, const T& value);
    Vector(const std::initializer_list<T> values);
    Vector(const Vector& other)                = delete;
    Vector& operator=(const Vector& other)     = delete;
    Vector(Vector&& other) noexcept            = delete;
    Vector& operator=(Vector&& other) noexcept = delete;
    ~Vector();

    void reserve(size_t capacity);
    void push_back(const T& value);
    template <class... Args>
    void      emplace_back(Args&&... args);
    void      clear();
    size_t    size() const { return m_size; }
    size_t    capacity() const { return m_capacity; }
    T*        data() { return m_data; }
    Allocator get_allocator() const { return m_allocator; }
};

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(size_t size, const T& value)
{
    reserve(size);

    for (size_t i{}; i < size; ++i)
        push_back(value);
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const std::initializer_list<T> values)
{
    reserve(values.size());

    for (const auto& value : values)
        push_back(value);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::reserve(size_t capacity)
{
    using Traits = std::allocator_traits<Allocator>;
    if (capacity <= m_capacity)
        return;

    T*     newData = Traits::allocate(m_allocator, capacity);
    size_t i       = 0;

    try
    {
        for (; i < m_size; ++i)
            Traits::construct(m_allocator, newData + i, std::move(m_data[i]));
    }
    catch (...)
    {
        for (size_t j{ 0 }; j < i; ++j)
            Traits::destroy(m_allocator, newData + j);

        Traits::deallocate(m_allocator, newData, capacity);

        throw;
    }

    deallocate();   // Destroy + deallocate old data
    m_data     = newData;
    m_capacity = capacity;
}

template <typename T, typename Allocator>
Vector<T, Allocator>::~Vector()
{
    deallocate();
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& value)
{
    try_increase_capacity();
    new (m_data + m_size++) T{ value };
}

template <typename T, typename Allocator>
template <typename... Args>
void Vector<T, Allocator>::emplace_back(Args&&... args)
{
    try_increase_capacity();
    new (m_data + m_size++) T(std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::clear()
{
    using Traits = std::allocator_traits<Allocator>;
    for (size_t i{ 0 }; i < m_size; ++i)
        Traits::destroy(m_allocator, m_data + i);

    m_size = 0;
}

int main()
{
    // Construct with size and value
    {
        Vector<int> v(5, 42);
        assert(v.size() == 5);
        for (size_t i = 0; i < v.size(); ++i)
        {
            assert(v.data()[i] == 42);
        }
    }
    // Construct with initializer list
    {
        Vector<int> v{ 1, 2, 3 };
        assert(v.size() == 3);
        assert(v.data()[0] == 1);
        assert(v.data()[1] == 2);
        assert(v.data()[2] == 3);
    }

    {
        Vector<int> v(3, 10);   // 3 elements, all 10
        // size = 3, capacity >= 3
        for (int i = 0; i < 5; ++i)
        {
            v.push_back(i);
        }
        // Now size() should be 8
        assert(v.size() == 8);

        // The first 3 are 10
        for (int i = 0; i < 3; ++i)
        {
            assert(v.data()[i] == 10);
        }
        // The next 5 are 0..4
        for (int i = 0; i < 5; ++i)
        {
            assert(v.data()[i + 3] == i);
        }
    }

    {
        Vector<int> v{ 1, 2, 3 };
        size_t      oldCap = v.capacity();
        // push until we force at least one reallocation
        for (int i = 0; i < 20; ++i)
        {
            v.push_back(i);
        }
        // Just check we didn't break anything
        assert(v.size() == 3 + 20);

        // Optionally see if capacity grew
        assert(v.capacity() >= v.size());
        // Now you can see if oldCap < v.capacity() is actually true (likely).
        assert(oldCap < v.capacity());
    }

    {
        Vector<int> v{ 1, 2, 3 };
        v.clear();
        assert(v.size() == 0);
        // The capacity doesn’t change after clear
        assert(v.capacity() >= 3);

        // push_back again after clear
        v.push_back(100);
        assert(v.size() == 1);
        assert(v.data()[0] == 100);
    }

    {
        struct Foo
        {
            int x, y;
            Foo(int a, int b)
                : x(a), y(b)
            {}
        };

        Vector<Foo> vf{
            { 1, 2 },
            { 3, 4 }
        };
        // size = 2
        vf.emplace_back(5, 6);
        // size = 3
        assert(vf.size() == 3);
        assert(vf.data()[2].x == 5);
        assert(vf.data()[2].y == 6);
    }
}
