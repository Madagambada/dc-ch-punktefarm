cmake_minimum_required(VERSION 3.13)
project (dc-ch-punktefarm)

include_directories($TOOLCHAIN/include)
link_directories($TOOLCHAIN/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")

set(LINKING_LIBRARIES "-lcurl -lwolfssl -lz -lcares -lnghttp2")

add_executable (dc-ch-punktefarm dc-ch-punktefarm.cpp)
target_link_libraries (dc-ch-punktefarm ${LINKING_LIBRARIES} -static)
