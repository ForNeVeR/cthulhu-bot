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

void ChtonianBot::handleMessage(const Message &message, MessageSession *session)
{
    log(UTF8(L"<") + message.from().full() + UTF8(L"> ") + message.body());
    
    string response = executeCommand(message.body(),
        getAccessLevel(message.from().bare()));
    if(response != "")
    {
        Message m(Message::Chat, message.from(), response);
        j->send(m);
        log(UTF8(L"-> ") + message.from().full() + UTF8(L": ") + response);
    }
}

void ChtonianBot::handleMUCMessage(MUCRoom *room, const Message &message,
    bool history)
{
    if(!history) // Ignore historical messages.
    {
        auto nick = message.from().username();

        string room_jid = room->name() + UTF8(L"@") + room->service();

        confLog(room_jid, nick, message.body());

        string response = executeCommand(message.body(), 0);
        if(response != "")
        {
            log(UTF8(L"<") + room_jid + UTF8(L"/") + nick + UTF8(L"> ")
                + message.body());
            
            if(message.subtype() == Message::Groupchat)
            {
                room->send(response);
                log(UTF8(L"-> ") + room_jid + UTF8(L": ") + response);
            }
            else
            {
                Message m(Message::Chat, JID(room_jid + UTF8(L"/") + nick),
                    response);
                j->send(m);
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
    const wregex quoted_arg(L"^\"(.*?(?<!\\\\)(\\\\\\\\)*)\".*");

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
                return cmd->execute(arguments, accessLevel, *this);
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
