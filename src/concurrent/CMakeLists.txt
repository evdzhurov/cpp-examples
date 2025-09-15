
add_executable(spsc-triple-buffer spsc-triple-buffer.cpp)
add_executable(publish-shared-ptr publish-shared-ptr.cpp)
add_executable(cond-var cond-var.cpp)

target_compile_features(spsc-triple-buffer PUBLIC cxx_std_20)
target_compile_features(publish-shared-ptr PUBLIC cxx_std_20)
target_compile_features(cond-var PUBLIC cxx_std_20)

# C++20 osyncstream doesn't work with libc++ unless using this flag
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(spsc-triple-buffer PRIVATE -fexperimental-library)
    target_compile_options(publish-shared-ptr PRIVATE -fexperimental-library)
    target_compile_options(cond-var PRIVATE -fexperimental-library)
endif()

