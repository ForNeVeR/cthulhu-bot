#include "bot.h"

#include <disco.h>

#include <fstream>
#include <sstream>

namespace po = boost::program_options;
using namespace gloox;
using namespace std;

Bot::Bot(const string &config_name)
    : mmod_here(false)
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
    j = auto_ptr<Client>(new Client(jid, config["login.password"].as<string>()));

    j->disco()->setVersion("cthulhu-bot", "~0.0~");

    j->registerMessageHandler(this);
    j->registerSubscriptionHandler(this);
    j->registerConnectionListener(this);
    j->setPresence(PresenceAvailable);

    log(utf8(L"Захожу на сервер (") + config["login.jid"].as<string>() + utf8(L")..."));

    j->connect();
}

bool Bot::handleIqID(gloox::Stanza *stanza, int context)
{
    ostringstream log_ss;
    log_ss << utf8(L"Получен iq пакет, stanza->body(): \"") << stanza->body()
        << utf8(L"\" по контексту ") << context << utf8(L".");
    log(log_ss.str());

    switch(context)
    {
    case PING_CONTEXT:
        {
            map<string, time_t>::iterator ping_time_iter;
            if((ping_time_iter = pingTimes.find(stanza->id())) != pingTimes.end())
            {
                clock_t raw_time = clock();

                float time_delta = static_cast<float>(raw_time - ping_time_iter->second) / CLOCKS_PER_SEC;

                ostringstream message;
                if(stanza->subtype() != StanzaIqResult)
                    message << utf8(L"Ваш клиент не поддерживает XEP-0199, тем не менее, время отклика было определено. ");
                message << utf8(L"Время отклика от вас составило ") << time_delta << utf8(L" секунд.");

                bool sent = false;

                for(int i = 0; i < rooms.size(); i++)
                {
                    if((rooms[i]->name() + "@" + rooms[i]->service()) == stanza->from().bare())
                    {
                        // this mean that message will be sent to conference
                        rooms[i]->send(stanza->from().resource() + utf8(L": ") + message.str());
                        sent = true;
                        break;
                    }
                }
                
                if(!sent)
                {
                    // this mean that ping query came not from muc, but from regular contact
                    Stanza *s = Stanza::createMessageStanza(stanza->from(), message.str());
                    j->send(s);
                }

                pingTimes.erase(ping_time_iter);
                
                log(utf8(L"Отправлен ответ на ping от ") + stanza->from().full() + utf8(L": \"")
                    + message.str() + utf8(L"\"."));

                return true;
            }
        }
        break;
    default:
        return false;
    }

    return false;
}

void Bot::handleMessage(Stanza *stanza, MessageSession *session)
{
    log(utf8(L"<") + stanza->from().full() + utf8(L"> ") + stanza->body());
    
    executeCommand(stanza->body(), stanza->from());
}

void Bot::handleSubscription(gloox::Stanza *stanza)
{
    vector<string> masters = config["master.jid"].as<vector<string> >();
    if(find(masters.begin(), masters.end(), stanza->from().bare()) != masters.end())
    {
        Stanza *s = Stanza::createSubscriptionStanza(stanza->from(), "", StanzaS10nSubscribed);
        j->send(s);
        log(utf8(L"Получен запрос авторизации от ") + stanza->from().full() + utf8(L". Попытка принять..."));
        s = Stanza::createMessageStanza(stanza->from(), utf8(L"Ваша авторизация принята, командир."));
        j->send(s);
    }
}

void Bot::handleMUCParticipantPresence(MUCRoom *room, const MUCRoomParticipant participant, Presence presence)
{
    if(!mmod_here && participant.nick->resource() == "mmod" && presence != PresenceUnavailable)
    {
        room->send("mmod: ");
        mmod_here = true;
    }
    
    if(participant.nick->resource() == "mmod" && presence == PresenceUnavailable)
    {
        mmod_here = false;
    }
}

void Bot::handleMUCMessage(MUCRoom *room, const string &nick, const string &message, bool history,
    const string &when, bool privateMessage)
{
    if(!history && !privateMessage)
        confLog(room->name() + "@" + room->service(), nick, message);

    if(!history && (executeCommand(message, JID(room->name() + "@" + room->service() + "/" + nick), true) || privateMessage))
    {
        log(utf8(L"<") + room->name() + "@" + room->service() + "/" + nick + utf8(L"> ")
            + message);
    }
}

void Bot::onConnect()
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

// This method parses incoming string into vector containing words.
vector<string> Bot::parseCommand(const std::string &str) const
{
    istringstream command(str);
    vector<string> result;
    string buff;
    while(command >> buff)
    {
        result.push_back(buff);
    }
    return result;
}

