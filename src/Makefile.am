INCLUDES=$(QT_INCLUDES) $(X_INCLUDES) $(DB3_INCLUDES) -DPKGDATADIR=\"$(pkgdatadir)/\" -I/usr/include/pcap $(USER_INCLUDES)

QTLIB=-lqt-mt

if USEDB3
DB3_SRCS=db3conv.cpp
DB3_LIBS=$(DB3_LDFLAGS) -ldb_cxx
endif

ITEMDB_SRCS=itemdb.cpp m_itemdb.cpp gdbmconv.cpp $(DB3_SRCS)
bin_PROGRAMS=showeq showeqitemdbtool
showeq_SOURCES=main.cpp spawn.cpp spawnshell.cpp spawnlist.cpp spellshell.cpp spelllist.cpp vpacket.cpp editor.cpp filter.cpp m_spawnshell.cpp m_spawnlist.cpp m_spellshell.cpp m_spelllist.cpp m_editor.cpp packetfragment.cpp packetstream.cpp m_packetstream.cpp packetinfo.cpp m_packetinfo.cpp packet.cpp m_packet.cpp packetcapture.cpp packetformat.cpp interface.cpp m_interface.cpp compass.cpp m_compass.cpp map.cpp m_map.cpp util.cpp experiencelog.cpp m_experiencelog.cpp combatlog.cpp m_combatlog.cpp player.cpp m_player.cpp skilllist.cpp m_skilllist.cpp statlist.cpp m_statlist.cpp filtermgr.cpp m_filtermgr.cpp mapcore.cpp category.cpp m_category.cpp compassframe.cpp m_compassframe.cpp group.cpp m_group.cpp guild.cpp m_guild.cpp netdiag.cpp m_netdiag.cpp logger.cpp m_logger.cpp xmlconv.cpp xmlpreferences.cpp m_xmlpreferences.cpp seqwindow.cpp m_seqwindow.cpp seqlistview.cpp m_seqlistview.cpp zonemgr.cpp m_zonemgr.cpp spawnmonitor.cpp m_spawnmonitor.cpp spawnpointlist.cpp m_spawnpointlist.cpp spawnlistcommon.cpp m_spawnlistcommon.cpp spawnlist2.cpp m_spawnlist2.cpp spells.cpp datetimemgr.cpp m_datetimemgr.cpp m_spawnlog.cpp spawnlog.cpp packetlog.cpp m_packetlog.cpp datalocationmgr.cpp eqstr.cpp messages.cpp m_messages.cpp message.cpp messagefilter.cpp m_messagefilter.cpp messagewindow.cpp m_messagewindow.cpp messageshell.cpp m_messageshell.cpp terminal.cpp m_terminal.cpp filteredspawnlog.cpp m_filteredspawnlog.cpp messagefilterdialog.cpp m_messagefilterdialog.cpp diagnosticmessages.cpp mapicon.cpp m_mapicon.cpp ui_mapicondialog.cpp m_ui_mapicondialog.cpp filternotifications.cpp m_filternotifications.cpp netstream.cpp guildshell.cpp m_guildshell.cpp guildlist.cpp m_guildlist.cpp $(ITEMDB_SRCS)

showeq_LDADD=$(QT_LDFLAGS) $(QTLIB) $(DB3_LIBS) $(LIBPTHREAD) $(MEMORY_LIBS) \
$(PROFILE_LIBS) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

TEST_PROGS=sortitem

if CGI
if HAVE_GD
GD_CGI_PROGS=drawmap.cgi
endif

CGI_PROGS=listitem.cgi showitem.cgi $(GD_CGI_PROGS) listspawn.cgi showspawn.cgi
endif

noinst_PROGRAMS=$(TEST_PROGS) $(CGI_PROGS)

