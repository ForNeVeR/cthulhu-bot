/*
 * command.h
 * Definition of class Command, representing command syntax handler.
 */
#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

class ChtonianBot;

/*
 * Class for command definition.
 */
class Command
{
    typedef std::string (* executionFunctionPtr)
        (const std::vector<std::string> &args, ChtonianBot &bot);

public:
    std::string name;
    std::string syntax;
    int argsCount;
    int accessLevel;
    executionFunctionPtr execute;

    Command(const std::string &name, const std::string &syntax,
        const int argsCount, const int accessLevel,
        executionFunctionPtr execute);
};

#endif COMMAND_H
