#include "import.h"

#include "history.h"
#include "unicode.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace boost;
using namespace boost::filesystem;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void miranda_import(const string &from, const string &to, const std::string &conf_name, int minutes_delta)
{
    static const wregex expr(L"^\\[([0-9]{2}):([0-9]{2}):([0-9]{2})\\] \\* (.*?) \\* (.*)\\r$");

    path from_path(from);

	directory_iterator end_itr; // default construction yields past-the-end
	for(directory_iterator itr(from_path); itr != end_itr; ++itr)
    {
        cout << "Converting file " << itr->path().filename() << "..." << endl;

        std::wifstream input_file(itr->path().string().c_str(), ios_base::in | ios::binary);

        // ignore UTF-16 BOM
        input_file.ignore(2);

        while(!input_file.eof())
        {
            wstring line;
            
            wchar_t c = L'\0';
            while(c != L'\r' && !input_file.eof())
            {
                wchar_t c1 = L'\0', c2 = L'\0';
                input_file.get(c1);
                input_file.get(c2);
                c = c2 << 8 | c1;
                line += c;
            }
            input_file.ignore(2); // ignoring following L'\n' character
            
            // check regex matches
            wsmatch match;
            if(regex_match(line, match, expr))
            {
                int year = atoi(itr->filename().substr(0, 4).c_str());
                int month = atoi(itr->filename().substr(5, 2).c_str());
                int day = atoi(itr->filename().substr(8, 2).c_str());

                date d(year, month, day);

                // TODO: Some other than utf8() here, wcsto...() maybe?
                int hour = atoi(utf8(match[1].str().c_str()).c_str());
                int min = atoi(utf8(match[2].str().c_str()).c_str());
                int sec = atoi(utf8(match[3].str().c_str()).c_str());

                ptime datetime(d, time_duration(hour, min, sec, 0) + minutes(minutes_delta));
                string nick = utf8(match[4].str().c_str());
                string message = utf8(match[5].str().c_str());

                history_add(datetime, conf_name, nick, message, to, true);
            }
        }

        input_file.close();
    }
}