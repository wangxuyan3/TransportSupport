# CMakeList.txt: TransportSupportCmake 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("TransportSupport")

include_directories (./ ./SocketTransport)

# 将源代码添加到此项目的可执行文件。
add_executable (TransportSupport "main.cpp")

# Add block directories
add_subdirectory(SocketTransport) #添加SocketTransport文件夹中的SocketTransport库, 注意库名和文件夹名相同

target_link_libraries(${PROJECT_NAME} SocketTransport)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TransportSupport PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
