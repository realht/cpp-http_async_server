#pragma once
#include <random> 
#include <boost/json.hpp>
#include "app.h"
#include "api_handler_static_name.h"

namespace api_handler {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    using StringResponse = http::response<http::string_body>;

    class Api {
    public:

        explicit Api(app::Application& apl) :
            apl_{ apl } {}


        StringResponse GetStringResponse(const std::string& str, unsigned http_version);

        StringResponse GetUserResponse(const std::string& str, unsigned http_version) ;

        StringResponse GetStateResponse(const std::string& str, unsigned http_version);

        StringResponse PostUserAuthResponse(const std::string& str, unsigned http_version) ;

        StringResponse PostUserMoveResponse(const std::string& str, const std::string& auth, unsigned http_version) ;

        StringResponse SetTickAndGetResponse(const std::string& str, unsigned http_version);

        StringResponse GetRecordsResponse(const std::string& str, unsigned http_version);

        /**
         * @brief Returns an error response.
         *
         * This method returns a response with an explanation of the error that occurred.
         *
         * @param http_version HTTP protocol version.
         * @param status HTTP request status.
         * @param body Error message text.
         * @param allow If true, the request was made with an incorrect method (default is false).
         * @param allow_method Allowed request method (default is an empty string).
         * @param type Response type (default is application/json).
         * @param cache Cachable response or not.
         * @return Error response (default is no-cache).
         */
        static StringResponse GetErrorResponse(unsigned http_version, http::status status, std::string_view body, 
            bool allow = false, std::string_view allow_method = "", std::string_view type = CONT_TYPE_JSON, 
            std::string_view cache = NO_CACHE);

        
        static bool TokenIsVadid(const std::string& str);
        static double GetMidleRange(double a, double b);
        static std::string URL_encode(const std::string& str);
        static int ConvertHexCharToInt(char c);
    
    private:

        static StringResponse GetTemplateResponse(unsigned http_version); 

        const void PrintMaps(std::ostream& out) const ;
        const void PrintMap(const model::Map* map, std::ostream& out) const;

        const json::object PrintRoadAsJson(const model::Road& road) const;
        const json::object PrintBuildingAsJson(const model::Building& build) const;
        const json::object PrintOfficeAsJson(const model::Office& office) const;

        std::string GetUserList(const std::string& token) const;

        std::string GetPlayersState(const std::string& token) const;

        std::string GetAnswerUserAuthSuccess(const app::PlayerInfo& pi ) const ;

        void ChangeMoveDirection(const std::string& auth, const std::string& dir);

        static std::pair< model::Speed, model::Direction> GetSpeedDirection(const std::string& dir, double spd);

        void UpdateWorldState(int delta_time);

        const std::string GetRecordTable(int offset, int max_elem);

    private:
        app::Application& apl_;
    };


    class ApiHandler {
    public:
        ApiHandler(app::Application& apl)
            : api_{ apl } {
        }

        static StringResponse GetErrorResponse(unsigned http_version, http::status status, std::string_view body,
            bool allow = false, std::string_view allow_method = "", std::string_view type = CONT_TYPE_JSON,
            std::string_view cache = NO_CACHE) {
            return Api::GetErrorResponse(http_version, status, body, allow, allow_method, type, cache);
        }

