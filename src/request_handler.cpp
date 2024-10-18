#include "request_handler.h"

namespace http_handler {

    FileResponse RequestHandler::GetFileResponse(const std::string& str, unsigned http_version) const {
        FileResponse response;
        response.version(http_version);
        response.set(http::field::server, "Game Server");
        response.result(http::status::ok);

        response.insert(http::field::content_type, GetMimeType(str));
        http::file_body::value_type file;

        std::string file_path_str = root_.string() + str;
        std::filesystem::path file_path(file_path_str);

        if (sys::error_code ec; file.open(file_path_str.c_str(), beast::file_mode::read, ec), ec) {
            std::cout << "Failed to open file " << file_path_str << std::endl;
            return response;
        }

        response.body() = std::move(file);
        response.prepare_payload();
        return response;
    }

    bool RequestHandler::IsValidPath(const std::string& str) const {
        std::filesystem::path base = root_;
        std::filesystem::path check(base.string() + str);

        base = std::filesystem::weakly_canonical(base);
        check = std::filesystem::weakly_canonical(check);
        for (auto b = base.begin(), p = check.begin(); b != base.end(); ++b, ++p) {
            if (p == check.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

    std::string_view RequestHandler::GetMimeType(std::string_view path) const {
        std::string file_path_str = root_.string() + path.data();
        std::filesystem::path file_path(file_path_str);
        if (!std::filesystem::exists(file_path)) {
            return "";
        }
        if (!std::filesystem::is_regular_file(file_path)) {
            return "";
        }

        using beast::iequals;
        auto const ext = [&path]
        {
            auto const pos = path.rfind(".");
            if (pos == std::string_view::npos)
                return std::string_view{};
            return path.substr(pos);
        }();

        if (iequals(ext, ".htm"))  return "text/html";
        if (iequals(ext, ".html")) return "text/html";
        if (iequals(ext, ".css"))  return "text/css";
        if (iequals(ext, ".txt"))  return "text/plain";
        if (iequals(ext, ".js"))   return "text/javascript";
        if (iequals(ext, ".json")) return "application/json";
        if (iequals(ext, ".xml"))  return "application/xml";
        if (iequals(ext, ".png"))  return "image/png";
        if (iequals(ext, ".jpe"))  return "image/jpeg";
        if (iequals(ext, ".jpeg")) return "image/jpeg";
        if (iequals(ext, ".jpg"))  return "image/jpeg";
        if (iequals(ext, ".gif"))  return "image/gif";
        if (iequals(ext, ".bmp"))  return "image/bmp";
        if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
        if (iequals(ext, ".tiff")) return "image/tiff";
        if (iequals(ext, ".tif"))  return "image/tiff";
        if (iequals(ext, ".svg"))  return "image/svg+xml";
        if (iequals(ext, ".svgz")) return "image/svg+xml";
        if (iequals(ext, ".mp3")) return "audio/mpeg";
        return "application/octet-stream";
    }

}  // http_handler
