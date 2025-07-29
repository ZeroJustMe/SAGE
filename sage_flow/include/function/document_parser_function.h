#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "function/function.h"

namespace sage_flow {

/**
 * @brief Document parser function for multi-format document processing
 * 
 * Supports parsing of various document formats including PDF, Word, HTML,
 * and plain text. Extracts textual content and metadata for further processing.
 * 
 * Based on candyFlow design: Function handles data processing logic only.
 */
class DocumentParserFunction final : public Function {
 public:
  enum class DocumentFormat : std::uint8_t {
    kAuto,      // Auto-detect format
    kPlainText,
    kHtml,
    kPdf,
    kDocx,
    kMarkdown
  };
  
  struct ParseConfig {
    DocumentFormat format_ = DocumentFormat::kAuto;
    bool extract_metadata_ = true;
    bool preserve_structure_ = false;
    std::string encoding_ = "utf-8";
  };
  
  explicit DocumentParserFunction(ParseConfig config);
  explicit DocumentParserFunction(std::string name, ParseConfig config);
  
  /**
   * @brief Execute document parsing function
   * @param response Function response containing document data
   * @return Processed response with parsed document content
   */
  auto execute(FunctionResponse& response) -> FunctionResponse override;
  
 private:
  ParseConfig config_;
  
  auto parseDocument(const std::string& content, DocumentFormat format) const -> std::string;
  auto detectFormat(const std::string& content) const -> DocumentFormat;
  auto parseHtml(const std::string& html_content) const -> std::string;
  auto parsePlainText(const std::string& text_content) const -> std::string;
  auto extractMetadata(const std::string& content, DocumentFormat format) const 
      -> std::unordered_map<std::string, std::string>;
};

}  // namespace sage_flow
