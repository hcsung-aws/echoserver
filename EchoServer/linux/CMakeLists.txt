cmake_minimum_required(VERSION 3.10)
project(EchoServer)

# Set C++11 as required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost package (used for networking and threading)
find_package(Boost 1.67.0 REQUIRED COMPONENTS system thread)

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})

# Collect source files
set(SOURCE_FILES
    ../EchoServer.cpp
    ../packethandler.cpp
    ../session.cpp
    ../sessionmanager.cpp
)

# Collect header files
set(HEADER_FILES
    ../packet.h
    ../packethandler.h
    ../server.h
    ../session.h
    ../sessionmanager.h
)

# Add the executable
add_executable(EchoServer ${SOURCE_FILES} ${HEADER_FILES})

# Link against Boost libraries
target_link_libraries(EchoServer
    ${Boost_LIBRARIES}
    pthread
)

# Include the parent directory for header files
target_include_directories(EchoServer PRIVATE ..)

# Set compiler flags
target_compile_options(EchoServer PRIVATE -Wall -Wextra)

# Add install rule
install(TARGETS EchoServer DESTINATION bin)

# Generate compile_commands.json for tooling support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output binary to linux/bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)