add_executable(compile-time-parse-int compile-time-parse-int.cpp)
target_compile_features(compile-time-parse-int PUBLIC cxx_std_23)

add_executable(compile-time-prime-sieve compile-time-prime-sieve.cpp)
target_compile_features(compile-time-prime-sieve PUBLIC cxx_std_20)

add_executable(compile-time-constexpr compile-time-constexpr.cpp)

add_executable(compile-time-merge-intervals compile-time-merge-intervals.cpp)
target_compile_features(compile-time-merge-intervals PUBLIC cxx_std_20)