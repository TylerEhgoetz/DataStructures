#include <iostream>
#include <utility>

template <typename T>
struct DefaultDeleter
{
    void operator()(T* ptr) const { delete ptr; }
};

template <class T, class Deleter = DefaultDeleter<T>>
class UniquePointer
{
private:
    T*      m_ptr{ nullptr };
    Deleter m_deleter{};

public:
    UniquePointer() {};
    explicit UniquePointer(T* ptr)
        : m_ptr{ ptr } {};
    UniquePointer(const UniquePointer&)            = delete;
    UniquePointer& operator=(const UniquePointer&) = delete;
    UniquePointer(UniquePointer&& other) noexcept
        : m_ptr{ std::exchange(other.m_ptr, nullptr) },
          m_deleter{ std::exchange(other.m_deleter, Deleter{}) } {};

    UniquePointer& operator=(UniquePointer&& other) noexcept
    {
        if (this != &other)
        {
            reset(other.release());
            m_deleter = std::move(other.m_deleter);
        }
        return *this;
    }

    ~UniquePointer() { m_deleter(m_ptr); }

    T* release() noexcept
    {
        T* temp = m_ptr;
        m_ptr   = nullptr;
        return temp;
    }

    void reset(T* ptr) noexcept
    {
        m_deleter(m_ptr);
        m_ptr = ptr;
    }

    void swap(UniquePointer& other) noexcept
    {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_deleter, other.m_deleter);
    }

    T*       operator->() const { return m_ptr; }
    T&       operator*() const { return *m_ptr; }
    T*       get() const { return m_ptr; }
    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    Deleter  get_deleter() const { return m_deleter; }
};

struct Test
{
    int value;
    Test(int v)
        : value(v)
    {
        std::cout << "Test object created with value " << value << "\n";
    }
    ~Test()
    {
        std::cout << "Test object with value " << value << " destroyed\n";
    }
};

int main()
{
    // Test 1: Basic construction and access
    {
        UniquePointer<Test> ptr(new Test(42));
        std::cout << "Value: " << ptr->value << "\n";             // Expect: 42
        std::cout << "Dereferencing: " << (*ptr).value << "\n";   // Expect: 42
    }   // ptr goes out of scope, should delete Test(42)

    std::cout << "----------------------\n";

    // Test 2: Move Constructor
    {
        UniquePointer<Test> ptr1(new Test(100));
        UniquePointer<Test> ptr2(std::move(ptr1));   // Move ptr1 into ptr2
        std::cout << "Ptr2 Value: " << ptr2->value << "\n";   // Expect: 100
        std::cout << "Ptr1 is now " << (ptr1 ? "not null" : "null")
                  << "\n";   // Expect: null
    }   // ptr2 goes out of scope, should delete Test(100)

    std::cout << "----------------------\n";

    // Test 3: Move Assignment
    {
        UniquePointer<Test> ptr1(new Test(200));
        UniquePointer<Test> ptr2(new Test(300));

        ptr2 = std::move(ptr1);   // Move ptr1 into ptr2

        std::cout << "Ptr2 Value: " << ptr2->value << "\n";   // Expect: 200
        std::cout << "Ptr1 is now " << (ptr1 ? "not null" : "null")
                  << "\n";   // Expect: null
    }   // ptr2 goes out of scope, should delete Test(200), Test(300) should
        // have been deleted earlier

    std::cout << "----------------------\n";

    // Test 4: Reset function
    {
        UniquePointer<Test> ptr(new Test(500));
        ptr.reset(new Test(600)
        );   // Expect: Test(500) destroyed, Test(600) created
        std::cout << "Ptr Value after reset: " << ptr->value
                  << "\n";   // Expect: 600
    }   // ptr goes out of scope, should delete Test(600)

    std::cout << "----------------------\n";

    // Test 5: Release function
    {
        UniquePointer<Test> ptr(new Test(700));
        Test*               rawPtr = ptr.release(
        );   // Expect: ptr releases ownership, but does not delete Test(700)
        std::cout << "Raw Pointer Value: " << rawPtr->value
                  << "\n";   // Expect: 700
        delete rawPtr;       // Manually delete to avoid leak
    }

    std::cout << "----------------------\n";

    // Test 6: Swap function
    {
        UniquePointer<Test> ptr1(new Test(800));
        UniquePointer<Test> ptr2(new Test(900));

        std::cout << "Before Swap:\n";
        std::cout << "Ptr1: " << ptr1->value << ", Ptr2: " << ptr2->value
                  << "\n";   // Expect: 800, 900

        ptr1.swap(ptr2);

        std::cout << "After Swap:\n";
        std::cout << "Ptr1: " << ptr1->value << ", Ptr2: " << ptr2->value
                  << "\n";   // Expect: 900, 800
    }   // Both pointers go out of scope, should delete Test(800) and Test(900)

    std::cout << "All tests completed.\n";
    return 0;
}
