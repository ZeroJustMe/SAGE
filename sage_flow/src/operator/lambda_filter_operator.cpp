#include "operator/lambda_filter_operator.h"
#include "operator/operator_types.h"
#include <stdexcept>

namespace sage_flow {

LambdaFilterOperator::LambdaFilterOperator(FilterFunction filter_func)
    : Operator(OperatorType::kFilter, "LambdaFilter"), filter_func_(std::move(filter_func)) {
  if (!filter_func_) {
    throw std::invalid_argument("FilterFunction cannot be null");
  }
}

auto LambdaFilterOperator::process(Response& input_record, int slot) -> bool {
  incrementProcessedCount();
  
  try {
    const auto messages = input_record.getMessages();
    std::vector<std::unique_ptr<MultiModalMessage>> filtered_messages;
    
    for (const auto& message_ptr : messages) {
      if (message_ptr && filter_func_(*message_ptr)) {
        // Create a new message with the same content
        std::unique_ptr<MultiModalMessage> new_message;
        if (message_ptr->isTextContent()) {
          new_message = CreateTextMessage(message_ptr->getUid(), message_ptr->getContentAsString());
        } else if (message_ptr->isBinaryContent()) {
          new_message = CreateBinaryMessage(message_ptr->getUid(), message_ptr->getContentAsBinary());
        }
        
        if (new_message) {
          filtered_messages.push_back(std::move(new_message));
        }
      }
    }
    
    if (!filtered_messages.empty()) {
      Response output_record(std::move(filtered_messages));
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

auto LambdaFilterOperator::open() -> void {
  Operator::open();
}

auto LambdaFilterOperator::close() -> void {
  Operator::close();
}

// Factory function implementation
auto CreateLambdaFilter(const FilterFunction& predicate) -> std::unique_ptr<LambdaFilterOperator> {
  return std::make_unique<LambdaFilterOperator>(predicate);
}

}  // namespace sage_flow
