#include "operator/lambda_sinks.h"
#include "operator/operator_types.h"
#include <iostream>
#include <fstream>

namespace sage_flow {

// ===============================
// TerminalSinkOperator Implementation
// ===============================

TerminalSinkOperator::TerminalSinkOperator(SinkFunction sink_func)
    : Operator(OperatorType::kSink, "TerminalSink"), sink_func_(std::move(sink_func)) {}

auto TerminalSinkOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    // Extract messages from the response and process them
    if (input_record.hasMessages()) {
        auto messages = input_record.getMessages();
        for (auto& message : messages) {
            if (message) {
                sink_func_(*message);
            }
        }
    }
    
    incrementOutputCount();
    return true;  // Sink operators always succeed
}

auto TerminalSinkOperator::open() -> void {
    // Terminal sink doesn't need initialization
}

auto TerminalSinkOperator::close() -> void {
    // Terminal sink doesn't need cleanup
}

// ===============================
// FileSinkOperator Implementation
// ===============================

FileSinkOperator::FileSinkOperator(std::string file_path, FileSinkConfig config)
    : Operator(OperatorType::kSink, "FileSink"), 
      file_path_(std::move(file_path)), config_(std::move(config)) {}

auto FileSinkOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    if (!output_file_.is_open()) {
        return false;  // Error: file not open
    }
    
    if (input_record.hasMessages()) {
        auto messages = input_record.getMessages();
        for (auto& message : messages) {
            if (message) {
                writeMessage(*message);
                message_count_++;
                
                // Flush periodically based on batch size
                if (config_.batch_size_ > 0 && message_count_ % config_.batch_size_ == 0) {
                    output_file_.flush();
                }
            }
        }
    }
    
    incrementOutputCount();
    return true;
}

auto FileSinkOperator::open() -> void {
    std::ios_base::openmode mode = std::ios::out;
    
    if (config_.append_mode_) {
        mode |= std::ios::app;
    }
    
    output_file_.open(file_path_, mode);
    if (!output_file_.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + file_path_);
    }
    
    // Write header if specified
    if (!config_.header_.empty()) {
        output_file_ << config_.header_ << '\n';
    }
}

auto FileSinkOperator::close() -> void {
    if (output_file_.is_open()) {
        output_file_.flush();
        output_file_.close();
    }
}

auto FileSinkOperator::writeMessage(const MultiModalMessage& message) -> void {
    switch (config_.format_) {
        case FileFormat::JSON:
            writeAsJson(message);
            break;
        case FileFormat::CSV:
            writeAsCsv(message);
            break;
        case FileFormat::TEXT:
        default:
            writeAsText(message);
            break;
    }
}

auto FileSinkOperator::writeAsText(const MultiModalMessage& message) -> void {
    output_file_ << "UID: " << message.getUid() << '\n';
    output_file_ << "Type: " << static_cast<int>(message.getContentType()) << '\n';
    
    if (message.isTextContent()) {
        output_file_ << "Content: " << message.getContentAsString() << '\n';
    } else if (message.isBinaryContent()) {
        const auto& binary_data = message.getContentAsBinary();
        output_file_ << "Binary Size: " << binary_data.size() << " bytes\n";
    }
    
    auto quality = message.getQualityScore();
    if (quality.has_value()) {
        output_file_ << "Quality: " << quality.value() << '\n';
    }
    
    output_file_ << "---\n";
}

auto FileSinkOperator::writeAsJson(const MultiModalMessage& message) -> void {
    output_file_ << "{\n";
    output_file_ << "  \"uid\": " << message.getUid() << ",\n";
    output_file_ << "  \"type\": " << static_cast<int>(message.getContentType()) << ",\n";
    output_file_ << "  \"timestamp\": " << message.getTimestamp() << ",\n";
    
    if (message.isTextContent()) {
        output_file_ << R"(  "content": ")" << escapeJsonString(message.getContentAsString()) << "\",\n";
    } else if (message.isBinaryContent()) {
        const auto& binary_data = message.getContentAsBinary();
        output_file_ << "  \"binary_size\": " << binary_data.size() << ",\n";
    }
    
    auto quality = message.getQualityScore();
    if (quality.has_value()) {
        output_file_ << "  \"quality\": " << quality.value() << ",\n";
    }
    
    output_file_ << "  \"has_embedding\": " << (message.hasEmbedding() ? "true" : "false") << "\n";
    output_file_ << "}\n";
}

