cmake_minimum_required(VERSION 3.16)

# 设置conda编译器（必须在project()之前）
if(DEFINED ENV{CONDA_PREFIX})
  message(STATUS "Found conda environment: $ENV{CONDA_PREFIX}")
  set(CMAKE_C_COMPILER "$ENV{CONDA_PREFIX}/bin/x86_64-conda-linux-gnu-gcc")
  set(CMAKE_CXX_COMPILER "$ENV{CONDA_PREFIX}/bin/x86_64-conda-linux-gnu-g++")
  message(STATUS "Using conda C compiler: ${CMAKE_C_COMPILER}")
  message(STATUS "Using conda C++ compiler: ${CMAKE_CXX_COMPILER}")
endif()

project(sage_flow VERSION 1.0.0 LANGUAGES CXX)

# C++17标准强制要求
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Google C++ Style编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(
    -Wall -Wextra -Werror        # 警告即错误
    -Wshadow -Wconversion        # 额外警告
    -Wno-unused-parameter        # 允许未使用参数
  )
  
  # Sanitizers已禁用 - 可在需要时重新启用
  # if(CMAKE_BUILD_TYPE MATCHES "Debug")
  #   message(STATUS "Adding sanitizers for debug build")
  #   add_compile_options(
  #     -fsanitize=address           # AddressSanitizer - 检测内存错误
  #     -fsanitize=undefined         # UndefinedBehaviorSanitizer - 检测未定义行为
  #     -fno-omit-frame-pointer      # 保留栈帧指针以获得更好的栈跟踪
  #     -g                           # 调试信息
  #   )
  #   add_link_options(
  #     -fsanitize=address
  #     -fsanitize=undefined
  #   )
  # endif()
endif()

# clang-tidy集成
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY 
    ${CLANG_TIDY_EXE};
    -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy;
    -header-filter=.*
  )
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 使用当前激活的conda环境的Python
if(DEFINED ENV{CONDA_PREFIX})
  # Python配置（编译器已在project()之前设置）
  set(Python3_EXECUTABLE "$ENV{CONDA_PREFIX}/bin/python3")
  set(Python3_INCLUDE_DIR "$ENV{CONDA_PREFIX}/include/python3.11")
  set(Python3_LIBRARY "$ENV{CONDA_PREFIX}/lib/libpython3.11.so")
endif()

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Using Python: ${Python3_EXECUTABLE}")

# 查找依赖
find_package(pybind11 REQUIRED)

# 核心库源文件
set(SAGE_FLOW_SOURCES
  src/api/datastream.cpp
  src/engine/stream_engine.cpp
  src/engine/execution_graph.cpp
  src/message/vector_data.cpp
  src/message/retrieval_context.cpp
  src/message/multimodal_message_core.cpp
  src/function/function.cpp
  src/function/map_function.cpp
  src/function/filter_function.cpp
  src/function/sink_function.cpp
  src/operator/operator.cpp
  src/operator/map_operator.cpp
  src/operator/filter_operator.cpp
  src/operator/sink_operator.cpp
  src/operator/terminal_sink_operator.cpp
  src/operator/file_sink_operator.cpp
  src/operator/vector_store_sink_operator.cpp
  src/index/index_operators.cpp
  src/index/brute_force_index.cpp
  src/index/hnsw.cpp
  src/index/ivf.cpp
  src/memory/memory_pool.cpp
  src/environment/sage_flow_environment.cpp
  src/sources/file_data_source.cpp
  src/sources/stream_data_source.cpp
  src/sources/kafka_data_source.cpp
  src/sources/data_source_factory.cpp
)

# 目标库定义
add_library(sage_flow_core SHARED ${SAGE_FLOW_SOURCES})

target_include_directories(sage_flow_core
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# C++17标准
target_compile_features(sage_flow_core PUBLIC cxx_std_17)

# DataStream Python bindings - Main module
pybind11_add_module(sage_flow_datastream
  src/python/datastream_bindings.cpp
  src/python/operators/terminal_sink_operator_bindings.cpp
  src/python/operators/file_sink_operator_bindings.cpp
  src/python/operators/vector_store_sink_operator_bindings.cpp
)

target_link_libraries(sage_flow_datastream PRIVATE sage_flow_core)

target_include_directories(sage_flow_datastream PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 安装规则
install(TARGETS sage_flow_core
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)
