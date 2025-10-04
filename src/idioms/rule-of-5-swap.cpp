
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
class MyArray
{
public:
    MyArray() : array{nullptr}, size{0}
    {
        std::cout << "Default construct empty array\n";
    }

    ~MyArray()
    {
        std::cout << "Destroying array " << array << " of size " << size << '\n';
        delete[] array;
    }

    MyArray(std::size_t size) : array{new T[size]{}}, size{size}
    {
        std::cout << "Constructing array " << array << " of size " << size << '\n';
    };

    MyArray(const MyArray& other) : array{new T[other.size]{}}, size{other.size}
    {
        std::cout << "Copy construct from " << other.array << " with size " << other.size << " to " << array << '\n';

        try
        {
            // Note: T can throw during copy which means that array gets leaked!
            std::copy_n(other.array, other.size, array);
        }
        catch (...)
        {
            std::cout << "Oops copying T threw...\n";
            delete[] array;
            throw;
        }
    }

    MyArray(MyArray&& other) noexcept : array{std::exchange(other.array, nullptr)}, size{std::exchange(other.size, 0)}
    {
        std::cout << "Move construct " << array << " with size " << size << '\n';
    }

    // Provides the strong exception guarantee
    // 'other' is fully constructed (copied or moved), may throw
    // swap performs the assignment, is noexcept
    MyArray& operator=(MyArray other) noexcept
    {
        std::cout << "By-value assignment operator " << array << " <- " << other.array << '\n';
        swap(*this, other);
        return *this;
    }

    bool operator==(const MyArray& other) const
    {
        if (size != other.size)
            return false;

        for (int i = 0; i < size; ++i)
        {
            if (array[i] != other.array[i])
                return false;
        }

        return true;
    }

    bool operator!=(const MyArray& other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] std::size_t Size() const
    {
        return size;
    }

    T& operator[](std::size_t idx)
    {
        return array[idx];
    }

private:
    friend void swap(MyArray& left, MyArray& right) noexcept
    {
        std::cout << "Using swap(MyArray,MyArray) hidden friend: " << left.array << " <-> " << right.array << '\n';
        using std::swap;
        swap(left.array, right.array);
        swap(left.size, right.size);
    }

    T* array;
    std::size_t size;
};

int main()
{
    struct Foo
    {
        Foo() : val{66}
        {
        }

        int val;
    };

    MyArray<Foo> myArrFoo{8};
    myArrFoo[0].val = 3333;
    std::cout << myArrFoo[0].val << '\n';

    MyArray<Foo> myArrFoo2{6};
    myArrFoo2[0].val = 1234;

    try
    {
        myArrFoo = myArrFoo2;
    }
    catch (...)
    {
        std::cout << "Failed to copy-assign\n";
    }

    std::cout << "dst value is " << myArrFoo[0].val << '\n';

    MyArray<int> myArr2{13};
    myArr2[1] = 33;

    MyArray<int> myArr3{0};

    assert(myArr3 != myArr2);

    myArr3 = myArr2;
    assert(myArr3[1] == 33 && myArr2[1] == 33);
    assert(myArr3 == myArr2);

    return 0;
}