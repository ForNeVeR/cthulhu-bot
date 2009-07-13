#ifndef BOT_H
#define BOT_H

#include "unicode.h"

#include <client.h>
#include <connectionlistener.h>
#include <iqhandler.h>
#include <messagehandler.h>
#include <mucroom.h>
#include <mucroomhandler.h>
#include <subscriptionhandler.h>

#include <boost/program_options.hpp>

#include <ctime>
#include <string>
#include <map>
#include <memory>

class Bot : public gloox::MessageHandler, public gloox::SubscriptionHandler, public gloox::IqHandler,
    public gloox::MUCRoomHandler, public gloox::ConnectionListener
{
private:
    // common resources
    std::map<std::string, std::time_t> pingTimes; // Ping packet id => ping packet send time
    std::auto_ptr<gloox::Client> j; // Why "j"? But why not?
    std::vector<gloox::MUCRoom *> rooms;
    //Ini config;
    boost::program_options::variables_map config;

    const static int PING_CONTEXT = 0;

public:
    Bot(const std::string &config_name);

    virtual void handleMessage(gloox::Stanza *stanza, gloox::MessageSession *session = 0);

    virtual bool handleIqID(gloox::Stanza *stanza, int context);
    virtual bool handleIq(gloox::Stanza *stanza) { return false; }

    virtual void handleSubscription(gloox::Stanza *stanza);

    virtual void handleMUCParticipantPresence(gloox::MUCRoom *room, const gloox::MUCRoomParticipant participant, gloox::Presence presence);
    virtual void handleMUCMessage(gloox::MUCRoom *room, const std::string &nick, const std::string &message, bool history,
        const std::string &when, bool privateMessage);
    virtual bool handleMUCRoomCreation(gloox::MUCRoom *room) { return true; }
    virtual void handleMUCSubject(gloox::MUCRoom *room, const std::string &nick, const std::string &subject) { }
    virtual void handleMUCInviteDecline(gloox::MUCRoom *room, const gloox::JID &invitee, const std::string &reason) { }
    virtual void handleMUCError(gloox::MUCRoom *room, gloox::StanzaError error) { }
    virtual void handleMUCInfo(gloox::MUCRoom *room, int features, const std::string &name, const gloox::DataForm *infoForm) { }
    virtual void handleMUCItems(gloox::MUCRoom *room, const gloox::StringMap &items) { }

    virtual void onConnect();
    virtual void onDisconnect(gloox::ConnectionError e) { }
    virtual void onResourceBindError(gloox::ResourceBindError error) { }
    virtual void onSessionCreateError(gloox::SessionCreateError error) { }
    virtual bool onTLSConnect(const gloox::CertInfo &info) { return true; }

    static const int exception_exit = 100500;

private:
    // internal methods
    std::string logSearch(const std::string &regex, const std::string &conf_name = "", int index = -1) const;
    static void log(std::string message);
    std::string logStat(const std::string &conf_name, const std::string &nick) const;
    void confLog(const std::string &conf_name, const std::string &nick, std::string message) const;
    gloox::MUCRoom *getRoom(const std::string &name);
    std::vector<std::string> parseCommand(const std::string &str) const;
    bool executeCommand(const std::string &command, const gloox::JID &source, bool from_muc = false);

    // mmod mod
    bool mmod_here;

    friend void history_add(const std::tm &utc_datetime, const std::string &conf_name, const std::string &nick,
        std::string message, bool ignore_dups);
    friend void full_history_sort(bool ignore_dups);
};

#endif // BOT_H
