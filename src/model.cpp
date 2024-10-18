#include <stdexcept>
#include "model.h"

namespace model {
using namespace std::literals;

std::ostream& operator<<(std::ostream& os, const Direction& dir) {
    switch (dir) {
    case Direction::North:
        os << "U";
        break;
    case Direction::South:
        os << "D";
        break;
    case Direction::West:
        os << "L";
        break;
    case Direction::East:
        os << "R";
        break;
    }
    return os;
};


//trophy
const size_t Trophy::GetId() const {
    return id_;
}

const int Trophy::GetType() const {
    return type_;
}

const Position Trophy::GetPosition() const {
    return pos_;
}
//end trophy

// road
Position Road::GetDefaultPosition() const {
    return { static_cast<double>(start_.x),static_cast<double>(start_.y) };
}

const std::pair<Position, Position> Road::GetBorderRoad() const {
    if (IsHorizontal()) {
        if (start_.x < end_.x) {
            return { {start_.x - BORDER_WIDTH, start_.y - BORDER_WIDTH}, {end_.x + BORDER_WIDTH, end_.y + BORDER_WIDTH} };
        }
        else {
            return { {end_.x - BORDER_WIDTH, end_.y - BORDER_WIDTH}, {start_.x + BORDER_WIDTH, start_.y + BORDER_WIDTH} };
        }
    }
    else {
        if (start_.y < end_.y) {
            return { {start_.x - BORDER_WIDTH, start_.y - BORDER_WIDTH}, {end_.x + BORDER_WIDTH, end_.y + BORDER_WIDTH} };
        }
        else {
            return { {end_.x - BORDER_WIDTH, end_.y - BORDER_WIDTH}, {start_.x + BORDER_WIDTH, start_.y + BORDER_WIDTH} };
        }
    }
}

bool Road::IsPointOnRoad(const Position& position) const {
    std::pair<Position, Position> boarder = GetBorderRoad();
    if (position.x_pos >= boarder.first.x_pos && position.x_pos <= boarder.second.x_pos &&
        position.y_pos >= boarder.first.y_pos && position.y_pos <= boarder.second.y_pos) {
        return true;
    }
    return false;
}

bool Road::IsPointOnBorder(const Position& position) const {
    std::pair<Position, Position> boarder = GetBorderRoad();
    if (position.x_pos == boarder.first.x_pos || position.x_pos == boarder.second.x_pos ||
        position.y_pos == boarder.first.y_pos || position.y_pos == boarder.second.y_pos) {
        return true;
    }
    return false;
}
// end road

//map
void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (std::exception) {
        offices_.pop_back();
        throw;
    }
}

const std::vector<const Road*> Map::GetRoadAtPoint(const Position& position) const {
    std::vector<const Road*> available_road;
    for (const auto& road : roads_) {
        if (road.IsPointOnRoad(position)) {
            available_road.push_back(&road);
        }
    }
    return available_road;
}

void Map::SetNumberTrophyTypes(int n) {
    number_of_trophy_types_ = n;
}

const int Map::GetNumberTrophyTypes() const {
    return number_of_trophy_types_;
}

void Map::SetDogSpeedOnMap(double n) {
    speed_ = n;
}

const double Map::GetDogSpeedOnMap() const {
    return speed_;
}

void Map::SetBagCapacity(int n) {
    bag_capacity_ = n;
}

const int Map::GetBagCapacity() const {
    return bag_capacity_;
}
//end map

// dog
const std::string Dog::GetName() const {
    return name_;
}

void Dog::SetPosition(const Position& position) {
    pos_ = position;
}

const Position Dog::GetPosition() const {
    return pos_;
}

const int Dog::GetBagCapacity() const {
    return bag_capacity_;
}

void Dog::SetSpeed(Speed spd) {
    speed_ = spd;
}

void Dog::AddScore(int sc) {
    score_ += sc;
}

const Speed Dog::GetSpeed() const {
    return speed_;
}

void Dog::SetDirection(Direction dir) {
    dir_ = dir;
}

const Direction Dog::GetDirection() const {
    return dir_;
}

void Dog::AddItemToBag(Trophy trophy) {
    bag_.push_back(trophy);
}

const std::vector<Trophy> Dog::GetItemFromBag() const {
    return bag_;
}

const bool Dog::HasMoreCapacity() const {
    return bag_capacity_ > bag_.size();
}

void Dog::ReturnTrophyToOffice() {
    for (const auto& trophy : bag_) {
        AddScore(trophy.GetType());
    }
    bag_.clear();
}

const size_t Dog::GetScore() const {
    return score_;
}

