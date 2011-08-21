/*
 * bot.h
 * Main bot header file, declares all methods.
 */
#ifndef BOT_H
#define BOT_H

//#include <ctime>
#include <string>
#include <map>
#include <memory>

#include <boost/program_options.hpp>

#include <client.h>
#include <connectionlistener.h>
#include <iqhandler.h>
#include <message.h>
#include <messagehandler.h>
#include <mucroom.h>
#include <mucroomhandler.h>
#include <subscriptionhandler.h>

#include "command.h"
#include "unicode.h"

class ChtonianBot : public gloox::MessageHandler,
    public gloox::SubscriptionHandler, public gloox::MUCRoomHandler,
    public gloox::ConnectionListener
{
private:
    // Common resources
    std::unique_ptr<gloox::Client> j; // Why "j"? But why not?
    std::vector<gloox::MUCRoom *> rooms;
    boost::program_options::variables_map config;
    std::vector<Command> commands;

    // Access levels
    int getAccessLevel(std::string source);

    // Logs
    //std::string logSearch(const std::string &regex, const std::string &conf_name = "", int index = -1) const;
    static void log(std::string message);
    //std::string logStat(const std::string &conf_name, const std::string &nick = "") const;

    // Working with conferences
    void enterRoom(const std::string &name);
    gloox::MUCRoom *getRoom(const std::string &name) const;
    std::string finishRoomName(const std::string &name) const;
    void confLog(const std::string &conf_name, const std::string &nick,
        std::string message) const;
    
    // Commands
    static bool parseCommand(const std::string &str,
        std::vector<std::string> &result);
    std::string executeCommand(const std::string &command,
        const int accessLevel, const bool fromMUC = false);
    void registerCommand(const Command &newCommand);

    // Basic commands stuff
    void registerAllBasicCommands();
    friend std::string enter_cmd(const std::vector<std::string> &args,
        const int accessLevel, ChtonianBot &bot);
    friend std::string exit_cmd(const std::vector<std::string> &args,
        const int accessLevel, ChtonianBot &bot);
    friend std::string help_cmd(const std::vector<std::string> &args,
        const int accessLevel, ChtonianBot &bot);
    friend std::string nick_cmd(const std::vector<std::string> &args,
        const int accessLevel, ChtonianBot &bot);
    friend std::string say_cmd(const std::vector<std::string> &args,
        const int accessLevel, ChtonianBot &bot);

    // History
    friend void history_add(const std::tm &utc_datetime,
        const std::string &conf_name, const std::string &nick,
        std::string message, bool ignore_dups);
    friend void full_history_sort(bool ignore_dups);

public:
    ChtonianBot(const std::string &config_name);

    virtual void handleMessage(const gloox::Message &message,
        gloox::MessageSession *session);

    virtual void handleSubscription(const gloox::Subscription &subscription);

    virtual void handleMUCParticipantPresence(gloox::MUCRoom *room,
        const gloox::MUCRoomParticipant participant,
        const gloox::Presence &presence);
    virtual void handleMUCMessage(gloox::MUCRoom *room,
        const gloox::Message &message, bool history);
    virtual bool handleMUCRoomCreation(gloox::MUCRoom *room);
    virtual void handleMUCSubject(gloox::MUCRoom *room,
        const std::string &nick, const std::string &subject);
    virtual void handleMUCInviteDecline(gloox::MUCRoom *room,
        const gloox::JID &invitee, const std::string &reason);
    virtual void handleMUCError(gloox::MUCRoom *room,
        gloox::StanzaError error);
    virtual void handleMUCInfo(gloox::MUCRoom *room, int features,
        const std::string &name, const gloox::DataForm *infoForm);
    virtual void handleMUCItems(gloox::MUCRoom *room,
        const gloox::Disco::ItemList &items);

    virtual void onConnect();
    virtual void onDisconnect(gloox::ConnectionError e);
    virtual void onResourceBindError(gloox::ResourceBindError error);
    virtual void onSessionCreateError(gloox::SessionCreateError error);
    virtual bool onTLSConnect(const gloox::CertInfo &info);
};

#endif // BOT_H