// Returns room from array of NULL if room doesn't connected
// Also uses ini default_server feature
gloox::MUCRoom *Bot::getRoom(const std::string &name)
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

// Returns true if command was executed.
bool Bot::executeCommand(const string &command, const JID &source, bool from_muc)
{
    vector<string> arguments = parseCommand(command);

    vector<string> masters = config["master.jid"].as<vector<string> >();
    if(!from_muc && find(masters.begin(), masters.end(), source.bare()) != masters.end() && arguments.size() > 0)
    {
        if(arguments[0] == "!exit")
        {
            log(utf8(L"Выхожу по запросу ") + source.full() + utf8(L"."));
            j->disconnect();
            
            throw exception_exit;
        }
        else if(arguments[0] == "!enter" && arguments.size() > 1)
        {
            if(!getRoom(arguments[1]))
            {
                string room_name = arguments[1];
                if(room_name.find_first_of('@') == string::npos)
                {
                    room_name += "@";
                    room_name += config["muc.default_service"].as<string>();
                }

                log(utf8(L"Вхожу в комнату ") + room_name + utf8(L" по запросу ")
                    + source.full() + utf8(L"."));
                JID nick(room_name + utf8(L"/") + config["info.nick"].as<string>());

                rooms.push_back(new MUCRoom(j.get(), nick, this));
                rooms.back()->join();
            }
            else
                log(utf8(L"Уже нахожусь в данной комнате."));
            
            return true;
        }
        else if(arguments[0] == "!say" && arguments.size() > 1)
        {
            MUCRoom *room = getRoom(arguments[1]);
            if(room)
            {
                string message = command.substr(command.find(arguments[1]) + arguments[1].length() + 1);
                room->send(message);
            }
            else
                log(utf8(L"Невозможно найти требуемую комнату в списке подключенных."));

            return true;
        }
    }

    if(arguments.size() > 0 && arguments[0] == "!ping")
    {
        string id = j->getID();
        Stanza *parent = Stanza::createIqStanza(source, id, StanzaIqGet);
        Tag *tag = new Tag(parent, "ping", "xmlns", "urn:xmpp:ping", false);

        j->trackID(this, id, PING_CONTEXT);

        clock_t raw_time = clock();
        
        j->send(parent);

        pingTimes[id] = raw_time;
            
        log(utf8(L"Отправлен пакет ping на адрес ") + source.full() + utf8(L"."));
        
        return true;
    }
    else if(arguments.size() > 1 && arguments[0] == "!log" && from_muc)
    {
        string mask = command.substr(5);
        string message = logSearch(mask, source.bare());
        
        MUCRoom *room = getRoom(source.bare());
        if(room)
            room->send(message);

        return true;
    }
    else if(arguments.size() > 2 && arguments[0] == "!log" && !from_muc)
    {
        string mask = command.substr(command.find(arguments[1]) + arguments[1].length() + 1);
        string message;
        
        MUCRoom *room = getRoom(arguments[1]);
        if(room)
            message = logSearch(mask, room->name() + "@" + room->service());
        else
            message = logSearch(mask, arguments[1]);

        Stanza *s = Stanza::createMessageStanza(source, message);

        j->send(s);
        
        return true;
    }
    else if(arguments.size() > 2 && arguments[0] == "!log_i" && from_muc)
    {
        string mask = command.substr(command.find(arguments[1]) + arguments[1].length() + 1);
        string message = logSearch(mask, source.bare(), atoi(arguments[1].c_str()));
        
        MUCRoom *room = getRoom(source.bare());
        if(room)
            room->send(message);

        return true;
    }
    else if(arguments.size() > 2 && arguments[0] == "!log_i" && !from_muc)
    {
        string mask = command.substr(command.find(arguments[2]) + arguments[2].length() + 1);
        string message;

        MUCRoom *room = getRoom(arguments[1]);
        if(room)
            message = logSearch(mask, room->name() + "@" + room->service(), atoi(arguments[2].c_str()));
        else
            message = logSearch(mask, arguments[1], atoi(arguments[2].c_str()));
        Stanza *s = Stanza::createMessageStanza(source, message);

        j->send(s);

        return true;
    }
    else if(arguments.size() > 2 && arguments[0] == "!stat" && !from_muc)
    {
        string conf_name = arguments[1];
        string nick = command.substr(command.find(arguments[1]) + arguments[1].length() + 1);
        string message;

        MUCRoom *room = getRoom(arguments[1]);
        if(room)
            message = logStat(room->name() + "@" + room->service(), nick);
        else
            message = logStat(arguments[1], nick);
        Stanza *s = Stanza::createMessageStanza(source, message);

        j->send(s);

        return true;
    }

    return false;
}