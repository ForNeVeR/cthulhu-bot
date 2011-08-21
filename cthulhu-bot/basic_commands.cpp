/*
 * basic_commands.cpp
 * Realizations of basic commands such as !say, !help etc.
 */
#include "bot.h"

using namespace gloox;
using namespace std;

string bf_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot);
string enter_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot);
string exit_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot);
string help_cmd(const vector<string> &arg, const int accessLevel,
    ChtonianBot &bot);
string nick_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot);
string say_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot);

void ChtonianBot::registerAllBasicCommands()
{
    // This macro registers command named "!name" as function name_cmd().
#define REGISTER_BASIC_COMMAND(name, syntax, args, access) \
    registerCommand(Command(UTF8(L"!" L#name),             \
        UTF8(L"!" L#name L" " L##syntax), args, access, &name##_cmd))

    log(UTF8(L"Registering basic commands..."));

    REGISTER_BASIC_COMMAND(bf, "<code>", 1, 0);
    REGISTER_BASIC_COMMAND(enter, "<conference>", 1, 100);
    REGISTER_BASIC_COMMAND(exit, "", 0, 100);
    REGISTER_BASIC_COMMAND(help, "", 0, 0);
    REGISTER_BASIC_COMMAND(nick, "<conference> <nick>", 2, 100);
    REGISTER_BASIC_COMMAND(say, "<conference> <message>", 2, 100);
        
#undef REGISTER_BASIC_COMMAND
}

string bf_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
{
    const int MEM_SIZE = 256;
    unsigned char memory[MEM_SIZE];
    memset(memory, 0, MEM_SIZE);
    int pos_in_mem = 0;
    int pos_in_code = 0;
    int time_to_live = 256 * 256;
    const string &code = args[1];
    vector<int> loops; // stack of adresses of open brackets '['
    string result;
    while(pos_in_code < code.length())
    {
        if(time_to_live-- == 0)
            return UTF8(L"Execution took too long. Aborted.");
        switch(code[pos_in_code])
        {
        case '-':
            --memory[pos_in_mem];
            ++pos_in_code;
            break;
        case '+':
            ++memory[pos_in_mem];
            ++pos_in_code;
            break;
        case '<':
            if(pos_in_mem != 0)
                --pos_in_mem;
            else
                pos_in_mem = MEM_SIZE - 1;
            ++pos_in_code;
            break;
        case '>':
            if(pos_in_mem != MEM_SIZE - 1)
                ++pos_in_mem;
            else
                pos_in_mem = 0;
            ++pos_in_code;
            break;
        case '[':
            loops.push_back(pos_in_code);
            ++pos_in_code;
            break;
        case ']':
            if(loops.size() == 0)
                return UTF8(L"ZOMG! Error!");
            if(memory[pos_in_mem])
            {
                pos_in_code = loops.back();
                loops.pop_back();
            }
            else
                ++pos_in_code;
            break;
        case ',':
            return UTF8(L"Operator ',' not supported.");
        case '.':
            if(memory[pos_in_mem] < 9)
            {
                result += L'?';
            }
            else
            {
                wchar_t *buff = new wchar_t[2];
                buff[0] = wchar_t(memory[pos_in_mem]);
                buff[1] = L'\0';
                result += UTF8(buff);
                delete[] buff;
            }

            ++pos_in_code;
            break;
        default:
            ++pos_in_code;
            break;
        }
    }
    if(result == "")
        result = "No output.";

    return result;
}

string enter_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
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

string exit_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
{
    bot.log(UTF8(L"Leaving..."));
    bot.j->disconnect();
    
    return "";
}

string help_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
{
    string response = UTF8(L"Available commands:");
    
    for(vector<Command>::iterator cmd = bot.commands.begin();
        cmd != bot.commands.end(); ++cmd)
    {
        if(cmd->accessLevel <= accessLevel)
            response += UTF8(L"\n") + cmd->syntax;
    }

    return response;
}

string nick_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
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

string say_cmd(const vector<string> &args, const int accessLevel,
    ChtonianBot &bot)
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
