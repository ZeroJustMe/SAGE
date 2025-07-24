#pragma once

#include "operator/base_operator.h"
#include "operator/response.h"
#include "message/multimodal_message_core.h"
#include <functional>
#include <memory>

namespace sage_flow {

// Forward declarations - actual definitions are in datastream.h  
using MapFunction = std::function<std::unique_ptr<MultiModalMessage>(const MultiModalMessage&)>;
using FilterFunction = std::function<bool(const MultiModalMessage&)>;
using GeneratorFunction = std::function<std::unique_ptr<MultiModalMessage>()>;

/**
 * @brief Lambda-based map operator for stream transformations
 */
class LambdaMapOperator : public Operator {
 public:
  explicit LambdaMapOperator(MapFunction map_func);
  ~LambdaMapOperator() override = default;
  
  auto process(Response& input_record, int slot) -> bool override;
  auto open() -> void override;
  auto close() -> void override;
  
 private:
  MapFunction map_func_;
};

/**
 * @brief Lambda-based filter operator for stream filtering  
 */
class LambdaFilterOperator : public Operator {
 public:
  explicit LambdaFilterOperator(FilterFunction filter_func);
  ~LambdaFilterOperator() override = default;
  
  auto process(Response& input_record, int slot) -> bool override;
  auto open() -> void override;
  auto close() -> void override;
  
 private:
  FilterFunction filter_func_;
};

/**
 * @brief Lambda-based source operator for data generation
 */
class LambdaSourceOperator : public Operator {
 public:
  explicit LambdaSourceOperator(GeneratorFunction generator, size_t max_messages = 0);
  ~LambdaSourceOperator() override = default;
  
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

// Factory functions
auto CreateLambdaMap(const MapFunction& func) -> std::unique_ptr<LambdaMapOperator>;
auto CreateLambdaFilter(const FilterFunction& predicate) -> std::unique_ptr<LambdaFilterOperator>;
auto CreateLambdaSource(const GeneratorFunction& generator, size_t max_messages = 0) 
    -> std::unique_ptr<LambdaSourceOperator>;

}  // namespace sage_flow
