Summary: ShowEQ A tool for helping with Everquest
Name: showeq
Version: 4.0
Release: 3
License: GPL
Group: Applications/Games
Source: showeq.tar.gz
Buildroot: %{_tmppath}/%{name}-tmp
BuildPrereq: libpcap-devel,gcc3,gcc3-c++
Prereq: /sbin/chkconfig
Requires: libpcap

%description
ShowEQ shows a realtime tactical map of the zone you are in on EQ.  To use this specfile you will need to get the source from CVS, then rename the showeq directory to showeq-4.0 and tar -cvf showeq.tar showeq-4.0  Then gzip the showeq.tar file and move it to /usr/src/redhat/SOURCES and cp this showeq.spec file into /usr/src/redhat/SPECS and then cd /usr/src/redhat/SPECS and issue the command rpm -ba showeq.spec  At that point so long as all your libraries are correct you will build an RPM of ShowEQ that you can use to install showEQ.

%prep
%setup -q
export CXX=g++3
export CC=gcc3
export QTDIR=/usr/lib/qt-2.3.2
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
make -f Makefile.dist

%build
export CXX=g++3
export CC=gcc3
export QTDIR=/usr/lib/qt-2.3.2
export LD_LIBRARY_PATH=$QTDIR/bin:$LD_LIBRARY_PATH
%configure --bindir=/usr/local/bin --datadir=/usr/local/share/
make

%install

%{__make} install \
        DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post

%postun

%preun

%files
%defattr(-,root,root)
%dir /usr/local/bin/*
%dir /usr/local/share/showeq/*

%changelog
* Wed Jan 02 2002 KennySP (KennySP@sourceforge.net)
- Updated for the new ShowEQ 4.0 release.  Can be considered  complete rewrite as the only thing in common with the old showeq.spec is in this changelog.
* Fri Feb 04 2000 Slicer (Slicer / #showeq / irc.giblets.com)
- Initial Release
