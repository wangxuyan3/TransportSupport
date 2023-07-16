cmake_minimum_required (VERSION 3.8)
project ("SocketTransport")

aux_source_directory(. DIR_TRANSPORT_SRCS)
add_library(${PROJECT_NAME} ${DIR_TRANSPORT_SRCS})

if(MINGW)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()