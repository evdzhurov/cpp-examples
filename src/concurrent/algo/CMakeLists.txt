add_executable(concurrent-qsort concurrent-qsort.cpp)

target_compile_features(concurrent-qsort PUBLIC cxx_std_20)

target_compile_options(concurrent-qsort PUBLIC -fsanitize=thread -g -fno-omit-frame-pointer)
target_link_options(concurrent-qsort PUBLIC -fsanitize=thread)