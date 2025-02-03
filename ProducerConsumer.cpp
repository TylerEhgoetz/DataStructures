#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class ProducerConsumer
{
public:
    ProducerConsumer()
        : m_producer{ [this] { Produce(); } },
          m_consumer{ [this] { Consume(); } }
    {}

private:
    void Produce();
    void Consume();
    void DoSomething(const T& item)
    {
        // do something
    }
    T GenerateData() { return T{}; }

    mutable std::mutex      m_mutex{};
    std::queue<T>           m_queue{};
    std::condition_variable m_condition{};
    std::thread             m_producer{};
    std::thread             m_consumer{};
};
