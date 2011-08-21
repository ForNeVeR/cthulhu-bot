/*
 * dummies.cpp
 * File for various bot dummy methods (methods that have to be implemented
 * because they are pure virtual in base classes, but that doesn't need to do
 * any real work).
 */
#include "bot.h"

using namespace gloox;
using namespace std;

void ChtonianBot::handleMUCParticipantPresence(MUCRoom *room,
    const MUCRoomParticipant participant, const Presence &presence)
{
    
}

bool ChtonianBot::handleMUCRoomCreation(MUCRoom *room)
{
    return false;
}

void ChtonianBot::handleMUCSubject(MUCRoom *room, const string &nick, 
    const string &subject)
{

}

void ChtonianBot::handleMUCInviteDecline(MUCRoom *room, const JID &invitee,
    const string &reason)
{

}

void ChtonianBot::handleMUCError(MUCRoom *room, StanzaError error)
{

}

void ChtonianBot::handleMUCInfo(MUCRoom *room, int features,
    const string &name, const gloox::DataForm *infoForm)
{

}

void ChtonianBot::handleMUCItems(MUCRoom *room, const Disco::ItemList &items)
{

}

void ChtonianBot::onDisconnect(gloox::ConnectionError e)
{

}

void ChtonianBot::onResourceBindError(gloox::ResourceBindError error)
{

}

void ChtonianBot::onSessionCreateError(gloox::SessionCreateError error)
{
    
}

bool ChtonianBot::onTLSConnect(const gloox::CertInfo &info)
{
    return true;
}
