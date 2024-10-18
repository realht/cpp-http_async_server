#pragma once
#include <variant>
#include "logger.h"
#include "request_handler.h"

namespace server_logger {
    using namespace std::literals;

    namespace beast = boost::beast;
    namespace http = beast::http;

    template<class SomeRequestHandler>
    class LoggingRequestHandler {

        template <typename Body, typename Allocator>
        static void LogRequest(const http::request<Body, http::basic_fields<Allocator>>& r, const std::string& ip);
        static void LogResponse(const std::string& ip, long time, unsigned int code, std::string_view ctype);
    
    public:

        explicit LoggingRequestHandler(SomeRequestHandler&& decorated)
            : decorated_{ std::move(decorated) } {
        }

        LoggingRequestHandler(const LoggingRequestHandler&) = delete;
        LoggingRequestHandler& operator=(const LoggingRequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator () (const std::string& ip_client, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            
            LogRequest(req, ip_client);
            
            std::chrono::system_clock::time_point begin_ts = std::chrono::system_clock::now();
            
            auto inter_send = [begin_ts, ip_client, send = std::forward<Send>(send)](http_handler::Response&& inter_resp) {
                auto resp = std::move(inter_resp);
                auto end_ts = std::chrono::system_clock::now();
                auto resp_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_ts - begin_ts);

                auto log_and_send = [&](auto&& response_data) {
                    LogResponse(ip_client, resp_time.count(), response_data.result_int(), response_data.at(http::field::content_type));
                    send(std::move(response_data));
                };

                std::visit([&](auto&& response_data) {
                    using ResponseType = std::decay_t<decltype(response_data)>;
                    if constexpr (std::is_same_v<http_handler::StringResponse, ResponseType> ||
                        std::is_same_v<http_handler::FileResponse, ResponseType>) {
                        log_and_send(std::forward<decltype(response_data)>(response_data));
                    }
                    }, resp);
            };

            decorated_(std::move(req), std::move(inter_send));
            
        }

    private:
        SomeRequestHandler decorated_;
    };


    template<class SomeRequestHandler>
    template<typename Body, typename Allocator>
    void LoggingRequestHandler<SomeRequestHandler>::LogRequest(const http::request<Body, 
                                http::basic_fields<Allocator>>& r, const std::string& ip) {
        std::string method;
            if (r.method() == http::verb::get) {
                method = "GET";
            }
            else if (r.method() == http::verb::head) {
                method = "HEAD";
            }
            else if (r.method() == http::verb::post) {
                method = "POST";
            }

        json::value request = { {"ip", ip}, {"URI", r.target()}, {"method", method}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, request)
            << "request received"sv;
    }

    template<class SomeRequestHandler>
    void LoggingRequestHandler<SomeRequestHandler>::LogResponse(const std::string& ip, long time, unsigned int code, std::string_view ctype) {
        json::value request = { {"ip", ip}, {"response_time", time}, {"code", code}, {"content_type", ctype} };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, request)
            << "response sent"sv;
    }

}

