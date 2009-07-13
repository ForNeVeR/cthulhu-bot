#include "unicode.h"

#include <unicode/unistr.h>

using namespace std;

string utf8(const wchar_t *wc_string)
{
    UnicodeString u16_string(wc_string);
    string u8_string;
    u16_string.toUTF8String(u8_string);
    return u8_string;
}

wstring deUtf8(const string u8_string)
{
    UnicodeString u16_string = UnicodeString::fromUTF8(u8_string);
    return wstring(u16_string.getBuffer());
}