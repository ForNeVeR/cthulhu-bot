/*
 * bot.cpp
 * This file implements general methods: connection, config and room handling.
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

        ("info.nick", po::value<string>()->default_value(UTF8(L"cthulhu-bot")))

        ("muc.default_service", po::value<string>()->default_value(""))

        ("autojoin.room", po::value<vector<string> >());
    ifstream ini_file(config_name.c_str());
    po::store(po::parse_config_file(ini_file, desc), config);
    po::notify(config);

    JID jid(config["login.jid"].as<string>());
    j = auto_ptr<Client>(new Client(jid,
        config["login.password"].as<string>()));

    j->disco()->setVersion("cthulhu-bot",
        "0.0 compiled at " __DATE__ " " __TIME__);

    j->registerMessageHandler(this);
    j->registerSubscriptionHandler(this);
    j->registerConnectionListener(this);
    j->setPresence(PresenceAvailable);

    log(UTF8(L"Entering server (") + config["login.jid"].as<string>()
        + UTF8(L")..."));

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
    if(config.count("autojoin.room"))
    {
        vector<string> autorooms
            = config["autojoin.room"].as<vector<string> >();
        for(vector<string>::const_iterator room_name = autorooms.begin();
            room_name != autorooms.end(); room_name++)
        {
            enterRoom(*room_name);
        }
    }
}

void ChtonianBot::enterRoom(const string &name)
{
    string room_name = finishRoomName(name);

    log(UTF8(L"Entering room ") + room_name + UTF8(L"."));

    JID nick(room_name + UTF8(L"/") + config["info.nick"].as<string>());
    rooms.push_back(new MUCRoom(j.get(), nick, this));
    rooms.back()->join();
}

// Returns room pointer or NULL if room doesn't connected.
gloox::MUCRoom *ChtonianBot::getRoom(const std::string &name) const
{
    string room_name = finishRoomName(name);
    
    for(vector<MUCRoom *>::const_iterator room = rooms.begin();
        room != rooms.end(); room++)
    {
        if((*room)->name() + UTF8(L"@") + (*room)->service() == room_name)
        {
            return *room;
        }
    }
    
    return NULL;
}

string ChtonianBot::finishRoomName(const string &name) const
{
    if(name.find_first_of(UTF8(L"@")) == string::npos)
    {
        return name + UTF8(L"@") + config["muc.default_service"].as<string>();
    }

    return name;
}
