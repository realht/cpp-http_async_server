#include "app.h"

namespace app {

    double Application::GetRandonValueDouble(double a, double b) {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution<double> dist(a, b);
        return dist(gen);
    }

    int Application::GetRandonValueInt(int a, int b) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(a, b);
        return dis(gen);
    }

    PlayerInfo Application::JoinGame(const std::string& map_id, const std::string& name) {
        
        if (name.empty()) {
            throw JoinPlayerErrorCode::wrong_name;
        }
        if (game_->GetMap(map_id) == nullptr) {
            throw JoinPlayerErrorCode::wrong_map;
        }
        
        const model::Map* map = game_->GetMap(map_id);
        model::Position dog_start_position;
        if (random_position_) {
            dog_start_position = {
                    GetRandonValueDouble(map->GetRoads().at(0).GetStart().x, map->GetRoads().at(0).GetEnd().x),
                    GetRandonValueDouble(map->GetRoads().at(0).GetStart().y, map->GetRoads().at(0).GetEnd().y) };
        }
        else {
            dog_start_position = map->GetRoads().at(0).GetDefaultPosition();
        }

        int bag_capacity = map->GetBagCapacity();

        model::Dog dog(name, dog_start_position, bag_capacity);

        model::GameSession* session = game_->AddSession(dog, *map);

        const model::Dog* dog_ptr = session->GetLastDog();

        app::Player player = players_->AddPlayer(dog_ptr, session);
        player_tokens_.insert(player.GetToken());

        return { player.GetToken(), player.GetId() };
    }

    bool Application::HasToken(const std::string& token) const {
        Token t(token);
        return player_tokens_.count(t);
    }

    const model::Map* Application::GetMap(const std::string& map_id) const {
        return game_->GetMap(map_id);
    }
    const model::Map* Application::GetMap(const model::Map::Id& id) const {
        return game_->FindMap(id);
    }

    const std::map<int, std::shared_ptr<Player>>& Application::GetPlayersList() const {
        return players_->GetPlayersList();
    }

    const std::vector<model::Map>& Application::GetMaps() const {
        return game_->GetMapList();
    }

    const model::GameSession* Application::GetSession(const std::string& token) const {
        Token t(token);
        return players_->FindByToken(t)->GetSession();
    }

    Player* Application::GetPlayerByToken(const std::string& token) const {
        Token t(token);
        return players_->FindByToken(t);
    }

    void Application::UpdateWorldState(size_t delta_time) {
        
        for (auto& session : game_->GetSessions()) {
            UpdateSessionState(*session, delta_time);
        }

        SendDogToRetirement();

        if (!saved_file_.empty()) {
            current_time_ += delta_time;
            if (current_time_ >= time_between_save_) {

                try {
                    SaveGameState();
                    }
                catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }

                current_time_ -= time_between_save_;
            }
        }
    }

    bool Application::isSelfMode() const {
        return self_update_;
    }

    const json::array Application::GetTrophies(const std::string& map_id) const {
        return tl_.GetTrophy(map_id);
    }

    void Application::UpdateSessionState(model::GameSession& session, size_t delta_time) {
        CalcCollisionDogsAndTrophy(session, delta_time);
        UpdateTrophyState(session, delta_time);
    }

    void Application::CalcCollisionDogsAndTrophy(model::GameSession& session, size_t delta_time) {

        std::vector<collision_detector::Gatherer> vec_gath = GetGathererList(session, delta_time);
        std::vector<collision_detector::Item> vec_items = GetTrophyList(session);
        std::vector<collision_detector::Office> vec_office = GetOfficeList(session);

        collision_detector::TrophyProvider tp{ vec_items, vec_gath, vec_office };
        auto events = collision_detector::FindGatherEvents(tp);

        UpdateSessionForCollectAndReturnTrophy(session, events);
    }

    std::vector<collision_detector::Gatherer> Application::GetGathererList(model::GameSession& session, size_t delta_time) {
        std::vector<collision_detector::Gatherer> vec_gath;
        vec_gath.reserve(session.GetDogs().size());
        for (const auto dog : session.GetDogs()) {
            model::Position old_position = dog.second->GetPosition();
            auto player_token = players_->FindByDogPtr(dog.second)->GetToken();
            model::Position new_position = UpdatePlayerState(player_token, delta_time);
            vec_gath.push_back({ player_token, { old_position.x_pos, old_position.y_pos },
                               { new_position.x_pos, new_position.y_pos }, collision_detector::DOG_COLLIDER_SIZE});
        }
        return vec_gath;
    }

    std::vector<collision_detector::Item> Application::GetTrophyList( model::GameSession& session)  {
        std::vector <collision_detector::Item> vec_items;
        vec_items.reserve(session.GetCountTrophyOnMap());
        for (const auto& trophy : session.GetTrophyList()) {
            collision_detector::Item item = { trophy.first, {trophy.second.GetPosition().x_pos, 
                                        trophy.second.GetPosition().y_pos}, collision_detector::ITEM_COLLIDER_SIZE };
            vec_items.push_back(item);
        }
        return vec_items;
    }

    std::vector<collision_detector::Office> Application::GetOfficeList( model::GameSession& session)  {
        std::vector<collision_detector::Office> vec_office;
        vec_office.reserve(session.GetMap()->GetOffices().size());
        for (const auto& office : session.GetMap()->GetOffices()) {
            collision_detector::Office ofc = { *office.GetId(), {static_cast<double>(office.GetPosition().x),
                                        static_cast<double>(office.GetPosition().y)}, collision_detector::OFFICE_COLLIDER_SIZE };
            vec_office.push_back(ofc);
        }
        return vec_office;
    }

    model::Position Application::UpdatePlayerState(const Token& token, size_t delta_time) {
        model::Dog* dog = players_->FindByToken(token)->GetDog();
        dog->UpdatePlayedTime(delta_time);

        if (dog->GetSpeed() != model::ZEROSPEED) {
            std::vector<const model::Road*> roads = players_->FindByToken(token)->GetSession()->GetMap()->GetRoadAtPoint(dog->GetPosition());
            std::pair<model::Position, bool> path_info = GetDogNewPosition(roads, dog, delta_time);
            MoveDogToNewPosiotion(dog, path_info.first, path_info.second);
            return path_info.first;
        }
        else {
            dog->UpdateStayTime(delta_time);
            if (dog->GetStayTime() >= game_->GetRetirementTime()) {
                to_retirement.push_back(token);
            }
            return dog->GetPosition();
        }
    }

    std::pair<model::Position, bool> Application::GetDogNewPosition(const std::vector<const model::Road*>& roads, const model::Dog* dog, size_t delta_time) {
        model::Position dog_pos = dog->GetPosition();
        model::Position new_pos;
        std::map<double, model::Position> route;
        std::vector<bool> collisions;
        for (const auto& road : roads) {
            std::pair<model::Position, bool> dog_route_info = model::Dog::GetMaxMovePosition(road, dog, delta_time);
            new_pos = dog_route_info.first;
            collisions.push_back(dog_route_info.second);
            route[std::abs(dog_pos.x_pos - new_pos.x_pos) + std::abs(dog_pos.y_pos - new_pos.y_pos)] = new_pos;
        }

        model::Position max_position = route.rbegin()->second;

        bool is_collised = false;

        if (std::any_of(collisions.begin(), collisions.end(), [](bool elem) { return elem != true; })) {
            is_collised = false;
        }

        return { max_position, is_collised };
    }

    void Application::MoveDogToNewPosiotion(model::Dog* dog, const model::Position& position, bool is_collised) {
        dog->SetPosition(position);
        if (is_collised) {
            dog->SetSpeed(model::ZEROSPEED);
        }
    }

    void Application::UpdateSessionForCollectAndReturnTrophy(model::GameSession& session,
        const std::vector<collision_detector::GatheringEvent>& ge) {
        std::unordered_set<size_t> collected_trophy;
        for (const auto& event : ge) {
            if (event.is_office == false && !collected_trophy.count(event.item_id)) {
                model::Dog* dog = GetPlayerByToken(*event.gatherer_token)->GetDog();
                if (dog->HasMoreCapacity()) {
                    dog->AddItemToBag(session.GetTrophy(event.item_id));
                    collected_trophy.emplace(event.item_id);
                }
            }
            if (event.is_office == true) {
                model::Dog* dog = GetPlayerByToken(*event.gatherer_token)->GetDog();
                dog->ReturnTrophyToOffice();
            }
        }

        //remove collectrd trophy
        for (const auto& id : collected_trophy) {
            session.RemoveTrophy(id);
        }
    }

    void Application::UpdateTrophyState(model::GameSession& session, size_t delta_time) {

        if (session.GetCountDogOnMap() > session.GetCountTrophyOnMap()) {
            int need_create_trophy = game_->GetCountNewTrophy(session, delta_time);

            if (need_create_trophy > 0) {
                for (int i = 0; i < need_create_trophy; ++i) {
                    int number_random_road = GetRandonValueInt(0, session.GetMap()->GetRoads().size() - 1);
                    
                    auto random_road_boarder = session.GetMap()->GetRoads().at(number_random_road).GetBorderRoad();
                    
                    double rand_x = GetRandonValueDouble(random_road_boarder.first.x_pos, random_road_boarder.second.x_pos);
                    double rand_y = GetRandonValueDouble(random_road_boarder.first.y_pos, random_road_boarder.second.y_pos);

                    model::Position trophy_position = { rand_x, rand_y };

                    int trophy_type = GetRandonValueInt(0, session.GetMap()->GetNumberTrophyTypes() - 1);
                    model::Trophy trophy(session.GetCountTrophyAdded(), trophy_type, trophy_position);
                    session.AddTrophyOnMap(trophy);
                }
            }
        }
    }

    void Application::SaveGameState() {
        std::vector<Player> player_list;
        std::vector<std::pair<model::Trophy, std::string>> trophy_list;

        for (const auto pl : players_->GetPlayersList()) {
            player_list.emplace_back(*pl.second.get());
        }

        for (auto& session : game_->GetSessions()) {

            for (const auto& trophy : session->GetTrophyList()) {
                trophy_list.push_back({ trophy.second, *session->GetMap()->GetId() });
            }
        }

        try{
            serialization::WriteGameDate(player_list, trophy_list, saved_file_);
        }
        catch (const std::exception&) { 
            throw;
        }

    }

    void Application::UploadGameState() {

        auto game_date = serialization::OpenGameDate(saved_file_);

        if (game_date.first.empty()) {
            return;
        }

        std::set<std::string> map_with_players;

        for (const auto& player : game_date.first) {


            model::Dog dog(player.GetDogRepr().Restore());

            map_with_players.insert(player.GetMapId());

            const model::Map* map = game_->GetMap(player.GetMapId());
            model::GameSession* session = game_->AddSession(dog, *map);

            const model::Dog* dog_ptr = session->GetLastDog();

            app::Player real_player = players_->AddPlayer(dog_ptr, session, player.GetToken());
            player_tokens_.insert(player.GetToken());

        }

        for (const auto& trophy : game_date.second) {
            if (map_with_players.count(trophy.GetMapID())) {

                model::Trophy real_trophy(trophy.GetId(), trophy.GetType(), trophy.GetPos());
                game_->GetSession(trophy.GetMapID())->AddTrophyOnMap(real_trophy);
            }
        }

    }

    void Application::SendDogToRetirement() {
        if (!to_retirement.empty()) {
            for (const Token& token : to_retirement) {
                SaveDogRecord(token);
                DeleteDog(token);
            }
        }
        to_retirement.clear();
    }

    void Application::SaveDogRecord(const Token& token) {
        model::Dog* dog = players_->FindByToken(token)->GetDog();
        db_.WriteToBD({ dog->GetName(), dog->GetScore(), dog->GetPlayedTime() });
    }

    void Application::DeleteDog(const Token& token) {
        player_tokens_.erase(token);

        Player* pl = GetPlayerByToken(*token);

        for (auto& session : game_->GetSessions()) {
            session->DeleteDog(pl->GetDog());
        }

        players_->DeletePlayer(*token);

    }
    
    const std::vector<db::GameRecords> Application::GetRecords(int offset, int max_elem) {
        return db_.GetRecords(offset, max_elem);
    }
}