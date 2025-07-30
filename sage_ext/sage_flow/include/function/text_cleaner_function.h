#pragma once

#include <regex>
#include <string>
#include <vector>
#include "function/function.h"

namespace sage_flow {

/**
 * @brief Configuration for text cleaning operations
 */
struct TextCleanConfig {
  std::vector<std::string> regex_patterns_;      // Patterns to remove
  bool remove_extra_whitespace_ = true;          // Remove extra spaces
  bool to_lowercase_ = false;                    // Convert to lowercase
  bool remove_punctuation_ = false;              // Remove punctuation
  std::string replacement_text_;                 // Replacement for matched patterns
  
  // Quality assessment parameters
  float min_length_ = 10.0F;                    // Minimum text length
  float max_length_ = 10000.0F;                 // Maximum text length
  float min_quality_score_ = 0.3F;              // Minimum quality threshold
};

/**
 * @brief Text cleaning function for processing textual content
 * 
 * This function implements comprehensive text cleaning and preprocessing
 * capabilities, including regex-based pattern removal, whitespace normalization,
 * and quality assessment. Based on candyFlow design principles.
 */
class TextCleanerFunction final : public Function {
 public:
  explicit TextCleanerFunction(TextCleanConfig config);
  explicit TextCleanerFunction(std::string name, TextCleanConfig config);
  
  // Disable copy operations for performance
  TextCleanerFunction(const TextCleanerFunction&) = delete;
  auto operator=(const TextCleanerFunction&) -> TextCleanerFunction& = delete;
  
  // Enable move operations
  TextCleanerFunction(TextCleanerFunction&& other) noexcept;
  auto operator=(TextCleanerFunction&& other) noexcept -> TextCleanerFunction&;
  
  // Function interface implementation
  auto execute(FunctionResponse& response) -> FunctionResponse override;
  
  // Configuration management
  auto getConfig() const -> const TextCleanConfig&;
  auto updateConfig(const TextCleanConfig& new_config) -> void;
  
 private:
  TextCleanConfig config_;
  std::vector<std::regex> compiled_patterns_;
  
  // Private helper methods
  auto cleanText(const std::string& input_text) const -> std::string;
  auto removePatterns(const std::string& text) const -> std::string;
  auto normalizeWhitespace(const std::string& text) const -> std::string;
  auto removePunctuation(const std::string& text) const -> std::string;
  auto toLowerCase(const std::string& text) const -> std::string;
  auto calculateQualityScore(const std::string& text) const -> float;
  auto assessTextLength(const std::string& text) const -> float;
  auto assessTextComplexity(const std::string& text) const -> float;
  auto assessLanguageDetection(const std::string& text) const -> float;
  auto compilePatterns() -> void;
  auto isValidText(const std::string& text) const -> bool;
};

}  // namespace sage_flow
