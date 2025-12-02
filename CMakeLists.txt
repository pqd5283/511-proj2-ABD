cmake_minimum_required(VERSION 3.20)
project(ABDProj LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# include all directories
include_directories(
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/shared
    ${PROJECT_SOURCE_DIR}/proto
)

# need to generate these but i think this is what itll look like
set(PROTO_SRCS
    proto/abdalgorithm.pb.cc
    proto/abdalgorithm.grpc.pb.cc
)
# make the client executable
add_executable(abd_client
    client/client_main.c
    client/client_funcs.c
    client/client_rpc.cpp
    ${PROTO_SRCS}
)

target_link_libraries(abd_client
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
)
# make the server executable 
add_executable(abd_server
    server/server_funcs.c
    server/server_rpc.cpp
    ${PROTO_SRCS}
)

target_link_libraries(abd_server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
)
