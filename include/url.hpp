#pragma once

#include <optional>
#include <string>

namespace crawler {
    struct Url{
        std::string port;
        std::string path;
        std::string host;
        std::string schema;

        static std::optional<Url> parse(const std::string& urlString);

        static std::optional<Url> resolve(const Url& baseUrl,const std::string& link);

        std::string toString() const;

    };
}