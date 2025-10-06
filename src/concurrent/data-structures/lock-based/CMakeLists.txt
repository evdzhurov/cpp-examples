add_executable(lock-based-queue lock-based-queue.cpp)
add_executable(lock-based-bounded-queue lock-based-bounded-queue.cpp)

target_compile_features(lock-based-bounded-queue PUBLIC cxx_std_20)

target_compile_options(lock-based-bounded-queue PUBLIC -fsanitize=thread -g -fno-omit-frame-pointer)
target_link_options(lock-based-bounded-queue PUBLIC -fsanitize=thread)