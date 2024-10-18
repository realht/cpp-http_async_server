#pragma once
#include <boost/filesystem.hpp>
#include <cerrno>
#include <fstream>
#include "model_serialization.h"


namespace serialization {

    void WriteGameDate(const std::vector<app::Player>& pl, std::vector<std::pair<model::Trophy, std::string>> trophy_list, std::string path);

    const std::pair< std::vector<PlayerPrep>, std::vector<TrophyPrep>> OpenGameDate(std::string path);

}
