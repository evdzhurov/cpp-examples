#pragma once

#include <concepts>
#include <expected>
#include <functional>
#include <type_traits>
#include <utility>

// Source: https://www.cppstories.com/2024/pipe-operator/

// Basic Example
// using Function = std::function<std::string(std::string&&)>;

// auto operator|(std::string&& s, Function f) -> std::string
// {
//     return f(std::move(s));
// }

// Generic pipe operator
template <typename T, typename Func>
    requires(std::invocable<Func, T>)
constexpr auto operator|(T&& t, Func&& f) -> typename std::invoke_result_t<Func, T>
{
    return std::invoke(std::forward<Func>(f), std::forward<T>(t));
}

// Concept for an expected-like type
template <typename T>
concept is_expected_like = requires(T t) {
    typename T::value_type; // T defines value_type
    typename T::error_type; // T defines error_type

    // 'requires' forces the result to be checked as opposed to simply if it compiles
    // without 'requires'.
    requires std::is_constructible_v<bool, T>; // Checks if T is convertible to a bool (std::expected defines an
                                               // explicit conversion to bool)
    requires std::same_as<std::remove_cvref<decltype(*t)>, typename T::value_type>;
    requires std::constructible_from<T, std::unexpected<typename T::error_type>>;
};

// Concept for std::expected template instantiation.
template <typename T>
struct std_expected_instance : std::false_type
{
};

template <typename T, typename E>
struct std_expected_instance<std::expected<T, E>> : std::true_type
{
};

template <typename T>
concept is_std_expected_instance = std_expected_instance<std::__remove_cvref_t<T>>::value;

// Generic pipe operator using std::expected (C++23)
template <typename T, typename E, typename Func>
    requires std::invocable<Func, T> && is_std_expected_instance<typename std::invoke_result_t<Func, T>>
constexpr auto operator|(std::expected<T, E>&& exp, Func&& f) -> typename std::invoke_result_t<Func, T>
{
    // Note: moving std::expected<T, E>&& because it is not a forwarding reference!
    using ResultT = std::invoke_result_t<Func, T>;
    return exp ? std::invoke(std::forward<Func>(f), *std::move(exp)) : ResultT{std::unexpected(std::move(exp).error())};
};
