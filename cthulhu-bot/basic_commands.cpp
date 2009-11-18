/*
 * basic_commands.cpp
 * Realizations of basic commands such as !say, !help etc.
 */
#include "bot.h"

using namespace gloox;
using namespace std;

string enter_cmd(const vector<string> &args, ChtonianBot &bot);
string exit_cmd(const vector<string> &args, ChtonianBot &bot);
string help_cmd(const vector<string> &args, ChtonianBot &bot);
string nick_cmd(const vector<string> &args, ChtonianBot &bot);
string say_cmd(const vector<string> &args, ChtonianBot &bot);


void ChtonianBot::registerAllBasicCommands()
{
#define REGISTER_BASIC_COMMAND(name, syntax, args, access)                   \
    registerCommand(Command(UTF8(L"!" L#name), UTF8(L"!" L#name L" " L ## syntax), \
        args, access, &name##_cmd))

    log(UTF8(L"Registering basic commands..."));

    REGISTER_BASIC_COMMAND(enter, "<conference>", 1, 100);
    REGISTER_BASIC_COMMAND(exit, "", 0, 100);
    REGISTER_BASIC_COMMAND(help, "", 0, 0);
    REGISTER_BASIC_COMMAND(nick, "<conference> <nick>", 2, 100);
    REGISTER_BASIC_COMMAND(say, "<conference> <message>", 2, 100);
        
#undef REGISTER_BASIC_COMMAND
}

string enter_cmd(const vector<string> &args, ChtonianBot &bot)
{
    if(!bot.getRoom(args[1]))
    {
        bot.enterRoom(args[1]);
    }
    else
        bot.log(UTF8(L"Already in this conference."));
    
    return UTF8(L"Entering conference ") + bot.finishRoomName(args[1])
        + UTF8(L".");
}

string exit_cmd(const vector<string> &args, ChtonianBot &bot)
{
    bot.log(UTF8(L"Leaving..."));
    bot.j->disconnect();
    
    return "";
}

string help_cmd(const vector<string> &args, ChtonianBot &bot)
{
    string response = UTF8(L"Avaliable commands:");
    
    for(vector<Command>::iterator cmd = bot.commands.begin();
        cmd != bot.commands.end(); ++cmd)
    {
        response += UTF8(L"\n") + (*cmd).syntax;
    }

    return response;
}

string nick_cmd(const vector<string> &args, ChtonianBot &bot)
{
    MUCRoom *room = bot.getRoom(args[1]);
    if(room)
    {
        room->setNick(args[2]);
        return UTF8(L"Nick has been set.");
    }
    else
        return UTF8(L"Cannot find this conference in list of connected "
            L"conferences.");
}

string say_cmd(const vector<string> &args, ChtonianBot &bot)
{
    MUCRoom *room = bot.getRoom(args[1]);
    if(room)
    {
        string message = args[2];
        room->send(message);
        bot.log(UTF8(L"-> ") + room->name() + UTF8(L"@") + room->service()
            + UTF8(L": ") + message);
        return UTF8(L"Message has been sent.");
    }
    else
        return UTF8(L"Cannot find this conference in list of connected "
            L"conferences.");
}
