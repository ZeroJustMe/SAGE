#pragma once

#include <string>
#include <vector>
#include <memory>
#include "function/function.h"

namespace sage_flow {

/**
 * @brief Text embedding configuration
 */
struct TextEmbeddingConfig {
    std::string model_name_ = "sentence-transformers/all-MiniLM-L6-v2";
    std::string device_ = "cpu";
    int batch_size_ = 32;
    bool normalize_ = true;
    std::string pooling_ = "mean";
    int max_length_ = 512;
    int embedding_dim_ = 384;
};

/**
 * @brief Text embedding function for converting text to vectors
 * 
 * Converts text content into dense vector representations using
 * pre-trained embedding models. Supports batch processing and
 * various pooling strategies.
 * 
 * Based on SAGE Flow Phase 3 requirements for vectorization support.
 */
class TextEmbeddingFunction final : public Function {
public:
    explicit TextEmbeddingFunction(const TextEmbeddingConfig& config = {});
    explicit TextEmbeddingFunction(const std::string& name, const TextEmbeddingConfig& config = {});
    
    /**
     * @brief Execute text embedding function
     * @param response Function response containing text data
     * @return Response with embedded vector data
     */
    auto execute(FunctionResponse& response) -> FunctionResponse override;
    
    /**
     * @brief Initialize embedding model
     */
    void init();
    
    /**
     * @brief Clean up model resources
     */
    void close();
    
    /**
     * @brief Get embedding configuration
     */
    auto getConfig() const -> const TextEmbeddingConfig&;
    
    /**
     * @brief Update embedding configuration
     */
    void updateConfig(const TextEmbeddingConfig& config);

private:
    TextEmbeddingConfig config_;
    bool model_initialized_ = false;
    
    /**
     * @brief Generate embeddings for a batch of texts
     */
    auto generateEmbeddings(const std::vector<std::string>& texts) const -> std::vector<std::vector<float>>;
    
    /**
     * @brief Tokenize text for model input
     */
    auto tokenizeText(const std::string& text) const -> std::vector<int>;
    
    /**
     * @brief Apply pooling strategy to token embeddings
     */
    auto applyPooling(const std::vector<std::vector<float>>& token_embeddings) const -> std::vector<float>;
    
    /**
     * @brief Normalize embedding vector
     */
    auto normalizeEmbedding(std::vector<float>& embedding) const -> void;
    
    /**
     * @brief Check if model is ready for inference
     */
    auto isModelReady() const -> bool;
    
    /**
     * @brief Prepare text for embedding (cleaning, truncation)
     */
    auto prepareText(const std::string& text) const -> std::string;
};

} // namespace sage_flow
