cmake_minimum_required(VERSION 3.25)
project(queue)

option(ENABLE_TESTS "Enable testing" ON)
option(ENABLE_DEBUG "Enable debugging" ON)

set(CMAKE_CXX_STANDARD 23)

if (ENABLE_DEBUG)
    add_definitions(-DDEBUG_ENABLED)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Boost 1.89.0 REQUIRED COMPONENTS
        program_options
        json
        charconv
)

include_directories(src/include)
list(APPEND USED_LIBRARIES ${Boost_LIBRARIES})

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/objects/*.cpp")

add_library(queue_core ${SOURCES})
target_link_libraries(queue_core ${USED_LIBRARIES})

include(FetchContent)

find_package(OpenSSL REQUIRED)
link_directories(${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})

if (ENABLE_TESTS)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    file(GLOB_RECURSE TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc")

    add_executable(tests
            deps/asio.cxx
            deps/beast.cxx
            deps/redis.cxx
            deps/mysql.cxx
            ${TESTS})

    target_link_libraries(tests GTest::gtest_main queue_core ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

    include(GoogleTest)
    gtest_discover_tests(tests)
endif ()

add_executable(queue deps/asio.cxx deps/beast.cxx deps/redis.cxx deps/mysql.cxx main.cpp)
target_link_libraries(queue queue_core ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
