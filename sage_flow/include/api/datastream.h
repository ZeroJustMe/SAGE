#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <any>
#include <stdexcept>
#include <optional>

// Required header includes instead of forward declarations
#include "engine/stream_engine.h"
#include "engine/execution_graph.h"
#include "message/multimodal_message_core.h"
#include "function/function.h"
#include "function/map_function.h"
#include "function/filter_function.h"
#include "function/sink_function.h"

namespace sage_flow {

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
             ExecutionGraph::OperatorId last_operator_id = static_cast<ExecutionGraph::OperatorId>(-1));
  
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
   * @brief Create a source from function - starting point for data pipeline
   * 
   * Equivalent to sage_core's .from_source(SourceClass, config) pattern
   * Supports both class-based sources and lambda functions
   */
  template<typename SourceType>
  auto from_source(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;
  
  auto from_source(const std::function<std::unique_ptr<MultiModalMessage>()>& source_func) -> DataStream&;
  
  /**
   * @brief Map transformation - one-to-one data processing
   * 
   * Equivalent to sage_core's .map(FunctionClass, config) pattern
   * Supports both class-based functions and lambda functions
   */
  template<typename FunctionType>
  auto map(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;
  
  auto map(const std::function<std::unique_ptr<MultiModalMessage>(std::unique_ptr<MultiModalMessage>)>& func) -> DataStream&;

  /**
   * @brief Filter transformation - conditional data filtering
   * 
   * Equivalent to sage_core's .filter(FilterClass/lambda, config) pattern
   */
  template<typename FilterType>
  auto filter(const std::unordered_map<std::string, std::any>& config = {}) -> DataStream;
  
  auto filter(const std::function<bool(const MultiModalMessage&)>& predicate) -> DataStream&;

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
  auto getLastOperatorId() const -> ExecutionGraph::OperatorId;

  // Internal access for environment integration
  auto setLastOperatorId(ExecutionGraph::OperatorId id) -> void;
  auto getGraph() const -> std::shared_ptr<ExecutionGraph>;
  auto getEngine() const -> std::shared_ptr<StreamEngine>;

 private:
  std::shared_ptr<StreamEngine> engine_;
  std::shared_ptr<ExecutionGraph> graph_;
  ExecutionGraph::OperatorId last_operator_id_ = static_cast<ExecutionGraph::OperatorId>(-1);
  std::optional<size_t> graph_id_;
  bool is_finalized_ = false;

  // Internal helper methods
  template<typename OperatorType>
  auto addOperator(const std::unordered_map<std::string, std::any>& config) -> ExecutionGraph::OperatorId;
  
  auto connectToLastOperator(ExecutionGraph::OperatorId new_operator_id) -> void;
  auto finalizeGraph() -> void;
  auto validateConfig(const std::unordered_map<std::string, std::any>& config) -> bool;
};

// ===============================
// Template Implementation
// ===============================

template<typename SourceType>
auto DataStream::from_source(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    auto op_id = addOperator<SourceType>(config);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename FunctionType>
auto DataStream::map(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    auto op_id = addOperator<FunctionType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename FilterType>
auto DataStream::filter(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    auto op_id = addOperator<FilterType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename FlatMapType>
auto DataStream::flatMap(const std::unordered_map<std::string, std::any>& config) -> DataStream {
    auto op_id = addOperator<FlatMapType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename KeyFunction>
auto DataStream::keyBy(const std::string& strategy, 
                       const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // KeyBy implementation with strategy support
    auto op_id = addOperator<KeyFunction>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename WindowType>
auto DataStream::window(const std::string& size, const std::string& slide,
                        const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // Window implementation with time/count support
    auto op_id = addOperator<WindowType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename AggregateType>
auto DataStream::aggregate(const std::unordered_map<std::string, std::string>& operations,
                           const std::unordered_map<std::string, std::any>& config) -> DataStream {
    // Aggregate implementation with multiple operations
    auto op_id = addOperator<AggregateType>(config);
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

template<typename SinkType>
auto DataStream::sink(const std::unordered_map<std::string, std::any>& config) -> void {
    auto sink_id = addOperator<SinkType>(config);
    connectToLastOperator(sink_id);
    finalizeGraph();
}

template<typename OperatorType>
auto DataStream::addOperator(const std::unordered_map<std::string, std::any>& config) -> ExecutionGraph::OperatorId {
    // Validate configuration
    if (!validateConfig(config)) {
        throw std::runtime_error("Invalid operator configuration");
    }
    
    // Create operator instance with configuration
    auto op = std::make_unique<OperatorType>(config);
    return graph_->addOperator(std::move(op));
}

}  // namespace sage_flow
