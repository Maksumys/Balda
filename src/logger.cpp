#include "src/logger.hpp"
#include <iostream>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void logger_initialize( const std::string &name_log )
{
    boost::log::add_file_log
            (
                    keywords::file_name = name_log + "_%N.log",
                    keywords::rotation_size = 10 * 1024 * 1024,
                    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                    keywords::format = "[%TimeStamp%]: %Message%",
                    keywords::auto_flush = true
            );

    boost::log::add_console_log( std::cout,
                                 keywords::rotation_size = 10 * 1024 * 1024,
                                 keywords::format = "[%TimeStamp%]: %Message%" );

    logging::add_common_attributes();
}