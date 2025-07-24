#!/usr/bin/env python3
"""
DataStream API Python Example

This script demonstrates how to use the SAGE DataStream API from Python,
following the patterns established in sage_examples and compatible with sage_core.
"""

import sys
import os

# Add the build directory to Python path to import our module
sys.path.insert(0, '/home/xinyan/SAGE/sage_flow/build')

try:
    import sage_flow_datastream as sfd
    print("✅ Successfully imported sage_flow_datastream module")
except ImportError as e:
    print(f"❌ Failed to import sage_flow_datastream: {e}")
    sys.exit(1)

def demonstrate_basic_datastream():
    """Basic DataStream API demonstration"""
    print("\n=== Basic DataStream API Example ===")
    
    # Create environment (similar to sage_core.environment patterns)
    env = sfd.Environment()
    
    # Message counter for source
    counter = 0
    
    def message_generator():
        nonlocal counter
        counter += 1
        if counter <= 3:
            return sfd.create_message(f"Hello World {counter}")
        return None
    
    def text_processor(msg):
        # Transform the message content
        new_content = f"Processed: {msg.get_content()}"
        return sfd.create_message(new_content)
    
    def length_filter(msg):
        # Filter messages based on content length
        return len(msg.get_content()) > 10
    
    def output_sink(msg):
        # Output the final message
        print(f"📤 Final Output: {msg}")
    
    print("🚀 Creating and executing DataStream pipeline...")
    
    # Create and execute the pipeline (fluent interface)
    env.create_datastream() \
       .from_source(message_generator) \
       .map(text_processor) \
       .filter(length_filter) \
       .sink(output_sink)
    
    print("✅ Pipeline execution completed!")

def demonstrate_advanced_datastream():
    """Advanced DataStream API demonstration with more complex transformations"""
    print("\n=== Advanced DataStream API Example ===")
    
    env = sfd.Environment()
    
    # More complex source generator
    import random
    word_list = ["apple", "banana", "cherry", "date", "elderberry"]
    count = 0
    
    def word_generator():
        nonlocal count
        count += 1
        if count <= 5:
            word = random.choice(word_list)
            return sfd.create_message(f"fruit_{count}: {word}")
        return None
    
    def uppercase_processor(msg):
        # Transform to uppercase
        content = msg.get_content().upper()
        return sfd.create_message(content)
    
    def add_metadata(msg):
        # Add some metadata
        content = f"[METADATA] {msg.get_content()} [LENGTH: {len(msg.get_content())}]"
        return sfd.create_message(content)
    
    def vowel_filter(msg):
        # Filter messages that contain vowels
        return any(vowel in msg.get_content().lower() for vowel in 'aeiou')
    
    def detailed_sink(msg):
        print(f"📋 Advanced Output: UID={msg.get_uid()}, Content='{msg.get_content()}'")
    
    print("🚀 Creating advanced DataStream pipeline...")
    
    # Chain multiple operations
    env.create_datastream() \
       .from_source(word_generator) \
       .map(uppercase_processor) \
       .map(add_metadata) \
       .filter(vowel_filter) \
       .sink(detailed_sink)
    
    print("✅ Advanced pipeline execution completed!")

def demonstrate_integration_patterns():
    """Demonstrate integration patterns following sage_core conventions"""
    print("\n=== SAGE Integration Patterns ===")
    
    print("🔗 DataStream API Features:")
    print("   ✓ Fluent interface (.from_source().map().filter().sink())")
    print("   ✓ Lambda function support for transformations")
    print("   ✓ Message-based data flow (compatible with MultiModalMessage)")
    print("   ✓ Python/C++ interoperability via pybind11")
    print("   ✓ Compatible with sage_core environment patterns")
    print("   ✓ Supports chainable operations as specified in TODO requirements")
    
    print("\n📚 Integration Points:")
    print("   • sage_core.api.datastream - Compatible API surface")
    print("   • sage_core.environment.BaseEnvironment - Environment pattern")
    print("   • sage_examples - Same usage patterns")
    print("   • sage_memory - Future vector operations")
    print("   • sage_libs - RAG and agent integrations")

def main():
    """Main demonstration function"""
    try:
        print("🎯 SAGE DataStream API - Python Integration Demo")
        print("Following TODO requirements and sage_core compatibility\n")
        
        # Run demonstrations
        demonstrate_basic_datastream()
        demonstrate_advanced_datastream()
        demonstrate_integration_patterns()
        
        print(f"\n🎉 All demonstrations completed successfully!")
        print("📋 Ready for integration with full SAGE framework")
        
    except Exception as e:
        print(f"❌ Error during demonstration: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
