#ifndef UNICODE_H
#define UNICODE_H

#include <string>

std::string utf8(const wchar_t *wc_string); // wide char string to utf8
std::wstring deUtf8(const std::string u8_string); // utf8 string to wstring

#endif // UNICODE_H
