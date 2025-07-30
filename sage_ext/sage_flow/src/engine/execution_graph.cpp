#include "engine/execution_graph.h"
#include "operator/operator.h"
#include <iostream>

namespace sage_flow {

auto ExecutionGraph::addOperator(std::shared_ptr<Operator> op) -> OperatorId {
    OperatorId id = next_operator_id_++;
    operators_[id] = std::move(op);
    adjacency_list_[id] = std::vector<OperatorId>();
    reverse_adjacency_list_[id] = std::vector<OperatorId>();
    return id;
}

auto ExecutionGraph::connectOperators(OperatorId source, OperatorId target) -> void {
    adjacency_list_[source].push_back(target);
    reverse_adjacency_list_[target].push_back(source);
}

auto ExecutionGraph::removeOperator(OperatorId id) -> void {
    // Remove from operators map
    operators_.erase(id);
    
    // Remove from adjacency lists
    adjacency_list_.erase(id);
    reverse_adjacency_list_.erase(id);
    
    // Remove references to this operator from other operators
    for (auto& [op_id, successors] : adjacency_list_) {
        successors.erase(std::remove(successors.begin(), successors.end(), id), successors.end());
    }
    for (auto& [op_id, predecessors] : reverse_adjacency_list_) {
        predecessors.erase(std::remove(predecessors.begin(), predecessors.end(), id), predecessors.end());
    }
}

auto ExecutionGraph::getTopologicalOrder() const -> std::vector<OperatorId> {
    std::vector<OperatorId> result;
    std::unordered_map<OperatorId, bool> visited;
    std::unordered_map<OperatorId, bool> rec_stack;
    
    try {
        std::cout << "[ExecutionGraph] Starting topological sort with " << operators_.size() << " operators\n";
        
        // Initialize visited and recursion stack
        for (const auto& [id, op] : operators_) {
            std::cout << "[ExecutionGraph] Initializing operator " << id << "\n";
            visited[id] = false;
            rec_stack[id] = false;
        }
        
        // Perform DFS from all unvisited nodes
        for (const auto& [id, op] : operators_) {
            if (!visited[id]) {
                std::cout << "[ExecutionGraph] Processing unvisited operator " << id << "\n";
                if (topologicalSortUtil(id, visited, rec_stack, result)) {
                    std::cout << "[ExecutionGraph] Cycle detected\n";
                    return {}; // Cycle detected
                }
            }
        }
        
        std::reverse(result.begin(), result.end());
        std::cout << "[ExecutionGraph] Topological sort completed successfully\n";
        return result;
    } catch (const std::exception& e) {
        std::cout << "[ExecutionGraph] Exception in getTopologicalOrder: " << e.what() << "\n";
        throw;
    }
}

auto ExecutionGraph::getSourceOperators() const -> std::vector<OperatorId> {
    std::vector<OperatorId> sources;
    for (const auto& [id, op] : operators_) {
        if (reverse_adjacency_list_.at(id).empty()) {
            sources.push_back(id);
        }
    }
    return sources;
}

auto ExecutionGraph::getSinkOperators() const -> std::vector<OperatorId> {
    std::vector<OperatorId> sinks;
    for (const auto& [id, op] : operators_) {
        if (adjacency_list_.at(id).empty()) {
            sinks.push_back(id);
        }
    }
    return sinks;
}

auto ExecutionGraph::getOperator(OperatorId id) -> std::shared_ptr<Operator> {
    auto it = operators_.find(id);
    return (it != operators_.end()) ? it->second : nullptr;
}

auto ExecutionGraph::getOperators() const -> const std::unordered_map<OperatorId, std::shared_ptr<Operator>>& {
    return operators_;
}

auto ExecutionGraph::getSuccessors(OperatorId id) const -> std::vector<OperatorId> {
    auto it = adjacency_list_.find(id);
    return (it != adjacency_list_.end()) ? it->second : std::vector<OperatorId>();
}

auto ExecutionGraph::getPredecessors(OperatorId id) const -> std::vector<OperatorId> {
    auto it = reverse_adjacency_list_.find(id);
    return (it != reverse_adjacency_list_.end()) ? it->second : std::vector<OperatorId>();
}

auto ExecutionGraph::size() const -> size_t {
    return operators_.size();
}

auto ExecutionGraph::empty() const -> bool {
    return operators_.empty();
}

auto ExecutionGraph::isValid() const -> bool {
    return !detectCycles();
}

auto ExecutionGraph::validate() const -> bool {
    return isValid();
}

auto ExecutionGraph::getOperatorCount() const -> size_t {
    return size();
}

auto ExecutionGraph::isRunning() const -> bool {
    // Simple implementation - would need actual state tracking
    return false;
}

auto ExecutionGraph::initialize() -> void {
    // Initialize operators if needed
}

auto ExecutionGraph::finalize() -> void {
    // Finalize operators if needed
}

auto ExecutionGraph::reset() -> void {
    operators_.clear();
    adjacency_list_.clear();
    reverse_adjacency_list_.clear();
    next_operator_id_ = 0;
}

// Private methods
auto ExecutionGraph::detectCycles() const -> bool {
    std::unordered_map<OperatorId, bool> visited;
    std::unordered_map<OperatorId, bool> rec_stack;
    
    for (const auto& [id, op] : operators_) {
        visited[id] = false;
        rec_stack[id] = false;
    }
    
    for (const auto& [id, op] : operators_) {
        if (!visited[id]) {
            std::vector<OperatorId> dummy;
            if (topologicalSortUtil(id, visited, rec_stack, dummy)) {
                return true; // Cycle found
            }
        }
    }
    
    return false;
}

auto ExecutionGraph::topologicalSortUtil(OperatorId id, 
                                         std::unordered_map<OperatorId, bool>& visited,
                                         std::unordered_map<OperatorId, bool>& rec_stack,
                                         std::vector<OperatorId>& topo_order) const -> bool {
    visited[id] = true;
    rec_stack[id] = true;
    
    // Visit all successors
    auto it = adjacency_list_.find(id);
    if (it != adjacency_list_.end()) {
        for (OperatorId successor : it->second) {
            if (!visited[successor]) {
                if (topologicalSortUtil(successor, visited, rec_stack, topo_order)) {
                    return true; // Cycle detected
                }
            } else if (rec_stack[successor]) {
                return true; // Back edge found - cycle detected
            }
        }
    }
    
    rec_stack[id] = false;
    topo_order.push_back(id);
    return false;
}

}  // namespace sage_flow
