#include "../include/markdown.hpp"
#include <cstddef>

namespace crawler {

    std::string MarkDown::convert(std::string& html){
        Markhtml = html;
        markdown.clear();
        pos = 0;

        while(pos < Markhtml.size()){
            if(Markhtml[pos] == '<'){
                parserTag();
            }else{
                parserText();
            }
        }

        return markdown;
    }

    void MarkDown::parserText(){
        size_t start = pos;

        while(pos<Markhtml.size() && Markhtml[pos]!='<'){
            pos++;
        }

        if (!in_script && !in_style) {
            markdown += Markhtml.substr(start, pos - start);
        }
    }

    void MarkDown::parserTag(){
        size_t end = Markhtml.find('>',pos);

        if(end == std::string::npos) {
            pos = Markhtml.size(); 
            return;
        }

        std::string tagContent = Markhtml.substr(pos + 1, end - pos - 1);
        std::string tag = tagContent;
        
        // Strip out any attributes to get just the tag name
        size_t space_pos = tag.find(' ');
        if (space_pos != std::string::npos) {
            tag = tag.substr(0, space_pos);
        }

        bool closing = false;
        if(!tag.empty() && tag[0] =='/')closing = true;

        if(closing)tag.erase(0,1);

        // Convert tag to lowercase for case-insensitive matching
        for (char& c : tag) c = std::tolower(c);

        if(!closing){
            if(tag == "h1") markdown += "\n# ";
            else if(tag == "h2") markdown += "\n## ";
            else if(tag == "h3") markdown += "\n### ";
            else if(tag == "h4") markdown += "\n#### ";
            else if(tag == "h5") markdown += "\n##### ";
            else if(tag == "h6") markdown += "\n###### ";
            else if(tag == "strong" || tag == "b") markdown += "**";
            else if(tag == "em" || tag == "i") markdown += "*";
            else if(tag == "p") markdown += "\n\n";
            else if(tag == "br") markdown += "\n";
            else if(tag == "hr") markdown += "\n---\n";
            else if(tag == "li") markdown += "\n- ";
            else if(tag == "script") in_script = true;
            else if(tag == "style") in_style = true;
            else if(tag == "a") {
                in_a = true;
                current_href = extractAttribute(tagContent, "href");
                markdown += "[";
            }
            else if(tag == "img") {
                std::string src = extractAttribute(tagContent, "src");
                std::string alt = extractAttribute(tagContent, "alt");
                markdown += "![" + alt + "](" + src + ")";
            }
        }else{
            if(tag == "h1" || tag == "h2" || tag == "h3" || 
               tag == "h4" || tag == "h5" || tag == "h6") {
                markdown += "\n";
            }
            else if(tag == "strong" || tag == "b") markdown += "**";
            else if(tag == "em" || tag == "i") markdown += "*";
            else if(tag == "p") markdown += "\n";
            else if(tag == "script") in_script = false;
            else if(tag == "style") in_style = false;
            else if(tag == "a") {
                in_a = false;
                markdown += "](" + current_href + ")";
                current_href = "";
            }
        }

        pos = end + 1;
    }

    std::string MarkDown::extractAttribute(const std::string& tagContent, const std::string& attrName) {
        std::string search = attrName + "=\"";
        size_t start = tagContent.find(search);
        if (start != std::string::npos) {
            start += search.length();
            size_t end = tagContent.find("\"", start);
            if (end != std::string::npos) {
                return tagContent.substr(start, end - start);
            }
        }
        
        // Try single quotes
        search = attrName + "='";
        start = tagContent.find(search);
        if (start != std::string::npos) {
            start += search.length();
            size_t end = tagContent.find("'", start);
            if (end != std::string::npos) {
                return tagContent.substr(start, end - start);
            }
        }
        return "";
    }
}