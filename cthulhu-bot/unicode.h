/*
 * unicode.h
 * Header file for UTF8 text conversion.
 */
#ifndef UNICODE_H
#define UNICODE_H

#include <string>

std::string UTF8(const wchar_t *wc_string); // wide char string to utf8
std::wstring deUTF8(const std::string u8_string); // utf8 string to wstring

#endif // UNICODE_H
