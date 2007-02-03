TEMPLATE = app
TARGET = showeq
MOC_DIR = tmp
OBJECTS_DIR = tmp
UI_DIR = tmp
CONFIG += qt thread stl warn_on debug
DEFINES += PKGDATADIR=\"/usr/local/share/showeq\" 
DEFINES += QMAKEBUILD
LIBS += -lpcap

SOURCES += main.cpp spawn.cpp spawnshell.cpp spawnlist.cpp spellshell.cpp \
	spelllist.cpp vpacket.cpp editor.cpp filter.cpp packetfragment.cpp packetstream.cpp \
	packetinfo.cpp packet.cpp packetcapture.cpp packetformat.cpp compass.cpp \
	map.cpp util.cpp experiencelog.cpp combatlog.cpp player.cpp skilllist.cpp \
	statlist.cpp filtermgr.cpp mapcore.cpp category.cpp compassframe.cpp group.cpp \
	guild.cpp netdiag.cpp logger.cpp xmlconv.cpp xmlpreferences.cpp seqwindow.cpp \
	seqlistview.cpp zonemgr.cpp spawnmonitor.cpp spawnpointlist.cpp spawnlistcommon.cpp \
	spawnlist2.cpp spells.cpp datetimemgr.cpp spawnlog.cpp packetlog.cpp \
	datalocationmgr.cpp eqstr.cpp messages.cpp message.cpp messagefilter.cpp messagewindow.cpp \
	messageshell.cpp terminal.cpp filteredspawnlog.cpp messagefilterdialog.cpp \
	diagnosticmessages.cpp mapicon.cpp filternotifications.cpp netstream.cpp guildshell.cpp \
	guildlist.cpp bazaarlog.cpp 

SOURCES += interface.cpp 

INCLUDEPATH += ..
INCLUDEPATH += /usr/include/pcap

HEADES += ../config.h
HEADERS += classes.h compass.h everquest.h interface.h main.h map.h filter.h vpacket.h editor.h packet.h packetcapture.h packetcommon.h packetformat.h packetstream.h packetfragment.h packetinfo.h races.h skills.h spells.h util.h experiencelog.h combatlog.h spawn.h spawnshell.h spawnlist.h spellshell.h spelllist.h languages.h weapons.h weapons1.h weapons27.h weapons28.h weapons29.h weapons2a.h weapons2b.h weapons2c.h decode.h cgiconv.h skilllist.h statlist.h db3conv.h dbcommon.h deity.h player.h crctab.h filtermgr.h point.h pointarray.h mapcore.h category.h compassframe.h group.h guild.h fixpt.h netdiag.h zones.h logger.h xmlconv.h xmlpreferences.h seqwindow.h seqlistview.h zonemgr.h spawnmonitor.h spawnpointlist.h typenames.h spawnlistcommon.h spawnlist2.h datetimemgr.h spawnlog.h packetlog.h datalocationmgr.h eqstr.h messages.h messagefilter.h messagewindow.h messageshell.h terminal.h filteredspawnlog.h messagefilterdialog.h diagnosticmessages.h mapicon.h mapicondialog.ui.h filternotifications.h netstream.h guildshell.h guildlist.h bazaarlog.h message.h s_everquest.h staticspells.h 
#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3 += mapicondialog.ui 

#The following line was inserted by qt3to4
QT += xml  qt3support 
#The following line was inserted by qt3to4
CONFIG += uic3

