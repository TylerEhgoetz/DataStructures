
#include <cassert>
#include <iostream>
#include <mutex>
#include <utility>

struct ControlBlock
{
    size_t             m_count{ 0 };
    mutable std::mutex m_mutex;

    explicit ControlBlock(size_t count)
        : m_count{ count }, m_mutex{}
    {}
};

template <typename T>
class SharedPointer
{
public:
    SharedPointer()
        : SharedPointer(nullptr)
    {}
    SharedPointer(std::nullptr_t)
        : m_ptr{ nullptr }, m_controlBlock{ nullptr }
    {}
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

    void reset(T* ptr)
    {
        release();

        if (ptr)
        {
            m_ptr          = ptr;
            m_controlBlock = new ControlBlock{ 1 };
        }
    }

    void reset() { release(); }

    void swap(SharedPointer& other) noexcept
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_controlBlock, other.m_controlBlock);
    }

    size_t get_count() const
    {
        std::lock_guard<std::mutex> lock(m_controlBlock->m_mutex);
        return m_controlBlock->m_count;
    }

    T* get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    operator bool() const noexcept { return m_ptr != nullptr; }

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
            bool          deletePtr{ false };
            ControlBlock* cb = m_controlBlock;
            {
                std::lock_guard<std::mutex> lock(cb->m_mutex);
                if (--cb->m_count == 0)
                {
                    deletePtr = true;
                }
            }

            if (deletePtr)
            {
                delete m_ptr;
                delete cb;
            }
            m_ptr          = nullptr;
            m_controlBlock = nullptr;
        }
    }

    void steal(SharedPointer& other) noexcept
    {
        m_ptr          = std::exchange(other.m_ptr, nullptr);
        m_controlBlock = std::exchange(other.m_controlBlock, nullptr);
    }
};

int main()
{
    // Test default construction.
    SharedPointer<int> sp_default;
    assert(!sp_default);
    std::cout << "Default constructor test passed.\n";

    // Test construction from a raw pointer.
    SharedPointer<int> sp1(new int(10));
    assert(sp1);
    assert(sp1.get_count() == 1);
    std::cout << "sp1 value: " << *sp1 << ", count: " << sp1.get_count()
              << "\n";

    // Test copy constructor.
    SharedPointer<int> sp2(sp1);
    assert(sp1.get_count() == 2);
    assert(sp2.get_count() == 2);
    std::cout << "After copying sp1 to sp2, count: " << sp1.get_count() << "\n";

    // Test copy assignment operator.
    SharedPointer<int> sp3;
    sp3 = sp1;
    assert(sp1.get_count() == 3);
    assert(sp3.get_count() == 3);
    std::cout << "After assigning sp1 to sp3, count: " << sp1.get_count()
              << "\n";

    // Test move constructor.
    SharedPointer<int> sp4(std::move(sp3));
    assert(!sp3);   // sp3 should be empty after move.
    assert(sp4.get_count() == 3);
    std::cout << "After moving sp3 to sp4, sp4 count: " << sp4.get_count()
              << "\n";

    // Test move assignment operator.
    SharedPointer<int> sp5;
    sp5 = std::move(sp4);
    assert(!sp4);
    assert(sp5.get_count() == 3);
    std::cout << "After moving sp4 to sp5, sp5 count: " << sp5.get_count()
              << "\n";

    // Test reset with a new pointer.
    sp5.reset(new int(20));
    assert(sp5.get_count() == 1);
    std::cout << "After resetting sp5 to new value, sp5 value: " << *sp5
              << ", count: " << sp5.get_count() << "\n";

    // Test reset to nullptr.
    sp5.reset();
    assert(!sp5);
    std::cout << "After resetting sp5 to null, sp5 is "
              << (sp5 ? "not null" : "null") << ".\n";

    // Test swap.
    SharedPointer<int> sp6(new int(30));
    SharedPointer<int> sp7(new int(40));
    std::cout << "Before swap: sp6 = " << *sp6 << ", sp7 = " << *sp7 << "\n";
    sp6.swap(sp7);
    std::cout << "After swap: sp6 = " << *sp6 << ", sp7 = " << *sp7 << "\n";

    std::cout << "All tests passed.\n";
    return 0;
}
