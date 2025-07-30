#include "function/function.h"
#include <stdexcept>

namespace sage_flow {

Function::Function(std::string name, FunctionType type) 
    : name_(std::move(name)), type_(type) {}

auto Function::getName() const -> const std::string& {
  return name_;
}

auto Function::getType() const -> FunctionType {
  return type_;
}

void Function::setName(const std::string& name) {
  name_ = name;
}

void Function::setType(FunctionType type) {
  type_ = type;
}

auto Function::execute(FunctionResponse& response) -> FunctionResponse {
  // Default implementation - pass through
  FunctionResponse result;
  for (auto& message : response.getMessages()) {
    result.addMessage(std::move(message));
  }
  response.clear();
  return result;
}

auto Function::execute(FunctionResponse& left, FunctionResponse& right) -> FunctionResponse {
  // Default implementation for dual-input functions
  throw std::runtime_error("Dual-input execute not implemented for function: " + name_);
}

}  // namespace sage_flow
