cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

find_package(Protobuf)

include_directories(${INCLUDE_DIRECTORIES} ${Protobuf_INCLUDE_DIRS})
include_directories(${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_BINARY_DIR}/..)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS random.proto mission.proto)

add_library(freeserf_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(freeserf_proto SYSTEM INTERFACE ${Protobuf_INCLUDE_DIRS})
target_include_directories(freeserf_proto INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
