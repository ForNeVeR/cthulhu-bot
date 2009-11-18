/*
 * command.cpp
 * Realization for Command class.
 */
#include "command.h"

using namespace std;

/*
 * Command constructor. Takes name in format "!cmd", command syntax for help
 * in format "!cmd <arg1> <arg2>", args count and pointer to execution
 * function.
 */
Command::Command(const string &name, const string &syntax, const int argsCount,
    const int accessLevel, executionFunctionPtr execute)
{
    this->name = name;
    this->syntax = syntax;
    this->argsCount = argsCount;
    this->accessLevel = accessLevel;
    this->execute = execute;
}