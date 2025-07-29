#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include "message/multimodal_message_core.h"

namespace sage_flow {

/**
 * @brief Function type enumeration
 * 
 * Based on candyFlow's FunctionType design, adapted for SAGE Flow
 */
enum class FunctionType : std::uint8_t {
  None,
  Source,
  Map,
  Filter,
  Sink,
  Join,
  Aggregate,
  Window,
  TopK,
  ITopK,  // Inverted TopK
  FlatMap,
  KeyBy
};

/**
 * @brief Response container for function execution results
 * 
 * Simplified version of candyFlow's Response, adapted for MultiModalMessage
 */
class FunctionResponse {
 public:
  FunctionResponse() = default;
  ~FunctionResponse() = default;
  
  // Disable copy constructor and assignment (can't copy unique_ptr)
  FunctionResponse(const FunctionResponse&) = delete;
  auto operator=(const FunctionResponse&) -> FunctionResponse& = delete;
  
  // Enable move constructor and assignment  
  FunctionResponse(FunctionResponse&&) = default;
  auto operator=(FunctionResponse&&) -> FunctionResponse& = default;
  
  // Add a message to the response
  void addMessage(std::unique_ptr<MultiModalMessage> message) {
    messages_.push_back(std::move(message));
  }
  
  // Get all messages
  auto getMessages() -> std::vector<std::unique_ptr<MultiModalMessage>>& {
    return messages_;
  }
  
  // Get messages (const)
  auto getMessages() const -> const std::vector<std::unique_ptr<MultiModalMessage>>& {
    return messages_;
  }
  
  // Check if response is empty
  auto isEmpty() const -> bool {
    return messages_.empty();
  }
  
  // Clear all messages
  void clear() {
    messages_.clear();
  }
  
  // Get message count
  auto size() const -> size_t {
    return messages_.size();
  }

 private:
  std::vector<std::unique_ptr<MultiModalMessage>> messages_;
};

/**
 * @brief Base Function class
 * 
 * Based on candyFlow's Function design, adapted for SAGE Flow.
 * Functions are independent processing units that do NOT inherit from operators.
 * They are used BY operators to perform actual data processing.
 */
class Function {
 public:
  explicit Function(std::string name, FunctionType type);
  virtual ~Function() = default;

  // Getters and setters
  auto getName() const -> const std::string&;
  auto getType() const -> FunctionType;
  void setName(const std::string& name);
  void setType(FunctionType type);

  // Main execution methods (based on candyFlow pattern)
  
  /**
   * @brief Execute function with single input
   * @param response Input response containing messages to process
   * @return Processed response
   */
  virtual auto execute(FunctionResponse& response) -> FunctionResponse;

  /**
   * @brief Execute function with two inputs (for join operations)
   * @param left Left input response
   * @param right Right input response  
   * @return Processed response
   */
  virtual auto execute(FunctionResponse& left, FunctionResponse& right) -> FunctionResponse;

 protected:
  std::string name_;
  FunctionType type_ = FunctionType::None;
};

}  // namespace sage_flow
