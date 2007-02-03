INCLUDES = $(QT_INCLUDES) $(X_INCLUDES) -DPKGDATADIR=\"$(pkgdatadir)/\" -I/usr/include/pcap $(USER_INCLUDES)

QTLIB = -lqt-mt

bin_PROGRAMS = showeq 

showeq_SOURCES = main.cpp spawn.cpp spawnshell.cpp spawnlist.cpp spellshell.cpp \
	spelllist.cpp vpacket.cpp editor.cpp filter.cpp packetfragment.cpp packetstream.cpp \
	packetinfo.cpp packet.cpp packetcapture.cpp packetformat.cpp interface.cpp compass.cpp \
	map.cpp util.cpp experiencelog.cpp combatlog.cpp player.cpp skilllist.cpp \
	statlist.cpp filtermgr.cpp mapcore.cpp category.cpp compassframe.cpp group.cpp \
	guild.cpp netdiag.cpp logger.cpp xmlconv.cpp xmlpreferences.cpp seqwindow.cpp \
	seqlistview.cpp zonemgr.cpp spawnmonitor.cpp spawnpointlist.cpp spawnlistcommon.cpp \
	spawnlist2.cpp spells.cpp datetimemgr.cpp spawnlog.cpp packetlog.cpp \
	datalocationmgr.cpp eqstr.cpp messages.cpp message.cpp messagefilter.cpp messagewindow.cpp \
	messageshell.cpp terminal.cpp filteredspawnlog.cpp messagefilterdialog.cpp \
	diagnosticmessages.cpp mapicon.cpp filternotifications.cpp netstream.cpp guildshell.cpp \
	guildlist.cpp bazaarlog.cpp 

showeq_moc_SRCS = bazaarlog.moc category.moc combatlog.moc compass.moc \
  compassframe.moc datetimemgr.moc editor.moc experiencelog.moc \
  filteredspawnlog.moc filtermgr.moc filternotifications.moc group.moc \
  guild.moc guildlist.moc guildshell.moc interface.moc logger.moc \
  map.moc mapicon.moc messagefilter.moc messagefilterdialog.moc messages.moc \
  messageshell.moc messagewindow.moc netdiag.moc packet.moc packetinfo.moc \
  packetlog.moc packetstream.moc player.moc seqlistview.moc \
  seqwindow.moc skilllist.moc spawnlist.moc spawnlist2.moc spawnlistcommon.moc \
  spawnlog.moc spawnmonitor.moc spawnpointlist.moc spawnshell.moc spelllist.moc \
  spellshell.moc statlist.moc terminal.moc xmlpreferences.moc zonemgr.moc

nodist_showeq_SOURCES = ui_mapicondialog.h ui_mapicondialog.cpp $(showeq_moc_SRCS) m_ui_mapicondialog.cpp 

$(srcdir)/bazaarlog.cpp: bazaarlog.moc
$(srcdir)/category.cpp: category.moc
$(srcdir)/combatlog.cpp: combatlog.moc
$(srcdir)/compass.cpp: compass.moc
$(srcdir)/compassframe.cpp: compassframe.moc
$(srcdir)/datetimemgr.cpp: datetimemgr.moc
$(srcdir)/editor.cpp: editor.moc
$(srcdir)/experiencelog.cpp: experiencelog.moc
$(srcdir)/filteredspawnlog.cpp: filteredspawnlog.moc
$(srcdir)/filtermgr.cpp: filtermgr.moc
$(srcdir)/filternotifications.cpp: filternotifications.moc
$(srcdir)/group.cpp: group.moc
$(srcdir)/guild.cpp: guild.moc
$(srcdir)/guildlist.cpp: guildlist.moc
$(srcdir)/guildshell.cpp: guildshell.moc
$(srcdir)/interface.cpp: interface.moc
$(srcdir)/logger.cpp: logger.moc
$(srcdir)/map.cpp: map.moc
$(srcdir)/mapicon.cpp: mapicon.moc
$(srcdir)/messagefilter.cpp: messagefilter.moc
$(srcdir)/messagefilterdialog.cpp: messagefilterdialog.moc
$(srcdir)/messages.cpp: messages.moc
$(srcdir)/messageshell.cpp: messageshell.moc
$(srcdir)/messagewindow.cpp: messagewindow.moc
$(srcdir)/netdiag.cpp: netdiag.moc
$(srcdir)/packet.cpp: packet.moc
$(srcdir)/packetinfo.cpp: packetinfo.moc
$(srcdir)/packetlog.cpp: packetlog.moc
$(srcdir)/packetstream.cpp: packetstream.moc
$(srcdir)/player.cpp: player.moc
$(srcdir)/seqlistview.cpp: seqlistview.moc
$(srcdir)/seqwindow.cpp: seqwindow.moc
$(srcdir)/skilllist.cpp: skilllist.moc
$(srcdir)/spawnlist.cpp: spawnlist.moc
$(srcdir)/spawnlist2.cpp: spawnlist2.moc
$(srcdir)/spawnlistcommon.cpp: spawnlistcommon.moc
$(srcdir)/spawnlog.cpp: spawnlog.moc
$(srcdir)/spawnmonitor.cpp: spawnmonitor.moc
$(srcdir)/spawnpointlist.cpp: spawnpointlist.moc
$(srcdir)/spawnshell.cpp: spawnshell.moc
$(srcdir)/spelllist.cpp: spelllist.moc
$(srcdir)/spellshell.cpp: spellshell.moc
$(srcdir)/statlist.cpp: statlist.moc
$(srcdir)/terminal.cpp: terminal.moc
$(srcdir)/xmlpreferences.cpp: xmlpreferences.moc
$(srcdir)/zonemgr.cpp: zonemgr.moc
$(srcdir)/packetinfo.cpp: s_everquest.h

