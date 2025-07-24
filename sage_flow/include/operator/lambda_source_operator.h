#pragma once

#include "operator/base_operator.h"
#include "operator/response.h"
#include "message/multimodal_message_core.h"
#include <functional>
#include <memory>

namespace sage_flow {

// Function type definitions
using GeneratorFunction = std::function<std::unique_ptr<MultiModalMessage>()>;

/**
 * @brief Lambda-based source operator for data generation
 * 
 * This operator generates data from a function and emits messages into the stream.
 * Follows the SAGE framework design patterns for source operator composition.
 */
class LambdaSourceOperator : public Operator {
 public:
  explicit LambdaSourceOperator(GeneratorFunction generator, size_t max_messages = 0);
  ~LambdaSourceOperator() override = default;
  
  // Prevent copying
  LambdaSourceOperator(const LambdaSourceOperator&) = delete;
  auto operator=(const LambdaSourceOperator&) -> LambdaSourceOperator& = delete;
  
  // Allow moving
  LambdaSourceOperator(LambdaSourceOperator&&) = default;
  auto operator=(LambdaSourceOperator&&) -> LambdaSourceOperator& = default;
  
  auto process(Response& input_record, int slot) -> bool override;
  auto open() -> void override;
  auto close() -> void override;
  
  auto hasNext() const -> bool;
  auto getGeneratedCount() const -> size_t;
  
 private:
  GeneratorFunction generator_;
  size_t max_messages_;
  size_t generated_count_{0};
  bool has_more_data_{true};
};

// Factory function
auto CreateLambdaSource(const GeneratorFunction& generator, size_t max_messages = 0) 
    -> std::unique_ptr<LambdaSourceOperator>;

}  // namespace sage_flow
