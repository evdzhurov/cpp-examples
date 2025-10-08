add_executable(lru-cache lru-cache.cpp)
add_executable(array-based-stack array-based-stack.cpp)
add_executable(array-based-queue array-based-queue.cpp)
target_compile_features(lru-cache PUBLIC cxx_std_20)


add_executable(doubly-linked-list doubly-linked-list.cpp)
target_compile_options(doubly-linked-list PUBLIC -fsanitize=address,undefined,leak -g -fno-omit-frame-pointer)
target_link_options(doubly-linked-list   PUBLIC -fsanitize=address,undefined,leak)

add_executable(binary-search-tree binary-search-tree.cpp)
target_compile_options(binary-search-tree PUBLIC -fsanitize=address,undefined,leak -g -fno-omit-frame-pointer)
target_link_options(binary-search-tree   PUBLIC -fsanitize=address,undefined,leak)
