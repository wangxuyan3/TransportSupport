cmake_minimum_required (VERSION 3.8)
aux_source_directory(. DIR_TRANSPORT_SRCS)
add_library(SocketTransport ${DIR_TRANSPORT_SRCS})