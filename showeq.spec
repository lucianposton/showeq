Summary: ShowEQ - Shows EQ Information
Name: showeq
Version: 2.3
Release: 1
Copyright: GPL
Group: Applications/Editors
Source0: http://www.hackersquest.gomp.ch/Files/showeq-%{version}.tar.gz
Buildroot: /var/tmp/showeq-buildroot

%description
ShowEQ shows a realtime tactical map of the zone you are in on EQ.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
make 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
make clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc src/README src/CHANGES src/BUGS
/usr/bin/showeq
/usr/share/showeq

%changelog
* Fri Feb 04 2000 Slicer (Slicer / #showeq / irc.giblets.com)
- Initial Release
