#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename T, std::size_t Size> class array
{
private:
    T* m_ptr{ nullptr };

public:
    array()
        : m_ptr{ new T[Size]{} }
    {}

    array(const array& other)
        : m_ptr{ new T[Size]{} }
    {
        for (std::size_t i{ 0 }; i < Size; ++i)
            m_ptr[i] = other.m_ptr[i];
    }

    array& operator=(const array& other)
    {
        if (this == &other)
            return *this;

        // Create a new array
        T* new_ptr = new T[Size]{};

        // Copy elements
        for (std::size_t i{ 0 }; i < Size; ++i)
            new_ptr[i] = other.m_ptr[i];

        // Swap pointers (old memory not deleted until allocation succeeds)
        std::swap(m_ptr, new_ptr);

        // Delete old memory after
        delete[] new_ptr;

        return *this;
    }

    array(array&& other) noexcept
        : m_ptr{ std::exchange(other.m_ptr, nullptr) }
    {}

    array& operator=(array&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_ptr = std::exchange(other.m_ptr, nullptr);
        return *this;
    }

    ~array() { delete[] m_ptr; }

    T& operator[](std::size_t index) { return m_ptr[index]; }
    T& at(std::size_t index) const
    {
        if (index >= Size)
            throw std::out_of_range{ "Index out of range" };
        return m_ptr[index];
    }
    void fill(const T& value)
    {
        for (std::size_t i{ 0 }; i < Size; ++i)
            m_ptr[i] = value;
    }
    const T&              front() const { return m_ptr[0]; }
    const T&              back() const { return m_ptr[Size - 1]; }
    T*                    data() { return m_ptr; }
    const T*              data() const { return m_ptr; }
    T*                    begin() { return m_ptr; }
    T*                    end() { return m_ptr + Size; }
    constexpr std::size_t size() const { return Size; }
    constexpr bool        empty() const { return Size == 0; }
};

int main()
{
    array<int, 5> arr;
    arr.fill(10);
    arr[2]    = 3;
    arr.at(3) = 4;

    for (std::size_t i{ 0 }; i < arr.size(); ++i)
        arr[i] *= 2;

    for (int i : arr)
        i *= 2;

    return 0;
}
