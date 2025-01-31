#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T, std::size_t Size> class array
{
private:
    T* m_ptr{ nullptr };

public:
    array()
        : m_ptr{ new T[Size]{} }
    {}

    array(const array& other)
        : m_ptr{ new T[Size]{} }
    {
        for (std::size_t i{ 0 }; i < Size; ++i)
            m_ptr[i] = other.m_ptr[i];
    }

    array& operator=(const array& other)
    {
        if (this == &other)
            return *this;

        // Create a new array
        T* new_ptr = new T[Size]{};

        // Copy elements
        for (std::size_t i{ 0 }; i < Size; ++i)
            new_ptr[i] = other.m_ptr[i];

        // Swap pointers (old memory not deleted until allocation succeeds)
        std::swap(m_ptr, new_ptr);

        // Delete old memory after
        delete[] new_ptr;

        return *this;
    }

    array(array&& other) noexcept
        : m_ptr{ std::exchange(other.m_ptr, nullptr) }
    {}

    array& operator=(array&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_ptr = std::exchange(other.m_ptr, nullptr);
        return *this;
    }

    ~array() { delete[] m_ptr; }

    T& operator[](std::size_t index) { return m_ptr[index]; }
    T& at(std::size_t index) const
    {
        if (index >= Size)
            throw std::out_of_range{ "Index out of range" };
        return m_ptr[index];
    }
    void fill(const T& value)
    {
        for (std::size_t i{ 0 }; i < Size; ++i)
            m_ptr[i] = value;
    }
    const T&              front() const { return m_ptr[0]; }
    const T&              back() const { return m_ptr[Size - 1]; }
    T*                    data() { return m_ptr; }
    const T*              data() const { return m_ptr; }
    T*                    begin() { return m_ptr; }
    T*                    end() { return m_ptr + Size; }
    constexpr std::size_t size() const { return Size; }
    constexpr bool        empty() const { return Size == 0; }
};

int main()
{
    bool allTestsPassed = true;

    auto check          = [&](bool condition, const char* message)
    {
        if (!condition)
        {
            allTestsPassed = false;
            std::cerr << "TEST FAILED: " << message << "\n";
        }
    };

    // 1) Create array and test fill()
    {
        array<int, 5> arr;
        arr.fill(10);

        for (std::size_t i = 0; i < arr.size(); ++i)
        {
            check(arr[i] == 10, "fill(10) should set all elements to 10");
        }
    }

    // 2) Test operator[] and at()
    {
        array<int, 5> arr;
        arr.fill(0);
        arr[2] = 42;
        check(
            arr[2] == 42, "operator[] should set/get correct value at index 2"
        );

        arr.at(3) = 99;
        check(arr[3] == 99, "at() should set/get correct value at index 3");

        // Test out_of_range exception
        bool caughtOutOfRange = false;
        try
        {
            arr.at(5
            ) = 123;   // invalid index (5 is out of range for array<...,5>)
        }
        catch (const std::out_of_range&)
        {
            caughtOutOfRange = true;
        }
        check(caughtOutOfRange, "at(5) should throw out_of_range");
    }

    // 3) Test copy constructor
    {
        array<int, 5> original;
        original.fill(77);

        array<int, 5> copy{ original };
        for (std::size_t i = 0; i < original.size(); ++i)
        {
            check(
                copy[i] == 77,
                "Copy constructor should duplicate values from original"
            );
        }

        // Modify original to ensure copy is independent
        original[0] = 999;
        check(
            copy[0] == 77,
            "Changing original after copy shouldn't affect the copy"
        );
    }

    // 4) Test copy assignment operator
    {
        array<int, 5> arr1;
        arr1.fill(88);

        array<int, 5> arr2;
        arr2.fill(11);

        arr2 = arr1;
        for (std::size_t i = 0; i < arr1.size(); ++i)
        {
            check(
                arr2[i] == 88,
                "Copy assignment should copy values from arr1 to arr2"
            );
        }

        // Modify arr1 to ensure arr2 is independent
        arr1[0] = 777;
        check(
            arr2[0] == 88,
            "Changing arr1 after assignment shouldn't affect arr2"
        );
    }

    // 5) Test move constructor
    {
        array<int, 5> temp;
        temp.fill(33);

        array<int, 5> moved{ std::move(temp) };
        // 'temp' is now in a valid but unspecified state (its pointer was
        // moved). We'll check only that 'moved' has the data we expect.
        for (std::size_t i = 0; i < moved.size(); ++i)
        {
            check(
                moved[i] == 33,
                "Move constructor should transfer values to 'moved'"
            );
        }
    }

    // 6) Test move assignment
    {
        array<int, 5> arr1;
        arr1.fill(111);

        array<int, 5> arr2;
        arr2.fill(222);

        arr2 = std::move(arr1);
        for (std::size_t i = 0; i < arr2.size(); ++i)
        {
            check(
                arr2[i] == 111, "Move assignment should transfer values to arr2"
            );
        }
    }

    // 7) Test front(), back()
    {
        array<int, 5> arr;
        arr.fill(1);
        arr[0] = 10;
        arr[4] = 50;
        check(arr.front() == 10, "front() should return first element");
        check(arr.back() == 50, "back()  should return last element");
    }

    // 8) Test begin(), end() iteration
    {
        array<int, 5> arr;
        arr.fill(5);

        int sum = 0;
        for (auto it = arr.begin(); it != arr.end(); ++it)
        {
            sum += *it;
        }
        check(
            sum == (5 * 5),
            "Iteration over begin() to end() should sum all 5s correctly (5 * "
            "5 = 25)"
        );
    }

    // 9) Test size(), empty()
    {
        array<int, 5> arr;
        check(arr.size() == 5, "size() should return template argument Size=5");
        check(!arr.empty(), "empty() should be false for Size=5");

        // Another array with size 0 to verify empty is true
        array<int, 0> emptyArr;
        check(emptyArr.empty(), "empty() should be true for Size=0");
        check(emptyArr.size() == 0, "size() should be 0 for array<int,0>");
    }

    // Report summary
    if (allTestsPassed)
    {
        std::cout << "All tests passed successfully!\n";
    }
    else
    {
        std::cout << "Some tests failed. Check error messages above.\n";
    }

    return 0;
}
