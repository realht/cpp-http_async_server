#include "logger.h"

namespace server_logger {

    void FormatLogWrite(logging::record_view const& rec, logging::formatting_ostream& strm) {
        strm << "{\"timestamp\":\"";

        auto ts = *rec[timestamp];
        strm << to_iso_extended_string(ts) << "\",\"data\":";

        if (rec[additional_data].get().is_object()) {
            strm << rec[additional_data].get().as_object();
        }

        strm << ",\"message\":\"" << rec[logging::expressions::smessage] << "\"}";
    }

    void SetLogFormat() {
        logging::add_common_attributes();

        logging::add_console_log(
            std::cout,
            keywords::format = &FormatLogWrite,
            keywords::auto_flush = true
        );
    }

}
