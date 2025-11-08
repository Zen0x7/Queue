cmake_minimum_required(VERSION 3.25)
project(queue)

option(ENABLE_TESTS "Enable testing" ON)
option(ENABLE_DEBUG "Enable debugging" ON)
option(ENABLE_STATIC_LINKING "Enable static linking feature" OFF)
option(ENABLE_NATIVE_OPTIMIZATION "Enable native CPU optimization feature" OFF)

if (ENABLE_STATIC_LINKING)
    message(STATUS "Using ENABLE_STATIC_LINKING = ${ENABLE_STATIC_LINKING}")
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    set(BOOST_ALL_NO_LIB ON)
    set(BUILD_SHARED_LIBS OFF)
    set(OPENSSL_USE_STATIC_LIBS TRUE)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(ARCH_LIB_DIR "/usr/lib/aarch64-linux-gnu")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(ARCH_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    else()
        message(FATAL_ERROR "Arquitectura no soportada: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(OPENSSL_CRYPTO_LIBRARY "${ARCH_LIB_DIR}/libcrypto.a")
    set(OPENSSL_SSL_LIBRARY "${ARCH_LIB_DIR}/libssl.a")
endif ()


message(STATUS "Using ENABLE_STATIC_LINKING = ${ENABLE_STATIC_LINKING}")
message(STATUS "Using ENABLE_NATIVE_OPTIMIZATION = ${ENABLE_NATIVE_OPTIMIZATION}")

set(CMAKE_CXX_STANDARD 23)

if (ENABLE_DEBUG)
    add_definitions(-DDEBUG_ENABLED)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (ENABLE_STATIC_LINKING)
    set(OpenSSL_USE_STATIC_LIBS TRUE)

    if (ENABLE_NATIVE_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native -mtune=native -g0 -static -static-libgcc -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -march=native -mtune=native -g0 -static -static-libgcc -DNDEBUG")
    else ()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -g0 -static -static-libgcc -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -g0 -static -static-libgcc -DNDEBUG")
    endif ()
else ()
    if (ENABLE_NATIVE_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native -mtune=native -g0 -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -march=native -mtune=native -g0 -DNDEBUG")
    else ()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()
endif ()

find_package(Boost REQUIRED COMPONENTS
        program_options
        json
        charconv
)

find_package(laserpants_dotenv)

include_directories(src/include)
list(APPEND USED_LIBRARIES ${Boost_LIBRARIES} bcrypt laserpants::dotenv)

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

    if(ENABLE_STATIC_LINKING)
        target_compile_options(tests PRIVATE -static -static-libgcc -static-libstdc++)
        target_link_options(tests PRIVATE -static -static-libgcc -static-libstdc++)
    endif ()

    target_link_libraries(tests GTest::gtest_main queue_core ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

    include(GoogleTest)
    gtest_discover_tests(tests)
endif ()

add_executable(server deps/asio.cxx deps/beast.cxx deps/redis.cxx deps/mysql.cxx main.cpp)
if (ENABLE_STATIC_LINKING)
    target_compile_options(server PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(server PRIVATE -static -static-libgcc -static-libstdc++)
endif ()
target_link_libraries(server queue_core ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
