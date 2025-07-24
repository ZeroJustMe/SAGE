#pragma once

#include "operator/base_operator.h"
#include "operator/response.h"
#include "message/multimodal_message_core.h"
#include <functional>
#include <memory>

namespace sage_flow {

// Function type definitions
using FilterFunction = std::function<bool(const MultiModalMessage&)>;

/**
 * @brief Lambda-based filter operator for stream filtering
 * 
 * This operator filters messages in the stream based on a predicate function.
 * Follows the SAGE framework design patterns for operator composition.
 */
class LambdaFilterOperator : public Operator {
 public:
  explicit LambdaFilterOperator(FilterFunction filter_func);
  ~LambdaFilterOperator() override = default;
  
  // Prevent copying
  LambdaFilterOperator(const LambdaFilterOperator&) = delete;
  auto operator=(const LambdaFilterOperator&) -> LambdaFilterOperator& = delete;
  
  // Allow moving
  LambdaFilterOperator(LambdaFilterOperator&&) = default;
  auto operator=(LambdaFilterOperator&&) -> LambdaFilterOperator& = default;
  
  auto process(Response& input_record, int slot) -> bool override;
  auto open() -> void override;
  auto close() -> void override;
  
 private:
  FilterFunction filter_func_;
};

// Factory function
auto CreateLambdaFilter(const FilterFunction& predicate) -> std::unique_ptr<LambdaFilterOperator>;

}  // namespace sage_flow
