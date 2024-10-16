#pragma once
#include <boost/log/trivial.hpp>     
#include <boost/log/core.hpp>        
#include <boost/log/expressions.hpp> 
#include <boost/log/utility/setup/common_attributes.hpp> 
#include <boost/log/utility/setup/console.hpp> 
#include <boost/date_time.hpp> 
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/json.hpp>

namespace server_logger {

    namespace keywords = boost::log::keywords;
    namespace logging = boost::log;
    namespace json = boost::json;

    BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
    BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value);

    void FormatLogWrite(logging::record_view const& rec, logging::formatting_ostream& strm);

    void SetLogFormat();
}