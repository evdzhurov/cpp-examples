add_executable(ip_packet_parser ip_packet_parser.cpp)
target_compile_options(ip_packet_parser PUBLIC -fsanitize=address,undefined,leak -g -fno-omit-frame-pointer)
target_link_options(ip_packet_parser   PUBLIC -fsanitize=address,undefined,leak)