showeq_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(MEMORY_LIBS) \
$(PROFILE_LIBS) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

TEST_PROGS = sortitem

if CGI
if HAVE_GD
GD_CGI_PROGS = drawmap.cgi
endif

CGI_PROGS = $(GD_CGI_PROGS) listspawn.cgi showspawn.cgi
endif

noinst_PROGRAMS = $(TEST_PROGS) $(CGI_PROGS)

 listspawn_cgi_SOURCES = listspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_listspawn_cgi_SOURCES = 
listspawn_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

showspawn_cgi_SOURCES = showspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_showspawn_cgi_SOURCES =
showspawn_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

drawmap_cgi_SOURCES = drawmap.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_drawmap_cgi_SOURCES = 
drawmap_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) -lgd $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

sortitem_SOURCES = sortitem.cpp util.cpp diagnosticmessageslight.cpp 
nodist_sortitem_SOURCES = 
sortitem_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

EXTRA_DIST = h2info.pl

noinst_HEADERS = classes.h compass.h everquest.h interface.h main.h map.h filter.h vpacket.h editor.h packet.h packetcapture.h packetcommon.h packetformat.h packetstream.h packetfragment.h packetinfo.h races.h skills.h spells.h util.h experiencelog.h combatlog.h spawn.h spawnshell.h spawnlist.h spellshell.h spelllist.h languages.h weapons.h weapons1.h weapons27.h weapons28.h weapons29.h weapons2a.h weapons2b.h weapons2c.h decode.h cgiconv.h skilllist.h statlist.h deity.h player.h crctab.h filtermgr.h point.h pointarray.h mapcore.h category.h compassframe.h group.h guild.h fixpt.h netdiag.h zones.h logger.h xmlconv.h xmlpreferences.h seqwindow.h seqlistview.h zonemgr.h spawnmonitor.h spawnpointlist.h typenames.h spawnlistcommon.h spawnlist2.h datetimemgr.h spawnlog.h packetlog.h datalocationmgr.h eqstr.h messages.h messagefilter.h messagewindow.h messageshell.h terminal.h filteredspawnlog.h messagefilterdialog.h diagnosticmessages.h mapicon.h mapicondialog.ui mapicondialog.ui.h filternotifications.h netstream.h guildshell.h guildlist.h bazaarlog.h message.h s_everquest.h staticspells.h 

CLEANFILES = $(nodist_showeq_SOURCES)

MAINTAINERCLEANFILES = Makefile.in s_everquest.h

map.o: map.cpp map.h ui_mapicondialog.h

.h.moc:
	$(MOC) $< -o $@

m_%.cpp: %.h
	$(MOC) $< -o $@

s_%.h: %.h
	$(srcdir)/h2info.pl $< $@

ui_%.h: %.ui
	$(UIC) -nounload $< > $@

ui_%.cpp: %.ui ui_%.h
	$(UIC) -nounload -impl ui_$*.h $< > $@

if CGI
install-exec-local: $(CGI_PROGS)
	$(mkinstalldirs) $(CGIDIR)
	@list='$(CGI_PROGS)'; for p in $$list; do \
	if test -f $$p; then \
		echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(CGIDIR)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
		$(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(CGIDIR)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
		else :; fi; \
	done
endif


