/*
 * dummies.cpp
 * File for various bot dummy methods (methods that have to be implemented
 * because they are pure virtual in base classes, but that doesn't need to do
 * any real work).
 */
#include "bot.h"

using namespace gloox;

void ChtonianBot::handleMUCParticipantPresence(MUCRoom *room,
    const MUCRoomParticipant participant, Presence presence)
{
    
}