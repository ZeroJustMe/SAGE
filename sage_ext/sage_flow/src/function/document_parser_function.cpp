#include "function/document_parser_function.h"
#include <iostream>
#include <cctype>
#include <algorithm>
#include "message/multimodal_message_core.h"

namespace sage_flow {

DocumentParserFunction::DocumentParserFunction(ParseConfig config)
    : Function("DocumentParser", FunctionType::Map), config_(std::move(config)) {}

DocumentParserFunction::DocumentParserFunction(std::string name, ParseConfig config)
    : Function(std::move(name), FunctionType::Map), config_(std::move(config)) {}

auto DocumentParserFunction::execute(FunctionResponse& response) -> FunctionResponse {
    FunctionResponse output_response;
    
    for (auto& message : response.getMessages()) {
        if (!message) {
            continue;
        }
        
        // Get text content from message
        std::string content;
        if (message->isTextContent()) {
            content = message->getContentAsString();
        } else {
            // Skip non-text content for now
            output_response.addMessage(std::move(message));
            continue;
        }
        
        if (content.empty()) {
            output_response.addMessage(std::move(message));
            continue;
        }
        
        // Detect format if auto-detection is enabled
        DocumentFormat format = config_.format_;
        if (format == DocumentFormat::kAuto) {
            format = detectFormat(content);
        }
        
        try {
            // Parse document based on format
            std::string parsed_content = parseDocument(content, format);
            
            // Create new message with parsed content
            auto parsed_message = CreateTextMessage(message->getUid(), parsed_content);
            
            // Add metadata if requested
            if (config_.extract_metadata_) {
                auto metadata = extractMetadata(content, format);
                for (const auto& [key, value] : metadata) {
                    parsed_message->setMetadata(key, value);
                }
            }
            
            // Copy original metadata
            for (const auto& [key, value] : message->getMetadata()) {
                parsed_message->setMetadata(key, value);
            }
            
            // Add processing step
            parsed_message->addProcessingStep("DocumentParser");
            
            output_response.addMessage(std::move(parsed_message));
            
        } catch (const std::exception& e) {
            std::cerr << "Error parsing document: " << e.what() << std::endl;
            // Add original message on error
            output_response.addMessage(std::move(message));
        }
    }
    
    return output_response;
}

auto DocumentParserFunction::parseDocument(const std::string& content, DocumentFormat format) const -> std::string {
    switch (format) {
        case DocumentFormat::kPlainText:
            return parsePlainText(content);
        case DocumentFormat::kHtml:
            return parseHtml(content);
        case DocumentFormat::kPdf:
            // TODO(xinyan): Implement PDF parsing (requires external library)
            std::cerr << "PDF parsing not yet implemented" << std::endl;
            return content;
        case DocumentFormat::kDocx:
            // TODO(xinyan): Implement DOCX parsing (requires external library)
            std::cerr << "DOCX parsing not yet implemented" << std::endl;
            return content;
        case DocumentFormat::kMarkdown:
            // For now, treat markdown as plain text
            return parsePlainText(content);
        default:
            return parsePlainText(content);
    }
}

auto DocumentParserFunction::detectFormat(const std::string& content) const -> DocumentFormat {
    // Simple format detection based on content
    if (content.find("<!DOCTYPE") != std::string::npos || 
        content.find("<html") != std::string::npos) {
        return DocumentFormat::kHtml;
    }
    
    if (content.find("%PDF") != std::string::npos) {
        return DocumentFormat::kPdf;
    }
    
    if (content.find("PK") == 0) {  // ZIP file header (DOCX is ZIP-based)
        return DocumentFormat::kDocx;
    }
    
    // Check for markdown indicators
    if (content.find("# ") != std::string::npos || 
        content.find("## ") != std::string::npos ||
        content.find("**") != std::string::npos) {
        return DocumentFormat::kMarkdown;
    }
    
    return DocumentFormat::kPlainText;
}

auto DocumentParserFunction::parseHtml(const std::string& html_content) const -> std::string {
    // Simple HTML tag removal (for basic HTML parsing)
    // TODO(xinyan): Use proper HTML parser library for production
    std::string result = html_content;
    
    // Remove script and style tags with content
    size_t pos = 0;
    while ((pos = result.find("<script", pos)) != std::string::npos) {
        size_t end = result.find("</script>", pos);
        if (end != std::string::npos) {
            result.erase(pos, end - pos + 9);
        } else {
            break;
        }
    }
    
    pos = 0;
    while ((pos = result.find("<style", pos)) != std::string::npos) {
        size_t end = result.find("</style>", pos);
        if (end != std::string::npos) {
            result.erase(pos, end - pos + 8);
        } else {
            break;
        }
    }
    
    // Remove HTML tags
    pos = 0;
    while ((pos = result.find('<', pos)) != std::string::npos) {
        size_t end = result.find('>', pos);
        if (end != std::string::npos) {
            result.erase(pos, end - pos + 1);
        } else {
            break;
        }
    }
    
    // Decode common HTML entities
    size_t entity_pos = 0;
    while ((entity_pos = result.find("&amp;", entity_pos)) != std::string::npos) {
        result.replace(entity_pos, 5, "&");
        entity_pos += 1;
    }
    while ((entity_pos = result.find("&lt;", entity_pos)) != std::string::npos) {
        result.replace(entity_pos, 4, "<");
        entity_pos += 1;
    }
    while ((entity_pos = result.find("&gt;", entity_pos)) != std::string::npos) {
        result.replace(entity_pos, 4, ">");
        entity_pos += 1;
    }
    while ((entity_pos = result.find("&nbsp;", entity_pos)) != std::string::npos) {
        result.replace(entity_pos, 6, " ");
        entity_pos += 1;
    }
    
    return result;
}

auto DocumentParserFunction::parsePlainText(const std::string& text_content) const -> std::string {
    if (!config_.preserve_structure_) {
        // Normalize whitespace
        std::string result = text_content;
        
        // Replace multiple whitespace with single space
        size_t pos = 0;
        while ((pos = result.find("  ", pos)) != std::string::npos) {
            result.replace(pos, 2, " ");
        }
        
        // Remove leading/trailing whitespace
        result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
        }));
        result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
        }).base(), result.end());
        
        return result;
    }
    
    return text_content;
}

auto DocumentParserFunction::extractMetadata(const std::string& content, DocumentFormat format) const 
    -> std::unordered_map<std::string, std::string> {
    
    std::unordered_map<std::string, std::string> metadata;
    
    // Add basic metadata
    metadata["format"] = [format]() {
        switch (format) {
            case DocumentFormat::kPlainText: return "text/plain";
            case DocumentFormat::kHtml: return "text/html";
            case DocumentFormat::kPdf: return "application/pdf";
            case DocumentFormat::kDocx: return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
            case DocumentFormat::kMarkdown: return "text/markdown";
            default: return "unknown";
        }
    }();
    
    metadata["content_length"] = std::to_string(content.length());
    metadata["encoding"] = config_.encoding_;
    
    // Extract title for HTML documents
    if (format == DocumentFormat::kHtml) {
        size_t title_start = content.find("<title>");
        if (title_start != std::string::npos) {
            title_start += 7; // length of "<title>"
            size_t title_end = content.find("</title>", title_start);
            if (title_end != std::string::npos) {
                metadata["title"] = content.substr(title_start, title_end - title_start);
            }
        }
    }
    
    return metadata;
}

} // namespace sage_flow
