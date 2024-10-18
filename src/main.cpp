//#include "sdk.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include "json_loader.h"
#include "logging_request_handler.h"
#include "parse_command_line.h"
#include "ticker.h"


using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace logging = boost::log;
namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value);

constexpr const char DB_URL[]{ "GAME_DB_URL" };

namespace {

template <typename Fn>
void RunWorkers(unsigned num_threads, const Fn& fn) {
    num_threads = std::max(1u, num_threads);
    std::vector<std::jthread> workers;
    workers.reserve(num_threads - 1);

    while (--num_threads) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    server_logger::SetLogFormat();
    command_line::Args args;

    try {
        args = command_line::ParseCommandLine(argc, argv).value();
        extra_data::TrophyList trophies;

        const char* db_url = std::getenv(DB_URL);

        // for local base test
        //const char* db_url = "postgres://postgres:1231234@localhost:5432/book_db"; 

        if (!db_url) {
            throw std::runtime_error("DB URL is not specified");
        }

        model::Game game = json_loader::LoadGame(args.config_path, trophies);

        app::Players players;
        bool self_control = (args.update_period == 0) ? true : false;
        app::AppConfig conf{ self_control, args.random_position, args.save_path, args.save_time_period, db_url };

        app::Application apl(game, players, trophies, conf);

        if (!args.save_path.empty() && std::filesystem::exists(args.save_path)) {
            try {
                apl.UploadGameState();
            }
            catch (const std::exception& e) {
                json::value custom_data{ {"exception", e.what()} };
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                    << "server state can`t be loaded, file state broken"sv;
            }
        }

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, &apl, args](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {

                if (!args.save_path.empty()) {
                    try {
                        apl.SaveGameState();
                    }
                    catch (const std::exception& e) {
                        json::value custom_data{ {"exception", e.what()} };
                        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                            << "server state not save"sv;
                    }
                }

                json::value custom_data{ {"code"s, 0} };
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                    << "server exited"sv;

                ioc.stop();
            }
            });

        // strand для выполнения запросов к API
        auto api_strand = net::make_strand(ioc);

        auto handler = std::make_shared<http_handler::RequestHandler>(apl, args.web_folder, api_strand);

        server_logger::LoggingRequestHandler log_handler{
           [handler](auto&& req, auto&& send) {
                (*handler)(
                    std::forward<decltype(req)>(req),
                    std::forward<decltype(send)>(send));
                    } };

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port}, [&log_handler](auto&& ip_client, auto&& req, auto&& send) {
            log_handler(std::forward<decltype(ip_client)>(ip_client), std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });
        
        json::value serv = {{"port", port}, {"address", address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, serv)
            << "server started"sv;

        if (args.update_period != 0) {
            std::chrono::milliseconds duration(args.update_period);
            auto ticker = std::make_shared<timer::Ticker>(api_strand, duration,
                [&apl](std::chrono::milliseconds delta) {
                    apl.UpdateWorldState(delta.count());
                }
            );
            ticker->Start();
        }

        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

    }
    catch (const std::exception& ex) {
        json::value custom_data{ {"code"s, EXIT_FAILURE}, {"exception", ex.what()} };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
            << "server exited"sv;
        return EXIT_FAILURE;
    }

}
