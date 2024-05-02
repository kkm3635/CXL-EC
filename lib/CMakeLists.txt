add_library(cxlEC STATIC cxlEC.cpp switch.cpp ConsistentHashing.cpp)

target_include_directories(cxlEC PUBLIC ${CMAKE_SOURCE_DIR}/includes)

target_compile_options(cxlEC PRIVATE -Wall -g)

# target_link_options(cxlEC PRIVATE -fsanitize=address)