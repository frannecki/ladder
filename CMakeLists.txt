cmake_minimum_required(VERSION 3.20.0)
project(ladder)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option(DEBUG OFF)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GTest REQUIRED)

file(GLOB LADDER_SOURCES ladder/src/*.cpp ladder/src/*/*.cpp)
add_library(ladder SHARED ${LADDER_SOURCES})
target_include_directories(ladder PUBLIC ${Protobuf_INCLUDE_DIRS} ladder/include)
target_link_libraries(ladder ${Protobuf_LIBRARIES} ${ZLIB_LIBRARIES} Threads::Threads)

file(GLOB LADDER_CLIENT_SOURCES ladder_client/src/*.cpp)
add_library(ladder_client SHARED ${LADDER_CLIENT_SOURCES})
target_include_directories(ladder_client PUBLIC 
  ladder/include
  ladder_client/include
)
target_link_directories(ladder_client PUBLIC build)
target_link_libraries(ladder_client ladder Threads::Threads)

file(GLOB PROTO_SOURCES tests/proto/*.proto)
protobuf_generate_cpp(PROTO_SRCS, PROTO_HDRS ${PROTO_SOURCES})

if(GTest_FOUND)
    file(GLOB LADDER_UNIT_TESTS_SOURCES tests/unittests/*.cpp tests/proto/*.pb.cc)
    add_executable(ladder_unit_tests ${LADDER_UNIT_TESTS_SOURCES})
    target_include_directories(ladder_unit_tests PUBLIC ladder/include tests)
    target_link_directories(ladder_unit_tests PUBLIC build)
    target_link_libraries(ladder_unit_tests ladder
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )
endif()

list(APPEND TESTS_EXECUTABLES "logger" "server" "proto_server" "event_poller")
foreach(child ${TESTS_EXECUTABLES})
    file(GLOB CUR_SOURCES tests/${child}/*.cpp tests/proto/*.pb.cc)
    add_executable(test_${child} ${CUR_SOURCES})
    target_include_directories(test_${child} PUBLIC ladder/include tests)
    target_link_directories(test_${child} PUBLIC build)
    target_link_libraries(test_${child} ladder)
endforeach()

list(APPEND TESTS_CLIENT_EXECUTABLES "timer" "client" "tcp_client" "mass_clients" "event_loop_thread")
foreach(child ${TESTS_CLIENT_EXECUTABLES})
    file(GLOB CUR_SOURCES tests_client/${child}/*.cpp)
    add_executable(test_client_${child} ${CUR_SOURCES})
    target_include_directories(test_client_${child} PUBLIC
        ladder/include
        ladder_client/include
    )
    target_link_directories(test_client_${child} PUBLIC build)
    target_link_libraries(test_client_${child} ladder ladder_client Threads::Threads)
endforeach()


if(DEBUG)
    target_compile_definitions(ladder DEBUG)
    target_compile_definitions(ladder_client DEBUG)
    if(GTest_FOUND)
        target_compile_definitions(ladder_unit_tests DEBUG)
    endif()
endif()
