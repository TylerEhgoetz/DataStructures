#include <atomic>
#include <condition_variable>
#include <mutex>

// Counting semaphore implementation
class CountSemaphore
{
public:
    CountSemaphore(int count = 1)
        : m_count(count)
    {}

    void acquire()
    {
        while (m_count.fetch_sub(1, std::memory_order_acquire) <= 0)
        {
            m_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void release() { m_count.fetch_add(1, std::memory_order_release); }

private:
    std::atomic<int> m_count;
};

// Await/Notify Semaphore implementation
class AwaitNotifySemaphore
{
public:
    AwaitNotifySemaphore(int count = 1)
        : m_count(count)
    {}

    void acquire()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_count > 0; });
        --m_count;
    }

    void release()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        ++m_count;
        m_cv.notify_one();
    }

private:
    int                     m_count;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
};
