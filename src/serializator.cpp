#include <boost/filesystem.hpp>
#include <iostream>
#include "serializator.h"

namespace serialization {

    void WriteGameDate(const std::vector<app::Player>& pl, std::vector<std::pair<model::Trophy, std::string>> trophy_list, std::string path) {
        using namespace std::literals;
        using namespace boost::filesystem;

        std::filesystem::path dir_temp = std::filesystem::path(path).parent_path();
        if (!dir_temp.empty() && !std::filesystem::exists(dir_temp)) {
            std::filesystem::create_directories(dir_temp);
        }

        std::string temp_path = path + ".bak";

        std::ofstream out(temp_path, std::ios::out);

        if (!out.is_open()) {
            boost::filesystem::path pa(path);
            boost::filesystem::path absolute_path = boost::filesystem::absolute(pa);

            int error = errno;
            std::cout << "Error opening file: " << strerror(error) << std::endl;
            std::cout << "Failed to write " << absolute_path << '\n';

            throw std::logic_error("Error saving file server state"s);
        }

        boost::archive::polymorphic_text_oarchive por{ out };

        std::vector<PlayerPrep> pp_list;
        std::vector<TrophyPrep> tp_list;

        for (const auto& player : pl) {
            PlayerPrep pr(player);
            pp_list.emplace_back(pr);
        }
        for (const auto& trophy : trophy_list) {
            TrophyPrep tp(trophy.second, trophy.first);
            tp_list.emplace_back(tp);
        }

        por << pp_list;
        por << tp_list;

        out.close();
         
        try {
            std::filesystem::rename(temp_path, path);
        }
        catch (std::exception) {
            throw std::logic_error("Error when writing from a temporary file to the main server state file"s);
        }
        permissions(path, add_perms | owner_exe | group_exe | group_write | others_write);
    }

    const std::pair<std::vector<PlayerPrep>, std::vector<TrophyPrep>> OpenGameDate(std::string path) {

        using namespace std::literals;

        std::ifstream in(path);
        if (in.bad()) {
            std::cerr << strerror(errno) << " Load: Bad Error File to open file - "s << std::endl;
            throw std::logic_error("Bad Error loading file server state"s);
        }
        if (in.fail()) {
            std::cerr << strerror(errno) << " Load: Fail Error File to open file - "s << std::endl;
            throw std::logic_error("Fail Error loading file server state"s);
        }
        if (!in.is_open()) {
            std::cerr << strerror(errno) << " Load: Fail Error File to open file - "s << std::endl;
            throw std::logic_error("Unable to open file state"s);
        }

        boost::archive::polymorphic_text_iarchive pir{ in };

        std::vector<PlayerPrep> vpp;
        std::vector<TrophyPrep> vtp;

        pir >> vpp;
        pir >> vtp;

        in.close();

        return { vpp, vtp };
    }



}