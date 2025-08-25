#include <cstddef>
#include <iostream>
#include <string>

/// @brief A RAII class that manages a c-style string
class RuleOfZero {
  public:
    RuleOfZero(const std::string &str) : m_str{str} {}

    operator const char *() const { return m_str.c_str(); }

  private:
    std::string m_str;
};

int main(int argc, char *argv[]) {
    RuleOfZero o1{"abc"};
    std::cout << o1 << ' ';
    auto o2{o1}; // I. Uses copy constructor
    std::cout << o2 << ' ';
    RuleOfZero o3("def");
    std::cout << o3 << ' ';
    o3 = o2; // III. Uses copy assignment
    std::cout << o3 << ' ';
    RuleOfZero o4{std::move(o3)}; // IV. Uses move constructor
    std::cout << o4 << ' ';
    o4 = std::move(o2); // V. Uses move assignment
    std::cout << o4 << '\n';
    return 0;
}