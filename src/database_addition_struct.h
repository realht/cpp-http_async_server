#pragma once
#include "tagged_uuid.h"

namespace db {

    const int MAX_DB_CONNECTION = 10;
    const int DEFAULT_OFFSET = 0;
    const double DEFAULT_MAX_ELEMENT = 100;
    const std::string OFFSET_STR = "start=";
    const std::string MAX_ELEMENT_STR = "maxItems=";


    struct DBSetting {
        std::string url;
        size_t num_connections;
    };

    struct GameRecords {
        std::string name;
        size_t score;
        double played_time;
    };
}

namespace util {
    namespace detail {
        struct PlayerTag {};
    }  // detail

    using PlayerId = util::TaggedUUID<detail::PlayerTag>;
}//util