#include "unix-like-pipe-operator.h"

#include <iostream>
#include <random>

std::string StringProc_1(std::string&& s)
{
    s += " proc by 1,";
    std::cout << "I'm in StringProc_1, s = " << s << '\n';
    return std::move(s);
}

std::string StringProc_2(std::string&& s)
{
    s += " proc by 2,";
    std::cout << "I'm in StringProc_2, s = " << s << '\n';
    return std::move(s);
}

std::string StringProc_3(std::string&& s)
{
    s += " proc by 3,";
    std::cout << "I'm in StringProc_3, s = " << s << '\n';
    return std::move(s);
}

enum class OpErrorType : unsigned char
{
    kInvalidInput,
    kOverflow,
    kUnderflow
};

struct Payload
{
    std::string str;
    int val = 0;
};

using PayloadOrError = std::expected<Payload, OpErrorType>;

PayloadOrError Payload_Proc_1(PayloadOrError&& s)
{
    if (s)
    {
        s->val += 1;
        s->str = " proc by 1, ";
        std::cout << "I'm in Payload_Proc_1, s = " << s->str << '\n';
    }

    return std::move(s);
}

PayloadOrError Payload_Proc_2(PayloadOrError&& s)
{
    if (s)
    {
        s->val += 1;
        s->str += " proc by 2, ";
        std::cout << "I'm in Payload_Proc_2, s = " << s->str << '\n';

        std::mt19937 rand_gen{std::random_device{}()};

        return (rand_gen() % 2 == 0)
                   ? s
                   : std::unexpected{rand_gen() % 2 == 0 ? OpErrorType::kOverflow : OpErrorType::kUnderflow};
    }

    return std::move(s);
}

PayloadOrError Payload_Proc_3(PayloadOrError&& s)
{
    if (s)
    {
        s->val += 1;
        s->str = " proc by 3, ";
        std::cout << "I'm in Payload_Proc_3, s = " << s->str << '\n';
    }

    return std::move(s);
}

int main()
{
    std::string start_str{"Start string "};
    std::cout << (std::move(start_str) | StringProc_1 | StringProc_2 | [](std::string&& s) {
        s += " proc by 3,";
        std::cout << "I'm in StringProc_3, s = " << s << '\n';
        return std::move(s);
    }) << '\n';

    auto res = PayloadOrError{Payload{.str = "Start string in payload", .val = 42}} | Payload_Proc_1 | Payload_Proc_2 |
               Payload_Proc_3;

    if (res)
        std::cout << "Success! Result of the pipe: " << res->str << ", " << res->val << '\n';
    else
        switch (res.error())
        {
        case OpErrorType::kInvalidInput:
            std::cout << "Error: OpErrorType::kInvalidInput\n";
            break;
        case OpErrorType::kOverflow:
            std::cout << "Error: OpErrorType::kOverflow\n";
            break;
        case OpErrorType::kUnderflow:
            std::cout << "Error: OpErrorType::kUnderflow\n";
            break;
        default:
            std::cout << "That's really an unexpected error ...\n";
            break;
        }

    return 0;
}