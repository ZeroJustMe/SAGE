#include "operator/lambda_source_operator.h"
#include "operator/operator_types.h"
#include <stdexcept>

namespace sage_flow {

LambdaSourceOperator::LambdaSourceOperator(GeneratorFunction generator, size_t max_messages)
    : Operator(OperatorType::kSource, "LambdaSource"), 
      generator_(std::move(generator)), 
      max_messages_(max_messages) {
  if (!generator_) {
    throw std::invalid_argument("GeneratorFunction cannot be null");
  }
}

auto LambdaSourceOperator::process(Response& input_record, int slot) -> bool {
  incrementProcessedCount();
  
  // Check if we've reached the maximum number of messages
  if (max_messages_ > 0 && generated_count_ >= max_messages_) {
    has_more_data_ = false;
    return false;
  }
  
  try {
    auto generated_message = generator_();
    if (generated_message) {
      std::vector<std::unique_ptr<MultiModalMessage>> messages;
      messages.push_back(std::move(generated_message));
      
      Response output_record(std::move(messages));
      emit(0, output_record);
      
      generated_count_++;
      incrementOutputCount();
      return true;
    }
    // Generator returned null, no more data
    has_more_data_ = false;
    return false;
  } catch (const std::exception& e) {
    // Generator threw an exception, consider this end of data
    has_more_data_ = false;
    return false;
  }
}

auto LambdaSourceOperator::open() -> void {
  Operator::open();
  generated_count_ = 0;
  has_more_data_ = true;
}

auto LambdaSourceOperator::close() -> void {
  Operator::close();
  has_more_data_ = false;
}

auto LambdaSourceOperator::hasNext() const -> bool {
  return has_more_data_ && (max_messages_ == 0 || generated_count_ < max_messages_);
}

auto LambdaSourceOperator::getGeneratedCount() const -> size_t {
  return generated_count_;
}

// Factory function implementation
auto CreateLambdaSource(const GeneratorFunction& generator, size_t max_messages) 
    -> std::unique_ptr<LambdaSourceOperator> {
  return std::make_unique<LambdaSourceOperator>(generator, max_messages);
}

}  // namespace sage_flow
