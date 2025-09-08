add_executable(mdspan mdspan.cpp)
target_compile_features(mdspan PUBLIC cxx_std_23)

add_executable(ranges ranges.cpp)
target_compile_features(ranges PUBLIC cxx_std_23)

add_executable(print print.cpp)
target_compile_features(print PUBLIC cxx_std_23)

add_executable(optional-monadic optional-monadic.cpp)
target_compile_features(optional-monadic PUBLIC cxx_std_23)

add_executable(generator generator.cpp)
target_compile_features(generator PUBLIC cxx_std_23)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(generator PRIVATE -stdlib=libstdc++)
    target_link_options(generator PRIVATE -stdlib=libstdc++)
endif()

add_executable(ranges-libstdc++ ranges-libstdc++.cpp)
target_compile_features(ranges-libstdc++ PUBLIC cxx_std_23)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(ranges-libstdc++ PRIVATE -stdlib=libstdc++)
    target_link_options(ranges-libstdc++ PRIVATE -stdlib=libstdc++)
endif()