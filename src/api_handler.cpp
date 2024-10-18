#include "api_handler.h"

namespace api_handler {

    StringResponse Api::GetTemplateResponse(unsigned http_version) {
        StringResponse response;
        response.version(http_version);
        response.set(http::field::content_type, CONT_TYPE_JSON);
        response.set(http::field::cache_control, NO_CACHE);
        response.result(http::status::ok);
        response.body() = ""; // здесь тело ответа
        return response;
    }

    StringResponse Api::GetStringResponse(const std::string& str, unsigned http_version) {
        StringResponse response = GetTemplateResponse(http_version);

            if (str == API_MAP) {
                std::ostringstream ss;
                PrintMaps(ss);
                response.body() = ss.str();
            }
            else if (str.find(API_MAP) != std::string::npos && str.size() > MAP_NAME_PREFIX_SIZE) {
                model::Map::Id id(str.substr(MAP_NAME_PREFIX_SIZE));
                if (apl_.GetMap(id) != nullptr) {
                    std::ostringstream ss;
                    PrintMap(apl_.GetMap(id), ss);
                    response.body() = ss.str();
                }
                else {
                    response.result(http::status::not_found);
                    response.body() = NO_MAP;
                }
            }
            else {
                response.result(http::status::bad_request);
                response.body() = BAD_REQUEST;
            }
        response.prepare_payload();
        return response;
    }

    StringResponse Api::GetUserResponse(const std::string& str, unsigned http_version) {
        StringResponse response = GetTemplateResponse(http_version);

        std::string auth_token = str.substr(7, str.size());

        if (!TokenIsVadid(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, INVALID_TOKEN);
        }

        if (!apl_.HasToken(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, TOKEN_NOT_FOUND);
        }
        
        response.body() = GetUserList(auth_token);
        response.prepare_payload();
        return response;
    }

    StringResponse Api::PostUserAuthResponse(const std::string& str, unsigned http_version) {
        json::value req_mes = json::parse(str);

        StringResponse response = GetTemplateResponse(http_version);

        std::string name;
        std::string map_id;
        try {
            name = req_mes.as_object().at("userName").as_string().c_str();
            map_id = req_mes.as_object().at("mapId").as_string().c_str();
        }
        catch (std::exception) {
            return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
        }

        app::PlayerInfo player_info;

        try {
            player_info.Write( apl_.JoinGame(map_id, name));
        }
        catch (const app::Application::JoinPlayerErrorCode& jpec) {
            if (jpec == app::Application::JoinPlayerErrorCode::wrong_name) {
                return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
            }
            if (jpec == app::Application::JoinPlayerErrorCode::wrong_map) {
                return GetErrorResponse(http_version, http::status::not_found, NO_MAP);
            }
        }

        response.body() = GetAnswerUserAuthSuccess(player_info);
        response.prepare_payload();
        return response;
    }
    
    StringResponse Api::GetStateResponse(const std::string& str, unsigned http_version) {
        StringResponse response = GetTemplateResponse(http_version);

        std::string auth_token = str.substr(7, str.size());

        if (!TokenIsVadid(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, INVALID_TOKEN);
        }

        if (!apl_.HasToken(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, TOKEN_NOT_FOUND);
        }

        response.body() = GetPlayersState(auth_token);
        response.prepare_payload();
        return response;
    }

    StringResponse Api::GetErrorResponse(unsigned http_version, http::status status, std::string_view body,
        bool allow, std::string_view allow_method, std::string_view type, std::string_view cache) {
        StringResponse response;
        response.version(http_version);

        response.set(http::field::content_type, (type.empty()) ? CONT_TYPE_JSON : type);

        response.set(http::field::cache_control, (cache.empty()) ? NO_CACHE : cache);

        response.result(status);
        if (allow) {
            response.set(http::field::allow, allow_method);
        }
        response.body() = body;
        response.prepare_payload();
        return response;
    }

    StringResponse Api::PostUserMoveResponse(const std::string& str, const std::string& auth, unsigned http_version) {
        json::value req_mes = json::parse(str);
        
        StringResponse response = GetTemplateResponse(http_version);

        std::string dir;
    
        try {
            dir = req_mes.as_object().at("move").as_string().c_str();
        }
        catch (std::exception) {
            return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
        }

        std::string auth_token = auth.substr(7, auth.size());

        if (!TokenIsVadid(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, INVALID_TOKEN);
        }

        if (!apl_.HasToken(auth_token)) {
            return GetErrorResponse(http_version, http::status::unauthorized, TOKEN_NOT_FOUND);
        }

        ChangeMoveDirection(auth_token, dir);

        json::object result;
        response.body() = json::serialize(result);
        response.prepare_payload();
        return response;
    }

