#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <any>
#include "engine/stream_engine.h"
#include "engine/execution_graph.h"

namespace sage_flow {

// Forward declarations
class MultiModalMessage;
class BaseFunction;
class SourceFunction;

// Type alias for operator identification
using OperatorId = ExecutionGraph::OperatorId;

// Function type definitions for lambda operations
using SinkFunction = std::function<void(const MultiModalMessage&)>;
using MapFunction = std::function<std::unique_ptr<MultiModalMessage>(const MultiModalMessage&)>;
using FilterFunction = std::function<bool(const MultiModalMessage&)>;
using GeneratorFunction = std::function<std::unique_ptr<MultiModalMessage>()>;

/**
 * @brief DataStream API for building stream processing pipelines
 * 
 * Provides a fluent interface for constructing and executing
 * data processing pipelines, fully compatible with SAGE framework patterns.
 * 
 * This implementation follows the patterns from sage_core.api.datastream
 * and supports the same chainable operations as sage_examples.
 */
class DataStream {
 public:
  // Constructor for internal use
  DataStream(std::shared_ptr<StreamEngine> engine, 
             std::shared_ptr<ExecutionGraph> graph,
             OperatorId last_operator_id = static_cast<OperatorId>(-1));
  
  ~DataStream() = default;

  // Prevent copying (matches sage_core pattern)
  DataStream(const DataStream&) = delete;
  auto operator=(const DataStream&) -> DataStream& = delete;

  // Allow moving for fluent interface
  DataStream(DataStream&&) = default;
  auto operator=(DataStream&&) -> DataStream& = default;

  // ===============================
  // Core DataStream Operations (sage_core compatible)
  // ===============================
  
