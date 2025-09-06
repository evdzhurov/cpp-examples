#include <numeric>
#include <print>
#include <string_view>
#include <unordered_map>

int main()
{
    // ====================================================================================
    // C++23
    // ====================================================================================
    // Table formatting
    {
        std::unordered_map<std::string_view, double> country_sizes = {
            {"USA", 9833517}, {"Canada", 9984670}, {"Australia", 7692024}, {"China", 9596961}, {"Poland", 312696},
        };

        constexpr double KM_TO_MI = 0.386102; // Conversion factor

        const double total_km = std::accumulate(country_sizes.begin(), country_sizes.end(), 0.0,
                                                [](double sum, const auto& entry) { return sum + entry.second; });

        const auto total_mi = total_km * KM_TO_MI;

        // Table headers
        std::println("{:<15} | {:>15} | {:>15}", "Country", "Size (km²)", "Size (mi²)");
        std::println("{:-<15}-+-{:-<15}-+-{:-<15}", "", "", ""); // Separator

        // Table rows
        for (const auto& [country, size_km] : country_sizes)
        {
            const auto size_mi = size_km * KM_TO_MI;
            std::println("{:<15} | {:>15.0f} | {:>15.2f}", country, size_km, size_mi);
        }

        // Footer
        std::println("{:-<15}-+-{:-<15}-+-{:-<15}", "", "", ""); // Separator
        std::println("{:<15} | {:>15.0f} | {:>15.2f}", "Total", total_km, total_mi);
    }

    return 0;
}