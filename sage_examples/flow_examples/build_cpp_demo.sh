#!/bin/bash

# Build script for SAGE DataStream C++ Demo
# Usage: ./build_cpp_demo.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔨 Building SAGE DataStream C++ Demo${NC}"

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SAGE_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$SAGE_ROOT/build"
SAGE_FLOW_DIR="$SAGE_ROOT/sage_flow"

echo -e "${YELLOW}📁 Directories:${NC}"
echo "   SAGE Root: $SAGE_ROOT"
echo "   Build Dir: $BUILD_DIR"
echo "   SAGE Flow: $SAGE_FLOW_DIR"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}❌ Build directory not found: $BUILD_DIR${NC}"
    echo -e "${YELLOW}💡 Please run 'make' in the SAGE root directory first${NC}"
    exit 1
fi

# Check if SAGE Flow library exists
SAGE_FLOW_LIB="$BUILD_DIR/libsage_flow_core.so"
if [ ! -f "$SAGE_FLOW_LIB" ]; then
    echo -e "${RED}❌ SAGE Flow library not found: $SAGE_FLOW_LIB${NC}"
    echo -e "${YELLOW}💡 Please run 'make' in the SAGE root directory first${NC}"
    exit 1
fi

# Compilation settings
CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -Wall -Wextra -O2 -fPIC"
INCLUDES="-I$SAGE_FLOW_DIR/include"
LIBDIRS="-L$BUILD_DIR"
LIBS="-lsage_flow_core"
RPATH="-Wl,-rpath,$BUILD_DIR"

# Source and output
SOURCE_FILE="$SCRIPT_DIR/datastream_cpp_demo.cpp"
OUTPUT_FILE="$SCRIPT_DIR/datastream_cpp_demo"

echo -e "${YELLOW}🔧 Compilation Command:${NC}"
echo "$CXX $CXXFLAGS $INCLUDES $LIBDIRS $SOURCE_FILE $LIBS $RPATH -o $OUTPUT_FILE"

# Compile
echo -e "${BLUE}🔨 Compiling...${NC}"
$CXX $CXXFLAGS $INCLUDES $LIBDIRS "$SOURCE_FILE" $LIBS $RPATH -o "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Compilation successful!${NC}"
    echo -e "${YELLOW}📝 Output executable: $OUTPUT_FILE${NC}"
    echo -e "${BLUE}🚀 To run the demo:${NC}"
    echo "   cd $SCRIPT_DIR"
    echo "   ./datastream_cpp_demo"
    echo ""
    echo -e "${YELLOW}📚 This demo shows the exact pattern you requested:${NC}"
    echo "   env.create_datastream()"
    echo "      .from_source(message_generator)"
    echo "      .map(text_processor)"
    echo "      .filter(length_filter)"
    echo "      .sink(output_sink)"
else
    echo -e "${RED}❌ Compilation failed!${NC}"
    exit 1
fi
