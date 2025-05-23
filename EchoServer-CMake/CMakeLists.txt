cmake_minimum_required(VERSION 3.10)
project(EchoServer)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Boost paths explicitly
set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/boost_1_78_0")
set(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/boost_1_78_0")
set(BOOST_LIBRARYDIR "${CMAKE_CURRENT_SOURCE_DIR}/boost_1_78_0/stage/lib")
set(Boost_NO_SYSTEM_PATHS ON)

# Find Boost
find_package(Boost 1.78.0 REQUIRED COMPONENTS system thread)

# Define source files
set(SOURCES
    EchoServer.cpp
    session.cpp    # Add this line to include session.cpp
    packethandler.cpp
    server.cpp
    sessionmanager.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Boost::system
        Boost::thread
)
