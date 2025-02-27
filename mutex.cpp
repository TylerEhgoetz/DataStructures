#include <atomic>
#include <thread>

class Mutex
{
public:
    Mutex()
        : m_flag(ATOMIC_FLAG_INIT)
    {}

    void lock()
    {
        while (m_flag.test_and_set(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
    }

    void unlock() { m_flag.clear(std::memory_order_release); }

    bool try_lock() { return !m_flag.test_and_set(std::memory_order_acquire); }

private:
    std::atomic_flag m_flag;
};
