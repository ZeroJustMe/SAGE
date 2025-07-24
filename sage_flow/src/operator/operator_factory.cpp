#include "operator/operator.h"

#include "operator/source_operator.h"
#include "operator/map_operator.h"
#include "operator/filter_operator.h"
#include "operator/sink_operator.h"
#include "operator/join_operator.h"
#include "operator/aggregate_operator.h"
#include "operator/topk_operator.h"
#include "operator/window_operator.h"
// Include concrete implementations
#include "operator/lambda_operators.h"
#include "operator/lambda_sinks.h"

namespace sage_flow {

auto CreateOperator(OperatorType type, const std::string& name) -> std::unique_ptr<Operator> {
  // TODO: Implement concrete operator creation based on type
  // These are abstract base classes, need concrete implementations like:
  // - FileSourceOperator, KafkaSourceOperator (for SourceOperator)
  // - LambdaMapOperator (for MapOperator) 
  // - PredicateFilterOperator (for FilterOperator)
  // - FileSinkOperator, TerminalSinkOperator (for SinkOperator)
  
  (void)type; // Suppress unused parameter warning
  (void)name; // Suppress unused parameter warning
  return nullptr;
}

auto CreateTopKOperator(const std::string& name, size_t k) -> std::unique_ptr<TopKOperator> {
  // TopKOperator is abstract - requires concrete implementation like:
  // - ScoreBasedTopKOperator: implements getScore() for custom scoring
  // - SimilarityTopKOperator: implements getScore() for similarity ranking
  // - TextLengthTopKOperator: implements getScore() for text length ranking
  
  (void)name; // Suppress unused parameter warning
  (void)k;    // Suppress unused parameter warning
  return nullptr;
}

auto CreateWindowOperator(const std::string& name, 
                         WindowOperator::WindowType window_type) -> std::unique_ptr<WindowOperator> {
  // Note: WindowOperator is abstract, would need concrete implementation
  (void)name;        // Suppress unused parameter warning
  (void)window_type; // Suppress unused parameter warning
  return nullptr;
}

}  // namespace sage_flow
