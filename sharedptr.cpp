
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

    SharedPointer(const SharedPointer& other) { copy(other); }
    SharedPointer& operator=(const SharedPointer& other)
    {
        if (this != &other)
        {
            if (m_ptr)
            {
                std::lock_guard<std::mutex> lock(m_controlBlock->m_mutex);
                if (--m_controlBlock->m_count == 0)
                {
                    delete m_ptr;
                    delete m_controlBlock;
                }

                m_ptr          = nullptr;
                m_controlBlock = nullptr;
            }

            copy(other);
        }
        return *this;
    }
    SharedPointer(SharedPointer&& other) noexcept;
    SharedPointer& operator=(SharedPointer&& other) noexcept;
    ~SharedPointer();

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
};
