#pragma once

#include "http_response.hpp"
#include <string>
#include <cstddef>

namespace http_parser {

    class HttpParser{
        public:

            HttpParser();
            enum State{
                StatusLine,
                Headers,
                Body,
                ChunkSize,
                ChunkExtension,
                ChunkData,
                ChunkCRLF,
                Trailer,
                Complete,
                Error
            };


            void reset();
            std::size_t excute(const char* data,std::size_t len,HttpResponse& resp);

            bool isComplete() const {return state_ == State::Complete;}
            bool isError() const {return state_ == State::Error;}

        private:
            State state_;
            std::string buffer_;
            size_t contentLength_;
            size_t chunkSize_;
            size_t bodyByteRead_;
            size_t chunkByteRead_;

            bool parserStatusLine(HttpResponse& resp);
            bool parserHeader(HttpResponse& resp);

            void trim(std::string& str);

            void toLower(std::string& str);

            

    };
}