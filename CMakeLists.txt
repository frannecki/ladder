cmake_minimum_required(VERSION 3.10.0)
project(ladder)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lz -std=c++11")

option(DEBUG OFF)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

file(GLOB LADDER_SOURCES ladder/src/*.cpp ladder/src/*/*.cpp)
add_library(ladder SHARED ${LADDER_SOURCES})
target_include_directories(ladder PUBLIC ${Protobuf_INCLUDE_DIRS} ladder/include)
target_link_libraries(ladder
  ${Protobuf_LIBRARIES}
  Threads::Threads
  ${OPENSSL_LIBRARIES}
)

file(GLOB LADDER_CLIENT_SOURCES ladder_client/src/*.cpp)
add_library(ladder_client SHARED ${LADDER_CLIENT_SOURCES})
target_include_directories(ladder_client PUBLIC
  ladder/include
  ladder_client/include
)
target_link_libraries(ladder_client ladder Threads::Threads)

file(GLOB PROTO_SOURCES tests/proto/*.proto)
protobuf_generate_cpp(PROTO_SRCS, PROTO_HDRS ${PROTO_SOURCES})

file(GLOB LADDER_UNIT_TESTS_SOURCES tests/unittests/*.cpp tests/unittests/ladtest/*.cpp tests/proto/*.pb.cc)
add_executable(ladder_unit_tests ${LADDER_UNIT_TESTS_SOURCES})
target_include_directories(ladder_unit_tests PUBLIC ladder/include tests)
target_link_libraries(ladder_unit_tests ladder)

list(APPEND TESTS_EXECUTABLES "logger" "server" "stop_server" "proto_server" "file_server" "ssl_server" "event_poller")
foreach(child ${TESTS_EXECUTABLES})
    file(GLOB CUR_SOURCES tests/${child}/*.cpp tests/proto/*.pb.cc)
    add_executable(test_${child} ${CUR_SOURCES})
    target_include_directories(test_${child} PUBLIC ladder/include tests)
    target_link_libraries(test_${child} ladder)
endforeach()

list(APPEND TESTS_CLIENT_EXECUTABLES "timer" "client" "tcp_client" "ssl_client" "mass_clients" "event_loop_thread")
foreach(child ${TESTS_CLIENT_EXECUTABLES})
    file(GLOB CUR_SOURCES tests_client/${child}/*.cpp)
    add_executable(test_client_${child} ${CUR_SOURCES})
    target_include_directories(test_client_${child} PUBLIC
        ladder/include
        ladder_client/include
    )
    target_link_libraries(test_client_${child} ladder ladder_client Threads::Threads)
endforeach()

file(GLOB HTTP_SERVER_SOURCES examples/http/*.cpp)
add_executable(test_http_server ${HTTP_SERVER_SOURCES})
target_include_directories(test_http_server PUBLIC ladder/include)
target_link_libraries(test_http_server ladder)

if(DEBUG)
    target_compile_definitions(ladder DEBUG)
    target_compile_definitions(ladder_client DEBUG)
    target_compile_definitions(ladder_unit_tests DEBUG)
endif()

install(TARGETS ladder DESTINATION lib)
install(DIRECTORY ladder/include/ DESTINATION include/ladder FILES_MATCHING PATTERN "*.h*")
install(TARGETS ladder_client DESTINATION lib)
install(DIRECTORY ladder_client/include/ DESTINATION include/ladder FILES_MATCHING PATTERN "*.h*")
