#pragma once
#include "collision_detector.h"
#include "extra_data.h"
#include "model.h"


namespace app {

    namespace json = boost::json;

    struct PlayerInfo {
        Token token;
        int id;

        PlayerInfo() :
            token{ "" }, id{}
        {}

        PlayerInfo(Token player_token, int player_id) {
            Token t(*player_token);
            token = t;
            id = player_id;
        }

        void Write(const PlayerInfo&& pi) {
            Token t(*pi.token);
            token = t;
            id = pi.id;
        }

    };

    class Player {
    public:
        Player(const model::GameSession* session, const model::Dog* dog, Token token, int id)
            : session_{ session },
            dog_(dog),
            token_(token),
            id_(id){
        }
        Player() : session_(nullptr), dog_(nullptr), token_(""), id_(0) {};

        Player(const Player& other)
            : session_(other.session_),
            dog_(other.dog_),
            token_(other.token_),
            id_(other.id_) {};

        bool operator==(const Player& other) const {
            return this->token_ == other.token_;
        }
    

        std::string GetName() const;
        const model::GameSession* GetSession() const;
        Token GetToken() const;
        const int GetId() const;
        model::Dog* GetDog() const;

    private:
        const model::GameSession* session_;
        const model::Dog* dog_;
        const Token token_;
        const int id_;
    };

    class Players {
    public:
        Player& AddPlayer(const model::Dog* dog, const model::GameSession* session, Token token = Token(""));
        Player* FindByToken(Token token);
        Player* FindByDogPtr(const model::Dog* dog);
        int GetPlayerID() const;
        const std::map<int, std::shared_ptr<Player>>& GetPlayersList() const;
        bool TokenAuthorized(const std::string& str) const;
        void DeletePlayer(const std::string& token);

    private:
        std::list<Player> player_list_;
        std::map<int, std::shared_ptr<Player>> players_table_;

        unsigned int count_players = 0;
    };

}