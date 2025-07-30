#include "api/datastream.h"
#include "engine/stream_engine.h"
#include "engine/execution_graph.h"
#include "message/multimodal_message_core.h"
#include "operator/map_operator.h"
#include "operator/filter_operator.h"
#include "operator/sink_operator.h"
#include "operator/source_operator.h"
#include "function/map_function.h"
#include "function/filter_function.h"
#include "function/sink_function.h"
#include "function/source_function.h"
#include <stdexcept>
#include <iostream>

namespace sage_flow {

// ===============================
// DataStream Implementation
// ===============================

DataStream::DataStream(std::shared_ptr<StreamEngine> engine,
                       std::shared_ptr<ExecutionGraph> graph,
                       ExecutionGraph::OperatorId last_operator_id)
    : engine_(std::move(engine)), graph_(std::move(graph)), last_operator_id_(last_operator_id) {}

// Lambda-based source operation (starting point for pipelines)
auto DataStream::from_source(const std::function<std::unique_ptr<MultiModalMessage>()>& source_func) -> DataStream& {
    // Create a concrete SourceFunction that wraps the lambda
    class LambdaSourceFunction : public SourceFunction {
    private:
        std::function<std::unique_ptr<MultiModalMessage>()> source_func_;
        bool has_next_ = true;
        
    public:
        explicit LambdaSourceFunction(const std::string& name, 
                                    std::function<std::unique_ptr<MultiModalMessage>()> func) 
            : SourceFunction(name), source_func_(std::move(func)) {}
            
        void init() override {}
        void close() override {}
        
        auto hasNext() -> bool override {
            return has_next_;
        }
        
        auto execute(FunctionResponse& response) -> FunctionResponse override {
            FunctionResponse result;
            auto message = source_func_();
            if (message) {
                result.addMessage(std::move(message));
            } else {
                has_next_ = false;
            }
            return result;
        }
    };
    
    // Create a concrete SourceOperator that uses the SourceFunction
    class FunctionSourceOperator : public SourceOperator {
    private:
        std::unique_ptr<SourceFunction> source_function_;
        
    public:
        explicit FunctionSourceOperator(std::string name, std::unique_ptr<SourceFunction> func)
            : SourceOperator(std::move(name)), source_function_(std::move(func)) {}
            
        auto hasNext() -> bool override {
            return source_function_->hasNext();
        }
        
        auto next() -> std::unique_ptr<MultiModalMessage> override {
            FunctionResponse response;
            auto function_output = source_function_->execute(response);
            if (!function_output.isEmpty()) {
                auto& messages = function_output.getMessages();
                return std::move(messages[0]);
            }
            return nullptr;
        }
        
        auto reset() -> void override {
            // Reset source to beginning if applicable
        }
    };
    
    auto source_function = std::make_unique<LambdaSourceFunction>("source_gen", source_func);
    auto source_operator = std::make_unique<FunctionSourceOperator>("source_op", std::move(source_function));
    
    ExecutionGraph::OperatorId op_id = graph_->addOperator(std::shared_ptr<Operator>(source_operator.release()));
    last_operator_id_ = op_id;
    return *this;
}

// Lambda-based map operation (for simple transformations)
auto DataStream::map(const std::function<std::unique_ptr<MultiModalMessage>(std::unique_ptr<MultiModalMessage>)>& func) -> DataStream& {
    // Adapt the user's function to the MapFunc signature (void(std::unique_ptr<MultiModalMessage>&))
    MapFunc adapted_func = [func](std::unique_ptr<MultiModalMessage>& input) -> void {
        // Apply the user's function and update the input in-place
        auto result = func(std::move(input));
        if (result) {
            input = std::move(result);
        }
        // If result is null, input becomes null (effectively filtered out)
    };
    
    // Create MapFunction with the adapted function
    auto map_function = std::make_unique<MapFunction>("map_transform", adapted_func);
    
    // Create MapOperator with the MapFunction
    auto map_operator = std::make_unique<MapOperator>("map_op", std::move(map_function));
    
    ExecutionGraph::OperatorId op_id = graph_->addOperator(std::shared_ptr<Operator>(map_operator.release()));
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return *this;
}

// Lambda-based filter operation (for simple predicates)
auto DataStream::filter(const std::function<bool(const MultiModalMessage&)>& predicate) -> DataStream& {
    // Create FilterFunction with the predicate
    auto filter_function = std::make_unique<FilterFunction>("filter_predicate", predicate);
    
    // Create FilterOperator with the FilterFunction
    auto filter_operator = std::make_unique<FilterOperator>("filter_op", std::move(filter_function));
    
    ExecutionGraph::OperatorId op_id = graph_->addOperator(std::shared_ptr<Operator>(filter_operator.release()));
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return *this;
}

// Stream connection operation
auto DataStream::connect(const DataStream& other) -> DataStream {
    // For now, implement simple merge - this would need more sophisticated logic
    // in a full implementation to handle proper stream joining
    (void)other; // Suppress unused parameter warning
    throw std::runtime_error("Stream connection not yet fully implemented - requires join operator");
}

// Stream union operation
auto DataStream::union_(const DataStream& other) -> DataStream {
    // Create a union operator that merges both streams
    (void)other; // Suppress unused parameter warning
    throw std::runtime_error("Stream union not yet fully implemented - requires union operator");
}

// Lambda-based sink operation (for simple output)
auto DataStream::sink(const std::function<void(const MultiModalMessage&)>& sink_func) -> void {
    // Create inline SinkFunction implementation
    class LambdaSinkFunction : public SinkFunction {
    public:
        LambdaSinkFunction(std::string name, SinkFunc func) 
            : SinkFunction(std::move(name), std::move(func)) {}
        void init() override {} // No-op for lambda sink
        void close() override {} // No-op for lambda sink
    };
    
    auto sink_function = std::make_unique<LambdaSinkFunction>("sink_output", sink_func);
    
    // Create SinkOperator with the SinkFunction
    auto sink_operator = std::make_unique<SinkOperator>("sink_op", std::move(sink_function));
    
    ExecutionGraph::OperatorId sink_id = graph_->addOperator(std::shared_ptr<Operator>(sink_operator.release()));
    connectToLastOperator(sink_id);
    finalizeGraph();
}

// Execution control methods
auto DataStream::execute() -> void {
    std::cout << "[DataStream] Starting execute()...\n";
    std::cout << "[DataStream] is_finalized_: " << is_finalized_ << "\n";
    std::cout << "[DataStream] graph_id_ has_value: " << graph_id_.has_value() << "\n";
    if (graph_id_.has_value()) {
        std::cout << "[DataStream] graph_id_ value: " << graph_id_.value() << "\n";
    }
    
    if (!is_finalized_) {
        std::cout << "[DataStream] Calling finalizeGraph()...\n";
        finalizeGraph();
    }
    
    std::cout << "[DataStream] After finalization, graph_id_ has_value: " << graph_id_.has_value() << "\n";
    
    if (!graph_id_.has_value()) {
        std::cout << "[DataStream] Calling submitGraph()...\n";
        graph_id_ = engine_->submitGraph(graph_);
        std::cout << "[DataStream] After submitGraph, graph_id_: " << graph_id_.value() << "\n";
    } else {
        std::cout << "[DataStream] Graph already submitted with ID: " << graph_id_.value() << "\n";
    }
    
    std::cout << "[DataStream] Calling executeGraph with ID: " << graph_id_.value() << "\n";
    engine_->executeGraph(graph_id_.value());
}

auto DataStream::executeAsync() -> void {
    if (!is_finalized_) {
        finalizeGraph();
    }
    
    if (!graph_id_.has_value()) {
        graph_id_ = engine_->submitGraph(graph_);
    }
    
    engine_->executeGraphAsync(graph_id_.value());
}

auto DataStream::stop() -> void {
    if (graph_id_.has_value()) {
        engine_->stopGraph(graph_id_.value());
    }
}

// Pipeline information methods
auto DataStream::getOperatorCount() const -> size_t {
    return graph_->getOperatorCount();
}

auto DataStream::isExecuting() const -> bool {
    return graph_->isRunning();
}

auto DataStream::getLastOperatorId() const -> ExecutionGraph::OperatorId {
    return last_operator_id_;
}

auto DataStream::setLastOperatorId(ExecutionGraph::OperatorId id) -> void {
    last_operator_id_ = id;
}

auto DataStream::getGraph() const -> std::shared_ptr<ExecutionGraph> {
    return graph_;
}

auto DataStream::getEngine() const -> std::shared_ptr<StreamEngine> {
    return engine_;
}

// Internal helper methods
auto DataStream::connectToLastOperator(ExecutionGraph::OperatorId new_operator_id) -> void {
    if (last_operator_id_ != static_cast<ExecutionGraph::OperatorId>(-1)) {
        graph_->connectOperators(last_operator_id_, new_operator_id);
    }
}

auto DataStream::finalizeGraph() -> void {
    if (!is_finalized_) {
        if (!graph_->validate()) {
            throw std::runtime_error("Invalid DAG: Graph contains cycles or invalid connections");
        }
        is_finalized_ = true;
    }
}

auto DataStream::validateConfig(const std::unordered_map<std::string, std::any>& config) -> bool {
    // Basic configuration validation
    // In a full implementation, this would validate operator-specific configuration
    return true;
}

}  // namespace sage_flow
