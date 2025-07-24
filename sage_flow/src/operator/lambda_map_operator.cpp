#include "operator/lambda_map_operator.h"
#include "operator/operator_types.h"
#include <stdexcept>

namespace sage_flow {

LambdaMapOperator::LambdaMapOperator(MapFunction map_func)
    : Operator(OperatorType::kMap, "LambdaMap"), map_func_(std::move(map_func)) {
  if (!map_func_) {
    throw std::invalid_argument("MapFunction cannot be null");
  }
}

auto LambdaMapOperator::process(Response& input_record, int slot) -> bool {
  incrementProcessedCount();
  
  try {
    const auto messages = input_record.getMessages();
    std::vector<std::unique_ptr<MultiModalMessage>> transformed_messages;
    
    for (const auto& message_ptr : messages) {
      if (message_ptr) {
        auto transformed = map_func_(*message_ptr);
        if (transformed) {
          transformed_messages.push_back(std::move(transformed));
        }
      }
    }
    
    if (!transformed_messages.empty()) {
      Response output_record(std::move(transformed_messages));
      emit(0, output_record);
      incrementOutputCount();
      return true;
    }
    
    return false;
  } catch (const std::exception& e) {
    // Log error and continue processing
    return false;
  }
}

auto LambdaMapOperator::open() -> void {
  Operator::open();
}

auto LambdaMapOperator::close() -> void {
  Operator::close();
}

// Factory function implementation
auto CreateLambdaMap(const MapFunction& func) -> std::unique_ptr<LambdaMapOperator> {
  return std::make_unique<LambdaMapOperator>(func);
}

}  // namespace sage_flow
