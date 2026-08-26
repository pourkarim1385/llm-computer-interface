#include "SearchTypes.h"
#include <sstream>

namespace WebSearch {

std::string SearchResponse::to_llm_context() const {
    std::ostringstream oss;
    oss << "[Source: " << provider_name << "]\n";
    oss << "Search Results for: \"" << query << "\"\n";
    if (!answer.empty()) {
        oss << "Summary: " << answer << "\n";
    }
    oss << "------------------------------------------\n";
    for (size_t i = 0; i < results.size(); ++i) {
        oss << "[" << (i + 1) << "] Title: " << results[i].title << "\n"
            << "URL: " << results[i].url << "\n"
            << "Snippet: " << results[i].content << "\n\n";
    }
    return oss.str();
}

}