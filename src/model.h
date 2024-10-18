#pragma once
#include <filesystem>
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "loot_generator.h"
#include "tagged.h"


namespace model {

using Dimension = int;
using Coord = Dimension;
const double BORDER_WIDTH = 0.4;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

enum class Direction {
    North,
    South,
    West,
    East
};

std::ostream& operator<<(std::ostream& os, const Direction& dir);

struct Position {
    double x_pos;
    double y_pos;
    bool operator==(const Position& other) const {
        return x_pos == other.x_pos && y_pos == other.y_pos;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

struct Speed {
    double h_speed;
    double v_speed;

    bool operator==(const Speed& other) const {
        return v_speed == other.v_speed && h_speed == other.h_speed;
    }

    bool operator!=(const Speed& other) const {
        return !(*this == other);
    }
};

const Speed ZEROSPEED = { 0, 0 };

 struct Trophy {
    Trophy(size_t id, int type, Position pos)
        : id_(id),
        type_(type),
        pos_(pos) {}

    Trophy() :
        id_(0),
        type_(0),
        pos_({ 0,0 }) {}

    const size_t GetId() const;
    const int GetType() const;
    const Position GetPosition() const;


    size_t id_;
    int type_;
    Position pos_;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        HorizontalTag() = default;
    };

    struct VerticalTag {
        VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    Road() noexcept
        : start_{}, end_{} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    Position GetDefaultPosition() const;

    const std::pair<Position, Position> GetBorderRoad() const;

    bool IsPointOnRoad(const Position& position) const;
    bool IsPointOnBorder(const Position& position) const;

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name))  {}

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    const std::vector<const Road*> GetRoadAtPoint(const Position& position) const;

    void SetDogSpeedOnMap(double n);
    const double GetDogSpeedOnMap() const;

    void SetNumberTrophyTypes(int n);
    const int GetNumberTrophyTypes() const;

    void SetBagCapacity(int n);
    const int GetBagCapacity() const;

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    double speed_ = 0;
    int number_of_trophy_types_ = 0;
    int bag_capacity_ = 0;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    Dog(const std::string& name, const Position& pos, int capacity) :
        name_(name),
        pos_( pos ),
        bag_capacity_(capacity)
    {}

    Dog() = default;

    const std::string GetName() const;
    void SetPosition(const Position& position);
    const Position GetPosition() const;
    const int GetBagCapacity() const;
    void SetSpeed(Speed spd);
    const Speed GetSpeed() const;
    void SetDirection(Direction dir);
    const Direction GetDirection() const;
    void AddItemToBag(Trophy trophy);
    const std::vector<Trophy> GetItemFromBag() const;
    const bool HasMoreCapacity() const;
    void ReturnTrophyToOffice();
    void AddScore(int sc);
    const size_t GetScore() const;
    static std::pair<Position,bool> GetMaxMovePosition(const Road* road, const Dog* dog, size_t delta_time);
    void UpdatePlayedTime(double time);
    const double GetPlayedTime() const;
    void UpdateStayTime(double time);
    const double GetStayTime();
    void EraseStayTime();

    bool operator==(const Dog& other) const {
        return name_ == other.name_; 
    }

private:
    std::string name_;
    Position pos_;
    Speed speed_ = {0.0,0.0};
    Direction dir_ = Direction::North;
    std::vector<Trophy> bag_;
    int bag_capacity_;
    size_t score_ = 0;
    double played_time = 0;
    double stay_time = 0;
};

class GameSession {
public:
    GameSession(const Dog& dog, const Map& map)
        : map_(&map)
    {
        dog_base_.push_back(dog);
        dogs_[count_add_dogs] = GetLastDog();
        ptr_to_id[GetLastDog()] = count_add_dogs;

        ++count_add_dogs;
    }

    const Map* GetMap() const;
    void AddDogToSession(const Dog& dog);
    Dog* GetLastDog() ;
    std::map<int, Dog*>& GetDogs();
    const size_t GetCountTrophyOnMap() const;
    void AddTrophyOnMap(const Trophy& tr);
    const size_t GetCountDogOnMap() const;
    const std::unordered_map<size_t, Trophy>& GetTrophyList() const;
    const int GetCountTrophyAdded() const;
    const Trophy GetTrophy(int id) const;
    void RemoveTrophy(int id);
    void DeleteDog(const Dog* dog);

private:
    const Map* map_;
    std::list<Dog> dog_base_;
    std::map<int, Dog*> dogs_;
    std::unordered_map<const Dog*, int> ptr_to_id;
    int trophy_added_ = 0;
    std::unordered_map<size_t, Trophy> trophy_;
    int count_add_dogs = 0;
};

class Game {
public:
    using Maps = std::vector<Map>;

    Game(int per, double prob):    
        lg_(std::chrono::milliseconds(per), prob){}

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const std::vector<Map>& GetMapList() const noexcept {
        return maps_;
    }

    const Map* GetMap(const std::string& map_id);

    GameSession* AddSession(const Dog& dog, const Map& map);
    GameSession* GetSession(const std::string& map_id);

    void SetRetirementTime(double time);
    const double GetRetirementTime() const;

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<GameSession>> GetSessions();
    int GetCountNewTrophy(const GameSession& session, size_t delta_time);

private:
    loot_gen::LootGenerator lg_;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::vector<std::shared_ptr<GameSession>> sessions_;
    double dog_retirement_time;
};

}  // model
