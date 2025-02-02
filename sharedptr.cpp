
#include <mutex>
#include <utility>

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

    SharedPointer(const SharedPointer& other) { copy(other); }
    SharedPointer& operator=(const SharedPointer& other)
    {
        if (this != &other)
        {
            release();
            copy(other);
        }
        return *this;
    }
    SharedPointer(SharedPointer&& other) noexcept { steal(other); }
    SharedPointer& operator=(SharedPointer&& other) noexcept
    {
        if (this != &other)
        {
            release();
            steal(other);
        }
        return *this;
    }
    ~SharedPointer() { release(); }

private:
    T*            m_ptr{ nullptr };
    ControlBlock* m_controlBlock{ nullptr };

    void copy(const SharedPointer& other) noexcept
    {
        if (other.m_ptr)
        {
            std::lock_guard<std::mutex> lock(other.m_controlBlock->m_mutex);
            m_ptr          = other.m_ptr;
            m_controlBlock = other.m_controlBlock;
            ++m_controlBlock->m_count;
        }
    }

    void release() noexcept
    {
        if (m_ptr)
        {
            std::lock_guard<std::mutex> lock(m_controlBlock->m_mutex);
            if (--m_controlBlock->m_count == 0)
            {
                delete m_ptr;
                delete m_controlBlock;
                m_ptr          = nullptr;
                m_controlBlock = nullptr;
            }
        }
    }

    void steal(SharedPointer& other) noexcept
    {
        m_ptr          = std::exchange(other.m_ptr, nullptr);
        m_controlBlock = std::exchange(other.m_controlBlock, nullptr);
    }
};
