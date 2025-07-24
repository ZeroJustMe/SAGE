#include "api/datastream.h"
#include "engine/stream_engine.h"
#include "engine/execution_graph.h"
#include "message/multimodal_message_core.h"
#include "operator/lambda_operators.h"
#include "operator/lambda_sinks.h"
#include <stdexcept>
#include <algorithm>

namespace sage_flow {

// ===============================
// DataStream Implementation
// ===============================

DataStream::DataStream(std::shared_ptr<StreamEngine> engine,
                       std::shared_ptr<ExecutionGraph> graph,
                       OperatorId last_operator_id)
    : engine_(std::move(engine)), graph_(std::move(graph)), last_operator_id_(last_operator_id) {}

// Lambda-based map operation (for simple transformations)
auto DataStream::map(const std::function<std::unique_ptr<MultiModalMessage>(std::unique_ptr<MultiModalMessage>)>& func) -> DataStream {
    auto lambda_op = std::make_unique<LambdaMapOperator>(func);
    int op_id = graph_->addOperator(std::move(lambda_op));
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

// Lambda-based filter operation (for simple predicates)
auto DataStream::filter(const std::function<bool(const MultiModalMessage&)>& predicate) -> DataStream {
    auto lambda_filter = std::make_unique<LambdaFilterOperator>(predicate);
    int op_id = graph_->addOperator(std::move(lambda_filter));
    connectToLastOperator(op_id);
    last_operator_id_ = op_id;
    return std::move(*this);
}

// Stream connection operation
auto DataStream::connect(const DataStream& other) -> DataStream {
    // Create a new graph that combines both streams
    auto new_graph = engine_->createGraph();
    
    // For now, implement simple merge - this would need more sophisticated logic
    // in a full implementation to handle proper stream joining
    throw std::runtime_error("Stream connection not yet fully implemented - requires join operator");
}

// Stream union operation
auto DataStream::union_(const DataStream& other) -> DataStream {
    // Create a union operator that merges both streams
    throw std::runtime_error("Stream union not yet fully implemented - requires union operator");
}

// Lambda-based sink operation (for simple output)
auto DataStream::sink(const std::function<void(const MultiModalMessage&)>& sink_func) -> void {
    auto lambda_sink = std::make_unique<TerminalSinkOperator>(sink_func);
    int sink_id = graph_->addOperator(std::move(lambda_sink));
    connectToLastOperator(sink_id);
    finalizeGraph();
}

// Execution control methods
auto DataStream::execute() -> void {
    if (!is_finalized_) {
        finalizeGraph();
    }
    
    if (graph_id_ == -1) {
        graph_id_ = engine_->submitGraph(graph_);
    }
    
    engine_->executeGraph(graph_id_);
}

auto DataStream::executeAsync() -> void {
    if (!is_finalized_) {
        finalizeGraph();
    }
    
    if (graph_id_ == -1) {
        graph_id_ = engine_->submitGraph(graph_);
    }
    
    engine_->executeGraphAsync(graph_id_);
}

auto DataStream::stop() -> void {
    if (graph_id_ != -1) {
        engine_->stopGraph(graph_id_);
    }
}

// Pipeline information methods
auto DataStream::getOperatorCount() const -> size_t {
    return graph_->getOperatorCount();
}

auto DataStream::isExecuting() const -> bool {
    return graph_->isRunning();
}

auto DataStream::getLastOperatorId() const -> int {
    return last_operator_id_;
}

auto DataStream::setLastOperatorId(int id) -> void {
    last_operator_id_ = id;
}

auto DataStream::getGraph() const -> std::shared_ptr<ExecutionGraph> {
    return graph_;
}

auto DataStream::getEngine() const -> std::shared_ptr<StreamEngine> {
    return engine_;
}

// Internal helper methods
auto DataStream::connectToLastOperator(int new_operator_id) -> void {
    if (last_operator_id_ != -1) {
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

// ===============================
// SageFlowEnvironment Implementation
// ===============================

SageFlowEnvironment::SageFlowEnvironment(std::string name, 
                                         StreamEngine::ExecutionMode mode)
    : name_(std::move(name)), 
      engine_(std::make_shared<StreamEngine>(mode)) {
    engine_->start();
}

SageFlowEnvironment::~SageFlowEnvironment() {
    close();
}

// Lambda-based source creation
auto SageFlowEnvironment::fromGenerator(const std::function<std::unique_ptr<MultiModalMessage>()>& generator,
                                        size_t max_messages) -> DataStream {
    auto graph = engine_->createGraph();
    
    // Create lambda source operator
    auto source = CreateLambdaSource(generator, max_messages);
    int source_id = graph->addOperator(std::move(source));
    
    // Create DataStream with the source as starting point
    DataStream result(engine_, graph, source_id);
    active_streams_.push_back(std::move(result));
    
    return std::move(active_streams_.back());
}

// Environment configuration methods
auto SageFlowEnvironment::setMemory(const std::unordered_map<std::string, std::string>& config) -> void {
    memory_config_ = config;
    setupMemoryIntegration();
}

auto SageFlowEnvironment::setThreadCount(int count) -> void {
    engine_->setThreadCount(count);
}

auto SageFlowEnvironment::setExecutionMode(StreamEngine::ExecutionMode mode) -> void {
    engine_->setExecutionMode(mode);
}

auto SageFlowEnvironment::setProperty(const std::string& key, const std::string& value) -> void {
    properties_[key] = value;
}

auto SageFlowEnvironment::getProperty(const std::string& key) const -> std::string {
    auto it = properties_.find(key);
    return it != properties_.end() ? it->second : "";
}

// Environment control methods
auto SageFlowEnvironment::submit() -> void {
    if (is_submitted_) {
        throw std::runtime_error("Environment already submitted");
    }
    
    if (!validateStreams()) {
        throw std::runtime_error("Invalid streams configuration");
    }
    
    // Move active streams to submitted streams
    submitted_streams_ = std::move(active_streams_);
    active_streams_.clear();
    
    is_submitted_ = true;
}

auto SageFlowEnvironment::runStreaming() -> void {
    if (!is_submitted_) {
        submit();
    }
    
    is_running_ = true;
    
    // Execute all submitted streams asynchronously
    for (auto& stream : submitted_streams_) {
        stream.executeAsync();
    }
}

auto SageFlowEnvironment::runBatch() -> void {
    if (!is_submitted_) {
        submit();
    }
    
    is_running_ = true;
    
    // Execute all submitted streams synchronously
    for (auto& stream : submitted_streams_) {
        stream.execute();
    }
    
    is_running_ = false;
}

auto SageFlowEnvironment::stop() -> void {
    if (!is_running_) {
        return;
    }
    
    // Stop all submitted streams
    for (auto& stream : submitted_streams_) {
        stream.stop();
    }
    
    // Stop all active streams
    for (auto& stream : active_streams_) {
        stream.stop();
    }
    
    engine_->stop();
    is_running_ = false;
}

auto SageFlowEnvironment::close() -> void {
    if (is_running_) {
        stop();
    }
    
    cleanupResources();
    
    active_streams_.clear();
    submitted_streams_.clear();
    memory_config_.clear();
    properties_.clear();
    is_submitted_ = false;
}

// Environment information methods
auto SageFlowEnvironment::getName() const -> const std::string& {
    return name_;
}

auto SageFlowEnvironment::isRunning() const -> bool {
    return is_running_;
}

auto SageFlowEnvironment::getActiveStreamCount() const -> size_t {
    return active_streams_.size();
}

auto SageFlowEnvironment::getSubmittedStreamCount() const -> size_t {
    return submitted_streams_.size();
}

auto SageFlowEnvironment::getExecutionMode() const -> StreamEngine::ExecutionMode {
    return engine_->getExecutionMode();
}

auto SageFlowEnvironment::getThreadCount() const -> int {
    return engine_->getThreadCount();
}

auto SageFlowEnvironment::getEngine() const -> std::shared_ptr<StreamEngine> {
    return engine_;
}

auto SageFlowEnvironment::addActiveStream(DataStream&& stream) -> void {
    active_streams_.push_back(std::move(stream));
}

// Internal methods
auto SageFlowEnvironment::validateStreams() -> bool {
    // Validate all active streams have valid configurations
    for (const auto& stream : active_streams_) {
        if (stream.getOperatorCount() == 0) {
            return false;
        }
    }
    return true;
}

auto SageFlowEnvironment::setupMemoryIntegration() -> void {
    // TODO: Integrate with sage_memory based on memory_config_
    // This would set up vector store connections, index configurations, etc.
}

auto SageFlowEnvironment::cleanupResources() -> void {
    // Cleanup any allocated resources
    // Close file handles, network connections, etc.
}

}  // namespace sage_flow
