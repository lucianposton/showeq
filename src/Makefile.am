INCLUDES = $(QT_INCLUDES) $(X_INCLUDES) $(DB3_INCLUDES) -DPKGDATADIR=\"$(pkgdatadir)/\" -I/usr/include/pcap $(USER_INCLUDES)

QTLIB = -lqt-mt

if USEDB3
DB3_SRCS = db3conv.cpp
DB3_LIBS = $(DB3_LDFLAGS) -ldb_cxx
endif

ITEMDB_SRCS = itemdb.cpp gdbmconv.cpp $(DB3_SRCS)
nodist_ITEMDB_SRCS = m_itemdb.cpp 

bin_PROGRAMS = showeq showeqitemdbtool

showeq_SOURCES = main.cpp spawn.cpp spawnshell.cpp spawnlist.cpp spellshell.cpp spelllist.cpp vpacket.cpp editor.cpp filter.cpp packetfragment.cpp packetstream.cpp packetinfo.cpp packet.cpp packetcapture.cpp packetformat.cpp interface.cpp compass.cpp map.cpp util.cpp experiencelog.cpp combatlog.cpp player.cpp skilllist.cpp statlist.cpp filtermgr.cpp mapcore.cpp category.cpp compassframe.cpp group.cpp guild.cpp netdiag.cpp logger.cpp xmlconv.cpp xmlpreferences.cpp seqwindow.cpp seqlistview.cpp zonemgr.cpp spawnmonitor.cpp spawnpointlist.cpp spawnlistcommon.cpp spawnlist2.cpp spells.cpp datetimemgr.cpp spawnlog.cpp packetlog.cpp datalocationmgr.cpp eqstr.cpp messages.cpp message.cpp messagefilter.cpp messagewindow.cpp messageshell.cpp terminal.cpp filteredspawnlog.cpp messagefilterdialog.cpp diagnosticmessages.cpp mapicon.cpp filternotifications.cpp netstream.cpp guildshell.cpp guildlist.cpp bazaarlog.cpp $(ITEMDB_SRCS)

nodist_showeq_SOURCES = m_spawnshell.cpp m_spawnlist.cpp m_spellshell.cpp m_spelllist.cpp m_editor.cpp m_packetstream.cpp m_packetinfo.cpp m_packet.cpp m_interface.cpp m_compass.cpp m_map.cpp m_experiencelog.cpp m_combatlog.cpp m_player.cpp m_skilllist.cpp m_statlist.cpp m_filtermgr.cpp m_category.cpp m_compassframe.cpp m_group.cpp m_guild.cpp m_netdiag.cpp m_logger.cpp m_xmlpreferences.cpp m_seqwindow.cpp m_seqlistview.cpp m_zonemgr.cpp m_spawnmonitor.cpp m_spawnpointlist.cpp m_spawnlistcommon.cpp m_spawnlist2.cpp m_datetimemgr.cpp m_spawnlog.cpp m_packetlog.cpp m_messages.cpp m_messagefilter.cpp m_messagewindow.cpp m_messageshell.cpp m_terminal.cpp m_filteredspawnlog.cpp m_messagefilterdialog.cpp m_mapicon.cpp m_filternotifications.cpp m_guildshell.cpp m_guildlist.cpp m_bazaarlog.cpp ui_mapicondialog.h ui_mapicondialog.cpp m_ui_mapicondialog.cpp $(nodist_ITEMDB_SRCS)

showeq_LDADD = $(QT_LDFLAGS) $(QTLIB) $(DB3_LIBS) $(LIBPTHREAD) $(MEMORY_LIBS) \
$(PROFILE_LIBS) $(SHOWEQ_RPATH) $(USER_LDFLAGS)


showeqitemdbtool_SOURCES = itemdbtool.cpp util.cpp diagnosticmessageslight.cpp datalocationmgr.cpp $(ITEMDB_SRCS)
nodist_showeqitemdbtool_SOURCES = $(nodist_ITEMDB_SRCS)
showeqitemdbtool_LDADD = $(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

TEST_PROGS = sortitem

if CGI
if HAVE_GD
GD_CGI_PROGS = drawmap.cgi
endif

CGI_PROGS = listitem.cgi showitem.cgi $(GD_CGI_PROGS) listspawn.cgi showspawn.cgi
endif

noinst_PROGRAMS = $(TEST_PROGS) $(CGI_PROGS)

listitem_cgi_SOURCES = listitem.cpp util.cpp diagnosticmessageslight.cpp  $(ITEMDB_SRCS) cgiconv.cpp
nodist_listitem_cgi_SOURCES = $(nodist_ITEMDB_SRCS)
listitem_cgi_LDADD = $(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

showitem_cgi_SOURCES = showitem.cpp util.cpp diagnosticmessageslight.cpp  $(ITEMDB_SRCS) cgiconv.cpp
nodist_showitem_cgi_SOURCES = $(nodist_ITEMDB_SRCS)
showitem_cgi_LDADD = $(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

listspawn_cgi_SOURCES = listspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_listspawn_cgi_SOURCES = $(nodist_ITEMDB_SRCS)
listspawn_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

showspawn_cgi_SOURCES = showspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_showspawn_cgi_SOURCES = $(nodist_ITEMDB_SRCS)
showspawn_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

drawmap_cgi_SOURCES = drawmap.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
nodist_drawmap_cgi_SOURCES = $(nodist_ITEMDB_SRCS)
drawmap_cgi_LDADD = $(QT_LDFLAGS) $(QTLIB) -lgd $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

sortitem_SOURCES = sortitem.cpp util.cpp diagnosticmessageslight.cpp 
nodist_sortitem_SOURCES = 
sortitem_LDADD = $(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

EXTRA_DIST = classes.h compass.h everquest.h interface.h main.h map.h filter.h vpacket.h editor.h packet.h packetcapture.h packetcommon.h packetformat.h packetstream.h packetfragment.h packetinfo.h races.h skills.h spells.h util.h experiencelog.h combatlog.h spawn.h spawnshell.h spawnlist.h spellshell.h spelllist.h languages.h weapons.h weapons1.h weapons27.h weapons28.h weapons2b.h decode.h itemdb.h gdbmconv.h cgiconv.h skilllist.h statlist.h db3conv.h dbcommon.h deity.h player.h crctab.h filtermgr.h point.h pointarray.h mapcore.h category.h compassframe.h group.h guild.h fixpt.h netdiag.h zones.h logger.h xmlconv.h xmlpreferences.h seqwindow.h seqlistview.h zonemgr.h spawnmonitor.h spawnpointlist.h typenames.h spawnlistcommon.h spawnlist2.h datetimemgr.h spawnlog.h packetlog.h datalocationmgr.h eqstr.h messages.h messagefilter.h messagewindow.h messageshell.h terminal.h filteredspawnlog.h messagefilterdialog.h diagnosticmessages.h mapicon.h mapicondialog.ui mapicondialog.ui.h filternotifications.h netstream.h guildshell.h guildlist.h bazaarlog.h message.h s_everquest.h staticspells.h h2info.pl

CLEANFILES = $(nodist_showeq_SOURCES)

MAINTAINERCLEANFILES = Makefile.in s_everquest.h

map.o: map.cpp map.h ui_mapicondialog.h
packetinfo.o: s_everquest.h

m_%.cpp: %.h
	$(MOC) -nw $< -o $@

s_%.h: %.h
	./h2info.pl $< $@

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
