add_executable(mdspan mdspan.cpp)
target_compile_features(mdspan PUBLIC cxx_std_23)

add_executable(ranges ranges.cpp)
target_compile_features(ranges PUBLIC cxx_std_23)

add_executable(print print.cpp)
target_compile_features(print PUBLIC cxx_std_23)