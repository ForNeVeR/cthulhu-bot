#include "bot.h"

int main()
{
    setlocale(LC_ALL, "Russian"); // for console messages localization

    try
    {
        for(;;)
            Bot b("config.ini");
    }
    catch(int &i)
    {
        if(i != Bot::exception_exit)
            throw i;
        else
            return 0;
    }
}