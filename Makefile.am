SUBDIRS=maps src conf

EXTRA_DIST=CHANGES BUGS showeq.1

MAINTAINERCLEANFILES= aclocal.m4 config.guess config.sub configure Makefile.in \
ltconfig libqt2.dbg libqt3.dbg Makefile.dist.dbg configure.dbg config.log \
ltmain.sh stamp-h.in stamp-h

CLEANFILES=configure.dbg libqt2.dbg libqt3.dbg

dist-hook:
	mkdir -p $(distdir)/doc
	cp -p $(srcdir)/doc/*.txt $(distdir)/doc
