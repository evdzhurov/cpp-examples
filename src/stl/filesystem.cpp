#include <chrono>
#include <ctime>
#include <expected>
#include <filesystem>
#include <iostream>
#include <system_error>

// Source: https://www.cppstories.com/2017/08/cpp17-details-filesystem/

// std::filesystem is based on boost filesystem and POSIX

namespace fs = std::filesystem;

void DisplayFileInfo(const fs::directory_entry& fileEntry, const std::string& lead, const fs::path& filename)
{
    std::cout << lead << std::format(" [F] {:<45} {}\n", filename.generic_string(), fileEntry.file_size());
}

void DisplayDirTree(const fs::path& pathToShow, size_t level)
{
    if (!fs::exists(pathToShow) || !fs::is_directory(pathToShow))
        return;

    auto lead = std::string(level * 3, ' ');
    for (const auto& entry : fs::directory_iterator{pathToShow})
    {
        const auto filename = entry.path().filename();
        if (fs::is_directory(entry.status()))
        {
            std::cout << lead << "[+] " << filename << '\n';
            DisplayDirTree(entry, level + 1);
            std::cout << '\n';
        }
        else if (fs::is_regular_file(entry.status()))
            DisplayFileInfo(entry, lead, filename);
        else
            std::cout << lead << " [?]" << filename.generic_string() << '\n';
    }
}

int main()
{
    std::cout << "current_path = " << fs::current_path() << '\n';
    fs::path pathToShow{"src/stl/filesystem.cpp"};

    {
        std::cout << "path: " << pathToShow << '\n'
                  << "exists: " << fs::exists(pathToShow) << '\n'
                  << "root_name: " << pathToShow.root_name() << '\n'
                  << "relative_path: " << pathToShow.relative_path() << '\n'
                  << "parent_path: " << pathToShow.parent_path() << '\n'
                  << "filename: " << pathToShow.filename() << '\n'
                  << "stem: " << pathToShow.stem() << '\n'
                  << "extension: " << pathToShow.extension() << '\n';
        std::cout << '\n';

        // C++20: range-based for loop init-statement
        for (int i = 0; const auto& part : pathToShow)
            std::cout << "path part: " << i++ << " = " << part << '\n';
    }

    {
        fs::path p1{"C:\\temp"};
        p1 /= "user";
        p1 /= "data";
        std::cout << p1 << '\n';

        fs::path p2{"C:\\temp"};
        p2 += "user";
        p2 += "data";
        std::cout << p2 << '\n';
    }

    // C++23: std::expected
    const auto computeFileSize = [](const fs::path& pathToCheck) -> std::expected<uintmax_t, std::error_code> {
        if (!fs::exists(pathToCheck))
        {
            return std::unexpected{std::make_error_code(std::errc::no_such_file_or_directory)};
        }

        if (!fs::is_regular_file(pathToCheck))
        {
            return std::unexpected{std::make_error_code(std::errc::is_a_directory)};
        }

        auto err = std::error_code{};
        const auto filesize = fs::file_size(pathToCheck, err);
        if (!err)
            return filesize;

        return std::unexpected{err};
    };

    if (const auto res = computeFileSize(pathToShow))
        std::cout << pathToShow << " filesize = " << res.value() << '\n';
    else
        std::cout << pathToShow << " filesize err: " << res.error().message() << '\n';

    {
        const auto timeEntry = fs::last_write_time(pathToShow);
        std::cout << pathToShow << " last_write_time = " << timeEntry << '\n';
    }

    DisplayDirTree(fs::path{"src/"}, 0);

    return 0;
}
