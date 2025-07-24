#include "operator/terminal_sink_operator.h"
#include "operator/operator_types.h"
#include <iostream>
#include <stdexcept>

namespace sage_flow {

TerminalSinkOperator::TerminalSinkOperator(SinkFunction sink_func)
    : Operator(OperatorType::kSink, "TerminalSink"), sink_func_(std::move(sink_func)) {
  if (!sink_func_) {
    throw std::invalid_argument("SinkFunction cannot be null");
  }
}

auto TerminalSinkOperator::process(Response& input_record, int slot) -> bool {
  incrementProcessedCount();
  
  try {
    const auto& messages = input_record.getMessages();
    for (const auto& message_ptr : messages) {
      if (message_ptr) {
        sink_func_(*message_ptr);
      }
    }
    
    incrementOutputCount();
    return true;
  } catch (const std::exception& e) {
    // Log error and continue processing
    std::cerr << "TerminalSinkOperator error: " << e.what() << '\n';
    return false;
  }
}

auto TerminalSinkOperator::open() -> void {
  Operator::open();
}

auto TerminalSinkOperator::close() -> void {
  Operator::close();
}

// Factory function implementation
auto CreateTerminalSink(const std::function<void(const MultiModalMessage&)>& sink_func) 
    -> std::unique_ptr<TerminalSinkOperator> {
  return std::make_unique<TerminalSinkOperator>(sink_func);
}

}  // namespace sage_flow
