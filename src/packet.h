/*
 * packet.h
 *
 *  ShowEQ Distributed under GPL
 *  http://www.sourceforge.net/projects/seq
 *
 *  Copyright 2000-2003 by the respective ShowEQ Developers
 *  Portions Copyright 2001-2003 Zaphod (dohpaz@users.sourceforge.net). 
 */

#ifndef _PACKET_H_
#define _PACKET_H_

#include <qobject.h>
#include "everquest.h"
#include "packetcommon.h"

#if defined (__GLIBC__) && (__GLIBC__ < 2)
#error "Need glibc 2.1.3 or better"
#endif

#if (defined(__FreeBSD__) || defined(__linux__)) && defined(__GLIBC__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ < 2)
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#endif

#include <netinet/in.h>

//----------------------------------------------------------------------
// forward declarations
class VPacket;
class PacketCaptureThread;
class EQPacketStream;
class EQUDPIPPacketFormat;

//----------------------------------------------------------------------
// EQPacket
class EQPacket : public QObject
{
   Q_OBJECT 
 public:
   
   EQPacket(QObject *parent = 0,
            const char *name = 0);
   ~EQPacket();           
   void start(int delay = 0);
   void stop(void);

   const QString pcapFilter();
   int packetCount(int);
   in_addr_t clientAddr(void);
   in_port_t clientPort(void);
   in_port_t serverPort(void);
   uint8_t session_tracking_enabled(void);
   int playbackSpeed(void);
   size_t currentCacheSize(int);
   uint16_t serverSeqExp(int);

 public slots:
   void processPackets(void);
   void processPlaybackPackets(void);
   void incPlayback(void);
   void decPlayback(void);
   void setPlayback(int);
   void monitorIPClient(const QString& address);   
   void monitorMACClient(const QString& address);   
   void monitorNextClient();   
   void monitorDevice(const QString& dev);   
   void session_tracking();
   void setArqSeqGiveUp(int giveUp);

 protected slots:
   void closeStream();
   void lockOnClient(in_port_t serverPort, in_port_t clientPort);

 signals:
   // used for net_stats display
   void cacheSize(int, int);
   void seqReceive(int, int);
   void seqExpect(int, int);
   void numPacket(int, int);
   void resetPacket(int, int);
   void playbackSpeedChanged(int);
   void clientChanged(in_addr_t);
   void clientPortLatched(in_port_t);
   void serverPortLatched(in_port_t);
   void sessionTrackingChanged(uint8_t);
   void toggle_session_tracking(void);
   void filterChanged(void);

   // new logging
   void newPacket(const EQUDPIPPacketFormat& packet);
   void rawWorldPacket(const uint8_t* data, size_t len, uint8_t dir, 
		       uint16_t opcode);
   void decodedWorldPacket(const uint8_t* data, size_t len, uint8_t dir,
			   uint16_t opcode);
   void rawZonePacket(const uint8_t* data, size_t len, uint8_t dir, 
		      uint16_t opcode);
   void decodedZonePacket(const uint8_t* data, size_t len, uint8_t dir,
			  uint16_t opcode);
   void decodedZonePacket(const uint8_t* data, size_t len, uint8_t dir,
			  uint16_t opcode, bool unknown);

   // Player signals
   void setPlayerID(uint16_t);
   void backfillPlayer(const charProfileStruct *, uint32_t, uint8_t);
   void increaseSkill(const skillIncStruct* skilli, uint32_t, uint8_t);
   void manaChange(const manaDecrementStruct* mana, uint32_t, uint8_t);
   void playerUpdate(const playerSpawnPosStruct* pupdate, uint32_t, uint8_t);
   void playerUpdate(const playerSelfPosStruct* pupdate, uint32_t, uint8_t);
   void updateExp(const expUpdateStruct* exp, uint32_t, uint8_t);
   void updateAltExp(const altExpUpdateStruct* altexp, uint32_t, uint8_t);
   void updateLevel(const levelUpUpdateStruct* levelup, uint32_t, uint8_t);
   void updateStamina(const staminaStruct* stam, uint32_t, uint8_t);

   void action(const actionStruct*, uint32_t, uint8_t);
   void attack2Hand1(const attack2Struct*, uint32_t, uint8_t);
   void action2Message(const action2Struct*, uint32_t, uint8_t);
   
   void consMessage(const considerStruct*, uint32_t, uint8_t);
   
   void clientTarget(const clientTargetStruct* target, uint32_t, uint8_t);
   void newDoorSpawns(const doorSpawnsStruct *, uint32_t, uint8_t);
   void newDoorSpawn(const doorStruct *, uint32_t, uint8_t);
   void spawnWearingUpdate(const wearChangeStruct*, uint32_t, uint8_t);

   void newGroundItem(const makeDropStruct*, uint32_t, uint8_t);
   void removeGroundItem(const remDropStruct*, uint32_t, uint8_t);

   void updateSpawns(const spawnPositionUpdate* updates, uint32_t, uint8_t);
   void updateSpawnMaxHP(const SpawnUpdateStruct* spawnupdate, uint32_t, uint8_t);
   void updateSpawnInfo(const SpawnUpdateStruct* spawnupdate, uint32_t, uint8_t);

   void newSpawn(const newSpawnStruct* spawn, uint32_t, uint8_t);
   void deleteSpawn(const deleteSpawnStruct* delspawn, uint32_t, uint8_t);
   void killSpawn(const newCorpseStruct* deadspawn, uint32_t, uint8_t);
   void corpseLoc(const corpseLocStruct*, uint32_t, uint8_t);
   void timeOfDay(const timeOfDayStruct *tday, uint32_t, uint8_t);

