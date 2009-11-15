/*
 * commands.cpp
 * This file implements various methods for command handling such as chat and
 * private message handlers.
 */
#include "bot.h"

using namespace gloox;
using namespace std;

void ChtonianBot::handleMessage(Stanza *stanza, MessageSession *session)
{
    log(UTF8(L"<") + stanza->from().full() + UTF8(L"> ") + stanza->body());
    
    string response = executeCommand(stanza->body(),
        getAccessLevel(stanza->from().bare()));
    if(response != "")
    {
        Stanza *s = Stanza::createMessageStanza(stanza->from(), response);
        j->send(s);
    }
}

void ChtonianBot::handleMUCMessage(MUCRoom *room, const string &nick,
    const string &message, bool history, const string &when,
    bool privateMessage)
{
    if(!history) // We ignore historical messages.
    {
        string room_jid = room->name() + UTF8(L"@") + room->service();

        confLog(room_jid, nick, message);

        string response = executeCommand(message, 0);
        if(response != "")
        {
            log(UTF8(L"<") + room_jid + UTF8(L"/") + nick + UTF8(L"> ")
                + message);
            
            if(!privateMessage)
                room->send(response);
            else
            {
                // TODO: Check this.
                Stanza *s = Stanza::createMessageStanza(JID(room_jid
                    + UTF8(L"/") + nick), response);
                j->send(s);
            }
        }
    }
}

// This method parses incoming string into vector containing words.
vector<string> ChtonianBot::parseCommand(const std::string &str) const
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

// Main command method. Tries to execute command and returns some response.
string ChtonianBot::executeCommand(const string &command,
    const int accessLevel, const bool fromMUC)
{
    vector<string> arguments = parseCommand(command);

    if(arguments.size() == 0)
        return "";

    // Admin commands
    if(accessLevel >= 100)
    {
        if(arguments[0] == "!exit")
        {
            log(UTF8(L"Leaving..."));
            j->disconnect();

            // TODO: Better throw exception here?
            return "";
        }
        else if(arguments[0] == "!enter" && arguments.size() > 1)
        {
            if(!getRoom(arguments[1]))
            {
                string room_name = arguments[1];
                enterRoom(room_name);
            }
            else
                log(UTF8(L"Already in this room."));
            
            return UTF8(L"Entering ") + arguments[0] + UTF8(L".");
        }
        /*else if(arguments[0] == "!say" && arguments.size() > 1)
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
        }*/
        /*else if(arguments[0] == "!ban" && arguments.size() > 2)
        {
            MUCRoom *room = getRoom(arguments[1]);
            if(room)
            {
              room->ban(arguments[2], "Banned by bot owner.");
            }
            else
                log(utf8(L"Невозможно найти требуемую комнату в списке подключенных."));

            return true;
        }*/
    }

    /*if(arguments.size() > 0 && arguments[0] == "!ping")
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
    else if(arguments.size() > 1 && arguments[0] == "!stat" && from_muc)
    {
        string nick = command.substr(arguments[0].length() + 1);
        string message = logStat(source.bare(), nick);
        
        MUCRoom *room = getRoom(source.bare());
        if(room)
            room->send(message);

        return true;
    }
    else if(arguments.size() > 1 && arguments[0] == "!topten" && !from_muc)
    {
        string conf_name = arguments[1];
        string message;

        MUCRoom *room = getRoom(arguments[1]);
        if(room)
            message = logStat(room->name() + "@" + room->service());
        else
            message = logStat(arguments[1]);
        Stanza *s = Stanza::createMessageStanza(source, message);

        j->send(s);

        return true;
    }*/

    return "";
}

bool ChtonianBot::handleIqID(gloox::Stanza *stanza, int context)
{
    /*ostringstream log_ss;
    log_ss << utf8(L"Получен iq пакет по контексту ") << context << utf8(L".");
    log(log_ss.str());

    switch(context)
    {
    case PING_CONTEXT:
        {
            map<string, time_t>::iterator ping_time_iter = pingTimes.find(stanza->id());
            if(ping_time_iter != pingTimes.end())
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
    }*/

    return false;
}

void ChtonianBot::handleSubscription(gloox::Stanza *stanza)
{
    if (getAccessLevel(stanza->from().bare()) >= 100)
    {
        Stanza *s = Stanza::createSubscriptionStanza(stanza->from(), "",
            StanzaS10nSubscribed);
        j->send(s);
        log(UTF8(L"Получен запрос авторизации от ") + stanza->from().full()
            + UTF8(L". Попытка принять..."));
        s = Stanza::createMessageStanza(stanza->from(),
            UTF8(L"Ваша авторизация принята, командир."));
        j->send(s);
    }
    else
    {
        log(UTF8(L"Получен запрос авторизации от ") + stanza->from().full()
            + UTF8(L". Игнорирую."));
    }
}
