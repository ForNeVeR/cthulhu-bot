#include "history.h"

#include "unicode.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace boost;
using namespace boost::filesystem;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void history_add(const ptime &utc_datetime, const string &conf_name, const string &nick, string message,
    const string &log_dirname, bool ignore_dups)
{
    static const regex expr("^\\[([0-9]{2}):([0-9]{2}):([0-9]{2})\\] <(.*?)> (.*)$");

    static string static_filename;
    static ptime static_datetime;
    static int static_position(-1);

    date utc_date = utc_datetime.date();
    time_duration utc_time = utc_datetime.time_of_day();

    ostringstream filename_ostream;
    filename_ostream << conf_name << '[' << setw(4) << setfill('0') << right << utc_date.year() << '.'
        << setw(2) << utc_date.month() << '.' << setw(2) << utc_date.day() << "].txt";

    ostringstream time_ostream;
    time_ostream << '[' << setw(2) << setfill('0') << right << utc_time.hours() << ':' << setw(2) 
        << utc_time.minutes() << ':' << setw(2) << utc_time.seconds() << ']';

    path log_directory = system_complete(log_dirname);
    if(!exists(log_directory))
    {
        cout << "Creating directory " << log_directory.string() << "." << endl;
        create_directory(log_directory);
    }

    if(!is_directory(log_directory))
    {
        cout << log_directory.string() << " is not a directory, cannot write logs there." << endl;
        return;
    }

    path filename(log_directory / filename_ostream.str());
    if(exists(filename) && !is_empty(filename))
    {
        ifstream in_file(filename.string().c_str(), fstream::in | fstream::binary);
        ptime current_datetime;

        if(static_filename == filename.filename() && static_datetime <= utc_datetime
            && static_position != -1)
        {
            in_file.seekg(static_position, fstream::beg);
            current_datetime = static_datetime;
        }
        else
        {
            in_file.ignore(3); // ignore BOM
        
            current_datetime = ptime(date(utc_date), time_duration(0,0,0,0));
        }
        
        int line_size = 0;

        while(current_datetime <= utc_datetime && !in_file.eof())
        {
            string line;
            getline(in_file, line, '\r');
            line_size = line.length() + 1;
            in_file.ignore(1); // ignoring following '\n' character
            line_size += in_file.gcount();
            
            smatch match;
            if(regex_match(line, match, expr))
            {
                int hour = atoi(match[1].str().c_str());
                int min = atoi(match[2].str().c_str());
                int sec = atoi(match[3].str().c_str());

                current_datetime = ptime(current_datetime.date(), time_duration(hour, min, sec));

                static_position = in_file.tellg();
                static_datetime = current_datetime;
                static_filename = filename.filename();

                if(ignore_dups && current_datetime == utc_datetime && match[4].str() == nick
                    && match[5].str() == message)
                {
                    cout << "Ignoring duplicate in file " << filename.filename() << ' '
                        << time_ostream.str() << '.' << endl;
                    return;
                }
            }
        }

        // 1. Read all remaining data from the file.
        // 2. Write our message into file at current position.
        // 3. Write backuped data.
        // 4. ?????
        // 5. PROFIT!

        if(!in_file.eof())
            in_file.seekg(-line_size, fstream::cur);
        else
            in_file.seekg(0, fstream::end);
        int file_pos = in_file.tellg();
        in_file.seekg(0, fstream::end);
        int file_end = in_file.tellg();

        char *data = new char[file_end - file_pos];

        in_file.seekg(file_pos, fstream::beg);
        in_file.read(data, file_end - file_pos);
        
        in_file.close();

        /*ostringstream out_line;
        out_line << time_ostream.str() << utf8(L" <") << nick << utf8(L"> ") << message << utf8(L"\r\n");

        FILE *out_file = fopen(filename.file_string().c_str(), "r+b");
        fseek(out_file, file_pos, SEEK_SET);
        fwrite(out_line.str().c_str(), out_line.str().length(), 1, out_file);
        fwrite(data, file_end - file_pos, 1, out_file);
        fclose(out_file);*/
        
        fstream out_file(filename.string().c_str(), fstream::in | fstream::out | fstream::binary);
        out_file.seekp(file_pos);
        out_file << time_ostream.str() << utf8(L" <") << nick << utf8(L"> ") << message << utf8(L"\r\n");
        out_file.write(data, file_end - file_pos);

        out_file.close();

        delete[] data;
    }
    else if(!is_directory(filename))
    {
        /*ostringstream out_line;
        out_line << time_ostream.str() << utf8(L" <") << nick << utf8(L"> ") << message << utf8(L"\r\n");

        FILE *out_file = fopen(filename.file_string().c_str(), "r+b");
        fwrite(out_line.str().c_str(), out_line.str().length(), 1, out_file);
        fclose(out_file);*/
        fstream out_file(filename.string().c_str(), ios::out | ios::binary);
        out_file << "\xEF\xBB\xBF";
        out_file << time_ostream.str() << utf8(L" <") << nick << utf8(L"> ") << message << utf8(L"\r\n");

        out_file.close();
    }
}

void full_history_sort(const std::string &log_dirname, bool ignore_dups, bool keep_backup)
{
    static const regex date_expr("^(.*?)\\[([0-9]{4})\\.([0-9]{2})\\.([0-9]{2})\\]\\.txt$");
    static const regex msg_expr("^\\[([0-9]{2}):([0-9]{2}):([0-9]{2})\\] <(.*?)> (.*)$");
    
    path log_directory(system_complete(path(log_dirname)));
    path backup_directory(system_complete(path(log_dirname + "_bak")));
    if(exists(backup_directory))
    {
        cout << "Directory " << backup_directory.string() << " already exists. Delete it before sorting."
            << endl;
        return;
    }
    rename(log_directory, backup_directory);

    directory_iterator end_itr; // default construction yields past-the-end
	for(directory_iterator itr(backup_directory); itr != end_itr; ++itr)
    {
        smatch date_match;
        string filename = itr->path().filename();
        if(regex_match(filename, date_match, date_expr))
        {
            int year = atoi(date_match[2].str().c_str());
            int month = atoi(date_match[3].str().c_str());
            int day = atoi(date_match[4].str().c_str());

            cout << "Processing file " << itr->path().filename() << "..." << endl;

            ifstream in_file(itr->path().string().c_str(), fstream::in | fstream::binary);
            in_file.ignore(3); // ignore BOM

            while(!in_file.eof())
            {
                string line;
                getline(in_file, line, '\r');
                in_file.ignore(1); // ignoring following '\n' character
                
                smatch match;
                if(regex_match(line, match, msg_expr))
                {
                    int hour = atoi(match[1].str().c_str());
                    int min = atoi(match[2].str().c_str());
                    int sec = atoi(match[3].str().c_str());

                    ptime current_log_time(date(year, month, day), time_duration(hour, min, sec));
                    string nick = match[4].str();
                    string message = match[5].str();
                    string conf_name = date_match[1].str();

                    history_add(current_log_time, conf_name, nick, message, log_dirname,
                        ignore_dups);
                }
            }
        }
    }
    if(!keep_backup)
    {
        cout << "Removing backup directory..." << endl;
        remove_all(backup_directory);
    }
}