#include "app_model.h"

namespace app {


    Player& Players::AddPlayer(const model::Dog* dog, const model::GameSession* session, Token token) {
        Token ptoken;

        if (*token == "") {
            PlayerTokens pt;
            ptoken = pt.GetToken();
        }
        else {
            ptoken = token;
        }

        Player pl(session, dog, ptoken, count_players);

        player_list_.push_back(pl);

        std::shared_ptr<Player> p_ptr = std::make_shared<Player>(player_list_.back());
        players_table_[count_players] = p_ptr;
        ++count_players;
        return player_list_.back();
    }

    Player* Players::FindByToken(Token token) {
        for (const auto& player : players_table_) {
            if (player.second->GetToken() == token) {
                return player.second.get();
            }
        }
        return nullptr;
    }

    Player* Players::FindByDogPtr(const model::Dog* dog) {
        for (const auto& player : players_table_) {
            if (player.second->GetDog() == dog) {
                return player.second.get();
            }
        }
        return nullptr;
    }

    const std::map<int, std::shared_ptr<Player>>& Players::GetPlayersList() const {
        return players_table_;
    }

    int Players::GetPlayerID() const {
        return count_players - 1;
    }

    bool Players::TokenAuthorized(const std::string& str) const {
        Token token(str);

        for (const auto& pl : players_table_) {
            if (*pl.second->GetToken() == str) {
                return true;
            }
        }
        return false;
    }

    std::string Player::GetName() const {
        return dog_->GetName();
    }
    Token Player::GetToken() const {
        return token_;
    }
    const int Player::GetId() const {
        return id_;
    }
    
    const model::GameSession* Player::GetSession() const {
        return session_;
    }

    model::Dog* Player::GetDog() const {
        return const_cast<model::Dog*>(dog_);
    }

    void Players::DeletePlayer(const std::string& token) {
        int id_to_erase = -1;
        for (auto& player : players_table_) {
            if (*player.second->GetToken() == token) {
                id_to_erase = player.first;
                break;
            }
        }
        if (id_to_erase != -1) {
            players_table_.erase(id_to_erase);

        }
    }
}