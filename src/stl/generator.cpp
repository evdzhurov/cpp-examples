#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <generator>
#include <iostream>
#include <string>

using namespace std::literals;
namespace fs = std::filesystem;

// Requires C++23 and libstdc++ (GCC 14)

std::string to_uppercase(std::string str)
{
    std::ranges::transform(str, str.begin(), ::toupper);
    return str;
}

struct ProcessedLine
{
    std::string original;
    std::string uppercase;
};

std::generator<ProcessedLine> read_and_process_lines(const fs::path& filepath)
{
    std::ifstream file{filepath};
    std::string line;
    while (std::getline(file, line))
    {
        ProcessedLine processed{.original = line, .uppercase = to_uppercase(line)};
        co_yield processed;
    }
}

int main()
{
    const auto temp_filepath = fs::temp_directory_path() / "generator-temp.txt";
    std::cout << "Temp filepath: " << temp_filepath << '\n';

    {
        std::ofstream temp_file{temp_filepath};
        temp_file << "Line 1: Hello, World!\n";
        temp_file << "Line 2: This is a test.\n";
        temp_file << "Line 3: C++ coroutines are cool!\n";
    }

    for (const auto& processed : read_and_process_lines(temp_filepath))
    {
        std::cout << "Original: " << processed.original << '\n';
        std::cout << "Uppercase: " << processed.uppercase << '\n';
    }

    return 0;
}