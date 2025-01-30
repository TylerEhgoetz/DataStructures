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
};

int main()
{
    array<int, 5> a1;
    return 0;
}
