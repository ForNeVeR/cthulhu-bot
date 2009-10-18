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
    log(utf8(L"<") + stanza->from().full() + utf8(L"> ") + stanza->body());
    
    string response = executeCommand(stanza->body(), );
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
    if(!history) // We are ignoring historical messages.
    {
        confLog(room->name() + UTF8(L"@") + room->service(), nick, message);

        response = executeCommand(message, 0);
        if(response != "")
        {
            log(utf8(L"<") + room->name() + UTF8(L"@") + room->service() +
                UTF8(L"/") + nick + utf8(L"> ") + message);
            
            if(!privateMessage)
                room->send(response);
            else
            {
                // TODO: Check this.
                Stanza *s = Stanza::createMessageStanza(JID(room->name
                    + UTF8(L"@") + room->service() + UTF8(L"/") + nick),
                    response);
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
    const int accessLevel)
{
    vector<string> arguments = parseCommand(command);

    vector<string> masters = config["master.jid"].as<vector<string> >();
    if(!from_muc && find(masters.begin(), masters.end(), source.bare()) != masters.end() && arguments.size() > 0)
    {
        if(arguments[0] == "!exit")
        {
            log(utf8(L"������ �� ������� ") + source.full() + utf8(L"."));
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

                log(utf8(L"����� � ������� ") + room_name + utf8(L" �� ������� ")
                    + source.full() + utf8(L"."));
                JID nick(room_name + utf8(L"/") + config["info.nick"].as<string>());

                rooms.push_back(new MUCRoom(j.get(), nick, this));
                rooms.back()->join();
            }
            else
                log(utf8(L"��� �������� � ������ �������."));
            
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
                log(utf8(L"���������� ����� ��������� ������� � ������ ������������."));

            return true;
        }
        else if(arguments[0] == "!ban" && arguments.size() > 2)
        {
            MUCRoom *room = getRoom(arguments[1]);
            if(room)
            {
              room->ban(arguments[2], "Banned by bot owner.");
            }
            else
                log(utf8(L"���������� ����� ��������� ������� � ������ ������������."));

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
            
        log(utf8(L"��������� ����� ping �� ����� ") + source.full() + utf8(L"."));
        
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
    }

    return false;
}

bool ChtonianBot::handleIqID(gloox::Stanza *stanza, int context)
{
    /*ostringstream log_ss;
    log_ss << utf8(L"������� iq ����� �� ��������� ") << context << utf8(L".");
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
                    message << utf8(L"��� ������ �� ������������ XEP-0199, ��� �� �����, ����� ������� ���� ����������. ");
                message << utf8(L"����� ������� �� ��� ��������� ") << time_delta << utf8(L" ������.");

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
                
                log(utf8(L"��������� ����� �� ping �� ") + stanza->from().full() + utf8(L": \"")
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
    if (getAccessLevel(stanza->from().bare()) == 100)
    {
        Stanza *s = Stanza::createSubscriptionStanza(stanza->from(), "",
            StanzaS10nSubscribed);
        j->send(s);
        log(utf8(L"������� ������ ����������� �� ") + stanza->from().full()
            + utf8(L". ������� �������..."));
        s = Stanza::createMessageStanza(stanza->from(),
            utf8(L"���� ����������� �������, ��������."));
        j->send(s);
    }
    else
    {
        log(utf8(L"������� ������ ����������� �� ") + stanza->from().full()
            + utf8(". ���������."));
    }
}