#pragma once

#include <functional>
#include "function/function.h"

namespace sage_flow {

/**
 * @brief Sink function type definition
 * 
 * Function that processes messages for output/storage
 */
using SinkFunc = std::function<void(const MultiModalMessage&)>;

/**
 * @brief Sink Function class
 * 
 * Abstract base class for data sink functions.
 * Based on candyFlow's SinkFunction design.
 */
class SinkFunction : public Function {
 public:
  explicit SinkFunction(const std::string& name) 
      : Function(name, FunctionType::Sink) {}

  SinkFunction(std::string name, SinkFunc sink_func)
      : Function(std::move(name), FunctionType::Sink), sink_func_(std::move(sink_func)) {}

  ~SinkFunction() override = default;

  /**
   * @brief Initialize the sink (e.g., open files, connect to external systems)
   */
  virtual void init() = 0;

  /**
   * @brief Execute processes messages for output. Returns empty response.
   * @param response Input response containing messages to sink
   * @return Empty response (sinks consume messages)
   */
  auto execute(FunctionResponse& response) -> FunctionResponse override;

  /**
   * @brief Close the sink and cleanup resources
   */
  virtual void close() = 0;

  /**
   * @brief Set the sink function
   * @param sink_func Function to process each message
   */
  void setSinkFunc(SinkFunc sink_func);

 protected:
  SinkFunc sink_func_;
};

}  // namespace sage_flow
