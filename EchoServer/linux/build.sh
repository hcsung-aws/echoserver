#!/bin/bash

# Set executable permission for this script
if [ ! -x "$0" ]; then
    echo "Setting executable permission for build script..."
    chmod +x "$0"
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build with make using all available cores
echo "Building with make..."
make -j$(nproc)

# Return to the linux directory
cd ..

# Check if build was successful
if [ -f "build/bin/EchoServer" ]; then
    echo "Build complete. Executable is located at: build/bin/EchoServer"
    echo "Run with: ./build/bin/EchoServer <port>"
else
    echo "Build failed. Please check the error messages above."
    exit 1
fi