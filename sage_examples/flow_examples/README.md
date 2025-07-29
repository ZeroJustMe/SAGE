# SAGE Flow Examples

This directory contains examples demonstrating the SAGE Flow DataStream API.

## Contents

### C++ Examples

- **`datastream_cpp_demo.cpp`** - Complete C++ demonstration of the DataStream API
- **`build_cpp_demo.sh`** - Build script for C++ examples
- **`datastream_cpp_demo`** - Compiled executable (generated after build)

## DataStream API Pattern

All examples demonstrate the core DataStream API pattern:

```cpp
env.create_datastream()
   .from_source(message_generator)
   .map(text_processor)
   .filter(length_filter)
   .sink(output_sink);
```

## Building and Running

### C++ Example

1. **Build the example:**
   ```bash
   cd /path/to/SAGE/sage_examples/flow_examples
   ./build_cpp_demo.sh
   ```

2. **Run the example:**
   ```bash
   ./datastream_cpp_demo
   ```

### Prerequisites

- SAGE Flow core library must be built first:
  ```bash
  cd /path/to/SAGE
  mkdir -p build && cd build
  cmake ../sage_flow
  make -j$(nproc)
  ```

## Example Features

### Basic DataStream Demo
- Shows fundamental pipeline construction
- Demonstrates lambda function integration
- Message generation, transformation, filtering, and output

### Advanced DataStream Demo  
- Complex multi-stage transformations
- Multiple map operations in sequence
- Content-based filtering
- Metadata addition

### Integration Patterns
- Environment management
- Type-safe message handling
- Fluent interface design
- Compatible with SAGE core patterns

## API Features Demonstrated

✅ **Fluent Interface**
- Chainable method calls
- Natural pipeline construction
- Type-safe operations

✅ **Lambda Support**
- Source generation functions
- Map transformation functions
- Filter predicate functions
- Sink output functions

✅ **Message Flow**
- MultiModalMessage handling
- Content transformation
- UID preservation
- Type-safe operations

✅ **Environment Integration**
- SageFlowEnvironment usage
- Resource management
- Configuration support

## Integration with SAGE Framework

These examples are designed to be compatible with:
- `sage_core.api.datastream` - API surface compatibility
- `sage_core.environment` - Environment patterns
- `sage_examples` - Consistent usage patterns
- `sage_memory` - Vector operations (future)
- `sage_libs` - RAG and agent integrations (future)

## Development Notes

The DataStream API provides:
- Zero-overhead abstractions in C++
- Compile-time type checking
- Direct memory management
- High-performance stream processing
- Easy integration with existing SAGE components

For more complex examples and integration patterns, see the main `sage_examples` directory.