        template <typename Body>
        StringResponse GetApiResponse(Body&& req) {
            auto version = req.version();
            std::string req_string = api_handler::Api::URL_encode(std::string(req.target()));
            auto authorization_header = req[http::field::authorization];
            std::string auth = std::string(authorization_header.data(), authorization_header.size());
            auto content_type = req[http::field::content_type];
            std::string type = std::string(content_type.data(), content_type.size());
            
            if (req_string.find(API_MAP) != std::string::npos) {
                return TemplateResponse(req, API_MAPS_CHECK_PARAM, ERROR_PARAM_NOT_GET_HEAD_METHOD,  [this](const std::string& str, unsigned http_version) {
                    return api_.GetStringResponse(str, http_version); }, req_string, version);
            }
            else if (req_string.find(API_JOIN) != std::string::npos) {
                return TemplateResponse(req, API_JOIN_CHECK_PARAM, ERROR_PARAM_NOT_POST_METHOD, [this](const std::string& str, unsigned http_version) {
                    return api_.PostUserAuthResponse(str, http_version); }, req.body(), version);
            }
            else if (req_string.find(API_PLAYERS) != std::string::npos) {
                return TemplateResponse(req, API_PLAYERS_STATE_CHECK_PARAM, ERROR_PARAM_NOT_GET_HEAD_METHOD, [this](const std::string& str, unsigned http_version) {
                    return api_.GetUserResponse(str, http_version); }, auth, version);
            }
            else if (req_string.find(API_STATE) != std::string::npos) {
                return TemplateResponse(req, API_PLAYERS_STATE_CHECK_PARAM, ERROR_PARAM_NOT_GET_HEAD_METHOD, [this](const std::string& str, unsigned http_version) {
                    return api_.GetStateResponse(str, http_version); }, auth, version);
            }
            else if (req_string.find(API_ACTION) != std::string::npos) {
                return TemplateResponse(req, API_ACTION_CHECK_PARAM, ERROR_PARAM_NOT_POST_METHOD, [this](const std::string& str, const std::string& auth, unsigned http_version) {
                    return api_.PostUserMoveResponse(str, auth, http_version); }, req.body(), auth, version);
            }
            else if (req_string.find(API_TICK) != std::string::npos) {
                return TemplateResponse(req, API_TICK_CHECK_PARAM, ERROR_PARAM_NOT_POST_METHOD, [this](const std::string& str, unsigned http_version) {
                    return api_.SetTickAndGetResponse(str, http_version); }, req.body(), version);
            }
            else if (req_string.find(API_RECORDS) != std::string::npos) {
                return TemplateResponse(req, API_MAPS_CHECK_PARAM, ERROR_PARAM_NOT_GET_HEAD_METHOD, [this](const std::string& str, unsigned http_version) {
                    return api_.GetRecordsResponse(str, http_version); }, std::string(req.target()), version);
            }
            return api_.GetErrorResponse(version, http::status::bad_request, BAD_REQUEST);
        }

    private:

        template <typename Body, typename Fn, typename... Args>
        StringResponse TemplateResponse(Body&& req, const CheckParam& cp, const ErrorParam& ep, Fn action, Args&&... args) {
            auto version = req.version();

            if (cp.allow_method2 != http::verb::delete_) {
                if (req.method() != cp.allow_method1 && req.method() != cp.allow_method2) {
                    return api_.GetErrorResponse(version, ep.http_status, ep.body, ep.need_allow, ep.allow_method, ep.cont_type, ep.cache);
                }
            }
            else if (cp.allow_method1 != http::verb::delete_) {
                if (req.method() != cp.allow_method1) {
                    return api_.GetErrorResponse(version, ep.http_status, ep.body, ep.need_allow, ep.allow_method, ep.cont_type, ep.cache);
                }
            }

            if (cp.auth_header) {
                auto authorization_header = req[http::field::authorization];
                std::string auth = std::string(authorization_header.data(), authorization_header.size());
                if (auth.size() < TOKEN_PREFIX_SIZE) {
                    return api_.GetErrorResponse(version, cp.ah.http_status, cp.ah.body, cp.ah.need_allow, cp.ah.allow_method, cp.ah.cont_type, cp.ah.cache);
                }
            }

            if (cp.cont_type) {
                auto content_type = req[http::field::content_type];
                std::string type = std::string(content_type.data(), content_type.size());
                if (type != cp.cont_name) {
                    return api_.GetErrorResponse(version, cp.ct.http_status, cp.ct.body, cp.ct.need_allow, cp.ct.allow_method, cp.ct.cont_type, cp.ct.cache);
                }
            }

            return action(std::forward<Args>(args)...);
        }



    private:
        Api api_;
    };
}