#include <iostream>

// Use CRTP for polymorphic chaining
// https://marcoarena.wordpress.com/2012/04/29/use-crtp-for-polymorphic-chaining/

struct NoCrtp;
// Derived == NoCrtp: CRTP disabled => selects 'Base' type
// Derived != NoCrtp: CRTP enabled => selects 'Derived' type
#define SELECT_CRTP_TYPE(Base) typename std::conditional_t<std::is_same_v<Derived, NoCrtp>, Base, Derived>

template <typename Derived>
class BasicPrinter
{
public:
    // Allows the 'print' function to return 'BasicPrinter &' when CRTP is
    // disabled or to return 'Derived &' when CRTP is enabled
    using CrtpType = SELECT_CRTP_TYPE(BasicPrinter);

    BasicPrinter(std::ostream &os) : m_stream{os}
    {
    }

    template <typename T>
    CrtpType &print(T &&t)
    {
        m_stream << t;
        return static_cast<CrtpType &>(*this);
    }

protected:
    std::ostream &m_stream;
};

// CRTP disabled case
using BasicPrinterUser = BasicPrinter<NoCrtp>;

// CRTP enabled case
class CoutBasicPrinter : public BasicPrinter<CoutBasicPrinter>
{
public:
    CoutBasicPrinter() : BasicPrinter{std::cout}
    {
    }
};

// By using SELECT_CRTP_TYPE in the inheritance we essentially either propagate
// 'Derived' or 'AdvancedPrinter<NoCrtp>' (aliased by AdvancedPrinterUser) to
// BasicPrinter
template <typename Derived>
class AdvancedPrinter : public BasicPrinter<SELECT_CRTP_TYPE(AdvancedPrinter<Derived>)>
{
public:
    // Similar to BasicPrinter allows the 'println' function to return
    // 'AdvancedPrinter &' when CRTP is disabled
    // or to return 'Derived &' when CRTP is enabled
    using CrtpType = SELECT_CRTP_TYPE(AdvancedPrinter);

    using BasicPrinter::BasicPrinter;

    template <typename T>
    CrtpType &println(T &&t)
    {
        m_stream << t << '\n';
        return static_cast<CrtpType &>(*this);
    }
};

// CRTP disabled case
using AdvancedPrinterUser = AdvancedPrinter<NoCrtp>;

// CRTP enabled case
class CoutAdvancedPrinter : public AdvancedPrinter<CoutAdvancedPrinter>
{
public:
    CoutAdvancedPrinter() : AdvancedPrinter{std::cout}
    {
    }
};

int main()
{
    BasicPrinterUser{std::cout}.print("BasicPrinterUser: ").print(11).print('\n');

    CoutBasicPrinter{}.print("CoutBasicPrinter: ").print(22).print('\n');

    AdvancedPrinterUser{std::cout}.println("AdvancedPrinterUser: ").print(33).println("");

    CoutAdvancedPrinter{}.println("CoutAdvancedPrinter: ").print(44).println("");

    return 0;
}