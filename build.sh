#!/bin/bash

#   ./build.sh                  # Build with all dependcies
#   ./build.sh --no-hardware    # Build without OAK(DepthAI) and MAVSDK
#   ./build.sh --clean          # Clean and rebuild
#   ./build.sh --help           # Gelp

set -e #Exit on error

BUILD_DIR="build"
ENABLE_OAK="ON"
ENABLE_PIXHAWK="ON"
CLEAN=false

# Parse arguments
for arg in "$@"; do
    case $arg in
        --no-hardware)
            ENABLE_OAK="OFF"
            ENABLE_PIXHAWK="OFF"
            ;;
        --no-oak)
            ENABLE_OAK="OFF"
            ;;
        --no-pixhawk)
            ENABLE_PIXHAWK="OFF"
            ;;
        --clean)
            CLEAN=true
            ;;
        --help)
            echo "Usage: ./build.sh [options]"
            echo ""
            echo "Options:"
            echo "  --no-hardware   Disable both OAK camera and Pixhawk"
            echo "  --no-oak        Disable OAK camera only"
            echo "  --no-pixhawk    Disable Pixhawk only"
            echo "  --clean         Clean build directory before building"
            echo "  --help          Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Clean scirpt
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Configure
echo "Configuring with ENABLE_OAK=$ENABLE_OAK, ENABLE_PIXHAWK=$ENABLE_PIXHAWK"
cmake -B "$BUILD_DIR" \
    -DENABLE_OAK="$ENABLE_OAK" \
    -DENABLE_PIXHAWK="$ENABLE_PIXHAWK" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
echo "Building..."
cmake --build "$BUILD_DIR"



echo ""
echo "Build is complete - Executable: $BUILD_DIR/naina"
