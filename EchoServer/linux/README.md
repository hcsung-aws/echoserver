# EchoServer - Linux Build Instructions

This directory contains the necessary CMake configuration to build the EchoServer project on Linux.

## Prerequisites

- CMake (version 3.10 or higher)
- C++ compiler with C++11 support (GCC or Clang)
- Boost libraries (version 1.67.0 or higher) with the following components:
  - system
  - thread

## Building the Project

### Using the build script

1. Make the build script executable:
   ```
   chmod +x build.sh
   ```

2. Run the build script:
   ```
   ./build.sh
   ```

This will create a `build` directory, configure the project with CMake, and build it using make.

### Manual build

1. Create a build directory:
   ```
   mkdir -p build
   cd build
   ```

2. Configure with CMake:
   ```
   cmake ..
   ```

3. Build with make:
   ```
   make
   ```

## Running the EchoServer

The executable will be located at `build/bin/EchoServer`. Run it with:

```
./build/bin/EchoServer <port>
```

Where `<port>` is the port number you want the server to listen on.

## Installation

To install the EchoServer to your system:

```
cd build
sudo make install
```

This will install the executable to `/usr/local/bin/` by default.