#include "bot.h"

#include "unicode.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <clocale>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace boost;
using namespace boost::filesystem;
using namespace std;

void ChtonianBot::log(string message)
{
    time_t raw_time = time(NULL);
    tm *ptm = gmtime(&raw_time);

    wcout << deUTF8(message) << endl;

    const char format[] = "[%Y.%m.%d %H:%M:%S]";
    // format time as "[2009.06.14 14:34:02]" - 21 characters + 1 for terminating null == 22
    const int buff_size = 22;

    char *buff = new char[buff_size];
    strftime(buff, buff_size, format, ptm);

    ofstream log_file("logfile.log", ios_base::app | ios_base::binary);
    
    // log control TODO: CHECK
    int found = string::npos;
    while((found = message.find('\r')) != string::npos)
    {
        message[found] = '\n';
    }

    log_file << buff << " " << message << "\r\n";
    log_file.close();

    delete[] buff;
}

void ChtonianBot::confLog(const string &conf_name, const string &nick, string message) const
{
    // prepare timestamp
    time_t raw_time = time(NULL);
    tm *ptm = gmtime(&raw_time);
    
    const string filename_format = conf_name + "[%Y.%m.%d].txt";
    const char timestamp_format[] = "[%H:%M:%S]";
    
    const int filename_size = conf_name.length() + 17;
    const int timestamp_size = 11;

    char *filename_buff = new char[filename_size];
    char *timestamp_buff = new char[timestamp_size];
    strftime(filename_buff, filename_size, filename_format.c_str(), ptm);
    strftime(timestamp_buff, timestamp_size, timestamp_format, ptm);

    string filename_string = filename_buff;
    string timestamp_string = timestamp_buff;
    delete[] filename_buff;
    delete[] timestamp_buff;
    
    path log_directory(system_complete(path("logs")));
    if(!exists(log_directory))
    {
        log(UTF8(L" аталог дл€ логов не найден. —оздаю каталог \"") + log_directory.string() + UTF8(L"\"."));
        create_directory(log_directory);
    }

    if(!is_directory(log_directory))
    {
        log(UTF8(L"\"") + log_directory.string() + UTF8(L"\" не €вл€етс€ каталогом, запись логов невозможна."));
        return;
    }

    path filename(log_directory / filename_string);

    ofstream log_file(filename.string().c_str(), ios_base::app | ios_base::binary);
	if(!exists(filename) || boost::filesystem::is_empty(filename))
        log_file << "\xEF\xBB\xBF";

    // log control TODO: CHECK
    int found = string::npos;
    while((found = message.find('\r')) != string::npos)
    {
        message[found] = '\n';
    }

    log_file << timestamp_string << UTF8(L" <") << nick << UTF8(L"> ") << message << "\r\n";
}

/*string ChtonianBot::logSearch(const string &mask, const string &conf_name, int index) const
{
    srand(time(NULL));

    try
    {
        regex expr(mask);

        vector<string> messages;
        path log_directory(system_complete(path("logs")));

        directory_iterator end_itr;
        for(directory_iterator logfile_itr(log_directory); logfile_itr != end_itr; ++logfile_itr)
        {
            if(!is_directory(logfile_itr->status()) && (conf_name == "" || logfile_itr->path().filename().find(conf_name) == 0))
            {
                ifstream logfile(logfile_itr->path().string().c_str(), ios_base::in | ios::binary);
                logfile.ignore(3); // ignore UTF-8 BOM

                string line, message;
                while(!getline(logfile, line).eof())
                {
                    if(line.empty() || line[line.length() - 1] != '\r')
                    {
                        message += line;
                        message += '\n';
                    }
                    else
                    {
                        message += line;
                        if(regex_match(message, expr))
                        {
                            message += "> ";
                            message += logfile_itr->path().filename() + " ";
                            messages.push_back(message);
                        }
                        message.clear();
                    }
                }
            }
        }
        
        if(!messages.empty())
        {
            int ind = (index == -1 || index >= messages.size()) ? rand() % messages.size() : index;

            ostringstream result;
            result << messages[ind] << utf8(L"[") << ind + 1 << utf8(L"/") << messages.size() << utf8(L"]");
            return result.str();
        }

        return utf8(L"Ќичего не найдено.");
    }
    catch(...)
    {
        // TODO: разобратьс€ конкретнее, что именно ловить.
        return utf8(L"–егекспы-то писать научись!");
    }
}

class Participant
{
public:
    int message_count;
    int word_count;

    Participant()
    {
        message_count = 0;
        word_count = 0;
    }
};

typedef pair<string, Participant> String_Participant;

class Transformer
{
public:
    static int i;
    vector<String_Participant> vec;

    void operator()(map<string, Participant>::const_reference p)
    {
        vec.push_back(String_Participant(p.first, p.second));
    }
};

static bool compare(const String_Participant &a, const String_Participant &b)
{
    return a.second.message_count < b.second.message_count;
}

/*string Bot::logStat(const string &conf_name, const string &nick) const
{
    /*static const regex msg_expr("^\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\] <(.*?)> (.*)$");
    static const wregex words_expr(L"\\<(.*?)\\>");

    map<string, Participant> stats;

    path log_directory(system_complete(path("logs")));
    if(!exists(log_directory) || !is_directory(log_directory))
    {
        log(utf8(L"Ќе могу открыть каталог с логами."));
        return utf8(L"Ќе могу открыть каталог с логами.");
    }

    directory_iterator end_itr; // default construction yields past-the-end
	for(directory_iterator itr(log_directory); itr != end_itr; ++itr)
    {
        if(itr->path().filename().find(conf_name) != 0)
            continue;

        ifstream in_file(itr->path().string().c_str(), fstream::in | fstream::binary);
        in_file.ignore(3); // ignore BOM

        while(!in_file.eof())
        {
            string line;
            getline(in_file, line, '\r');
            in_file.ignore(1); // ignoring following '\n' character

            smatch msg_match;
            if(regex_match(line, msg_match, msg_expr))
            {
                string log_nick = msg_match[1].str();
                if(nick.empty() || nick == log_nick)
                {
                    wstring wmessage = deUtf8(msg_match[2].str());

                    stats[log_nick].message_count++;

                    // parse message
                    wsmatch words_match;
                    wstring::const_iterator start = wmessage.begin();
                    wstring::const_iterator end = wmessage.end();
                    while(regex_search(start, end, words_match, words_expr))
                    {
                        wstring word = words_match[1].str();
                        stats[log_nick].word_count++;
                        start += word.length();
                    }
                }
            }
        }

        in_file.close();
    }
    
    ostringstream result;
    if(!nick.empty())
    {
        result << nick << " : " << stats[nick].message_count << " : " << stats[nick].word_count;
    }
    else
    {
        Transformer transformer;
        transformer.i = 0;
        transformer.vec.clear();
        for_each(stats.begin(), stats.end(), transformer);
        partial_sort(transformer.vec.begin(), transformer.vec.begin() + min<int>(9, transformer.vec.size()),
            transformer.vec.end(), compare);

        vector<String_Participant>::iterator it = transformer.vec.begin();
        for(int i = 0; i < 10 && it != transformer.vec.end(); ++it, i++)
        {
            result << i << " : " << it->first << " : " << it->second.message_count << " : " << it->second.word_count
                << '\n';
        }
    }
    return result.str();*/
  /*return "";
}*/
