AUTOMAKE_OPTIONS = dist-bzip2

SUBDIRS = src conf

EXTRA_DIST = BUGS ChangeLog FAQ INSTALL.newbies Makefile.dist README.libEQ ROADMAP showeq.1 TODO

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure Makefile.in \
ltconfig libqt2.dbg libqt3.dbg Makefile.dist.dbg configure.dbg config.log \
ltmain.sh stamp-h.in stamp-h

CLEANFILES = configure.dbg libqt2.dbg libqt3.dbg

dist-hook:
	mkdir -p $(distdir)/doc
	cp -p $(srcdir)/doc/*.txt $(distdir)/doc
	cp -p $(srcdir)/doc/*.doc $(distdir)/doc
	cp -p $(srcdir)/doc/*.html $(distdir)/doc
