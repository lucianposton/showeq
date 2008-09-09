/*
 * messageshell.h
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2002-2003 Zaphod (dohpaz@users.sourceforge.net)
 *
 */

#ifndef _MESSAGESHELL_H_
#define _MESSAGESHELL_H_

#include "messages.h"

#include <stdint.h>

#include <qobject.h>

//----------------------------------------------------------------------
// forward declarations
class QString;
class QDateTime;

class EQStr;
class Spells;
class ZoneMgr;
class SpawnShell;
class Item;
class Player;

struct ClientZoneEntryStruct;
struct ServerZoneEntryStruct;
struct charProfileStruct;
struct zoneChangeStruct;

//----------------------------------------------------------------------
// MessageShell
class MessageShell : public QObject
{
  Q_OBJECT
 public:
  MessageShell(Messages* messages, EQStr* eqStrings, Spells* spells,
	       ZoneMgr* zoneMgr, SpawnShell* spawnShell, 
               Player* player, QObject* parent = 0, const char* name = 0);

 public slots:
   void channelMessage(const uint8_t* cmsg, size_t, uint8_t);
   void formattedMessage(const uint8_t* cmsg, size_t, uint8_t);
   void simpleMessage(const uint8_t* cmsg, size_t, uint8_t);
   void specialMessage(const uint8_t* smsg, size_t, uint8_t);
   void guildMOTD(const uint8_t* gmotd, size_t, uint8_t);
   void consent(const uint8_t* consent, size_t, uint8_t);
   void moneyOnCorpse(const uint8_t* money);
   void moneyUpdate(const uint8_t* money);
   void moneyThing(const uint8_t* money);
   void randomRequest(const uint8_t* randr);
   void random(const uint8_t* randr);
   void emoteText(const uint8_t* emotetext);
   void inspectData(const uint8_t* inspt);

   void logOut(const uint8_t*, size_t, uint8_t);
   void zoneEntryClient(const ClientZoneEntryStruct* zsentry);
   void zoneNew(const uint8_t* zoneNew, size_t, uint8_t);
   void zoneChanged(const zoneChangeStruct*, size_t, uint8_t);
   void zoneBegin(const QString& shortZoneName);
   void zoneEnd(const QString& shortZoneName, const QString& longZoneName);
   void zoneChanged(const QString& shortZoneName);

   void worldMOTD(const uint8_t* motd);

   void handleSpell(const uint8_t* mem, size_t, uint8_t);
   void beginCast(const uint8_t* bcast);
   void spellFaded(const uint8_t* sf);
   void interruptSpellCast(const uint8_t*icast);
   void startCast(const uint8_t* cast);

   void groupUpdate(const uint8_t* gmem, size_t, uint8_t);
   void groupInvite(const uint8_t* gmem);
   void groupDecline(const uint8_t* gmem);
   void groupFollow(const uint8_t* gmem);
   void groupDisband(const uint8_t* gmem);
   void groupLeaderChange(const uint8_t* gmem);

   void syncDateTime(const QDateTime&);

   void player(const charProfileStruct* player);
   void increaseSkill(const uint8_t* data);
   void updateLevel(const uint8_t* data);
   void consMessage(const uint8_t* data, size_t, uint8_t dir);

   void setExp(uint32_t totalExp, uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);

   void newExp(uint32_t newExp, uint32_t totalExp, uint32_t totalTick,
	       uint32_t minExpLevel, uint32_t maxExpLevel, 
	       uint32_t tickExpLevel);
   void setAltExp(uint32_t totalExp,
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);
   void newAltExp(uint32_t newExp, uint32_t totalExp, uint32_t totalTick, 
		  uint32_t maxExp, uint32_t tickExp, uint32_t aapoints);

   void addItem(const Item* item);
   void delItem(const Item* item);
   void killSpawn(const Item* item);
   void filterMessage(const QString& prefix, MessageType type,
		      const Item* item);

 protected:
   Messages* m_messages;
   EQStr* m_eqStrings;
   Spells* m_spells;
   ZoneMgr* m_zoneMgr;
   SpawnShell* m_spawnShell;
   Player* m_player;
};


#endif // _MESSAGESHELL_H_

