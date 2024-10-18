#pragma once
#include <boost/json.hpp>
#include <filesystem>
#include "extra_data.h"
#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path, extra_data::TrophyList& tl);

model::Road GetRoad(const boost::json::value& road_info);

model::Building GetBuilding(const boost::json::value& build_info);

model::Office GetOffice(const boost::json::value& office_info);
}  // json_loader