listitem_cgi_SOURCES=listitem.cpp util.cpp diagnosticmessageslight.cpp  $(ITEMDB_SRCS) cgiconv.cpp
listitem_cgi_LDADD=$(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
showitem_cgi_SOURCES=showitem.cpp util.cpp diagnosticmessageslight.cpp  $(ITEMDB_SRCS) cgiconv.cpp
showitem_cgi_LDADD=$(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
listspawn_cgi_SOURCES=listspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
listspawn_cgi_LDADD=$(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
showspawn_cgi_SOURCES=showspawn.cpp spawn.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
showspawn_cgi_LDADD=$(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
drawmap_cgi_SOURCES=drawmap.cpp util.cpp diagnosticmessageslight.cpp cgiconv.cpp
drawmap_cgi_LDADD=$(QT_LDFLAGS) $(QTLIB) -lgd $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
sortitem_SOURCES=sortitem.cpp util.cpp diagnosticmessageslight.cpp 
sortitem_LDADD=$(QT_LDFLAGS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)
showeqitemdbtool_SOURCES=itemdbtool.cpp util.cpp diagnosticmessageslight.cpp datalocationmgr.cpp $(ITEMDB_SRCS)
showeqitemdbtool_LDADD=$(QT_LDFLAGS) $(DB3_LIBS) $(QTLIB) $(LIBPTHREAD) $(SHOWEQ_RPATH) $(USER_LDFLAGS)

EXTRA_DIST=classes.h compass.h everquest.h interface.h main.h map.h filter.h vpacket.h editor.h packet.h packetcapture.h packetcommon.h packetformat.h packetstream.h packetfragment.h packetinfo.h races.h skills.h spells.h util.h experiencelog.h combatlog.h spawn.h spawnshell.h spawnlist.h spellshell.h spelllist.h languages.h weapons.h weapons27.h decode.h opcodes.h itemdb.h gdbmconv.h cgiconv.h skilllist.h statlist.h db3conv.h dbcommon.h deity.h player.h crctab.h filtermgr.h point.h mapcore.h category.h compassframe.h group.h guild.h fixpt.h weapons28.h weapons2b.h netdiag.h zones.h logger.h xmlconv.h xmlpreferences.h seqwindow.h seqlistview.h zonemgr.h spawnmonitor.h spawnpointlist.h typenames.h spawnlistcommon.h spawnlist2.h datetimemgr.h spawnlog.h packetlog.h datalocationmgr.h eqstr.h messages.h messagefilter.h messagewindow.h messageshell.h terminal.h filteredspawnlog.h messagefilterdialog.h diagnosticmessages.h mapicon.h mapicondialog.ui mapicondialog.ui.h filternotifications.h netstream.h guildshell.h guildlist.h

BUILT_SOURCES=ui_mapicondialog.h ui_mapicondialog.cpp m_packet.cpp m_packetstream.cpp s_everquest.h m_packetinfo.cpp m_spawnshell.cpp m_spawnlist.cpp m_spellshell.cpp m_spelllist.cpp m_editor.cpp m_interface.cpp m_compass.cpp m_map.cpp m_experiencelog.cpp m_combatlog.cpp m_player.cpp m_skilllist.cpp m_statlist.cpp m_filtermgr.cpp m_category.cpp m_compassframe.cpp m_group.cpp m_guild.cpp m_netdiag.cpp m_logger.cpp m_xmlpreferences.cpp m_seqwindow.cpp m_seqlistview.cpp m_zonemgr.cpp m_spawnmonitor.cpp m_spawnpointlist.cpp m_spawnlistcommon.cpp m_spawnlist2.cpp m_itemdb.cpp m_datetimemgr.cpp m_spawnlog.cpp m_packetlog.cpp m_messages.cpp m_messagefilter.cpp m_messagewindow.cpp m_messageshell.cpp m_terminal.cpp m_filteredspawnlog.cpp m_messagefilterdialog.cpp m_mapicon.cpp m_ui_mapicondialog.cpp m_filternotifications.cpp m_guildshell.cpp m_guildlist.cpp

CLEANFILES=$(BUILT_SOURCES)

MAINTAINERCLEANFILES= Makefile.in

map.o: map.cpp map.h ui_mapicondialog.h
packetinfo.o: packetinfo.cpp packetinfo.h s_everquest.h

m_%.cpp: %.h
	$(MOC) -nw $< -o $@

s_%.h: %.h
	./h2info.pl $< $@

ui_%.h: %.ui
	$(UIC) $< > $@

ui_%.cpp: %.ui ui_%.h
	$(UIC) -impl ui_$*.h $< > $@

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
