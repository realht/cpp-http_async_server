#pragma once
#include "http_server.h"

namespace api_handler {

    namespace beast = boost::beast;
    namespace http = beast::http;

    struct ErrorParam {
        http::status http_status;
        std::string_view body;
        bool need_allow;
        std::string_view allow_method;
        std::string_view cont_type;
        std::string_view cache;
    };

    struct CheckParam {
        http::verb allow_method1;
        http::verb allow_method2;
        bool auth_header;
        bool cont_type;
        std::string_view cont_name;
        ErrorParam ah;
        ErrorParam ct;
    };

    constexpr std::string_view NOT_FOUND = "{\n\t\"code\": \"notFound\",\n\t\"message\": \"Not found\"\n}";
    constexpr std::string_view NO_MAP = "{\n\t\"code\": \"mapNotFound\",\n\t\"message\": \"Map not found\"\n}";
    constexpr std::string_view BAD_REQUEST = "{\n\t\"code\": \"badRequest\",\n\t\"message\": \"Bad request\"\n}";
    constexpr std::string_view PARSE_JSON_ERROR = "{\n\t\"code\": \"invalidArgument\",\n\t\"message\": \"The data is incorrect. Error parsing json\"\n}";
    constexpr std::string_view INVALID_TOKEN = "{\n\t\"code\": \"invalidToken\",\n\t\"message\": \"No authorization\"\n}";
    constexpr std::string_view TOKEN_NOT_FOUND = "{\n\t\"code\": \"unknownToken\",\n\t\"message\": \"Player token has not been found\"\n}";
    constexpr std::string_view POST_INVALID_METHOD = "{\n\t\"code\": \"invalidMethod\",\n\t\"message\": \"Invalid method, only POST method\"\n}";
    constexpr std::string_view GET_HEAD_INVALID_METHOD = "{\n\t\"code\": \"invalidMethod\",\n\t\"message\": \"Invalid method, only GET or HEAD method\"\n}";
    constexpr std::string_view INVALID_CONTENT = "{\n\t\"code\": \"invalidArgument\",\n\t\"message\": \"Invalid content type\"\n}";

    constexpr std::string_view API_MAP = "/api/v1/maps";
    constexpr std::string_view API_JOIN = "/api/v1/game/join";
    constexpr std::string_view API_PLAYERS = "/api/v1/game/players";
    constexpr std::string_view API_STATE = "/api/v1/game/state";
    constexpr std::string_view API_ACTION = "/api/v1/game/player/action";
    constexpr std::string_view API_TICK = "/api/v1/game/tick";
    constexpr std::string_view API_RECORDS = "api/v1/game/records";

    constexpr std::string_view ALLOW_METHOD_GET_HEAD = "GET, HEAD";
    constexpr std::string_view ALLOW_METHOD_POST = "POST";

    constexpr int MAP_NAME_PREFIX_SIZE = 13;
    constexpr int TOKEN_PREFIX_SIZE = 7;

    constexpr std::string_view CONT_TYPE_JSON = "application/json";
    constexpr std::string_view NO_CACHE = "no-cache";

    constexpr std::string_view ID_STR = "id";
    constexpr std::string_view NAME_STR = "name";
    constexpr std::string_view ROADS_STR = "roads";
    constexpr std::string_view BUILDINGS_STR = "buildings";
    constexpr std::string_view OFFICES_STR = "offices";
    constexpr std::string_view LOOT_TYPES_STR = "lootTypes";
    constexpr std::string_view X_STR = "x";
    constexpr std::string_view Y_STR = "y";
    constexpr std::string_view X0_STR = "x0";
    constexpr std::string_view Y0_STR = "y0";
    constexpr std::string_view X1_STR = "x1";
    constexpr std::string_view Y1_STR = "y1";
    constexpr std::string_view EMPTY_STR = "";

    constexpr http::status METHOD_NOT_ALLOWED = http::status::method_not_allowed;
    constexpr http::status METHOD_UNAUTHIRIZED = http::status::unauthorized;
    constexpr http::status METHOD_BAD_REQUEST = http::status::bad_request;

    constexpr http::verb GET_METHOD = http::verb::get;
    constexpr http::verb HEAD_METHOD = http::verb::head;
    constexpr http::verb POST_METHOD = http::verb::post;
    constexpr http::verb NULL_METHOD = http::verb::delete_;

    //func param
    constexpr ErrorParam ERROR_PARAM_NOT_POST_METHOD{ .http_status = METHOD_NOT_ALLOWED, 
        .body = POST_INVALID_METHOD, .need_allow = true, .allow_method = ALLOW_METHOD_POST };
    constexpr ErrorParam ERROR_PARAM_NOT_GET_HEAD_METHOD{ .http_status = METHOD_NOT_ALLOWED, 
        .body = GET_HEAD_INVALID_METHOD, .need_allow = true, .allow_method = ALLOW_METHOD_GET_HEAD };
    constexpr ErrorParam ERROR_PARAM_BAD_AUTHORIZE{ .http_status = METHOD_UNAUTHIRIZED, .body = INVALID_TOKEN };
    constexpr ErrorParam ERROR_PARAM_WRONG_CONTENT_TYPE{ .http_status = METHOD_BAD_REQUEST, .body = INVALID_CONTENT };

    constexpr CheckParam API_MAPS_CHECK_PARAM{ .allow_method1 = GET_METHOD, .allow_method2 = HEAD_METHOD, 
        .auth_header = false, .cont_type = false, .cont_name = EMPTY_STR };
    constexpr CheckParam API_JOIN_CHECK_PARAM{ .allow_method1 = POST_METHOD, .allow_method2 = NULL_METHOD, 
        .auth_header = false, .cont_type = false, .cont_name = EMPTY_STR };
    constexpr CheckParam API_PLAYERS_STATE_CHECK_PARAM{ .allow_method1 = GET_METHOD, .allow_method2 = HEAD_METHOD, 
        .auth_header = true, .cont_type = false, .cont_name = EMPTY_STR, .ah = ERROR_PARAM_BAD_AUTHORIZE };
    constexpr CheckParam API_ACTION_CHECK_PARAM{ .allow_method1 = POST_METHOD, .allow_method2 = NULL_METHOD, 
        .auth_header = true, .cont_type = true, .cont_name = CONT_TYPE_JSON, .ah = ERROR_PARAM_BAD_AUTHORIZE, 
        .ct = ERROR_PARAM_WRONG_CONTENT_TYPE };
    constexpr CheckParam API_TICK_CHECK_PARAM{ .allow_method1 = POST_METHOD, .allow_method2 = NULL_METHOD, 
        .auth_header = false, .cont_type = false, .cont_name = CONT_TYPE_JSON, .ah = {}, .ct = ERROR_PARAM_WRONG_CONTENT_TYPE };

}