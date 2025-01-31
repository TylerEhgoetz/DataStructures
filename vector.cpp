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
    void reserve(size_t capacity);
    void push_back(const T& value);
    template <class... Args>
    void      emplace_back(Args&&... args);
    void      clear();
    size_t    size() const { return m_size; }
    size_t    capacity() const { return m_capacity; }
    T*        data() { return m_data; }
    Allocator get_allocator() const { return m_allocator; }
    ~Vector();
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

    for (auto&& value : values)
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
            Traits::construct(m_allocator, newData + i, m_data[i]);
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
    if (m_data == nullptr)
        return;

    for (size_t i{ 0 }; i < m_size; ++i)
        m_data[i].~T();

    m_size = 0;
}

int main()
{
    Vector<int> v{ 1, 2, 3 };
    std::cout << v.size() << std::endl;
    return 0;
}