std::pair<Position, bool> Dog::GetMaxMovePosition(const Road* road, const Dog* dog, size_t delta_time) {
    Speed dog_speed = dog->GetSpeed();
    Position dog_position = dog->GetPosition();
    std::pair<Position, Position> road_border = road->GetBorderRoad();
    double temp_;
    if (dog_speed.v_speed == 0) {
        temp_ = dog_position.x_pos + (dog_speed.h_speed * delta_time / 1000);
        if (dog_speed.h_speed > 0) {
            if (temp_ >= road_border.second.x_pos) {
                return { {road_border.second.x_pos, dog_position.y_pos}, true };
            }
            return { {temp_, dog_position.y_pos}, false };
        }
        else {
            if (temp_ <= road_border.first.x_pos) {
                return { {road_border.first.x_pos, dog_position.y_pos}, true };
            }
            return { {temp_, dog_position.y_pos}, false };
        }
    }
    else {
        temp_ = dog_position.y_pos + (dog_speed.v_speed * delta_time / 1000);
        if (dog_speed.v_speed > 0) {
            if (temp_ >= road_border.second.y_pos) {
                return { {dog_position.x_pos, road_border.second.y_pos},true };
            }
            return { {dog_position.x_pos, temp_},false };
        }
        else {
            if (temp_ <= road_border.first.y_pos) {
                return { {dog_position.x_pos, road_border.first.y_pos}, true };
            }
            return { {dog_position.x_pos, temp_},false };
        }
    }
}

void Dog::UpdatePlayedTime(double time) {
    played_time += time;
}
const double Dog::GetPlayedTime() const {
    return played_time;
}
void Dog::UpdateStayTime(double time) {
    stay_time += time;
}
const double Dog::GetStayTime() {
    return stay_time;
}
void Dog::EraseStayTime() {
    stay_time = 0;
}
// end dog

//session
const Map* GameSession::GetMap() const {
    return map_;
}

void GameSession::AddDogToSession(const Dog& dog) {
    dog_base_.push_back(dog);
    dogs_[count_add_dogs] = GetLastDog();
    ptr_to_id[GetLastDog()] = count_add_dogs;
    ++count_add_dogs;
}

Dog* GameSession::GetLastDog() {
    if(!dog_base_.empty()) {
        return &dog_base_.back();
    }
    return nullptr;
}
std::map<int, Dog*>& GameSession::GetDogs() {
    return dogs_;
}

const size_t GameSession::GetCountTrophyOnMap() const {
    return trophy_.size();
}

void GameSession::AddTrophyOnMap(const Trophy& tr) {
    trophy_[tr.GetId()] = tr;
    ++trophy_added_;
}

const size_t GameSession::GetCountDogOnMap() const {
    return dogs_.size();
}

const std::unordered_map<size_t, Trophy>& GameSession::GetTrophyList() const {
    return trophy_;
}

const int GameSession::GetCountTrophyAdded() const {
    return trophy_added_;
}

const Trophy GameSession::GetTrophy(int id) const {
    return trophy_.at(id);
}

void GameSession::RemoveTrophy(int id) {
    trophy_.erase(id);
}

void GameSession::DeleteDog(const Dog* dog) {
    int id = -1;
    id = ptr_to_id.at(dog);
    if (id != -1) {
        dogs_.erase(id);
        ptr_to_id.erase(dog);
    }
}
//end session

//game
void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (std::exception) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

const Map* Game::GetMap(const std::string& map_id) {
    for (const auto& map : maps_) {
        if (*map.GetId() == map_id) {
            return &map;
        }
    }
    return nullptr;
}

GameSession* Game::AddSession(const Dog& dog, const Map& map) {
    bool session_added = false;
    for (auto& sess : sessions_) {
        if (sess.get()->GetMap()->GetName() == map.GetName()) {
            sess.get()->AddDogToSession(dog);
            session_added = true;
            return sess.get();
        }
    }
    if (!session_added) {
        std::shared_ptr<GameSession> session = std::make_shared<GameSession>(dog, map);
        sessions_.push_back(session);
        return sessions_.back().get();
    }
    return nullptr;
}

GameSession* Game::GetSession(const std::string& map_id) {
    for(auto& sess : sessions_) {
        if (*sess.get()->GetMap()->GetId() == map_id) {
            return sess.get();
       }
    }

    return nullptr;
}

std::vector<std::shared_ptr<GameSession>> Game::GetSessions() {
    return sessions_;
}

void Game::SetRetirementTime(double time) {
    dog_retirement_time = time;
}
const double Game::GetRetirementTime() const {
    return dog_retirement_time;
}

int Game::GetCountNewTrophy(const GameSession& session, size_t delta_time) {
    return lg_.Generate(std::chrono::milliseconds(delta_time), session.GetCountTrophyOnMap(), session.GetCountDogOnMap());
}
//end game

}  // namespace model