auto FileSinkOperator::writeAsCsv(const MultiModalMessage& message) -> void {
    // CSV format: uid,type,timestamp,content,quality,has_embedding
    output_file_ << message.getUid() << ",";
    output_file_ << static_cast<int>(message.getContentType()) << ",";
    output_file_ << message.getTimestamp() << ",";
    
    if (message.isTextContent()) {
        output_file_ << "\"" << escapeCsvString(message.getContentAsString()) << "\",";
    } else {
        output_file_ << "\"\",";
    }
    
    auto quality = message.getQualityScore();
    if (quality.has_value()) {
        output_file_ << quality.value();
    }
    output_file_ << ",";
    
    output_file_ << (message.hasEmbedding() ? "true" : "false");
    output_file_ << "\n";
}

auto FileSinkOperator::escapeJsonString(const std::string& str) -> std::string {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

auto FileSinkOperator::escapeCsvString(const std::string& str) -> std::string {
    std::string escaped;
    for (char c : str) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

// ===============================
// VectorStoreSinkOperator Implementation
// ===============================

VectorStoreSinkOperator::VectorStoreSinkOperator(VectorStoreConfig config)
    : Operator(OperatorType::kSink, "VectorStoreSink"), 
      config_(std::move(config)), message_count_(0) {
    batch_messages_.reserve(config_.batch_size_);
}

auto VectorStoreSinkOperator::process(Response& input_record, int slot) -> bool {
    incrementProcessedCount();
    
    if (input_record.hasMessages()) {
        auto messages = input_record.getMessages();
        for (auto& message : messages) {
            if (message && message->hasEmbedding()) {
                batch_messages_.push_back(message.get());
                
                // Process batch when it reaches the configured size
                if (batch_messages_.size() >= config_.batch_size_) {
                    processBatch();
                }
            }
        }
    }
    
    incrementOutputCount();
    return true;
}

auto VectorStoreSinkOperator::open() -> void {
    // TODO(developer): Initialize connection to sage_memory vector store
    // This would establish connection to the vector database
}

auto VectorStoreSinkOperator::close() -> void {
    // Process any remaining messages in the batch
    if (!batch_messages_.empty()) {
        processBatch();
    }
    
    // TODO(developer): Close connection to sage_memory vector store
}

auto VectorStoreSinkOperator::processBatch() -> void {
    if (batch_messages_.empty()) {
        return;
    }
    
    // TODO(developer): Integrate with sage_memory vector store
    // For now, simulate vector storage processing
    for (const auto* message : batch_messages_) {
        if (message != nullptr && message->hasEmbedding()) {
            auto embedding_opt = message->getEmbedding();
            if (embedding_opt.has_value()) {
                // Simulate vector storage operation
                message_count_++;
            }
        }
    }
    
    batch_messages_.clear();
}

// ===============================
// Factory Functions
// ===============================

auto CreateTerminalSink(const std::function<void(const MultiModalMessage&)>& sink_func) 
    -> std::unique_ptr<TerminalSinkOperator> {
    return std::make_unique<TerminalSinkOperator>(sink_func);
}

auto CreateFileSink(const std::string& file_path, FileFormat format, bool append_mode) 
    -> std::unique_ptr<FileSinkOperator> {
    FileSinkConfig config;
    config.format_ = format;
    config.append_mode_ = append_mode;
    config.batch_size_ = 100;  // Default batch size
    
    return std::make_unique<FileSinkOperator>(file_path, config);
}

auto CreateVectorStoreSink(const std::string& collection_name, size_t batch_size, bool update_index) 
    -> std::unique_ptr<VectorStoreSinkOperator> {
    VectorStoreConfig config;
    config.collection_name_ = collection_name;
    config.batch_size_ = batch_size;
    config.update_index_ = update_index;
    
    return std::make_unique<VectorStoreSinkOperator>(config);
}

}  // namespace sage_flow