    StringResponse Api::SetTickAndGetResponse(const std::string& str, unsigned http_version) {
        if (!apl_.isSelfMode()) {
            return GetErrorResponse(http_version, http::status::bad_request, BAD_REQUEST);
        }
        if (str.empty()) {
            return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
        }

        json::value req_mes = json::parse(str);

        StringResponse response = GetTemplateResponse(http_version);

        int delta_time;

        try {
            delta_time = req_mes.as_object().at("timeDelta").as_int64();
        }
        catch (std::exception) {
            return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
        }
        if (delta_time < 1) {
            return GetErrorResponse(http_version, http::status::bad_request, PARSE_JSON_ERROR);
        }

        UpdateWorldState(delta_time);

        json::object result;
        response.body() = json::serialize(result);
        response.prepare_payload();
        return response;
    }

    StringResponse Api::GetRecordsResponse(const std::string& str, unsigned http_version) {
        StringResponse response = GetTemplateResponse(http_version);

        int offset, max_elem;
        
        offset = db::DEFAULT_OFFSET;
        max_elem = db::DEFAULT_MAX_ELEMENT;

        size_t pos_start = str.find(db::OFFSET_STR);
        if (pos_start != std::string::npos) {
            size_t pos_end = std::min(str.find("&", pos_start), str.size());
            std::string str_start = str.substr(pos_start + db::OFFSET_STR.size(), pos_end - pos_start - db::OFFSET_STR.size());
            offset = std::stoi(str_start);
        }

        size_t pos_max_elem = str.find(db::MAX_ELEMENT_STR);
        if (pos_max_elem != std::string::npos) {
            std::string str_max_elem = str.substr(pos_max_elem + db::MAX_ELEMENT_STR.size(), str.size() - pos_max_elem - db::MAX_ELEMENT_STR.size());
            max_elem = std::stoi(str_max_elem);
        }

        if (max_elem > 100 || max_elem < 0) {
            return GetErrorResponse(http_version, http::status::bad_request, BAD_REQUEST);
        }

        response.body() = GetRecordTable(offset, max_elem);
        response.prepare_payload();
        return response;
    }

    void Api::ChangeMoveDirection(const std::string& auth, const std::string& dir){
        double speed = apl_.GetSession(auth)->GetMap()->GetDogSpeedOnMap();
        if (dir.empty()) {
            apl_.GetPlayerByToken(auth)->GetDog()->SetSpeed(model::ZEROSPEED);
        }
        else {
            apl_.GetPlayerByToken(auth)->GetDog()->EraseStayTime();
            std::pair<model::Speed, model::Direction> pair_s_d = GetSpeedDirection(dir, speed);
            apl_.GetPlayerByToken(auth)->GetDog()->SetSpeed(pair_s_d.first);
            apl_.GetPlayerByToken(auth)->GetDog()->SetDirection(pair_s_d.second); 
        }
    }

    std::pair<model::Speed, model::Direction> Api::GetSpeedDirection(const std::string& dir, double spd) {
        std::pair<model::Speed, model::Direction> speed_dir;
        if (dir == "U") {
            return speed_dir = { { 0,-spd }, model::Direction::North };
        }
        else if (dir == "D") {
            return speed_dir = { { 0,spd },model::Direction::South };
        }
        else if (dir == "L") {
            return speed_dir = { { -spd,0 },model::Direction::West };
        }
        else if (dir == "R") {
            return speed_dir = { { spd,0 }, model::Direction::East };
        }
        else {
            speed_dir = { { 0,0 },model::Direction::North };
        }
        return speed_dir;
    }


