cmake_minimum_required(VERSION 3.16)
project(torrent-client-prototype CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

include(FetchContent)

set(CPR_USE_SYSTEM_CURL ON)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 2553fc41450301cd09a9271c8d2c3e0cf3546b73) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        peer.h
        torrent_file.h
        peer_connect.cpp
        peer_connect.h
        tcp_connect.cpp
        tcp_connect.h
        torrent_tracker.cpp
        torrent_tracker.h
        torrent_file.cpp
        bencode.cpp
        bencode.h
        message.cpp
        message.h
        byte_tools.h
        byte_tools.cpp
        piece_storage.cpp
        piece_storage.h
        piece.cpp
        piece.h
)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENSSL_LIBRARIES} cpr::cpr)
