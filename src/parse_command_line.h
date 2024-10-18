#pragma once
#include <boost/program_options.hpp>
#include <iostream>
#include <optional>

namespace command_line {

    using namespace std::literals;

    struct Args {
        int update_period = 0;
        std::string config_path;
        std::string web_folder;
        bool random_position = false;
        std::string save_path = "";
        int save_time_period = 0;
    };

    [[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
        namespace po = boost::program_options;

        po::options_description desc{ "All server configurate data"s };

        Args args;
        desc.add_options()
            ("help,h", "Show help")
            ("tick-period,t", po::value(&args.update_period)->value_name("ms"s), "Time between server state updates")
            ("config-file,c", po::value(&args.config_path)->value_name("file"s), "Game server data file")
            ("www-root,w", po::value(&args.web_folder)->value_name("folder"s), "Directory with frontend game data")
            ("randomize-spawn-points", po::bool_switch(&args.random_position)->value_name("bool"), "spawn dogs at random positions")
            ("state-file", po::value(&args.save_path)->value_name("path"), "Path to file for saving date")
            ("save-state-period", po::value(&args.save_time_period)->value_name("miliseconds"), "Period between state saving");

        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help"s)) {
            std::cout << desc;
            return std::nullopt;
        }

        if (!vm.contains("config-file"s)) {
            throw std::runtime_error("Game server data file have not been specified"s);
        }

        if (!vm.contains("www-root"s)) {
            throw std::runtime_error("Directory with frontend game data is not specified"s);
        }

        return args;
    }
}