add_executable(lru-cache lru-cache.cpp)
add_executable(array-based-stack array-based-stack.cpp)
add_executable(array-based-queue array-based-queue.cpp)

target_compile_features(lru-cache PUBLIC cxx_std_20)
