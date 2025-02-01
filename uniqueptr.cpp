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

    T* release() noexcept
    {
        T* temp = m_ptr;
        m_ptr   = nullptr;
        return temp;
    }

    void reset(T* ptr) noexcept
    {
        delete m_ptr;
        m_ptr = ptr;
    }

    void swap(UniquePointer& other) noexcept
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_deleter, other.m_deleter);
    }

    T*      operator->() const { return m_ptr; }
    T&      operator*() const { return *m_ptr; }
    T*      get() const { return m_ptr; }
    Deleter get_deleter() const { return m_deleter; }
};
