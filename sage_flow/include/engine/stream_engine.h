#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdint>
#include <unordered_map>

#include "execution_graph.h"

namespace sage_flow {

class Operator;
class ExecutionGraph;

/**
 * @brief Stream processing engine for SAGE flow
 * 
 * Manages the execution of stream processing operators and provides
 * different execution modes for optimal performance.
 */
class StreamEngine {
 public:
  using GraphId = size_t;
  
  enum class ExecutionMode : std::uint8_t {
    SINGLE_THREADED = 0,
    MULTI_THREADED = 1,
    ASYNC = 2
  };
  
  enum class GraphState : std::uint8_t {
    UNKNOWN = 0,
    SUBMITTED = 1,
    RUNNING = 2,
    COMPLETED = 3,
    STOPPED = 4,
    ERROR = 5
  };
  
  explicit StreamEngine(ExecutionMode mode = ExecutionMode::MULTI_THREADED);
  ~StreamEngine();
  
  // Prevent copying
  StreamEngine(const StreamEngine&) = delete;
  auto operator=(const StreamEngine&) -> StreamEngine& = delete;
  
  // Delete moving due to atomic members
  StreamEngine(StreamEngine&&) = delete;
  auto operator=(StreamEngine&&) -> StreamEngine& = delete;
  
  // Core execution methods
  auto execute(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto start() -> void;
  auto stop() -> void;
  auto isRunning() const -> bool;
  
  // Graph management
  auto createGraph() -> std::shared_ptr<ExecutionGraph>;
  auto submitGraph(const std::shared_ptr<ExecutionGraph>& graph) -> GraphId;
  auto executeGraph(GraphId graph_id) -> void;
  auto executeGraphAsync(GraphId graph_id) -> void;
  auto stopGraph(GraphId graph_id) -> void;
  
  // Configuration
  auto setExecutionMode(ExecutionMode mode) -> void;
  auto getExecutionMode() const -> ExecutionMode;
  auto setThreadCount(size_t thread_count) -> void;
  auto getThreadCount() const -> size_t;
  
  // Performance monitoring
  auto getTotalProcessedMessages() const -> uint64_t;
  auto getThroughput() const -> double;  // messages per second
  auto resetMetrics() -> void;
  
  // Graph state management
  auto getGraphState(GraphId graph_id) const -> GraphState;
  auto isGraphRunning(GraphId graph_id) const -> bool;
  auto removeGraph(GraphId graph_id) -> void;
  auto getSubmittedGraphs() const -> std::vector<GraphId>;
  
 private:
  ExecutionMode execution_mode_;
  size_t thread_count_ = 1;
  std::atomic<bool> is_running_{false};
  std::atomic<uint64_t> total_processed_messages_{0};
  std::vector<std::thread> worker_threads_;
  
  // Graph management
  GraphId next_graph_id_ = 0;
  std::unordered_map<GraphId, std::shared_ptr<ExecutionGraph>> submitted_graphs_;
  std::unordered_map<GraphId, GraphState> graph_states_;
  
  // Internal execution methods
  auto executeSingleThreaded(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto executeMultiThreaded(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto executeAsync(std::shared_ptr<ExecutionGraph> graph) -> void;
  
  auto workerThreadFunc(std::shared_ptr<ExecutionGraph> graph, size_t thread_id) -> void;
};

}  // namespace sage_flow
