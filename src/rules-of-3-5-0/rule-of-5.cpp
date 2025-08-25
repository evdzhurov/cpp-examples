#include <cstddef>
#include <cstring>
#include <iostream>
#include <utility>

// A RAII class that manages a c-style string
class RuleOfFive {
  public:
    RuleOfFive(const char *s, std::size_t n) : m_cstr(new char[n]) {
        std::memcpy(m_cstr, s, n);
    }

    RuleOfFive(const char *s = "") : RuleOfFive(s, strlen(s) + 1) {}

    /// @brief I. Destructor
    ~RuleOfFive() { delete[] m_cstr; }

    /// @brief II. Copy constructor
    RuleOfFive(const RuleOfFive &other) : RuleOfFive(other.m_cstr) {}

    /// @brief III. Move constructor
    RuleOfFive(RuleOfFive &&other) noexcept
        : m_cstr{std::exchange(other.m_cstr, nullptr)} {}

    /// @brief IV. Copy assignment
    /// @param other
    /// @return
    RuleOfFive &operator=(const RuleOfFive &other) {
        return *this = RuleOfFive(other); // This triggers move assignment since
                                          // we're assigning from a temporary
    }

    /// @brief V. Move assignment
    /// @param other
    /// @return
    RuleOfFive &operator=(RuleOfFive &&other) noexcept {
        std::swap(m_cstr, other.m_cstr);
        return *this;
    }

    // Alternatively (IV. Copy assignment) and (V. Move assignment) can be
    // combined RuleOfFive& operator=(RuleOfFive other) noexcept {
    //     std::swap(m_cstr, other.m_cstr);
    //     return *this;
    // }

    operator const char *() const { return m_cstr; }

  private:
    char *m_cstr;
};

int main(int argc, char *argv[]) {
    RuleOfFive o1{"abc"};
    std::cout << o1 << ' ';
    auto o2{o1}; // I. Uses copy constructor
    std::cout << o2 << ' ';
    RuleOfFive o3("def");
    std::cout << o3 << ' ';
    o3 = o2; // III. Uses copy assignment
    std::cout << o3 << ' ';
    RuleOfFive o4{std::move(o3)}; // IV. Uses move constructor
    std::cout << o4 << ' ';
    o4 = std::move(o2); // V. Uses move assignment
    std::cout << o4 << '\n';
    return 0;
}