#include "engine/stream_engine.h"
#include "engine/execution_graph.h"
#include "operator/operator.h"
#include <iostream>

namespace sage_flow {

StreamEngine::StreamEngine(ExecutionMode mode) : execution_mode_(mode) {}

StreamEngine::~StreamEngine() {
    // Clean up any running graphs
    for (auto& [id, graph] : submitted_graphs_) {
        stopGraph(id);
    }
}

auto StreamEngine::submitGraph(const std::shared_ptr<ExecutionGraph>& graph) -> GraphId {
    std::cout << "[StreamEngine] Attempting to submit graph...\n";
    
    if (!graph) {
        std::cout << "[StreamEngine] Graph is null!\n";
        throw std::runtime_error("Invalid execution graph submitted - null graph");
    }
    
    std::cout << "[StreamEngine] Graph has " << graph->getOperatorCount() << " operators\n";
    
    if (!graph->validate()) {
        std::cout << "[StreamEngine] Graph validation failed!\n";
        throw std::runtime_error("Invalid execution graph submitted - validation failed");
    }
    
    std::cout << "[StreamEngine] Graph validation passed\n";
    
    GraphId id = next_graph_id_++;
    submitted_graphs_[id] = graph;
    graph_states_[id] = GraphState::SUBMITTED;
    
    std::cout << "[StreamEngine] Graph " << id << " submitted with " 
              << graph->getOperatorCount() << " operators\n";
    
    return id;
}

auto StreamEngine::executeGraph(GraphId graph_id) -> void {
    std::cout << "[StreamEngine] Starting executeGraph with ID " << graph_id << '\n';
    std::cout << "[StreamEngine] Number of submitted graphs: " << submitted_graphs_.size() << '\n';
    
    // Debug: print all submitted graph IDs
    std::cout << "[StreamEngine] Submitted graph IDs: ";
    for (const auto& [id, graph] : submitted_graphs_) {
        std::cout << id << " ";
    }
    std::cout << '\n';
    
    try {
        auto it = submitted_graphs_.find(graph_id);
        if (it == submitted_graphs_.end()) {
            std::cout << "[StreamEngine] Graph ID " << graph_id << " not found\n";
            throw std::runtime_error("Graph ID " + std::to_string(graph_id) + " not found");
        }
        
        std::cout << "[StreamEngine] Found graph, getting pointer...\n";
        auto graph = it->second;
        if (!graph) {
            std::cout << "[StreamEngine] Graph pointer is null!\n";
            throw std::runtime_error("Graph pointer is null");
        }
        
        std::cout << "[StreamEngine] Setting graph state to RUNNING...\n";
        graph_states_[graph_id] = GraphState::RUNNING;
        
        std::cout << "[StreamEngine] Executing graph " << graph_id << " synchronously\n";
        
        // Simple execution: iterate through operators in topological order
        std::cout << "[StreamEngine] Getting topological order...\n";
        auto topo_order = graph->getTopologicalOrder();
        if (topo_order.empty()) {
            throw std::runtime_error("Graph contains cycles or is invalid");
        }
    
        // For demonstration purposes, just print the execution order
        std::cout << "[StreamEngine] Execution order: ";
        for (auto op_id : topo_order) {
            std::cout << op_id << " ";
        }
        std::cout << "\n";
        
        // Simple execution simulation
        for (auto op_id : topo_order) {
            auto op = graph->getOperator(op_id);
            if (op) {
                std::cout << "[StreamEngine] Executing operator " << op_id << "\n";
                // Here we would actually execute the operator
                // For now, just simulate execution
            }
        }
        
        graph_states_[graph_id] = GraphState::COMPLETED;
        std::cout << "[StreamEngine] Graph " << graph_id << " execution completed\n";
    } catch (const std::exception& e) {
        std::cout << "[StreamEngine] Exception in executeGraph: " << e.what() << '\n';
        throw;
    }
}

auto StreamEngine::executeGraphAsync(GraphId graph_id) -> void {
    auto it = submitted_graphs_.find(graph_id);
    if (it == submitted_graphs_.end()) {
        throw std::runtime_error("Graph ID " + std::to_string(graph_id) + " not found");
    }
    
    graph_states_[graph_id] = GraphState::RUNNING;
    std::cout << "[StreamEngine] Starting async execution for graph " << graph_id << "\n";
    
    // For demonstration, just mark as completed immediately
    // In real implementation, this would start a background thread
    graph_states_[graph_id] = GraphState::COMPLETED;
}

auto StreamEngine::stopGraph(GraphId graph_id) -> void {
    auto it = submitted_graphs_.find(graph_id);
    if (it == submitted_graphs_.end()) {
        return; // Graph not found, nothing to stop
    }
    
    graph_states_[graph_id] = GraphState::STOPPED;
    std::cout << "[StreamEngine] Graph " << graph_id << " stopped\n";
}

auto StreamEngine::getGraphState(GraphId graph_id) const -> GraphState {
    auto it = graph_states_.find(graph_id);
    return (it != graph_states_.end()) ? it->second : GraphState::UNKNOWN;
}

auto StreamEngine::isGraphRunning(GraphId graph_id) const -> bool {
    return getGraphState(graph_id) == GraphState::RUNNING;
}

auto StreamEngine::removeGraph(GraphId graph_id) -> void {
    stopGraph(graph_id);
    submitted_graphs_.erase(graph_id);
    graph_states_.erase(graph_id);
    std::cout << "[StreamEngine] Graph " << graph_id << " removed\n";
}

auto StreamEngine::getSubmittedGraphs() const -> std::vector<GraphId> {
    std::vector<GraphId> graph_ids;
    graph_ids.reserve(submitted_graphs_.size());
    for (const auto& [id, graph] : submitted_graphs_) {
        graph_ids.push_back(id);
    }
    return graph_ids;
}

}  // namespace sage_flow