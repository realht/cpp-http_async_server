#pragma once
#include <set>
#include <string>
#include "tagged_uuid.h"

namespace app {
    struct AppConfig {
        bool self_update;
        bool random_position;
        std::string saved_file;
        int time_between_save;
        std::string db_url;
    };
}