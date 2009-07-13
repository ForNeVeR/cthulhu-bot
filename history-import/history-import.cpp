#include "history.h"
#include "import.h"
#include "unicode.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <clocale>
#include <iostream>

using namespace boost::filesystem;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    try
    {
        po::options_description desc("Options");
        desc.add_options()
            ("help", "Produce help message. If this option found, all other options are ignored.")
            ("miranda", "Logs will be imported from UTF-16LE coded Miranda IM logfiles. Note that there"
                " should be ONLY files named \"YYYY.MM.DD.log\" in input directory. You also should specify"
                " an 'conf-name' option if you've chosen Miranda IM as the source of history.")
            ("conf-name", po::value<string>(), "Full name of conference in form of <conference>@<server>,"
                " for example, conf@conference.example.com.")
            ("from", po::value<string>(), "Path to directory contains input files.")
            ("to", po::value<string>(), "Path to directory that will contain output files.")
            ("sort", "Sorts logs in directory 'to' after any other operation.")
            ("remove-backup", "Removes backup logs in case of sorting.")
            ("minutes-delta", po::value<int>()->default_value(0), "Implements time delta, as logs are storing"
                " in UTC time. entered value will be added (or substracted in case of 'minutes-delta' < 0)"
                " from time values loading from logfile.");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        cout << "Detected options:" << endl;
        if(vm.count("help"))
            cout << " --help" << endl;
        if(vm.count("miranda"))
            cout << " --miranda" << endl;
        if(vm.count("from"))
            cout << " --from = " << vm["from"].as<string>() << endl;
        if(vm.count("to"))
            cout << " --to = " << vm["to"].as<string>() << endl;
        if(vm.count("conf-name"))
            cout << " --conf-name = " << vm["conf-name"].as<string>() << endl;
        if(vm.count("minutes-delta"))
            cout << " --minutes-delta = " << vm["minutes-delta"].as<int>() << endl;
        if(vm.count("sort"))
            cout << " --sort" << endl;
        if(vm.count("remove-backup"))
            cout << " --remove-backup" << endl;
        cout << endl;

        if (vm.count("help"))
        {
            cout << desc << endl;
            return 0;
        }

        if(vm.count("miranda") && vm.count("from") && vm.count("to") && vm.count("conf-name"))
        {
            // check parameters
            path from(vm["from"].as<string>());
            path to(vm["to"].as<string>());
            if(is_directory(from))
            {
                cout << "Importing Miranda IM logfiles..." << endl << endl;
                miranda_import(vm["from"].as<string>(), vm["to"].as<string>(), vm["conf-name"].as<string>(),
                    vm["minutes-delta"].as<int>());
            }
            else
            {
                cerr << "Cannot open input directory." << endl;
                return 1;
            }
        }

        if(vm.count("sort") && vm.count("to"))
        {
            full_history_sort(vm["to"].as<string>(), false, !vm.count("remove_backup"));
        }
    }
    catch(exception& e)
    {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }
    catch(...)
    {
        cerr << "Exception of unknown type!" << endl;
        return 1;
    }

    return 0;
}