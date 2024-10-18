#pragma once
#include "api_handler.h"
#include <boost/beast.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <variant>


namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace sys = boost::system;
    namespace net = boost::asio;
    namespace json = boost::json;

    using StringResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;
    using Response = std::variant<StringResponse, FileResponse>;


    class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
    public:

        using Strand = net::strand<net::io_context::executor_type>;      

        explicit RequestHandler(app::Application& apl, std::filesystem::path root, Strand api_strand)
            : root_(std::move(root))
            , api_strand_{ api_strand }
            , apiHandlerPtr_{ std::make_shared<api_handler::ApiHandler>(apl) }
        { }

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()( http::request<Body, http::basic_fields<Allocator>> && req, Send && send) {
            std::string req_string = api_handler::Api::URL_encode(std::string(req.target()));
            auto version = req.version();

            if (req_string.find("/api/") != std::string::npos) {

                net::dispatch(api_strand_, [self = shared_from_this(), req = std::forward<decltype(req)>(req), version, send]{
                    try {
                        assert(self->api_strand_.running_in_this_thread());
                        send(std::move(self->apiHandlerPtr_->GetApiResponse(std::move(req))));
                        }
                    catch (std::exception) {
                        send(std::move(self->apiHandlerPtr_->GetErrorResponse(version, http::status::bad_request,
                            api_handler::BAD_REQUEST)));
                        }
                });
            }
            else { //this case for file or error not file
                if (req_string.back() == '/') {
                    req_string += "index.html";
                }
                if (IsValidPath(req_string) && !GetMimeType(req_string).empty()) {
                    send(std::move(GetFileResponse(req_string, req.version())));
                }

                else if (!IsValidPath(req_string)) {
                    send(std::move(apiHandlerPtr_->GetErrorResponse(version, http::status::bad_request, api_handler::BAD_REQUEST, false, "", "text/plain")));
                }
                else if (GetMimeType(req_string).empty()) {
                    send(std::move(apiHandlerPtr_->GetErrorResponse(version, http::status::not_found, api_handler::NOT_FOUND, false,"","text/plain")));
                }
            }
        }

    private:
        std::shared_ptr<api_handler::ApiHandler> apiHandlerPtr_;
        const std::filesystem::path root_;
        Strand api_strand_;

        FileResponse GetFileResponse(const std::string& str, unsigned http_version) const;
        std::string_view GetMimeType(std::string_view path) const;
        
        bool IsValidPath(const std::string& str) const;
    };

}  // http_handler
