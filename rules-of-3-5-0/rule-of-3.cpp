#include <cstddef>
#include <cstring>
#include <iostream>

/// @brief A RAII class that manages a c-style string
class RuleOfThree
{
public:
    RuleOfThree(const char *s, std::size_t n)
        : m_cstr(new char[n])
    {
        std::memcpy(m_cstr, s, n);
    }

    RuleOfThree(const char *s = "") : RuleOfThree(s, strlen(s) + 1) {}

    /// @brief I. Destructor 
    ~RuleOfThree()
    {
        delete[] m_cstr;
    }

    /// @brief II. Copy constructor
    RuleOfThree(const RuleOfThree &other) : RuleOfThree(other.m_cstr) {}

    /// @brief III. Copy assignment operator
    RuleOfThree &operator=(const RuleOfThree &other)
    {
        if (this == &other)
            return *this;

        std::size_t n = std::strlen(other.m_cstr) + 1;
        char *new_cstr = new char[n];
        std::memcpy(new_cstr, other.m_cstr, n);

        delete[] m_cstr;
        m_cstr = new_cstr;

        return *this;
    }
    
    operator const char *() const
    {
        return m_cstr;
    }

private:
    char *m_cstr;
};

int main(int argc, char *argv[])
{
    RuleOfThree o1{"abc"};
    std::cout << o1 << ' ';
    auto o2{o1}; // I. Uses copy constructor
    std::cout << o2 << ' ';
    RuleOfThree o3("def");
    std::cout << o3 << ' ';
    o3 = o2; // III. Uses copy assignment
    std::cout << o3 << " \n";
    return 0;
}