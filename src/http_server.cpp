#include "http_server.h"

#include <boost/asio/dispatch.hpp>
#include <iostream>

namespace http_server {

    void SessionBase::Run() {
        net::dispatch(stream_.get_executor(),
            beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
    }

    void ReportError(beast::error_code ec, std::string_view what) {
        using namespace std::literals;
        std::cerr << what << ": "sv << ec.message() << std::endl;
    }

    void SessionBase::Read() {
        using namespace std::literals;

        request_ = {};
        stream_.expires_after(30s);

        http::async_read(stream_, buffer_, request_,
            beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
    }

    void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
        using namespace std::literals;
        if (ec == http::error::end_of_stream) {
            return Close();
        }
        if (ec) {
            json::value custom_data{ {"code"s, ec.value()}, {"text", ec.message()}, {"where", "read"} };
            BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                << "error"sv;

            return /*ReportError(ec, "read"sv)*/;
        }
        HandleRequest(std::move(request_));
    }

    void SessionBase::Close() {
        stream_.socket().shutdown(tcp::socket::shutdown_send);
    }

    void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
        using namespace std::literals;
        if (ec) {
            json::value custom_data{ {"code"s, ec.value()}, {"text", ec.message()}, {"where", "write"} };
            BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                << "error"sv;
            return /*ReportError(ec, "write"sv)*/;
        }

        if (close) {
            // Семантика ответа требует закрыть соединение
            return Close();
        }

        Read();
    }


}  //http_server
