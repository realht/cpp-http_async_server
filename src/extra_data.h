#pragma once
#include <boost/json.hpp>
#include <unordered_map>

namespace extra_data {
    namespace json = boost::json;

    class TrophyList {
    public:
        void AddTrophy(const std::string& map_id, const json::array& trophies);
        const json::array GetTrophy(const std::string& map_id) const;

    private:
        std::unordered_map<std::string, json::array> trophy_list_;
    };
}