   void moneyOnCorpse(const moneyOnCorpseStruct* money, uint32_t, uint8_t);
   void channelMessage(const channelMessageStruct* cmsg, uint32_t, uint8_t);
   void formattedMessage(const formattedMessageStruct* fmsg, uint32_t, uint8_t);
   void simpleMessage(const simpleMessageStruct* fmsg, uint32_t, uint8_t);
   void specialMessage(const specialMessageStruct* smsg, uint32_t, uint8_t);
   void guildMOTD(const guildMOTDStruct* smsg, uint32_t, uint8_t);
   void item(const itemPacketStruct* item, uint32_t, uint8_t);
   void itemInfoReq(const itemInfoReqStruct* item, uint32_t, uint8_t);
   void itemInfo(const itemInfoStruct* item, uint32_t, uint8_t);
   void playerItem(const char* serializedItem, uint32_t, uint8_t);
   void random(const randomReqStruct* randr, uint32_t, uint8_t);
   void random(const randomStruct* randr, uint32_t, uint8_t);
   void emoteText(const emoteTextStruct* emotetext, uint32_t, uint8_t);
   void inspectData(const inspectDataStruct* inspt, uint32_t, uint8_t);
   void spMessage(const spMesgStruct* spmsg, uint32_t, uint8_t);
   void handleSpell(const memSpellStruct* mem, uint32_t, uint8_t);
   void beginCast(const beginCastStruct* bcast, uint32_t, uint8_t);
   void startCast(const startCastStruct* cast, uint32_t, uint8_t);
   void spellFaded(const spellFadedStruct* cast, uint32_t, uint8_t);
   void systemMessage(const sysMsgStruct* smsg, uint32_t, uint8_t);
   void moneyUpdate(const moneyUpdateStruct* money, uint32_t, uint8_t);
   void moneyThing(const moneyThingStruct* money, uint32_t, uint8_t);
   void groupInfo(const groupMemberStruct* gmem, uint32_t, uint8_t);
   void groupInvite(const groupInviteStruct* gmem, uint32_t, uint8_t);
   void groupDecline(const groupDeclineStruct* gmem, uint32_t, uint8_t);
   void groupAccept(const groupAcceptStruct* gmem, uint32_t, uint8_t);
   void groupDelete(const groupDeleteStruct* gmem, uint32_t, uint8_t);
   void zoneSpawns(const zoneSpawnsStruct* zspawns, uint32_t, uint8_t);
   void zoneEntry(const ServerZoneEntryStruct* zsentry, uint32_t, uint8_t);
   void zoneEntry(const ClientZoneEntryStruct* zsentry, uint32_t, uint8_t);
   void zoneChange(const zoneChangeStruct* zoneChange, uint32_t, uint8_t);
   void zoneNew(const newZoneStruct* zoneNew, uint32_t, uint8_t);
   void zonePoints(const zonePointsStruct* zp, uint32_t, uint8_t);
   void logOut(const uint8_t*, uint32_t, uint8_t);
   void buff(const buffStruct*, uint32_t, uint8_t);
   void msgReceived(const QString &);
   void stsMessage(const QString &, int = 0);

   // Spell signals
   void interruptSpellCast(const badCastStruct *, uint32_t, uint8_t);

   // other signals
   void zoneServerInfo(const uint8_t*, uint32_t, uint8_t);
   void bookText(const bookTextStruct*, uint32_t, uint8_t);
   void doorOpen(const uint8_t*, uint32_t, uint8_t);
   void illusion(const uint8_t*, uint32_t, uint8_t);
   void castOn(const castOnStruct*, uint32_t, uint8_t);
   void openVendor(const uint8_t*, uint32_t, uint8_t);
   void closeVendor(const uint8_t*, uint32_t, uint8_t);
   void openGM(const uint8_t*, uint32_t, uint8_t);
   void closeGM(const uint8_t*, uint32_t, uint8_t);
   void spawnAppearance(const spawnAppearanceStruct*, uint32_t, uint8_t);
   void newGuildInZone(const newGuildInZoneStruct*, uint32_t, uint8_t);
   void bindWound(const bindWoundStruct*, uint32_t, uint8_t);

   void updateNpcHP(const hpNpcUpdateStruct* hpupdate, uint32_t, uint8_t);
   void tradeSpellBookSlots(const tradeSpellBookSlotsStruct*, uint32_t, uint8_t);

   // World server signals
   void worldGuildList(const char*, uint32_t);
   void worldMOTD(const worldMOTDStruct*, uint32_t, uint8_t);

 private:
      
   PacketCaptureThread* m_packetCapture;
   VPacket* m_vPacket;
   QTimer* m_timer;

   in_port_t m_serverPort;
   in_port_t m_clientPort;
   bool m_busy_decoding;
   bool m_detectingClient;
   in_addr_t m_client_addr;

   EQPacketStream* m_client2WorldStream;
   EQPacketStream* m_world2ClientStream;
   EQPacketStream* m_client2ZoneStream;
   EQPacketStream* m_zone2ClientStream;
   EQPacketStream* m_streams[MAXSTREAMS];

   void dispatchPacket   (int size, unsigned char *buffer);
 protected slots:
   void resetEQPacket();
   void dispatchWorldChatData (uint32_t len, uint8_t* data, uint8_t direction = 0);
   void dispatchWorldData(const uint8_t* data, size_t len, uint8_t direction = 0, uint16_t = 0);
   void dispatchZoneData(const uint8_t* data, size_t len, uint8_t direction = 0, uint16_t = 0);
};

inline in_addr_t EQPacket::clientAddr(void)
{
   return m_client_addr;
}

inline in_port_t EQPacket::clientPort(void)
{
  return m_clientPort;
}

inline in_port_t EQPacket::serverPort(void)
{
  return m_serverPort;
}

#endif // _PACKET_H_
