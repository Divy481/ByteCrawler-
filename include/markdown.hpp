#pragma once

#include <cstddef>
#include <string>

namespace crawler {
    class MarkDown{
        public:
            std::string convert(std::string& html);
            std::string markdown;

        private:
            std::string Markhtml;
            
            std::size_t pos = 0;
            
            // Parser state
            bool in_script = false;
            bool in_style = false;
            std::string current_href = "";
            bool in_a = false;
            
            void parserTag();
            void parserText();
            std::string getTagName(const std::string& tag);
            std::string extractAttribute(const std::string& tagContent, const std::string& attrName);
    };
}