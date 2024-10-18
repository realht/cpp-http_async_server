#pragma once
#include <algorithm>
#include <iostream>
#include <map>
#include "app_addition_struct.h"
#include "database.h"
#include "serializator.h"

namespace app {
    namespace json = boost::json;

    class Application {
    public:
        Application(model::Game& game, app::Players& players, const extra_data::TrophyList& tl, const AppConfig& conf) :
            game_{ &game },
            players_(&players),
            tl_(tl),
            self_update_(conf.self_update),
            random_position_(conf.random_position),
            saved_file_(conf.saved_file),
            time_between_save_(conf.time_between_save),
            db_({conf.db_url, db::MAX_DB_CONNECTION})
        {}

        PlayerInfo JoinGame(const std::string& map_id, const std::string& name) ;
        bool HasToken(const std::string& token) const;
        const model::Map* GetMap(const std::string& map_id) const;
        const model::Map* GetMap(const model::Map::Id& id) const;
        const std::map<int, std::shared_ptr<Player>>& GetPlayersList() const;
        const std::vector<model::Map>& GetMaps() const;
        const model::GameSession* GetSession(const std::string& token) const;
        Player* GetPlayerByToken(const std::string& token) const;
        static double GetRandonValueDouble(double a, double b);
        static int GetRandonValueInt(int a, int b);
        void UpdateWorldState(size_t delta_time);
        bool isSelfMode() const;
        const json::array GetTrophies(const std::string& map_id) const;
        void UploadGameState();
        void SaveGameState();
        const std::vector<db::GameRecords> GetRecords(int offset, int max_elem);

    public:
        enum JoinPlayerErrorCode {
            wrong_name,
            wrong_map
        };

    private:
        struct TokenHash {
            std::size_t operator()(const Token& token) const {
                return std::hash<std::string>()(*token);
            }
        };
        
        void UpdateSessionState(model::GameSession& session, size_t delta_time);
        void CalcCollisionDogsAndTrophy(model::GameSession& session, size_t delta_time);

        std::vector<collision_detector::Gatherer> GetGathererList(model::GameSession& session, size_t delta_time);
        std::vector<collision_detector::Item> GetTrophyList(model::GameSession& session);
        std::vector<collision_detector::Office> GetOfficeList(model::GameSession& session);
        model::Position UpdatePlayerState(const Token& token, size_t delta_time);
        
        void UpdateSessionForCollectAndReturnTrophy(model::GameSession& session, const std::vector<collision_detector::GatheringEvent>& ge);

        std::pair<model::Position, bool> GetDogNewPosition(const std::vector<const model::Road*>& roads, const model::Dog* dog, size_t delta_time);
        void MoveDogToNewPosiotion(model::Dog* dog, const model::Position& position, bool is_collised);
        void UpdateTrophyState(model::GameSession& session, size_t delta_time);
        void SendDogToRetirement();
        void SaveDogRecord(const Token& token);
        void DeleteDog(const Token& token);
        

    private:
        model::Game* game_;
        Players* players_;
        const extra_data::TrophyList& tl_;
        bool self_update_;
        bool random_position_;
        std::unordered_set<Token, TokenHash> player_tokens_;
        std::string saved_file_;
        int time_between_save_;
        db::Database db_;
        int current_time_ = 0;
        std::vector<Token> to_retirement;
    };

}