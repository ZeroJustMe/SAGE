#include "function/text_embedding_function.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <regex>

namespace sage_flow {

TextEmbeddingFunction::TextEmbeddingFunction(const TextEmbeddingConfig& config)
    : Function("TextEmbeddingFunction", FunctionType::Map), config_(config) {
}

TextEmbeddingFunction::TextEmbeddingFunction(const std::string& name, const TextEmbeddingConfig& config)
    : Function(name, FunctionType::Map), config_(config) {
}

auto TextEmbeddingFunction::execute(FunctionResponse& response) -> FunctionResponse {
    FunctionResponse output_response;
    
    // Process each message in the response
    for (auto& message : response.getMessages()) {
        if (!message) continue;
        
        // Extract text content from message
        std::string text_content = message->getText();
        if (text_content.empty()) continue;
        
        // Prepare text for embedding
        std::string prepared_text = prepareText(text_content);
        if (prepared_text.empty()) continue;
        
        // Generate embedding (mock implementation for now)
        std::vector<float> embedding = generateMockEmbedding(prepared_text);
        
        // Create new message with embedding data
        auto output_message = std::make_unique<MultiModalMessage>();
        output_message->setText(text_content);
        
        // Store embedding as metadata (in real implementation, use proper vector storage)
        std::ostringstream embedding_str;
        for (size_t i = 0; i < embedding.size(); ++i) {
            if (i > 0) embedding_str << ",";
            embedding_str << embedding[i];
        }
        output_message->setMetadata("embedding", embedding_str.str());
        output_message->setMetadata("embedding_dim", std::to_string(config_.embedding_dim_));
        output_message->setMetadata("model", config_.model_name_);
        
        output_response.addMessage(std::move(output_message));
    }
    
    return output_response;
}

void TextEmbeddingFunction::init() {
    // Initialize embedding model (mock implementation)
    // In real implementation: load model, initialize ONNX/LibTorch session
    model_initialized_ = true;
}

void TextEmbeddingFunction::close() {
    // Clean up model resources
    model_initialized_ = false;
}

auto TextEmbeddingFunction::getConfig() const -> const TextEmbeddingConfig& {
    return config_;
}

void TextEmbeddingFunction::updateConfig(const TextEmbeddingConfig& config) {
    config_ = config;
    model_initialized_ = false; // Require re-initialization
}

auto TextEmbeddingFunction::generateEmbeddings(const std::vector<std::string>& texts) const -> std::vector<std::vector<float>> {
    std::vector<std::vector<float>> embeddings;
    embeddings.reserve(texts.size());
    
    for (const auto& text : texts) {
        embeddings.push_back(generateMockEmbedding(text));
    }
    
    return embeddings;
}

auto TextEmbeddingFunction::tokenizeText(const std::string& text) const -> std::vector<int> {
    // Mock tokenization - in real implementation use proper tokenizer
    std::vector<int> tokens;
    tokens.reserve(text.length() / 4); // Rough estimate
    
    for (size_t i = 0; i < text.length(); i += 4) {
        tokens.push_back(static_cast<int>(text[i]) + i);
    }
    
    return tokens;
}

auto TextEmbeddingFunction::applyPooling(const std::vector<std::vector<float>>& token_embeddings) const -> std::vector<float> {
    if (token_embeddings.empty()) {
        return std::vector<float>(config_.embedding_dim_, 0.0f);
    }
    
    std::vector<float> pooled_embedding(config_.embedding_dim_, 0.0f);
    
    if (config_.pooling_ == "mean") {
        // Mean pooling
        for (const auto& token_emb : token_embeddings) {
            for (size_t i = 0; i < pooled_embedding.size() && i < token_emb.size(); ++i) {
                pooled_embedding[i] += token_emb[i];
            }
        }
        
        float count = static_cast<float>(token_embeddings.size());
        for (auto& val : pooled_embedding) {
            val /= count;
        }
    } else if (config_.pooling_ == "max") {
        // Max pooling
        for (const auto& token_emb : token_embeddings) {
            for (size_t i = 0; i < pooled_embedding.size() && i < token_emb.size(); ++i) {
                pooled_embedding[i] = std::max(pooled_embedding[i], token_emb[i]);
            }
        }
    }
    
    return pooled_embedding;
}

auto TextEmbeddingFunction::normalizeEmbedding(std::vector<float>& embedding) const -> void {
    if (!config_.normalize_) return;
    
    // L2 normalization
    float norm = 0.0f;
    for (float val : embedding) {
        norm += val * val;
    }
    norm = std::sqrt(norm);
    
    if (norm > 1e-8f) {
        for (auto& val : embedding) {
            val /= norm;
        }
    }
}

auto TextEmbeddingFunction::isModelReady() const -> bool {
    return model_initialized_;
}

auto TextEmbeddingFunction::prepareText(const std::string& text) const -> std::string {
    std::string prepared = text;
    
    // Remove extra whitespace
    std::regex whitespace_regex("\\s+");
    prepared = std::regex_replace(prepared, whitespace_regex, " ");
    
    // Trim
    prepared.erase(prepared.begin(), std::find_if(prepared.begin(), prepared.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    prepared.erase(std::find_if(prepared.rbegin(), prepared.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), prepared.end());
    
    // Truncate if too long
    if (static_cast<int>(prepared.length()) > config_.max_length_) {
        prepared = prepared.substr(0, config_.max_length_);
    }
    
    return prepared;
}

auto TextEmbeddingFunction::generateMockEmbedding(const std::string& text) const -> std::vector<float> {
    // Mock embedding generation - in real implementation use actual model
    std::vector<float> embedding(config_.embedding_dim_);
    
    // Simple hash-based mock embedding
    std::hash<std::string> hasher;
    size_t hash = hasher(text);
    
    for (int i = 0; i < config_.embedding_dim_; ++i) {
        float val = static_cast<float>((hash + i) % 1000) / 1000.0f - 0.5f;
        embedding[i] = val;
    }
    
    // Normalize if required
    if (config_.normalize_) {
        std::vector<float> temp_embedding = embedding;
        normalizeEmbedding(temp_embedding);
        embedding = temp_embedding;
    }
    
    return embedding;
}

} // namespace sage_flow
