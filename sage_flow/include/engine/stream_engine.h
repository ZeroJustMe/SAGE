#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>

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
  enum class ExecutionMode {
    SINGLE_THREADED = 0,
    MULTI_THREADED = 1,
    ASYNC = 2
  };
  
  explicit StreamEngine(ExecutionMode mode = ExecutionMode::MULTI_THREADED);
  ~StreamEngine();
  
  // Prevent copying
  StreamEngine(const StreamEngine&) = delete;
  auto operator=(const StreamEngine&) -> StreamEngine& = delete;
  
  // Allow moving
  StreamEngine(StreamEngine&&) = default;
  auto operator=(StreamEngine&&) -> StreamEngine& = default;
  
  // Core execution methods
  auto execute(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto start() -> void;
  auto stop() -> void;
  auto isRunning() const -> bool;
  
  // Graph management
  auto createGraph() -> std::shared_ptr<ExecutionGraph>;
  auto submitGraph(std::shared_ptr<ExecutionGraph> graph) -> size_t;
  auto executeGraph(size_t graph_id) -> void;
  auto executeGraphAsync(size_t graph_id) -> void;
  auto stopGraph(size_t graph_id) -> void;
  
  // Configuration
  auto setExecutionMode(ExecutionMode mode) -> void;
  auto getExecutionMode() const -> ExecutionMode;
  auto setThreadCount(size_t thread_count) -> void;
  auto getThreadCount() const -> size_t;
  
  // Performance monitoring
  auto getTotalProcessedMessages() const -> uint64_t;
  auto getThroughput() const -> double;  // messages per second
  auto resetMetrics() -> void;
  
 private:
  ExecutionMode execution_mode_;
  size_t thread_count_;
  std::atomic<bool> is_running_;
  std::atomic<uint64_t> total_processed_messages_;
  std::vector<std::thread> worker_threads_;
  
  // Internal execution methods
  auto executeSingleThreaded(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto executeMultiThreaded(std::shared_ptr<ExecutionGraph> graph) -> void;
  auto executeAsync(std::shared_ptr<ExecutionGraph> graph) -> void;
  
  auto workerThreadFunc(std::shared_ptr<ExecutionGraph> graph, size_t thread_id) -> void;
};

}  // namespace sage_flow