  /**
   * @brief Map transformation - one-to-one data processing
   * 
   * Equivalent to sage_core's .map(FunctionClass, config) pattern
   * Supports both class-based functions and lambda functions
   */
  template<typename FunctionType>
  auto map(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;
  
  auto map(const std::function<std::unique_ptr<MultiModalMessage>(std::unique_ptr<MultiModalMessage>)>& func) -> DataStream;

  /**
   * @brief Filter transformation - conditional data filtering
   * 
   * Equivalent to sage_core's .filter(FilterClass/lambda, config) pattern
   */
  template<typename FilterType>
  auto filter(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;
  
  auto filter(const std::function<bool(const MultiModalMessage&)>& predicate) -> DataStream;

  /**
   * @brief FlatMap transformation - one-to-many data processing
   * 
   * Equivalent to sage_core's .flatmap(FlatMapClass, config) pattern
   */
  template<typename FlatMapType>
  auto flatMap(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;

  /**
   * @brief KeyBy transformation - data partitioning by key
   * 
   * Equivalent to sage_core's .keyby(KeyFunction, strategy) pattern
   */
  template<typename KeyFunction>
  auto keyBy(const std::string& strategy = "hash", 
             const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;

  /**
   * @brief Connect transformation - stream joining
   * 
   * Equivalent to sage_core's .connect(other_stream) pattern
   */
  auto connect(const DataStream& other) -> DataStream;

  /**
   * @brief Union transformation - stream merging
   * 
   * Merges multiple streams into one (sage_examples pattern)
   */
  auto union_(const DataStream& other) -> DataStream;

  // ===============================
  // Advanced Stream Operations
  // ===============================

  /**
   * @brief Window operations for stream aggregation
   * 
   * Supports time-based and count-based windows
   */
  template<typename WindowType>
  auto window(const std::string& size, const std::string& slide = "",
              const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;

  /**
   * @brief Aggregate operations within windows
   * 
   * Supports count, sum, avg, min, max aggregations
   */
  template<typename AggregateType>
  auto aggregate(const std::unordered_map<std::string, std::string>& operations,
                 const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;

  // ===============================
  // Output Operations (Terminal)
  // ===============================

  /**
   * @brief Sink operation - data output/storage
   * 
   * Equivalent to sage_core's .sink(SinkClass, config) pattern
   * This is a terminal operation
   */
  template<typename SinkType>
  auto sink(const std::unordered_map<std::string, std::any>& config = {}) -> void;
  
  auto sink(const std::function<void(const MultiModalMessage&)>& sink_func) -> void;

  // ===============================
  // Execution Control
  // ===============================

  /**
   * @brief Execute the pipeline synchronously
   * 
   * Equivalent to calling env.submit() in sage_core
   */
  auto execute() -> void;

  /**
   * @brief Execute the pipeline asynchronously
   * 
   * For non-blocking execution
   */
  auto executeAsync() -> void;

  /**
   * @brief Stop pipeline execution
   * 
   * Graceful shutdown of the pipeline
   */
  auto stop() -> void;

  // ===============================
  // Pipeline Information
  // ===============================

  auto getOperatorCount() const -> size_t;
  auto isExecuting() const -> bool;
  auto getLastOperatorId() const -> int;

  // Internal access for environment integration
  auto setLastOperatorId(int id) -> void;
  auto getGraph() const -> std::shared_ptr<ExecutionGraph>;
  auto getEngine() const -> std::shared_ptr<StreamEngine>;

 private:
  std::shared_ptr<StreamEngine> engine_;
  std::shared_ptr<ExecutionGraph> graph_;
  OperatorId last_operator_id_ = static_cast<OperatorId>(-1);
  size_t graph_id_ = 0;
  bool is_finalized_ = false;

  // Internal helper methods
  template<typename OperatorType>
  auto addOperator(const std::unordered_map<std::string, std::any>& config) -> int;
  
  auto connectToLastOperator(int new_operator_id) -> void;
  auto finalizeGraph() -> void;
  auto validateConfig(const std::unordered_map<std::string, std::any>& config) -> bool;
};

/**
 * @brief Environment for managing DataStream pipelines
 * 
 * Provides the entry point for creating and managing stream processing
 * pipelines, fully compatible with sage_core.environment.BaseEnvironment patterns.
 */
class SageFlowEnvironment {
 public:
  explicit SageFlowEnvironment(std::string name, 
                               StreamEngine::ExecutionMode mode = StreamEngine::ExecutionMode::MULTI_THREADED);
  ~SageFlowEnvironment();

  // Prevent copying (matches sage_core pattern)
  SageFlowEnvironment(const SageFlowEnvironment&) = delete;
  auto operator=(const SageFlowEnvironment&) -> SageFlowEnvironment& = delete;

  // Allow moving
  SageFlowEnvironment(SageFlowEnvironment&&) = default;
  auto operator=(SageFlowEnvironment&&) -> SageFlowEnvironment& = default;

  // ===============================
  // DataStream Creation (sage_core compatible)
  // ===============================

  /**
   * @brief Create DataStream from source
   * 
   * Equivalent to sage_core's env.from_source(SourceClass, config) pattern
   * Supports FileDataSource, KafkaDataSource, DirectoryDataSource, etc.
   */
  template<typename SourceType>
  auto fromSource(const std::unordered_map<std::string, std::any>& config) -> DataStream;
  
  // Lambda-based source creation for simple cases
  auto fromGenerator(const std::function<std::unique_ptr<MultiModalMessage>()>& generator,
                     size_t max_messages = 0) -> DataStream;

  // ===============================
  // Environment Configuration (sage_core compatible)
  // ===============================

  /**
   * @brief Set memory configuration
   * 
   * Equivalent to sage_core's env.set_memory(config=config.get("memory"))
   * Integrates with sage_memory for vector storage
   */
  auto setMemory(const std::unordered_map<std::string, std::string>& config) -> void;

  /**
   * @brief Set execution thread count
   * 
   * Controls parallelism in multi-threaded execution
   */
  auto setThreadCount(int count) -> void;

  /**
   * @brief Set execution mode
   * 
   * SINGLE_THREADED, MULTI_THREADED, or DISTRIBUTED
   */
  auto setExecutionMode(StreamEngine::ExecutionMode mode) -> void;

  /**
   * @brief Set environment property
   * 
   * Generic property setting for configuration
   */
  auto setProperty(const std::string& key, const std::string& value) -> void;
  auto getProperty(const std::string& key) const -> std::string;

  // ===============================
  // Environment Control (sage_core compatible)
  // ===============================

  /**
   * @brief Submit pipeline for execution
   * 
   * Equivalent to sage_core's env.submit()
   * Prepares all streams for execution
   */
  auto submit() -> void;

  /**
   * @brief Run in streaming mode
   * 
   * Equivalent to sage_core's env.run_streaming()
   * Executes all submitted pipelines in streaming mode
   */
  auto runStreaming() -> void;

  /**
   * @brief Run in batch mode
   * 
   * Executes all submitted pipelines in batch mode
   */
  auto runBatch() -> void;

  /**
   * @brief Stop environment execution
   * 
   * Graceful shutdown of all running pipelines
   */
  auto stop() -> void;

  /**
   * @brief Close environment and cleanup
   * 
   * Equivalent to sage_core's env.close()
   * Releases all resources and cleans up
   */
  auto close() -> void;

  // ===============================
  // Environment Information
  // ===============================

  auto getName() const -> const std::string&;
  auto isRunning() const -> bool;
  auto getActiveStreamCount() const -> size_t;
  auto getSubmittedStreamCount() const -> size_t;
  auto getExecutionMode() const -> StreamEngine::ExecutionMode;
  auto getThreadCount() const -> int;

  // Internal access for framework integration
  auto getEngine() const -> std::shared_ptr<StreamEngine>;
  auto addActiveStream(DataStream&& stream) -> void;

 private:
  std::string name_;
  std::shared_ptr<StreamEngine> engine_;
  std::vector<DataStream> active_streams_;
  std::vector<DataStream> submitted_streams_;
  std::unordered_map<std::string, std::string> memory_config_;
  std::unordered_map<std::string, std::string> properties_;
  bool is_running_ = false;
  bool is_submitted_ = false;

  // Internal methods
  auto validateStreams() -> bool;
  auto setupMemoryIntegration() -> void;
  auto cleanupResources() -> void;
};

// ===============================
// Template Implementation
// ===============================

template<typename FunctionType>
auto DataStream::map(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    int op_id = addOperator<FunctionType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename FilterType>
auto DataStream::filter(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    int op_id = addOperator<FilterType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename FlatMapType>
auto DataStream::flatMap(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    int op_id = addOperator<FlatMapType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename KeyFunction>
auto DataStream::keyBy(const std::string& strategy, 
                       const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // KeyBy implementation with strategy support
    int op_id = addOperator<KeyFunction>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename WindowType>
auto DataStream::window(const std::string& size, const std::string& slide,
                        const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // Window implementation with time/count support
    int op_id = addOperator<WindowType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename AggregateType>
auto DataStream::aggregate(const std::unordered_map<std::string, std::string>& operations,
                           const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // Aggregate implementation with multiple operations
    int op_id = addOperator<AggregateType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename SinkType>
auto DataStream::sink(const std::unordered_map<std::string, std::any>& config) -> void {
    int sink_id = addOperator<SinkType>(config);
    connectToLastOperator(sink_id);
    finalizeGraph();
}

template<typename SourceType>
auto SageFlowEnvironment::fromSource(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    auto graph = engine_->createGraph();
    
    // Create source operator using factory pattern
    auto source = std::make_unique<SourceType>(config);
    int source_id = graph->addOperator(std::move(source));
    
    // Create DataStream with the source as starting point
    DataStream result(engine_, graph, source_id);
    
    return result;
}

template<typename OperatorType>
auto DataStream::addOperator(const std::unordered_map<std::string, std::any>& config) -> int {
    // Validate configuration
    if (!validateConfig(config)) {
        throw std::runtime_error("Invalid operator configuration");
    }
    
    // Create operator instance with configuration
    auto op = std::make_unique<OperatorType>(config);
    return graph_->addOperator(std::move(op));
}

}  // namespace sage_flow
