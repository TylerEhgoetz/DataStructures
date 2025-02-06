#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
struct Data
{
    T    data{};
    bool processed{ false };
};

template <typename T>
class ProducerConsumer
{
public:
    ProducerConsumer()
        : m_producer{ [this] { Produce(); } },
          m_consumer{ [this] { Consume(); } }
    {}

private:
    void Produce()
    {
        while (!m_done)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            {
                std::lock_guard lock{ m_mutex };
                Data<T>         data;
                data.data = GenerateData();
                m_queue.push(data);
            }
            m_condition.notify_one();
        }
    }

    void Consume()
    {
        while (!m_done)
        {
            std::unique_lock lock{ m_mutex };
            m_condition
                .wait(lock, [this] { return !m_queue.empty() || m_done; });
            if (m_done && m_queue.empty())
            {
                break;
            }
            auto item = m_queue.front();
            m_queue.pop();
            lock.unlock();
            ProcessData(item);
            if (item.processed)
            {
                break;
            }
        }
    }

    void ProcessData(Data<T>& data)
    {
        // Process data
    }

    T GenerateData() { return T{}; }

    mutable std::mutex      m_mutex{};
    std::queue<Data<T>>     m_queue{};
    std::condition_variable m_condition{};
    std::thread             m_producer{};
    std::thread             m_consumer{};
    std::atomic<bool>       m_done{ false };
};
