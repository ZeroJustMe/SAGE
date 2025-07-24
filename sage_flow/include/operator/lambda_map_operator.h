#pragma once

#include "operator/base_operator.h"
#include "operator/response.h"
#include "message/multimodal_message_core.h"
#include <functional>
#include <memory>

namespace sage_flow {

// Function type definitions
using MapFunction = std::function<std::unique_ptr<MultiModalMessage>(const MultiModalMessage&)>;

/**
 * @brief Lambda-based map operator for stream transformations
 * 
 * This operator applies a transformation function to each message in the stream.
 * Follows the SAGE framework design patterns for operator composition.
 */
class LambdaMapOperator : public Operator {
 public:
  explicit LambdaMapOperator(MapFunction map_func);
  ~LambdaMapOperator() override = default;
  
  // Prevent copying
  LambdaMapOperator(const LambdaMapOperator&) = delete;
  auto operator=(const LambdaMapOperator&) -> LambdaMapOperator& = delete;
  
  // Allow moving
  LambdaMapOperator(LambdaMapOperator&&) = default;
  auto operator=(LambdaMapOperator&&) -> LambdaMapOperator& = default;
  
  auto process(Response& input_record, int slot) -> bool override;
  auto open() -> void override;
  auto close() -> void override;
  
 private:
  MapFunction map_func_;
};

// Factory function
auto CreateLambdaMap(const MapFunction& func) -> std::unique_ptr<LambdaMapOperator>;

}  // namespace sage_flow
