#include <utility>

template <typename T>
struct DefaultDeleter
{
    void operator()(T* ptr) const { delete ptr; }
};

template <class T, class Deleter = DefaultDeleter<T>>
class UniquePointer
{
private:
    T*      m_ptr{ nullptr };
    Deleter m_deleter{};

    void exchange(UniquePointer&& other)
    {
        m_ptr = std::exchange(other.m_ptr, nullptr);
    }

public:
    UniquePointer() {};
    UniquePointer(T* ptr)
        : m_ptr{ ptr } {};
    UniquePointer(const UniquePointer&)            = delete;
    UniquePointer& operator=(const UniquePointer&) = delete;
    UniquePointer(UniquePointer&& other) noexcept { exchange(other); }

    UniquePointer& operator=(UniquePointer&& other) noexcept
    {
        if (*this == &other)
            return *this;

        exchange(other);
        return *this;
    }

    ~UniquePointer() { m_deleter(m_ptr); }
};
