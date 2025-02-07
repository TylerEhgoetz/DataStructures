#include <atomic>
#include <thread>

class Mutex
{
public:
    Mutex()
        : m_locked(false)
    {}

    void lock()
    {
        while (m_locked.exchange(true, std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
    }

    void unlock() { m_locked.store(false, std::memory_order_release); }

    bool try_lock()
    {
        return !m_locked.exchange(true, std::memory_order_acquire);
    }

private:
    std::atomic<bool> m_locked;
};
