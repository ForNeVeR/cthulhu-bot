/*
 * unicode.cpp
 * Source file for UTF8 text conversion. Uses utfcpp, see
 * <http://utfcpp.sourceforge.net/>.
 */
#include "unicode.h"

#include <vector>
using namespace std;

#include "../3rd-party/utfcpp/utf8.h"

string UTF8(const wchar_t *wc_string)
{
    string utf8_string;
    utf8::utf16to8(&wc_string[0], &wc_string[wcslen(wc_string)],
        back_inserter(utf8_string));

    return utf8_string;
}

wstring deUTF8(const string utf8_string)
{
    wstring utf16_string;
    utf8::utf8to16(utf8_string.begin(), utf8_string.end(),
        back_inserter(utf16_string));

    return utf16_string;
}
