#ifndef HISTORY_H
#define HISTORY_H
// various history-related functions

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>

void history_add(const boost::posix_time::ptime &utc_datetime, const std::string &conf_name, const std::string &nick,
    std::string message, const std::string &log_dirname, bool ignore_dups = false);
void full_history_sort(const std::string &log_dirname, bool ignore_dups = false,
    bool keep_backup = true);

#endif // HISTORY_H