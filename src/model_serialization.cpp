#include "model_serialization.h"

namespace serialization {

    const int PlayerPrep::GetID() const {
        return id_;
    }
    const Token PlayerPrep::GetToken() const {
        return token_;
    }
    const std::string PlayerPrep::GetMapId() const {
        return map_id_;
    }
    const DogRepr PlayerPrep::GetDogRepr() const {
        return dogr_;
    }

    const std::string TrophyPrep::GetMapID() const {
        return map_id_;
    }
    const size_t TrophyPrep::GetId() const {
        return id_;
    }
    const int TrophyPrep::GetType() const {
        return type_;
    }
    const model::Position TrophyPrep::GetPos() const {
        return pos_;
    }
}
