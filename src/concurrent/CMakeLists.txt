
add_executable(spsc-triple-buffer spsc-triple-buffer.cpp)
add_executable(publish-shared-ptr publish-shared-ptr.cpp)
add_executable(cond-var cond-var.cpp)
add_executable(latch latch.cpp)
add_executable(barrier barrier.cpp)
add_executable(counting-sema counting-sema.cpp)
add_executable(binary-sema binary-sema.cpp)

target_compile_features(spsc-triple-buffer PUBLIC cxx_std_20)
target_compile_features(publish-shared-ptr PUBLIC cxx_std_20)
target_compile_features(cond-var PUBLIC cxx_std_20)
target_compile_features(latch PUBLIC cxx_std_20)
target_compile_features(barrier PUBLIC cxx_std_20)
target_compile_features(counting-sema PUBLIC cxx_std_20)
target_compile_features(binary-sema PUBLIC cxx_std_20)

