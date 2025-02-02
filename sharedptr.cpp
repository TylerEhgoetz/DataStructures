
#include <mutex>

struct ControlBlock
{
    int                m_count{ 0 };
    mutable std::mutex m_mutex;
};

template <typename T>
class SharedPointer
{

private:
    T*            m_ptr{ nullptr };
    ControlBlock* m_controlBlock{ nullptr };
};
