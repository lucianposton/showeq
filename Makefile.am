ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = 1.9 dist-bzip2

SUBDIRS = src conf

include doxygen.am

EXTRA_DIST = BUGS ChangeLog FAQ INSTALL.newbies Makefile.dist README.libEQ \
ROADMAP showeq.1 TODO showeq.kdevelop Doxyfile depcomp $(DX_CONFIG)

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
Makefile.in ltconfig libqt2.dbg libqt3.dbg Makefile.dist.dbg \
configure.dbg config.log ltmain.sh stamp-h.in stamp-h qt.tag depcomp

@DX_DOCDIR@/@PACKAGE@.tag: qt.tag

qt.tag: $(QT_DOCS)/index.html $(QT_DOCS)/qobject.html
	doxytag -t qt.tag $(QT_DOCS)

doxygen-installdox: doxygen-doc dox/html/installdox
	if [ -x "@DX_DOCDIR@/html/installdox" ]; then \
          cd @DX_DOCDIR@/html; \
          ./installdox -l qt.tag@http://doc.trolltech.com/3.3/ -q; \
        fi

MOSTLYCLEANFILES = $(DX_CLEANFILES)

CLEANFILES = configure.dbg libqt2.dbg libqt3.dbg

dist-hook:
	mkdir -p $(distdir)/doc
	cp -p $(srcdir)/doc/*.txt $(distdir)/doc
	cp -p $(srcdir)/doc/*.doc $(distdir)/doc
	cp -p $(srcdir)/doc/*.html $(distdir)/doc
