/*
 * commands.cpp
 * This file implements various methods for command handling such as chat and
 * private message handlers.
 */
#include "bot.h"

#include <boost/regex.hpp>

using namespace boost;
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
        log(UTF8(L"-> ") + stanza->from().full() + UTF8(L": ") + response);
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
            {
                room->send(response);
                log(UTF8(L"-> ") + room_jid + UTF8(L": ") + response);
            }
            else
            {
                Stanza *s = Stanza::createMessageStanza(JID(room_jid
                    + UTF8(L"/") + nick), response);
                j->send(s);
                log(UTF8(L"-> <") + room_jid + UTF8(L"/") + nick + UTF8(L">: ")
                    + response);
            }
        }
    }
}

/*
 * This method parses incoming string into vector containing command name and
 * its arguments.
 * Returns true if parsed string is proper command; false otherwise.
 */
bool ChtonianBot::parseCommand(const std::string &str,
    vector<string> &result)
{
    wstring cmd = deUTF8(str);
    
    const wregex command_name(L"^(![^\\s]+).*");
    const wregex simple_arg(L"^([^\\s]+).*");
    const wregex quoted_arg(L"^\"(.*?[^\\\\])\".*");
    // TODO: This variant of quoted_arg does not accept escaping of anything
    // but quote character '"'.

    result.clear();

    wsmatch arg_match;
    if(regex_match(cmd, arg_match, command_name))
    {
        wstring cmd_name = arg_match[1].str();
        cmd.erase(0, cmd_name.length());
        result.push_back(UTF8(cmd_name.c_str()));

        while(!cmd.empty())
        {
            // Trim whitespace from beginning of string
            const wregex trimmed(L"^(\\s*).*");
            wsmatch trimmed_str;
            if(regex_match(cmd, trimmed_str, trimmed))
            {
                cmd.erase(0, trimmed_str[1].length());
            }

            wstring arg;
            int arg_size = 0; // amount of characters that need to be removed
            if(regex_match(cmd, arg_match, quoted_arg))
            {
                arg = arg_match[1].str();
                arg_size = arg.length() + 2; // because we also need to remove
                                             // 2 '"' characters
                /*const wregex escape(L"\\\\(.)");
                arg = regex_replace(arg, escape, L"(?1)");
                TODO: Do PROPER escaping here!
                (Now we only replace \" -> ")*/
                const wregex escape(L"\\\\\"");
                arg = regex_replace(arg, escape, L"\"");
            }
            else if(regex_match(cmd, arg_match, simple_arg))
            {
                arg = arg_match[1].str();
                arg_size = arg.length();
            }
            
            cmd.erase(0, arg_size);
            result.push_back(UTF8(arg.c_str()));
        }

        return true;
    }

    return false;
}

// Main command method. Tries to execute command and returns some response.
string ChtonianBot::executeCommand(const string &command,
    const int accessLevel, const bool fromMUC)
{
    vector<string> arguments;
    if(parseCommand(command, arguments))
    {
        for(vector<Command>::iterator cmd = commands.begin();
            cmd != commands.end(); ++cmd)        
        {
            if(arguments[0] == cmd->name
                && arguments.size() == cmd->argsCount + 1
                && accessLevel >= cmd->accessLevel)
            {
                return cmd->execute(arguments, *this);
            }
        }
    }

    return "";
}

/*
 * Registers new command for bot.
 */
void ChtonianBot::registerCommand(const Command &newCommand)
{
    log(UTF8(L"Registering command ") + newCommand.name + UTF8(L"."));
    commands.push_back(newCommand);
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
