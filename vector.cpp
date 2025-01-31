#include <initializer_list>
#include <memory>

template <class T, class Allocator = std::allocator<T>> class vector
{
private:
    size_t m_size{};
    size_t m_capacity{};
    T*     m_data{ nullptr };

public:
    Vector(size_t size, const T& value);
    Vector(const std::initializer_list<T> values);
    void                          reserve(size_t capacity);
    void                          push_back(const T& value);
    template <class... Args> void emplace_back(Args&&... args);
    void                          clear();
    size_t                        size() const { return m_size; }
    size_t                        capacity() const { return m_capacity; }
    Allocator                     get_allocator() const { return {}; }
    T*                            data() { return m_data; }
};
