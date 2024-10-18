#include "extra_data.h"

namespace extra_data {

    void TrophyList::AddTrophy(const std::string& map_id, const json::array& trophies) {
        trophy_list_[map_id] = trophies;
    }

    const json::array TrophyList::GetTrophy(const std::string& map_id) const {
        return trophy_list_.at(map_id);
    }
}