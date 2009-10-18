/*
 * main.cpp
 * Main program file. Starts bot and manages bot restarting caused by server
 * problems.
 */

#include "bot.h"

#include <locale>

int main()
{
    setlocale(LC_ALL, ""); // for console messages localization

    ChtonianBot b("config.ini");
}