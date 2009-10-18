/*
 * bot.cpp
 * This file implements main bot methods such as connection handling, config
 * properties and access level workaround.
 * Various "dummy" methods also goes here.
 */

#include "bot.h"

#include <fstream>
#include <sstream>

#include <disco.h>

namespace po = boost::program_options;
using namespace gloox;
using namespace std;

ChtonianBot::ChtonianBot(const string &config_name)
{
    po::options_description desc("Config file options");
    desc.add_options()
        ("login.jid", po::value<string>())
        ("login.password", po::value<string>())

        ("master.jid", po::value<vector<string> >())

        ("info.nick", po::value<string>()->default_value(utf8(L"cthulhu-bot")))

        ("muc.default_service", po::value<string>()->default_value(""))

        ("autojoin.room", po::value<vector<string> >());
    ifstream ini_file(config_name.c_str());
    po::store(po::parse_config_file(ini_file, desc), config);
    po::notify(config);

    JID jid(config["login.jid"].as<string>());
    j = auto_ptr<Client>(new Client(jid,
        config["login.password"].as<string>()));

    j->disco()->setVersion("cthulhu-bot", "~0.0~");

    j->registerMessageHandler(this);
    j->registerSubscriptionHandler(this);
    j->registerConnectionListener(this);
    j->setPresence(PresenceAvailable);

    log(utf8(L"Захожу на сервер (") + config["login.jid"].as<string>()
        + utf8(L")..."));

    j->connect();
}

int ChtonianBot::getAccessLevel(string source)
{
    // TODO: Fixes for room members and others
    vector<string> masters = config["master.jid"].as<vector<string> >();
    if(find(masters.begin(), masters.end(), source) != masters.end())
        return 100;
    else
        return 0;
}

void ChtonianBot::onConnect()
{
    // TODO: !!! FULLY REWORK THIS!
    if(config.count("autojoin.room"))
    {
        vector<string> autorooms = config["autojoin.room"].as<vector<string> >();
        for(vector<string>::const_iterator i = autorooms.begin(); i != autorooms.end(); i++)
        {
            executeCommand(string("!enter ") + *i, config["master.jid"].as<vector<string> >()[0]);
        }
    }
}

// Returns room from array of NULL if room doesn't connected
// Also uses ini default_server feature
gloox::MUCRoom *ChtonianBot::getRoom(const std::string &name)
{
    string room_name = name;
    if(room_name.find_first_of('@') == string::npos)
    {
        room_name += "@";
        room_name += config["muc.default_service"].as<string>();
    }

    for(int i = 0; i < rooms.size(); i++)
    {
        if(rooms[i]->name() + "@" + rooms[i]->service() == room_name)
        {
            return rooms[i];
        }
    }
    
    return NULL;
}
