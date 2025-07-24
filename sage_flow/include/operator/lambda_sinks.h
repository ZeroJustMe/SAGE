#pragma once

#include "operator/base_operator.h"
#include "operator/response.h"
#include "message/multimodal_message_core.h"
#include <functional>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

namespace sage_flow {

// Forward declaration - actual definition is in datastream.h
using SinkFunction = std::function<void(const MultiModalMessage&)>;

// Configuration for file sink operations
enum class FileFormat : std::uint8_t {
    TEXT = 0,
    JSON = 1,
    CSV = 2
};

struct FileSinkConfig {
    FileFormat format_ = FileFormat::TEXT;
    bool append_mode_ = false;
    size_t batch_size_ = 100;
    std::string header_;
};

// Configuration for vector store sink operations
struct VectorStoreConfig {
    std::string collection_name_;
    size_t batch_size_ = 50;
    bool update_index_ = true;
    std::string index_type_ = "HNSW";
};

// ===============================
// TerminalSinkOperator - Outputs to terminal/console
// ===============================
class TerminalSinkOperator : public Operator {
public:
    explicit TerminalSinkOperator(SinkFunction sink_func);
    ~TerminalSinkOperator() override = default;

    auto process(Response& input_record, int slot) -> bool override;
    auto open() -> void override;
    auto close() -> void override;

private:
    SinkFunction sink_func_;
};

// ===============================
// FileSinkOperator - Outputs to file with various formats
// ===============================
class FileSinkOperator : public Operator {
public:
    FileSinkOperator(std::string file_path, FileSinkConfig config);
    ~FileSinkOperator() override = default;

    auto process(Response& input_record, int slot) -> bool override;
    auto open() -> void override;
    auto close() -> void override;

private:
    auto writeMessage(const MultiModalMessage& message) -> void;
    auto writeAsText(const MultiModalMessage& message) -> void;
    auto writeAsJson(const MultiModalMessage& message) -> void;
    auto writeAsCsv(const MultiModalMessage& message) -> void;
    
    static auto escapeJsonString(const std::string& str) -> std::string;
    static auto escapeCsvString(const std::string& str) -> std::string;

    std::string file_path_;
    FileSinkConfig config_;
    std::ofstream output_file_;
    size_t message_count_{0};
};

// ===============================
// VectorStoreSinkOperator - Outputs to vector database
// ===============================
class VectorStoreSinkOperator : public Operator {
public:
    explicit VectorStoreSinkOperator(VectorStoreConfig config);
    ~VectorStoreSinkOperator() override = default;

    auto process(Response& input_record, int slot) -> bool override;
    auto open() -> void override;
    auto close() -> void override;

private:
    auto processBatch() -> void;

    VectorStoreConfig config_;
    size_t message_count_{0};
    std::vector<const MultiModalMessage*> batch_messages_;
};

// ===============================
// Factory Functions for Creating Sink Operators
// ===============================

// Create a terminal sink that prints messages
auto CreateTerminalSink(const std::function<void(const MultiModalMessage&)>& sink_func) 
    -> std::unique_ptr<TerminalSinkOperator>;

// Create a file sink with specified format
auto CreateFileSink(const std::string& file_path, 
                   FileFormat format = FileFormat::TEXT, 
                   bool append_mode = false) 
    -> std::unique_ptr<FileSinkOperator>;

// Create a vector store sink for embeddings
auto CreateVectorStoreSink(const std::string& collection_name, 
                          size_t batch_size = 50, 
                          bool update_index = true) 
    -> std::unique_ptr<VectorStoreSinkOperator>;

}  // namespace sage_flow