    std::string Api::URL_encode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str.at(i) == '%') {
                int temp1 = ConvertHexCharToInt(str.at(++i)) * 16;
                int temp2 = ConvertHexCharToInt(str.at(++i));
                char temp = static_cast<char>(temp1 + temp2);
                result.push_back(temp);
                continue;
            }
            else if (str.at(i) == '+') {
                result.push_back(' ');
                ++i;
            }
            result.push_back(str.at(i));
        }
        return result;
    }

    int Api::ConvertHexCharToInt(char c) {
        if (isdigit(c)) {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        else {
            return -1;
        }
    }

    double Api::GetMidleRange(double a, double b)  {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    bool Api::TokenIsVadid(const std::string& str) {
        return str.size() == 32;
    }

    void Api::UpdateWorldState(int delta_time) {
        apl_.UpdateWorldState(delta_time);
    }

    std::string Api::GetAnswerUserAuthSuccess(const app::PlayerInfo& pi) const {
        std::ostringstream ss;
        ss << "{\"authToken\":\"" << *pi.token << "\",\"playerId\":" << pi.id << "}";
        return ss.str();
    }

    std::string Api::GetUserList(const std::string& token) const {
        const model::GameSession* session = apl_.GetSession(token);
        boost::json::object result;

        for (const auto& player : apl_.GetPlayersList()) {
            if (player.second.get()->GetSession() == session) {
                json::object player_info;
                player_info["name"] = player.second.get()->GetName();
                result[std::to_string(player.first)] = player_info;
            }
        }

        return boost::json::serialize(result);
    }

    std::string Api::GetPlayersState(const std::string& token) const {
        const model::GameSession* session = apl_.GetSession(token);
        boost::json::object result;
        boost::json::object players_json;
        boost::json::object trophy_json;

        for (const auto& player : apl_.GetPlayersList()) {
            if (player.second.get()->GetSession() == session) {
                boost::json::object player_info;
                boost::json::array pos_array = {
                    player.second.get()->GetDog()->GetPosition().x_pos,
                    player.second.get()->GetDog()->GetPosition().y_pos
                };
                boost::json::array speed_array = {
                    player.second.get()->GetDog()->GetSpeed().h_speed,
                    player.second.get()->GetDog()->GetSpeed().v_speed
                };
                std::ostringstream oss;
                oss << player.second.get()->GetDog()->GetDirection();
                boost::json::array bag_array;
                boost::json::object b_id, b_type;
                for (const auto& bi : player.second.get()->GetDog()->GetItemFromBag()) {
                    b_id["id"] = bi.GetId();
                    b_type["type"] = bi.GetType();
                    bag_array.push_back({ b_id,b_type });
                }

                player_info["pos"] = pos_array;
                player_info["speed"] = speed_array;
                player_info["dir"] = oss.str();
                player_info["bag"] = bag_array;
                player_info["score"] = player.second.get()->GetDog()->GetScore();


                players_json[std::to_string(player.first)] = player_info;
            }
        }
        for (const auto& loot : session->GetTrophyList()) {
            boost::json::object trophy_info;
            boost::json::array pos_array = {
                    loot.second.GetPosition().x_pos,
                    loot.second.GetPosition().y_pos
            };
            trophy_info["type"] = loot.second.GetType();
            trophy_info["pos"] = pos_array;
            trophy_json[std::to_string(loot.first)] = trophy_info;
        }

        result["players"] = players_json;
        result["lostObjects"] = trophy_json;

        return boost::json::serialize(result);
    }

    const void Api::PrintMap(const model::Map* map, std::ostream& out) const {
        using namespace boost::json;
        object json_map;

        json_map[ID_STR.data()] = *map->GetId();
        json_map[NAME_STR.data()] = map->GetName();

        array roads_array;
        for (const auto& road : map->GetRoads()) {
            roads_array.push_back(PrintRoadAsJson(road));
        }
        json_map[ROADS_STR.data()] = roads_array;

        array buildings_array;
        for (const auto& build : map->GetBuildings()) {
            buildings_array.push_back(PrintBuildingAsJson(build));
        }
        json_map[BUILDINGS_STR.data()] = buildings_array;

        array offices_array;
        for (const auto& office : map->GetOffices()) {
            offices_array.push_back(PrintOfficeAsJson(office));
        }
        json_map[OFFICES_STR.data()] = offices_array;

        json_map[LOOT_TYPES_STR.data()] = apl_.GetTrophies(*map->GetId());

        out << serialize(json_map);
    }

    const json::object Api::PrintRoadAsJson(const model::Road& road) const {
        json::object road_object;
        road_object[X0_STR.data()] = road.GetStart().x;
        road_object[Y0_STR.data()] = road.GetStart().y;
        if (road.IsHorizontal()) {
            road_object["x1"] = road.GetEnd().x;
        }
        else {
            road_object["y1"] = road.GetEnd().y;
        }
        return road_object;
    }

    const json::object Api::PrintBuildingAsJson(const model::Building& build) const {
        json::object building_object;
        building_object[X_STR.data()] = build.GetBounds().position.x;
        building_object[Y_STR.data()] = build.GetBounds().position.y;
        building_object["w"] = build.GetBounds().size.width;
        building_object["h"] = build.GetBounds().size.height;
        return building_object;
    }

    const json::object Api::PrintOfficeAsJson(const model::Office& office) const {
        json::object office_object;
        office_object[ID_STR.data()] = *office.GetId();
        office_object[X_STR.data()] = office.GetPosition().x;
        office_object[Y_STR.data()] = office.GetPosition().y;
        office_object["offsetX"] = office.GetOffset().dx;
        office_object["offsetY"] = office.GetOffset().dy;
        return office_object;
    }

    const void Api::PrintMaps( std::ostream& out) const {
        using namespace boost::json;

        json::array maps_array;

        for (const auto& map : apl_.GetMaps()) {
            json::object map_object;
            map_object[ID_STR.data()] = *map.GetId();
            map_object[NAME_STR.data()] = map.GetName();
            maps_array.push_back(map_object);
        }

        out << serialize(maps_array);
    }

    const std::string Api::GetRecordTable(int start, int max) {
        json::array record;
        for (const auto& rec : apl_.GetRecords(start, max)) {
            boost::json::object rec_info;
            rec_info["name"] = rec.name;
            rec_info["score"] = rec.score;
            rec_info["playTime"] = rec.played_time;
            record.push_back(rec_info);
        }
        return serialize(record);
    }
}