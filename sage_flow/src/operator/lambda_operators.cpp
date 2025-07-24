#include "operator/lambda_operators.h"
#include "operator/operator_types.h"
#include "message/multimodal_message_core.h"
#include <stdexcept>

namespace sage_flow {

// ===============================
// LambdaMapOperator Implementation
// ===============================

LambdaMapOperator::LambdaMapOperator(MapFunction map_func)
    : Operator(OperatorType::kMap, "LambdaMap"), map_func_(std::move(map_func)) {}

auto LambdaMapOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    if (input_record.hasMessages()) {
        auto messages = input_record.getMessages();
        std::vector<std::unique_ptr<MultiModalMessage>> transformed_messages;
        
        for (auto& message : messages) {
            if (message) {
                auto transformed = map_func_(*message);
                if (transformed) {
                    transformed_messages.push_back(std::move(transformed));
                }
            }
        }
        
        // Create new response with transformed messages
        if (!transformed_messages.empty()) {
            Response output_record(std::move(transformed_messages));
            emit(0, output_record);
            incrementOutputCount();
        }
    }
    
    return true;
}

auto LambdaMapOperator::open() -> void {
    // Map operator doesn't need special initialization
}

auto LambdaMapOperator::close() -> void {
    // Map operator doesn't need special cleanup
}

// ===============================
// LambdaFilterOperator Implementation
// ===============================

LambdaFilterOperator::LambdaFilterOperator(FilterFunction filter_func)
    : Operator(OperatorType::kFilter, "LambdaFilter"), filter_func_(std::move(filter_func)) {}

auto LambdaFilterOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    if (input_record.hasMessages()) {
        auto messages = input_record.getMessages();
        std::vector<std::unique_ptr<MultiModalMessage>> filtered_messages;
        
        for (auto& message : messages) {
            if (message && filter_func_(*message)) {
                filtered_messages.push_back(std::move(message));
            }
        }
        
        // Emit filtered messages if any passed the filter
        if (!filtered_messages.empty()) {
            Response output_record(std::move(filtered_messages));
            emit(0, output_record);
            incrementOutputCount();
        }
    }
    
    return true;
}

auto LambdaFilterOperator::open() -> void {
    // Filter operator doesn't need special initialization
}

auto LambdaFilterOperator::close() -> void {
    // Filter operator doesn't need special cleanup
}

// ===============================
// LambdaSourceOperator Implementation
// ===============================

LambdaSourceOperator::LambdaSourceOperator(GeneratorFunction generator, size_t max_messages)
    : Operator(OperatorType::kSource, "LambdaSource"), 
      generator_(std::move(generator)), max_messages_(max_messages), 
      generated_count_(0), has_more_data_(true) {}

auto LambdaSourceOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    if (!has_more_data_) {
        return false;  // No more data to generate
    }
    
    // Check if we've reached the limit
    if (max_messages_ > 0 && generated_count_ >= max_messages_) {
        has_more_data_ = false;
        return false;
    }
    
    try {
        auto generated_message = generator_();
        if (generated_message) {
            std::vector<std::unique_ptr<MultiModalMessage>> messages;
            messages.push_back(std::move(generated_message));
            
            Response output_record(std::move(messages));
            emit(0, output_record);
            
            generated_count_++;
            incrementOutputCount();
            return true;
        }
        // Generator returned null, no more data
        has_more_data_ = false;
        return false;
    } catch (const std::exception& e) {
        // Generator threw an exception, consider this end of data
        has_more_data_ = false;
        return false;
    }
}

auto LambdaSourceOperator::open() -> void {
    generated_count_ = 0;
    has_more_data_ = true;
}

auto LambdaSourceOperator::close() -> void {
    has_more_data_ = false;
}

auto LambdaSourceOperator::hasNext() const -> bool {
    return has_more_data_ && (max_messages_ == 0 || generated_count_ < max_messages_);
}

auto LambdaSourceOperator::getGeneratedCount() const -> size_t {
    return generated_count_;
}

// ===============================
// Factory Functions
// ===============================

auto CreateLambdaMap(const MapFunction& func) -> std::unique_ptr<LambdaMapOperator> {
    return std::make_unique<LambdaMapOperator>(func);
}

auto CreateLambdaFilter(const FilterFunction& predicate) -> std::unique_ptr<LambdaFilterOperator> {
    return std::make_unique<LambdaFilterOperator>(predicate);
}

auto CreateLambdaSource(const GeneratorFunction& generator, size_t max_messages) 
    -> std::unique_ptr<LambdaSourceOperator> {
    return std::make_unique<LambdaSourceOperator>(generator, max_messages);
}

}  // namespace sage_flow
