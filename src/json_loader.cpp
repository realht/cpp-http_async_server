#include <boost/json/src.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include "json_loader.h"

namespace json = boost::json;
using namespace std::literals;


namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path, extra_data::TrophyList& tl) {

    std::ifstream json_file(json_path);
    if (!json_file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + json_path.string());
    }
    std::string json_content((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());

    auto maps_info = json::parse(json_content);
    double default_speed;
    int default_bag_capasity;
    
    //dog speed global
    if (maps_info.as_object().count("defaultDogSpeed") && maps_info.as_object().at("defaultDogSpeed").if_double()) {
        default_speed = maps_info.as_object().at("defaultDogSpeed").as_double();
    }else {
        default_speed = 1.0;
    }

    //bag capacity global
    if (maps_info.as_object().count("defaultBagCapacity") && maps_info.as_object().at("defaultBagCapacity").if_double()) {
        default_bag_capasity = maps_info.as_object().at("defaultBagCapacity").as_int64();
    }
    else {
        default_bag_capasity = 3;
    }

    //trophy update info
    int period = maps_info.as_object().at("lootGeneratorConfig").as_object().at("period").as_double();
    double probability = maps_info.as_object().at("lootGeneratorConfig").as_object().at("probability").as_double();

    //time to retirement dog
    double dog_retirement_time;
    if (maps_info.as_object().count("dogRetirementTime") && maps_info.as_object().at("dogRetirementTime").if_double()) {
        dog_retirement_time = maps_info.as_object().at("dogRetirementTime").as_double();
    }
    else {
        dog_retirement_time = 60;
    }

   
    model::Game game(period, probability);
    game.SetRetirementTime(dog_retirement_time * 1000);

    for (const auto& map_info : maps_info.as_object().at("maps").as_array()) {

        model::Map::Id map_id(map_info.as_object().at("id").as_string().c_str());
        std::string name = map_info.as_object().at("name").as_string().c_str();

        double local_speed = default_speed;
        int local_bag_capacity = default_bag_capasity;

        model::Map map(map_id, name);

        //dog speed local
        if (map_info.as_object().count("dogSpeed") && map_info.as_object().at("dogSpeed").if_double()) {
            local_speed = map_info.as_object().at("dogSpeed").as_double();
        }
        map.SetDogSpeedOnMap(local_speed);

        //bag capacity local
        if (map_info.as_object().count("bagCapacity") && map_info.as_object().at("bagCapacity").if_double()) {
            local_bag_capacity = map_info.as_object().at("bagCapacity").as_int64();
        }
        map.SetBagCapacity(local_bag_capacity);
        

        //add road, not be empty
        for (const auto& road_info : map_info.as_object().at("roads").as_array()) {
            map.AddRoad(GetRoad(road_info));
        }

        //add buildings, may be empty
        if (!map_info.as_object().at("buildings").as_array().empty()) {
            for (const auto& build : map_info.as_object().at("buildings").as_array()) {
                map.AddBuilding(GetBuilding(build));
            }
        }

        //add safe point, may be empty
        if (!map_info.as_object().at("offices").as_array().empty()) {
            for (const auto& office : map_info.as_object().at("offices").as_array()) {
                map.AddOffice(GetOffice(office));
            }
        }

        //add trophy list
        if (!map_info.as_object().at("lootTypes").as_array().empty()) {
            tl.AddTrophy(map_info.as_object().at("id").as_string().c_str(), map_info.as_object().at("lootTypes").as_array());
        }
        map.SetNumberTrophyTypes(map_info.as_object().at("lootTypes").as_array().size());
        
        game.AddMap(map);
    }

    return game;
}



model::Road GetRoad(const boost::json::value& road_info) {
    model::Point zero(road_info.as_object().at("x0").as_int64(), road_info.as_object().at("y0").as_int64());
    if (road_info.if_object()->count("x1")) {
        model::Road road(model::Road::HORIZONTAL, zero, road_info.as_object().at("x1").as_int64());
        return road;
    }
    else if (road_info.if_object()->count("y1")) {
        model::Road road(model::Road::VERTICAL, zero, road_info.as_object().at("y1").as_int64());
        return road;
    }
    else {
        return {};
    }
}

model::Building GetBuilding(const boost::json::value& build_info) {
    model::Point zero(build_info.as_object().at("x").as_int64(), build_info.as_object().at("y").as_int64());
    model::Size size(build_info.as_object().at("w").as_int64(), build_info.as_object().at("h").as_int64());
    model::Building ready_build({ zero,size });
    return ready_build;
}

model::Office GetOffice(const boost::json::value& office_info) {
    model::Office::Id id(office_info.as_object().at("id").as_string().c_str());
    model::Point zero(office_info.as_object().at("x").as_int64(), office_info.as_object().at("y").as_int64());
    model::Offset offset(office_info.as_object().at("offsetX").as_int64(), office_info.as_object().at("offsetY").as_int64());
    model::Office ready_office(id, zero, offset);
    return ready_office;
}

}  // json_loader
