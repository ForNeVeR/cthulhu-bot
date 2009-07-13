#ifndef IMPORT_H
#define IMPORT_H

#include <string>

void miranda_import(const std::string &from, const std::string &to, const std::string &conf_name,
    int minutes_delta);

#endif // IMPORT_H