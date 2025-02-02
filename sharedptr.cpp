
#include <mutex>

struct ControlBlock
{
    int                m_count{ 0 };
    mutable std::mutex m_mutex;
};

template <typename T>
class SharedPointer
{
public:
    SharedPointer()
        : SharedPointer(nullptr)
    {}
    SharedPointer(std::nullptr_t) {}
    explicit SharedPointer(T* ptr)
        : m_ptr{ ptr }, m_controlBlock{ new ControlBlock{ 1 } }
    {}

    SharedPointer(const SharedPointer& other);
    SharedPointer& operator=(const SharedPointer& other);
    SharedPointer(SharedPointer&& other) noexcept;
    SharedPointer& operator=(SharedPointer&& other) noexcept;
    ~SharedPointer();

private:
    T*            m_ptr{ nullptr };
    ControlBlock* m_controlBlock{ nullptr };
};
