#pragma once
#include <boost/serialization/vector.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "app_model.h"
#include "geom.h"
#include "model.h"

namespace geom {

template <typename Archive>
void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}

template <typename Archive>
void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}

}  // geom

namespace model {


template <typename Archive>
void serialize(Archive& ar, Trophy& obj, [[maybe_unused]] const unsigned version) {
    ar& (obj.id_);
    ar& (obj.type_);
}

template <typename Archive>
void serialize(Archive& ar, Speed& obj, [[maybe_unused]] const unsigned version) {
    ar& (obj.h_speed);
    ar& (obj.v_speed);
}

template <typename Archive>
void serialize(Archive& ar, Position& obj, [[maybe_unused]] const unsigned version) {
    ar& (obj.x_pos);
    ar& (obj.y_pos);
}

}  // model

namespace serialization {

class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : name_(dog.GetName())
        , pos_(dog.GetPosition())
        , speed_(dog.GetSpeed())
        , dir_(dog.GetDirection())
        , bag_(dog.GetItemFromBag())
        , bag_capacity_(dog.GetBagCapacity())
        , score_(dog.GetScore()) {
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{name_, pos_, bag_capacity_};
        dog.SetSpeed(speed_);
        dog.SetDirection(dir_);
        dog.AddScore(score_);
        for (const auto& item : bag_) {
            if (!dog.HasMoreCapacity()) {
                throw std::runtime_error("Failed to put bag content");
            }
            dog.AddItemToBag(item);
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& name_;
        ar& pos_;
        ar& speed_;
        ar& dir_;
        ar& bag_;
        ar& bag_capacity_;
        ar& score_;
    }

private:
    std::string name_;
    model::Position pos_;
    model::Speed speed_ = model::ZEROSPEED;
    model::Direction dir_ = model::Direction::North;
    std::vector<model::Trophy> bag_;
    int bag_capacity_;
    size_t score_ = 0;

};



class PlayerPrep {
public:
    PlayerPrep() = default;

    explicit PlayerPrep(const app::Player& player)
        : id_(player.GetId())
        , token_(player.GetToken())
        , map_id_(*player.GetSession()->GetMap()->GetId())
        , dogr_(*player.GetDog()) {}

    void serialize(
        boost::archive::polymorphic_iarchive& ar,
        [[maybe_unused]] const unsigned int file_version) {
        ar& id_;
        ar& (*token_);
        ar& map_id_;
        ar& dogr_;
    }

    void serialize(
        boost::archive::polymorphic_oarchive& ar,
        [[maybe_unused]] const unsigned int file_version) {
        ar& id_;
        ar& (*token_);
        ar& map_id_;
        ar& dogr_;
    }

    const int GetID() const;
    const Token GetToken() const;
    const std::string GetMapId() const;
    const DogRepr GetDogRepr() const;

private:
    int id_;
    Token token_;
    std::string map_id_;
    DogRepr dogr_;
    
};

class TrophyPrep {
public:
    TrophyPrep() = default;

    explicit TrophyPrep(std::string map_id, const model::Trophy& trophy)
        : map_id_(map_id)
        , id_(trophy.GetId())
        , type_(trophy.GetType())
        , pos_(trophy.GetPosition()) {}

    void serialize(
        boost::archive::polymorphic_iarchive& ar,
        [[maybe_unused]] const unsigned int file_version) {
        ar& map_id_;
        ar& id_;
        ar& type_;
        ar& pos_;
    }

    void serialize(
        boost::archive::polymorphic_oarchive& ar,
        [[maybe_unused]] const unsigned int file_version) {
        ar& map_id_;
        ar& id_;
        ar& type_;
        ar& pos_;
    }

    const std::string GetMapID() const;
    const size_t GetId() const;
    const int GetType() const;
    const model::Position GetPos() const;

private:
    std::string map_id_;
    size_t id_;
    int type_;
    model::Position pos_;

};

}  // serialization
