##   -*- autoconf -*-

dnl    This file is part of the KDE libraries/packages
dnl    Copyright (C) 1997 Janos Farkas (chexum@shadow.banki.hu)
dnl              (C) 1997,98,99 Stephan Kulow (coolo@kde.org)

dnl    This file is free software; you can redistribute it and/or
dnl    modify it under the terms of the GNU Library General Public
dnl    License as published by the Free Software Foundation; either
dnl    version 2 of the License, or (at your option) any later version.

dnl    This library is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl    Library General Public License for more details.

dnl    You should have received a copy of the GNU Library General Public License
dnl    along with this library; see the file COPYING.LIB.  If not, write to
dnl    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl    Boston, MA 02111-1307, USA.

dnl IMPORTANT NOTE:
dnl Please do not modify this file unless you expect your modifications to be
dnl carried into every other module in the repository. If you decide that you
dnl really want to modify it, contact coolo@kde.org mentioning that you have
dnl and that the modified file should be committed to every module.
dnl
dnl Single-module modifications are best placed in configure.in for kdelibs
dnl and kdebase or configure.in.in if present.


dnl ------------------------------------------------------------------------
dnl Find a file (or one of more files in a list of dirs)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

AC_DEFUN([MOC_ERROR_MESSAGE],
[

HEADER="No working Qt meta object compiler (moc) found!
"

FOOTER="

As a last resort, it may be possible to eliminate this error by typing:

	export MOC=\`updatedb && locate moc | grep bin/moc\`  (with the \`s)"

if [[ -e "$ac_cv_path_moc" ]]; then
  if ! [[ -x "$ac_cv_path_moc" ]]; then
    AC_MSG_ERROR([$HEADER
Configure found and tried to use '$ac_cv_path_moc', but failed...

The problem appears to be the lack of an executable flag for the file...
Try changing the permissions of '$ac_cv_path_moc' by issuing the following:

	chmod 774 $ac_cv_path_moc    (NOTE: > 774 may pose a security risk...)])
  else
    AC_MSG_ERROR([$HEADER
Configure found and tried to use '$ac_cv_path_moc', but failed...

If configure shouldn't have tried '$ac_cv_path_moc', please set the environment 
variable MOC to point to the location of your prefered moc binary and run
configure over. $FOOTER])
  fi
else
  AC_MSG_ERROR([$HEADER
Configure was unable to locate a moc binary anywhere on your system!

If you have a working moc binary, please set the environment variable MOC
to point to the location of your moc binary and run configure over. $FOOTER])
fi
])

dnl ------------------------------------------------------------------------
dnl Find the meta object compiler in the PATH, in $QTDIR/bin, and some
dnl more usual places
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_PATH_QT_MOC],
[
  if [[ -n "$ac_qt_includes" ]]; then
   
     AC_MSG_CHECKING([for Qt MOC]);
     AC_FIND_FILE(moc, [ $ac_qt_bindir              \
                         $QTDIR/bin                 \
                         $QTDIR/src/moc             \
                         /usr/local/qt3/bin	    \
                         /usr/local/qt/bin          \
                         /usr/local/qt2/bin         \
	                 /usr/local/qt-2.3.2/lib    \
                         /usr/local/qt*/bin         \
	                 /usr/lib/qtgcc3-2.3.2/lib  \
	                 /usr/lib/qtgcc3-*/lib  \
	                 /opt/qt-gcc3-2.3.2/lib	    \
	                 /opt/qt-gcc3-*/lib	    \
	                 /opt/qt-2.3.2/lib          \
                         /usr/bin                   \
                         /usr/X11R6/bin             \
                         /usr/X11R6/bin/qt3	    \
                         /usr/X11R6/bin/qt          \
                         /usr/X11R6/bin/qt2         \
                         /usr/X11R6/bin/qt*         \
                         /usr/X11R6/bin/qt2/bin     \
                         /usr/X11R6/bin/qt3/bin     \
                         /usr/X11R6/bin/qt/bin      \
                         /usr/X11R6/bin/qt*/bin     \
                         /usr/X11R6/bin/X11/qt3     \
                         /usr/X11R6/bin/X11/qt      \
                         /usr/X11R6/bin/X11/qt2     \
                         /usr/X11R6/bin/X11/qt*     \
                         /usr/X11R6/bin/X11/qt3/bin \
                         /usr/X11R6/bin/X11/qt/bin  \
                         /usr/X11R6/bin/X11/qt2/bin \
                         /usr/X11R6/bin/X11/qt*/bin \
                         /usr/lib/qt3/bin	    \
                         /usr/lib/qt/bin            \
                         /usr/lib/qt2/bin           \
                         /usr/lib/qt*/bin           \
                         /usr/src/qt-*/bin            ],

             MOC)

     MOC="$MOC/moc"
     ac_cv_path_moc="$MOC"

     if [[ -n "$ac_cv_path_moc" ]]; then
       if ! [[ -e "$ac_cv_path_moc" ]]; then
         MOC_ERROR_MESSAGE
       fi

       if ! [[ -x "$ac_cv_path_moc" ]]; then
         MOC_ERROR_MESSAGE
       fi

       output=`eval "$ac_cv_path_moc --help  2>&1 | sed -e '1q' | grep Qt"`

       echo "configure:__oline__: tried to call $ac_cv_path_moc --help 2>&1 | sed -e '1q' | grep Qt" >&AC_FD_CC
       echo "configure:__oline__: moc output: $output" >&AC_FD_CC

       if [[ -z "$output" ]]; then
         MOC_ERROR_MESSAGE
       fi
     fi

     AC_SUBST(MOC)
     AC_MSG_RESULT(yes)
  fi
])

AC_DEFUN([UIC_ERROR_MESSAGE],
[

HEADER="No working Qt user interface compiler (uic) found!
"

FOOTER="

As a last resort, it may be possible to eliminate this error by typing:

	export UIC=\`updatedb && locate uic | grep bin/uic\`  (with the \`s)"

if [[ -e "$ac_cv_path_uic" ]]; then
  if ! [[ -x "$ac_cv_path_uic" ]]; then
    AC_MSG_ERROR([$HEADER
Configure found and tried to use '$ac_cv_path_uic', but failed...

The problem appears to be the lack of an executable flag for the file...
Try changing the permissions of '$ac_cv_path_uic' by issuing the following:

	chmod 774 $ac_cv_path_uic    (NOTE: > 774 may pose a security risk...)])
  else
    AC_MSG_ERROR([$HEADER
Configure found and tried to use '$ac_cv_path_uic', but failed...

If configure shouldn't have tried '$ac_cv_path_uic', please set the environment 
variable UIC to point to the location of your prefered uic binary and run
configure over. $FOOTER])
  fi
else
  AC_MSG_ERROR([$HEADER
Configure was unable to locate a uic binary anywhere on your system!

If you have a working uic binary, please set the environment variable UIC
to point to the location of your uic binary and run configure over. $FOOTER])
fi
])


dnl ------------------------------------------------------------------------
dnl Find the meta object compiler in the PATH, in $QTDIR/bin, and some
dnl more usual places
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_PATH_QT_UIC],
[
  if [[ -n "$ac_qt_includes" ]]; then
   
     AC_MSG_CHECKING([for Qt UIC]);
     AC_FIND_FILE(uic, [ $ac_qt_bindir              \
                         $QTDIR/bin                 \
                         $QTDIR/src/uic             \
                         /usr/local/qt3/bin	    \
                         /usr/local/qt/bin          \
                         /usr/local/qt2/bin         \
	                 /usr/local/qt-2.3.2/lib    \
                         /usr/local/qt*/bin         \
	                 /usr/lib/qtgcc3-2.3.2/lib  \
	                 /usr/lib/qtgcc3-*/lib  \
	                 /opt/qt-gcc3-2.3.2/lib	    \
	                 /opt/qt-gcc3-*/lib	    \
	                 /opt/qt-2.3.2/lib          \
                         /usr/bin                   \
                         /usr/X11R6/bin             \
                         /usr/X11R6/bin/qt3	    \
                         /usr/X11R6/bin/qt          \
                         /usr/X11R6/bin/qt2         \
                         /usr/X11R6/bin/qt*         \
                         /usr/X11R6/bin/qt2/bin     \
                         /usr/X11R6/bin/qt3/bin     \
                         /usr/X11R6/bin/qt/bin      \
                         /usr/X11R6/bin/qt*/bin     \
                         /usr/X11R6/bin/X11/qt3     \
                         /usr/X11R6/bin/X11/qt      \
                         /usr/X11R6/bin/X11/qt2     \
                         /usr/X11R6/bin/X11/qt*     \
                         /usr/X11R6/bin/X11/qt3/bin \
                         /usr/X11R6/bin/X11/qt/bin  \
                         /usr/X11R6/bin/X11/qt2/bin \
                         /usr/X11R6/bin/X11/qt*/bin \
                         /usr/lib/qt3/bin	    \
                         /usr/lib/qt/bin            \
                         /usr/lib/qt2/bin           \
                         /usr/lib/qt*/bin           \
                         /usr/src/qt-*/bin            ],

             UIC)

     UIC="$UIC/uic"
     ac_cv_path_uic="$UIC"

     if [[ -n "$ac_cv_path_uic" ]]; then
       if ! [[ -e "$ac_cv_path_uic" ]]; then
         UIC_ERROR_MESSAGE
       fi

       if ! [[ -x "$ac_cv_path_uic" ]]; then
         UIC_ERROR_MESSAGE
       fi

       output=`eval "$ac_cv_path_uic --help  2>&1 | grep -i 'Qt user interface'"`

       echo "configure:__oline__: tried to call $ac_cv_path_uic --help 2>&1 | sed -e '1q' | grep Qt" >&AC_FD_CC
       echo "configure:__oline__: uic output: $output" >&AC_FD_CC

       if [[ -z "$output" ]]; then
         UIC_ERROR_MESSAGE
       fi
     fi

     AC_SUBST(UIC)
     AC_MSG_RESULT(yes)
  fi
])

AC_DEFUN([KDE_MISC_TESTS],
[
   AC_LANG_C
   dnl Checks for libraries.
   AC_CHECK_LIB(compat, main, [LIBCOMPAT="-lcompat"]) dnl for FreeBSD
   AC_SUBST(LIBCOMPAT)
   kde_have_crypt=
   AC_CHECK_LIB(crypt, crypt, [LIBCRYPT="-lcrypt"; kde_have_crypt=yes],
      AC_CHECK_LIB(c, crypt, [kde_have_crypt=yes], [
        AC_MSG_WARN([you have no crypt in either libcrypt or libc.
You should install libcrypt from another source or configure with PAM
support])
	kde_have_crypt=no
      ]))
   AC_SUBST(LIBCRYPT)
   if test $kde_have_crypt = yes; then
      AC_DEFINE_UNQUOTED(HAVE_CRYPT, 1, [Defines if your system has the crypt function])
   fi
   AC_CHECK_KSIZE_T
   AC_LANG_C
   AC_CHECK_LIB(dnet, dnet_ntoa, [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet"])
   if test $ac_cv_lib_dnet_dnet_ntoa = no; then
      AC_CHECK_LIB(dnet_stub, dnet_ntoa,
        [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet_stub"])
   fi
   AC_CHECK_FUNC(inet_ntoa)
   if test $ac_cv_func_inet_ntoa = no; then
     AC_CHECK_LIB(nsl, inet_ntoa, X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl")
   fi
   AC_CHECK_FUNC(connect)
   if test $ac_cv_func_connect = no; then
      AC_CHECK_LIB(socket, connect, X_EXTRA_LIBS="-lsocket $X_EXTRA_LIBS", ,
        $X_EXTRA_LIBS)
   fi

   AC_CHECK_FUNC(remove)
   if test $ac_cv_func_remove = no; then
      AC_CHECK_LIB(posix, remove, X_EXTRA_LIBS="$X_EXTRA_LIBS -lposix")
   fi

   # BSDI BSD/OS 2.1 needs -lipc for XOpenDisplay.
   AC_CHECK_FUNC(shmat)
   if test $ac_cv_func_shmat = no; then
     AC_CHECK_LIB(ipc, shmat, X_EXTRA_LIBS="$X_EXTRA_LIBS -lipc")
   fi

   LIBSOCKET="$X_EXTRA_LIBS"
   AC_SUBST(LIBSOCKET)
   AC_SUBST(X_EXTRA_LIBS)
   AC_CHECK_LIB(ucb, killpg, [LIBUCB="-lucb"]) dnl for Solaris2.4
   AC_SUBST(LIBUCB)

   case $host in  dnl this *is* LynxOS specific
   *-*-lynxos* )
        AC_MSG_CHECKING([LynxOS header file wrappers])
        [CFLAGS="$CFLAGS -D__NO_INCLUDE_WARN__"]
        AC_MSG_RESULT(disabled)
        AC_CHECK_LIB(bsd, gethostbyname, [LIBSOCKET="-lbsd"]) dnl for LynxOS
         ;;
    esac

   AC_REQUIRE([KDE_CHECK_LIBDL])
   AC_CHECK_BOOL
])

dnl ------------------------------------------------------------------------
dnl Find the header files and libraries for X-Windows. Extended the
dnl macro AC_PATH_X
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([K_PATH_X],
[
AC_REQUIRE([AC_PROG_CPP])
dnl AC_MSG_CHECKING(for X)
AC_LANG_SAVE
AC_LANG_C
AC_CACHE_VAL(ac_cv_have_x,
[# One or both of the vars are not set, and there is no cached value.
if test "{$x_includes+set}" = set || test "$x_includes" = NONE; then
   kde_x_includes=NO
else
   kde_x_includes=$x_includes
fi
if test "{$x_libraries+set}" = set || test "$x_libraries" = NONE; then
   kde_x_libraries=NO
else
   kde_x_libraries=$x_libraries
fi

# below we use the standard autoconf calls
ac_x_libraries=$kde_x_libraries
ac_x_includes=$kde_x_includes

AC_PATH_X

if test -z "$ac_x_includes"; then
ac_x_includes="."
fi
if test -z "$ac_x_libraries"; then
ac_x_libraries="/usr/lib"
fi
#from now on we use our own again

# when the user already gave --x-includes, we ignore
# what the standard autoconf macros told us.
if test "$kde_x_includes" = NO; then
  kde_x_includes=$ac_x_includes
fi

if test "$kde_x_includes" = NO; then
  AC_MSG_ERROR([Can't find X includes. Please check your installation and add the correct paths!])
fi

if test "$ac_x_libraries" = NO; then
  AC_MSG_ERROR([Can't find X libraries. Please check your installation and add the correct paths!])
fi

# Record where we found X for the cache.
ac_cv_have_x="have_x=yes \
         kde_x_includes=$kde_x_includes ac_x_libraries=$ac_x_libraries"
])dnl
eval "$ac_cv_have_x"

if test "$have_x" != yes; then
  AC_MSG_ERROR($have_x)
  no_x=yes
fi

if test -z "$kde_x_includes" || test "x$kde_x_includes" = xNONE; then
  X_INCLUDES=""
  x_includes="."; dnl better than nothing :-
 else
  x_includes=$kde_x_includes
  X_INCLUDES="-I$x_includes"
fi

if test -z "$ac_x_libraries" || test "x$ac_x_libraries" = xNONE; then
  X_LDFLAGS=""
  x_libraries="/usr/lib"; dnl better than nothing :-
 else
  x_libraries=$ac_x_libraries
  X_LDFLAGS="-L$x_libraries"
fi
all_includes="$X_INCLUDES"
all_libraries="$X_LDFLAGS"

AC_SUBST(X_INCLUDES)
AC_SUBST(X_LDFLAGS)
AC_SUBST(x_libraries)
AC_SUBST(x_includes)

# Check for libraries that X11R6 Xt/Xaw programs need.
ac_save_LDFLAGS="$LDFLAGS"
test -n "$x_libraries" && LDFLAGS="$LDFLAGS -L$x_libraries"
# SM needs ICE to (dynamically) link under SunOS 4.x (so we have to
# check for ICE first), but we must link in the order -lSM -lICE or
# we get undefined symbols.  So assume we have SM if we have ICE.
# These have to be linked with before -lX11, unlike the other
# libraries we check for below, so use a different variable.
#  --interran@uluru.Stanford.EDU, kb@cs.umb.edu.
AC_CHECK_LIB(ICE, IceConnectionNumber,
  [LIBSM="$X_PRELIBS -lSM"], , $X_EXTRA_LIBS)
AC_SUBST(LIBSM)
LDFLAGS="$ac_save_LDFLAGS"

AC_SUBST(X_PRE_LIBS)

LIB_X11='-lX11 $(LIBSOCKET)'
AC_SUBST(LIB_X11)

AC_MSG_CHECKING(for libXext)
AC_CACHE_VAL(kde_cv_have_libXext,
[
kde_ldflags_safe="$LDFLAGS"
kde_libs_safe="$LIBS"

LDFLAGS="$X_LDFLAGS $USER_LDFLAGS"
LIBS="-lXext -lX11 $LIBSOCKET"

AC_TRY_LINK([
#include <stdio.h>
],
[
printf("hello Xext\n");
],
kde_cv_have_libXext=yes,
kde_cv_have_libXext=no
   )

LDFLAGS=$kde_ldflags_safe
LIBS=$kde_libs_safe
 ])

AC_MSG_RESULT($kde_cv_have_libXext)

if test "kde_cv_have_libXext" = "no"; then
  AC_MSG_ERROR([We need a working libXext to proceed. Since configure
can't find it itself, we stop here assuming that make wouldn't find
them either.])
fi

AC_LANG_RESTORE
])
])

AC_DEFUN([PRINT_QT_PROGRAM],
[
cat > conftest.$ac_ext <<EOF
#define QT_THREAD_SUPPORT 1
#include "confdefs.h"
#include <qglobal.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qthread.h>
EOF

echo "#if ! (QT_VERSION > 230)" >> conftest.$ac_ext
cat >> conftest.$ac_ext <<EOF
#error 1
#endif

int main() {
    QStringList *t = new QStringList();
    QMutex mutex;
    return 0;
}
EOF
])

AC_DEFUN([CHECK_QT_DIRECT],
[
AC_MSG_CHECKING([if Qt compiles without flags])
AC_CACHE_VAL(cv_qt_direct,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
ac_LD_LIBRARY_PATH_safe=$LD_LIBRARY_PATH
ac_LIBRARY_PATH="$LIBRARY_PATH"
ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_includes"
LDFLAGS="$X_LDFLAGS"
LIBS="-lqt-mt -lXext -lX11 $LIBSOCKET"
LD_LIBRARY_PATH=
export LD_LIBRARY_PATH
LIBRARY_PATH=
export LIBRARY_PATH

PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  cv_qt_direct="yes"
else
  cv_qt_direct="no"
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
fi

rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

LD_LIBRARY_PATH="$ac_LD_LIBRARY_PATH_safe"
export LD_LIBRARY_PATH
LIBRARY_PATH="$ac_LIBRARY_PATH"
export LIBRARY_PATH
AC_LANG_RESTORE
])

if test "$cv_qt_direct" = "yes"; then
  AC_MSG_RESULT(yes)
  $1
else
  AC_MSG_RESULT(no)
  $2
fi
])

dnl ------------------------------------------------------------------------
dnl Try to find the Qt headers and libraries.
dnl $(QT_LDFLAGS) will be -Lqtliblocation (if needed)
dnl and $(QT_INCLUDES) will be -Iqthdrlocation (if needed)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_PATH_QT_1_3],
[
AC_REQUIRE([K_PATH_X])
LIBQT="-lqt-mt"

AC_REQUIRE([AC_FIND_PNG])
LIBQT="$LIBQT $LIBPNG"

AC_MSG_CHECKING([for Qt])

LIBQT="$LIBQT $X_PRE_LIBS -lXext -lX11 $LIBSOCKET"

ac_qt_includes=""
ac_qt_libraries=""
ac_qt_bindir=""
ac_qt_docs=""

qt_libraries=""
qt_includes=""
qt_docs=""

AC_ARG_WITH(qt-dir,
    [  --with-qt-dir=DIR       where the root of Qt is installed ],
    [  ac_qt_includes="$withval"/include
       ac_qt_libraries="$withval"/lib
       ac_qt_bindir="$withval"/bin
       ac_qt_docs="$withval"/doc/html
    ])

AC_ARG_WITH(qt-includes,
    [  --with-qt-includes=DIR  where the Qt includes are. ],
    [
       ac_qt_includes="$withval"
    ])

AC_ARG_WITH(qt-docs,
    [  --with-qt-docs=DIR  where the Qt docs are. ],
    [
       ac_qt_docs="$withval"
    ])

ac_qt_libs_given=no

AC_ARG_WITH(qt-libraries,
    [  --with-qt-libraries=DIR where the Qt library is installed. ],
    [  ac_qt_libraries="$withval"
       ac_qt_libs_given=yes
    ])

AC_ARG_WITH(qt-validation,
   [  --without-qt-validation    disables extra Qt checks... ],
   [  ac_validate_qt="yes" ]
   [  ac_validate_qt="no" ])

AC_CACHE_VAL(ac_cv_have_qt,
[
dnl  *******************************************
dnl  * Build a relatively large search path... *
dnl  *******************************************

qt_incdirs=" $QTDIR/include                     \
             $QTINC                             \
	     /usr/local/qt3/include		\
             /usr/local/qt/include              \
             /usr/local/qt*/include             \
	     /usr/qt/3/include			\
	     /usr/qt/*/include			\
	     /opt/qt-gcc3-*/include		\
	     /usr/include/qt3			\
             /usr/include/qt                    \
             /usr/include/qt*                   \
             /usr/include                       \
	     /usr/lib/qt3/include		\
             /usr/lib/qt/include                \
             /usr/lib/qt*/include               \
	     /usr/lib/qtgcc3-*/include		\
	     /usr/lib/qt3/include		\
             /usr/lib/qt/include                \
             /usr/lib/qt*/include               \
	     /usr/X11R6/include/X11/qt3		\
             /usr/X11R6/include/X11/qt          \
             /usr/X11R6/include/X11/qt*         \
	     /usr/X11R6/include/X11/qt3/include \
             /usr/X11R6/include/X11/qt/include  \
             /usr/X11R6/include/X11/qt*/include \
	     /usr/X11R6/include/qt3		\
             /usr/X11R6/include/qt              \
             /usr/X11R6/include/qt*             \
 	     /usr/X11R6/include/qt3/include	\
             /usr/X11R6/include/qt/include      \
             /usr/X11R6/include/qt*/include     \
             /usr/src/qt-*/include              \
             $x_includes			\
	     $qt_incdirs"

[[ "$ac_qt_includes" != "NO" ]]   &&   \
qt_incdirs="$ac_qt_includes $qt_incdirs"

qt_version_source="qglobal.h"


AC_FIND_FILE("qstyle.h", $qt_incdirs, qt_incdir1)
AC_FIND_FILE($qt_version_source, $qt_incdir1, qt_incdir)


dnl  *************************************************
dnl  * At this point, we extract the Qt version from *
dnl  * the includes that we located...               *
dnl  *************************************************
dnl  * This is necessary to ensure that we don't end *
dnl  * up using the wrong Qt lib path when linking!! *
dnl  *************************************************

qt_version_string=`cat $qt_incdir/$qt_version_source 2> configure.dbg |\
		   grep "#define QT_VERSION_STR"                      |\
		   sed 's/QT_VERSION_STR//'                           |\
                   sed 's/#define//g'                                 |\
		   sed 's/ //g'                                       |\
		   sed 's/	//g'		                      |\
                   sed 's/"//g'`;

qt_version_number=`cat $qt_incdir/$qt_version_source 2> configure.dbg |\
                   grep "#define QT_VERSION[^_]"                      |\
                   sed 's/QT_VERSION//'                               |\
                   sed 's/#define//g'                                 |\
                   sed 's/ //g'                                       |\
                   sed 's/	//g'                                  |\
                   sed 's/0x//'`;

 qt_major_version=`echo ${[qt_version_string%%.[0-9]*]}`
 qt_minor_version=`echo ${[qt_version_string#[0-9]*.]}`
 qt_minor_version=`echo ${[qt_minor_version%.[0-9]*]}`
   qt_major_build=`echo ${[qt_version_string##[0-9]*.]}`
 qt_lib_major_ver="libqt-mt.so.$qt_major_version"

 qt_lib_minor_ver="libqt-mt.so.$qt_major_version.$qt_minor_version"
 qt_lib_major_bld="libqt-mt.so.$qt_major_version.$qt_minor_version.$qt_major_build"

if [[ -n "$qt_version_string" ]]; then
  echo -e "yes\n>> Found version.:\t$qt_version_string"
  echo -e ">>> Headers......:\t$qt_incdir/"
  ac_qt_includes="$qt_incdir"
else
  echo -e "no\n>> Unable to locate your Qt includes..."
  AC_MSG_ERROR([Please verify your Qt devel install!]);
fi;

qt_target_version="3.2.0"

if test $qt_major_version -le 2 ; then
  AC_MSG_ERROR([ShowEQ requires qt $qt_target_version or later, but does not support qt 4 yet. Please make sure qt $qt_target_version or later is installed!!!]); 
elif test $qt_major_version -eq 3 ; then
  if test $qt_minor_version -lt 2 ; then
    AC_MSG_ERROR([ShowEQ requires qt $qt_target_version or later, but does not support qt 4 yet. Please make sure qt $qt_target_version or later is installed!!!]); 
  fi;
else
  AC_MSG_ERROR([ShowEQ requires qt $qt_target_version or later, but does not support qt 4 yet. Please make sure qt $qt_target_version or later is installed!!!]); 
fi;

dnl ************************************
dnl * Build yet another search path... *
dnl ************************************

qt_libdirs=" $QTDIR/lib			\
             $LD_LIBRARY_PATH		\
             $QTLIB			\
	     /usr/local/qt3/lib		\
             /usr/local/qt/lib		\
             /usr/local/qt*/lib		\
	     /usr/qt/3/lib		\
	     /usr/qt/*/lib		\
	     /usr/lib/qt3/lib		\
             /usr/lib/qt/lib		\
	     /usr/lib/qt*/lib		\
	     /usr/lib/qt3		\
             /usr/lib/qt		\
             /usr/lib/qt*		\
             /usr/lib			\
	     /usr/X11R6/lib/X11/qt3	\
             /usr/X11R6/lib/X11/qt	\
             /usr/X11R6/lib/X11/qt*	\
	     /usr/X11R6/lib/X11/qt3/lib	\
             /usr/X11R6/lib/X11/qt/lib	\
             /usr/X11R6/lib/X11/qt*/lib	\
	     /usr/X11R6/lib/qt3		\
             /usr/X11R6/lib/qt		\
             /usr/X11R6/lib/qt*		\
	     /usr/X11R6/lib/qt3/lib	\
             /usr/X11R6/lib/qt/lib	\
	     /usr/X11R6/lib/qt*/lib	\
             /usr/src/qt-*/lib		\
             $x_libraries		\
             $qt_libdirs"

case $host_cpu in
powerpc64 | s390x | sparc64 | x86_64)
qt_libdirs=" $QTDIR/lib64			\
             $QTLIB			\
	     /usr/local/qt3/lib64	\
             /usr/local/qt/lib64	\
             /usr/local/qt*/lib64	\
	     /usr/qt/3/lib64		\
	     /usr/qt/*/lib64		\
	     /usr/lib/qt3/lib64		\
             /usr/lib/qt/lib64		\
	     /usr/lib/qt*/lib64		\
             /usr/lib64			\
	     /usr/X11R6/lib/X11/qt3/lib64 \
             /usr/X11R6/lib/X11/qt/lib64  \
             /usr/X11R6/lib/X11/qt*/lib64 \
	     /usr/X11R6/lib/qt3/lib64	\
             /usr/X11R6/lib/qt/lib64	\
	     /usr/X11R6/lib/qt*/lib64	\
             /usr/src/qt-*/lib64	\
             $qt_libdirs"
;;
esac

[[ "$ac_qt_libraries" != "NO" ]]   &&   \
qt_libdirs="$ac_qt_libraries $qt_libdirs"


qt_libdir="";
for dir in $qt_libdirs; do

  if [[ -e "$dir/$qt_lib_major_ver" ]] ||\
     [[ -e "$dir/$qt_lib_minor_ver" ]] ||\
     [[ -e "$dir/$qt_lib_major_bld" ]]; then
    qt_libdir=$dir;
    break;
  else
    echo "tried $dir" >&AC_FD_CC;
  fi
done

ac_qt_libraries="$qt_libdir"

if [[ -n "$ac_qt_libraries" ]]; then
  echo -e ">>>> Libraries...:\t$ac_qt_libraries/"
else
  echo -e ">>>> Libraries...:\tUnable to locate?!?"
  AC_MSG_ERROR([Please verify your Qt lib install!]);
fi;

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_incdir $all_includes"
LDFLAGS="-L$qt_libdir $all_libraries"
LIBS="$LIBS $LIBQT"

PRINT_QT_PROGRAM

if [[ -e "$qt_libdir/libqt-mt.so.2" ]]; then
  echo "DEBUG INFO for: $qt_libdir/libqt-mt.so.2" > libqt2.dbg;
  ldd "$qt_libdir/libqt-mt.so.2" >> libqt2.dbg 2>&1;
  echo -e "\nOther Qt libraries in this directory:" >> libqt2.dbg;
  ls -lah "$qt_libdir/"libqt* >> libqt2.dbg;
  ac_qt_libraries="$qt_libdir"
fi

if [[ -e "$qt_libdir/libqt-mt.so.3" ]]; then
  echo "DEBUG INFO for: $qt_libdir/libqt-mt.so.3" > libqt3.dbg;
  ldd "$qt_libdir/libqt-mt.so.3" >> libqt3.dbg 2>&1;
  echo -e "\nOther Qt libraries in this directory:" >> libqt3.dbg;
  ls -lah "$qt_libdir/"libqt* >> libqt3.dbg;
  ac_qt_libraries="$qt_libdir"
fi

if [[ -n "$ac_qt_libraries" ]] && [[ "$ac_validate_qt" != "no" ]]; then
  if AC_TRY_EVAL(ac_link) && test -s conftest; then
    rm -f conftest*
  else
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.$ac_ext >&AC_FD_CC
    ac_qt_libraries=""
  fi
fi

rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

AC_LANG_RESTORE

dnl ************************************
dnl * Build yet another search path... *
dnl ************************************

qt_docdirs=" $QTDIR/doc/html		     \
	     $qt_incdir../doc/html     	     \
             /usr/local/qt3/doc/html	     \
             /usr/local/qt/doc/html	     \
             /usr/local/qt*/doc/html	     \
	     /usr/qt/3/doc/html	     	     \
	     /usr/qt/*/doc/html	     	     \
	     /usr/lib/qt3/doc/html	     \
             /usr/lib/qt/doc/html	     \
	     /usr/lib/qt*/doc/html	     \
	     /usr/lib/doc/html	     	     \
             /usr/lib/qt/doc/html	     \
             /usr/lib/qt*/doc/html	     \
	     /usr/X11R6/lib/X11/qt3/doc/html \
             /usr/X11R6/lib/X11/qt/doc/html  \
             /usr/X11R6/lib/X11/qt*/doc/html \
	     /usr/X11R6/lib/X11/qt3/doc/html \
             /usr/X11R6/lib/X11/qt/doc/html  \
             /usr/X11R6/lib/X11/qt*/doc/html \
	     /usr/X11R6/lib/qt3/doc/html     \
             /usr/X11R6/lib/qt/doc/html	     \
             /usr/X11R6/lib/qt*/doc/html     \
	     /usr/X11R6/lib/qt3/doc/html     \
             /usr/X11R6/lib/qt/doc/html	     \
	     /usr/X11R6/lib/qt*/doc/html     \
             /usr/src/qt-*/doc/html	     \
             $qt_docdirs"

[[ "$ac_qt_docs" != "NO" ]]   &&   \
qt_docdirs="$ac_qt_docs $qt_docdirs"

AC_FIND_FILE("qobject.html", $qt_docdirs, qt_docdir)

ac_qt_docs="$qt_docdir"

if [[ -n "$ac_qt_docs" -a "$ac_qt_docs" != "NO" ]]; then
  echo -e ">>>> Documentation...:\t$ac_qt_docs/"
else
  echo -e ">>>> Documentation...:\tUnable to locate?!?"
  AC_MSG_WARN([Qt documenation not installed?!]);
fi;

if [[ -z "$ac_qt_includes" ]] || [[ -z "$ac_qt_libraries" ]]; then
  ac_cv_have_qt="have_qt=no";
  ac_qt_notfound="";
  if [[ -z "$ac_qt_includes" ]]; then
    if [[ -z "$ac_qt_libraries" ]]; then
      ac_qt_notfound="headers AND libraries";
    else
      ac_qt_notfound="headers";
    fi
  elif [[ -z "$ac_qt_libraries" ]]; then
    ac_qt_notfound="libraries";
  else
    have_qt="yes";
  fi
else
  have_qt="yes";
fi

if [[ "$have_qt" != "yes" ]]; then
  AC_MSG_ERROR([Something wicked happened while trying to validate your Qt installation!!! Please verify your $ac_qt_notfound...]);
fi
])



dnl *****************************************************
dnl * Yeah, I got bored...  Can you blame me though? :) *
dnl *****************************************************

if [[ $have_qt != "yes" ]]; then

  NO=`echo -en "\[\e[0;1m-*{ \e[0;31;1mNO!\e[0;1m }*-\e[0;0m" &&
      echo "]]]]]" > /dev/null 2>&1`;

  if [[ -n "$NO" ]]; then
    AC_MSG_ERROR([>>>>> Workable...:	$NO]);
  else
    AC_MSG_ERROR([>>>>> Workable...:	-*{ NO! }*-]);
  fi

elif [[ $qt_major_version -lt 3 ]]; then

  MBY=`echo -en "\[\e[0;1m-*{ \e[0;33;1m???\e[0;1m }*-\e[0;0m" &&
       echo "]]]]]" > /dev/null 2>&1`;

  if [[ -n "$MBY" ]]; then
    AC_MSG_RESULT([>>>>> NOTE.......:	ShowEQ is designed for Qt 3.1.0+, please upgrade
>>>>>> Workable..:	$MBY]);
  else
    AC_MSG_RESULT([>>>>> NOTE.......:	ShowEQ is designed for Qt 3.1.0+, please upgrade
>>>>>> Workable..:  -*{ ??? }*-]);
  fi

  qt_libraries=$ac_qt_libraries
  qt_includes=$ac_qt_includes
  qt_docs=$ac_qt_docs

else

  YES=`echo -en "\[\e[0;1m-*{ \e[0;32;1mYES\e[0;1m }*-\e[0;0m" &&
       echo "]]]]]" > /dev/null 2>&1`;

  if [[ -n "$YES" ]]; then
    AC_MSG_RESULT([>>>>> Workable...:	$YES]);
  else
    AC_MSG_RESULT([>>>>> Workable...:	-*{ YES }*-]);
  fi

  qt_libraries=$ac_qt_libraries
  qt_includes=$ac_qt_includes
  qt_docs=$ac_qt_docs

fi

AC_PATH_QT_MOC
AC_PATH_QT_UIC
CHECK_QT_DIRECT(qt_libraries= ,[])

AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)
AC_SUBST(qt_docs)

if [[ "$qt_includes" == "$x_includes" ]] ||
   [[ -z "$qt_includes" ]]; then
 QT_INCLUDES="";
else
 QT_INCLUDES="-I$qt_includes"
 all_includes="$QT_INCLUDES $all_includes"
fi

if [[ "$qt_libraries" == "$x_libraries" ]] ||
   [[ -z "$qt_libraries" ]]; then
 QT_LDFLAGS=""
else
 QT_LDFLAGS="-L$qt_libraries"
 all_libraries="$all_libraries $QT_LDFLAGS"
fi

QT_DOCS="$qt_docs"

AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LDFLAGS)
AC_SUBST(QT_DOCS)

LIB_QT='-lqt-mt $(LIBPNG) -lXext $(LIB_X11) $(X_PRE_LIBS)'
AC_SUBST(LIB_QT)

])

AC_DEFUN([AC_PATH_QT],
[
AC_PATH_QT_1_3
])

AC_DEFUN([KDE_CHECK_FINAL],
[
  AC_ARG_ENABLE(final, [  --enable-final          build size optimized apps (needs lots of memory)],
	kde_use_final=yes, kde_use_final=no, kde_use_final=no)

  if test "x$kde_use_final" = "xyes"; then
      KDE_USE_FINAL_TRUE=""
      KDE_USE_FINAL_FALSE="#"
      KDE_CHECK_REPO
   else
      KDE_USE_FINAL_TRUE="#"
      KDE_USE_FINAL_FALSE=""
  fi
  AC_SUBST(KDE_USE_FINAL_TRUE)
  AC_SUBST(KDE_USE_FINAL_FALSE)
])

AC_DEFUN([SHOWEQ_CHECK_EXTRA_LIBS],
[
AC_MSG_CHECKING(for extra includes)
AC_ARG_WITH(extra-includes, [  --with-extra-includes=DIR
                          adds non standard include paths],
  showeq_use_extra_includes="$withval",
  showeq_use_extra_includes=NONE
)
showeq_extra_includes=
if test -n "$showeq_use_extra_includes" && \
   test "$showeq_use_extra_includes" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $showeq_use_extra_includes; do
     showeq_extra_includes="$showeq_extra_includes $dir"
     USER_INCLUDES="$USER_INCLUDES -I$dir"
   done
   IFS=$ac_save_ifs
   showeq_use_extra_includes="added"
else
   showeq_use_extra_includes="no"
fi
AC_SUBST(USER_INCLUDES)

AC_MSG_RESULT($showeq_use_extra_includes)

showeq_extra_libs=
AC_MSG_CHECKING(for extra libs)
AC_ARG_WITH(extra-libs, [  --with-extra-libs=DIR   adds non standard library paths],
  showeq_use_extra_libs=$withval,
  showeq_use_extra_libs=NONE
)
if test -n "$showeq_use_extra_libs" && \
   test "$showeq_use_extra_libs" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $showeq_use_extra_libs; do
     showeq_extra_libs="$showeq_extra_libs $dir"
     SHOWEQ_EXTRA_RPATH="$SHOWEQ_EXTRA_RPATH -rpath $dir"
     USER_LDFLAGS="$USER_LDFLAGS -L$dir"
   done
   IFS=$ac_save_ifs
   showeq_use_extra_libs="added"
else
   showeq_use_extra_libs="no"
fi

AC_SUBST(USER_LDFLAGS)

AC_MSG_RESULT($showeq_use_extra_libs)

])

AC_DEFUN([KDE_CHECK_KIMGIO],
[
   AC_REQUIRE([AC_BASE_PATH_KDE])
   AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])
   AC_REQUIRE([AC_FIND_TIFF])
   AC_REQUIRE([AC_FIND_JPEG])
   AC_REQUIRE([AC_FIND_PNG])
   AC_REQUIRE([KDE_CREATE_LIBS_ALIASES])

   if test "$1" = "existance"; then
     AC_LANG_SAVE
     AC_LANG_CPLUSPLUS
     kde_save_LIBS="$LIBS"
     LIBS="$LIBS $all_libraries $LIBJPEG $LIBTIFF $LIBPNG $LIBQT -lm"
     AC_CHECK_LIB(kimgio, kimgioRegister, [
      LIBKIMGIO_EXISTS=yes],LIBKIMGIO_EXISTS=no)
      LIBS="$kde_save_LIBS"
      AC_LANG_RESTORE
   else
      LIBKIMGIO_EXISTS=yes
   fi

   if test "$LIBKIMGIO_EXISTS" = "yes"; then
     LIB_KIMGIO='-lkimgio'
   else
     LIB_KIMGIO=''
   fi
   AC_SUBST(LIB_KIMGIO)
])

AC_DEFUN([KDE_CREATE_LIBS_ALIASES],
[
   AC_REQUIRE([KDE_MISC_TESTS])
   AC_REQUIRE([KDE_CHECK_LIBDL])
   AC_REQUIRE([K_PATH_X])

if test $kde_qtver = 2; then
   LIB_KDECORE='-lkdecore'
   AC_SUBST(LIB_KDECORE)
   LIB_KDEUI='-lkdeui'
   AC_SUBST(LIB_KDEUI)
   LIB_KFORMULA='-lformula'
   AC_SUBST(LIB_KFORMULA)
   LIB_KIO='-lkio'
   AC_SUBST(LIB_KIO)
   LIB_SMB='-lsmb'
   AC_SUBST(LIB_SMB)
   LIB_KFILE='-lkfile'
   AC_SUBST(LIB_KFILE)
   LIB_KAB='-lkab'
   AC_SUBST(LIB_KAB)
   LIB_MEDIATOOL='-lmediatool'
   AC_SUBST(LIB_MEDIATOOL)
   LIB_KHTML='-lkhtml'
   AC_SUBST(LIB_KHTML)
   LIB_KSPELL='-lkspell'
   AC_SUBST(LIB_KSPELL)
   LIB_KPARTS='-lkparts'
   AC_SUBST(LIB_KPARTS)
else
   LIB_KDECORE='-lkdecore -lXext $(LIB_QT)'
   AC_SUBST(LIB_KDECORE)
   LIB_KDEUI='-lkdeui $(LIB_KDECORE)'
   AC_SUBST(LIB_KDEUI)
   LIB_KFM='-lkfm $(LIB_KDECORE)'
   AC_SUBST(LIB_KFM)
   LIB_KFILE='-lkfile $(LIB_KFM) $(LIB_KDEUI)'
   AC_SUBST(LIB_KFILE)
   LIB_KAB='-lkab $(LIB_KIMGIO) $(LIB_KDECORE)'
   AC_SUBST(LIB_KAB)
   LIB_MEDIATOOL='-lmediatool $(LIB_KDECORE)'
   AC_SUBST(LIB_MEDIATOOL)
fi
])

AC_DEFUN([AC_PATH_KDE],
[
  AC_BASE_PATH_KDE
  AC_ARG_ENABLE(path-check, [  --disable-path-check    don't try to find out, where to install],
  [
  if test "$enableval" = "no";
    then ac_use_path_checking="default"
    else ac_use_path_checking=""
  fi
  ], [ac_use_path_checking="default"]
  )

  AC_CREATE_KFSSTND($ac_use_path_checking)

  AC_SUBST_KFSSTND
  KDE_CREATE_LIBS_ALIASES
])

dnl slightly changed version of AC_CHECK_FUNC(setenv)
AC_DEFUN([AC_CHECK_SETENV],
[AC_MSG_CHECKING([for setenv])
AC_CACHE_VAL(ac_cv_func_setenv,
[AC_LANG_C
AC_TRY_LINK(
dnl Don't include <ctype.h> because on OSF/1 3.0 it includes <sys/types.h>
dnl which includes <sys/select.h> which contains a prototype for
dnl select.  Similarly for bzero.
[#include <assert.h>
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
#include <stdlib.h>
], [
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
setenv("TEST", "alle", 1);
#endif
], eval "ac_cv_func_setenv=yes", eval "ac_cv_func_setenv=no")])

if test "$ac_cv_func_setenv" = "yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_FUNC_SETENV, 1, [Define if you have setenv])
else
  AC_MSG_RESULT(no)
fi
])

AC_DEFUN([AC_CHECK_GETDOMAINNAME],
[
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
if test "$GCC" = "yes"; then
CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi
AC_MSG_CHECKING(for getdomainname)
AC_CACHE_VAL(ac_cv_func_getdomainname,
[
AC_TRY_COMPILE([
#include <stdlib.h>
#include <unistd.h>
],
[
char buffer[200];
getdomainname(buffer, 200);
],
ac_cv_func_getdomainname=yes,
ac_cv_func_getdomainname=no)
])
AC_MSG_RESULT($ac_cv_func_getdomainname)
if eval "test \"`echo $ac_cv_func_getdomainname`\" = yes"; then
  AC_DEFINE(HAVE_GETDOMAINNAME, 1, [Define if you have getdomainname])
fi
CXXFLAGS="$save_CXXFLAGS"
])

AC_DEFUN([AC_CHECK_GETHOSTNAME],
[
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
if test "$GCC" = "yes"; then
CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi

AC_MSG_CHECKING([for gethostname])
AC_CACHE_VAL(ac_cv_func_gethostname,
[
AC_TRY_COMPILE([
#include <stdlib.h>
#include <unistd.h>
],
[
char buffer[200];
gethostname(buffer, 200);
],
ac_cv_func_gethostname=yes,
ac_cv_func_gethostname=no)
])
AC_MSG_RESULT($ac_cv_func_gethostname)
if eval "test \"`echo $ac_cv_func_gethostname`\" = yes"; then
  AC_DEFINE(HAVE_GETHOSTNAME, 1, [Define if you have getdomainname])
fi
CXXFLAGS="$save_CXXFLAGS"
])

AC_DEFUN([AC_CHECK_USLEEP],
[
AC_LANG_CPLUSPLUS

AC_MSG_CHECKING([for usleep])
AC_CACHE_VAL(ac_cv_func_usleep,
[
ac_libs_safe="$LIBS"
LIBS="$LIBS $LIBUCB"
AC_TRY_LINK([
#include <stdlib.h>
#include <unistd.h>
],
[
usleep(200);
],
ac_cv_func_usleep=yes,
ac_cv_func_usleep=no)
])
AC_MSG_RESULT($ac_cv_func_usleep)
if eval "test \"`echo $ac_cv_func_usleep`\" = yes"; then
  AC_DEFINE(HAVE_USLEEP, 1, [Define if you have the usleep function])
fi
LIBS="$ac_libs_safe"
])

AC_DEFUN([AC_CHECK_RANDOM],
[
AC_LANG_CPLUSPLUS

AC_MSG_CHECKING([for random])
AC_CACHE_VAL(ac_cv_func_random,
[
ac_libs_safe="$LIBS"
LIBS="$LIBS $LIBUCB"
AC_TRY_LINK([
#include <stdlib.h>
],
[
random();
],
ac_cv_func_random=yes,
ac_cv_func_random=no)
])
AC_MSG_RESULT($ac_cv_func_random)
if eval "test \"`echo $ac_cv_func_random`\" = yes"; then
  AC_DEFINE(HAVE_RANDOM, 1, [Define if you have random])
fi
LIBS="$ac_libs_safe"
])

AC_DEFUN([AC_FIND_GIF],
   [AC_MSG_CHECKING([for giflib])
AC_CACHE_VAL(ac_cv_lib_gif,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lgif -lX11 $LIBSOCKET"
AC_TRY_LINK(dnl
[
#ifdef __cplusplus
extern "C" {
#endif
int GifLastError(void);
#ifdef __cplusplus
}
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
],
            [return GifLastError();],
            eval "ac_cv_lib_gif=yes",
            eval "ac_cv_lib_gif=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo $ac_cv_lib_gif`\" = yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_LIBGIF, 1, [Define if you have libgif])
else
  AC_MSG_ERROR(You need giflib30. Please install the kdesupport package)
fi
])

AC_DEFUN([KDE_FIND_JPEG_HELPER],
[
AC_MSG_CHECKING([for libjpeg$2])
AC_CACHE_VAL(ac_cv_lib_jpeg_$1,
[
AC_LANG_C
ac_save_LIBS="$LIBS"
LIBS="$all_libraries -ljpeg$2 -lm"
AC_TRY_LINK(
[/* Override any gcc2 internal prototype to avoid an error.  */
struct jpeg_decompress_struct;
typedef struct jpeg_decompress_struct * j_decompress_ptr;
typedef int size_t;
#ifdef __cplusplus
extern "C" {
#endif
    void jpeg_CreateDecompress(j_decompress_ptr cinfo,
                                    int version, size_t structsize);
#ifdef __cplusplus
}
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
],
            [jpeg_CreateDecompress(0L, 0, 0);],
            eval "ac_cv_lib_jpeg_$1=-ljpeg$2",
            eval "ac_cv_lib_jpeg_$1=no")
LIBS="$ac_save_LIBS"
])

if eval "test ! \"`echo $ac_cv_lib_jpeg_$1`\" = no"; then
  LIBJPEG="$ac_cv_lib_jpeg_$1"
  AC_MSG_RESULT($ac_cv_lib_jpeg_$1)
else
  AC_MSG_RESULT(no)
  $3
fi

])

AC_DEFUN([AC_FIND_JPEG],
[
KDE_FIND_JPEG_HELPER(6b, 6b,
   KDE_FIND_JPEG_HELPER(normal, [],
    [
dnl what to do, if the normal way fails:
	if test -f "$kde_libraries/libjpeg.so"; then
	   test -f ./libjpegkde.so || $LN_S $kde_libraries/libjpeg.so ./libjpegkde.so
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"
	elif test -f "$kde_libraries/libjpeg.sl"; then
	   test -f ./libjpegkde.sl ||$LN_S $kde_libraries/libjpeg.sl ./libjpegkde.sl
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"	
	elif test -f "$kde_libraries/libjpeg.a"; then
	   test -f ./libjpegkde.a || $LN_S $kde_libraries/libjpeg.a ./libjpegkde.a
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"
        else
	  AC_MSG_ERROR([
You need the development package of libjpeg6b. You can get
a source package of libjpeg from http://www.ijg.org/
])
	fi
      fi
   fi

   LIBJPEG=$ac_cv_lib_jpeg
]))

AC_SUBST(LIBJPEG)
AC_DEFINE_UNQUOTED(HAVE_LIBJPEG, 1, [Define if you have libjpeg])

])

AC_DEFUN([AC_FIND_ZLIB],
[
AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])
AC_MSG_CHECKING([for libz])
AC_CACHE_VAL(ac_cv_lib_z,
[
AC_LANG_C
kde_save_LIBS="$LIBS"
LIBS="$all_libraries -lz $LIBSOCKET"
kde_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes"
AC_TRY_LINK(dnl
[
#include<zlib.h>
],
            [return (zlibVersion() == ZLIB_VERSION); ],
            eval "ac_cv_lib_z='-lz'",
            eval "ac_cv_lib_z=no")
LIBS="$kde_save_LIBS"
CFLAGS="$kde_save_CFLAGS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_z`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBZ, 1, [Define if you have libz])
  LIBZ="$ac_cv_lib_z"
  AC_SUBST(LIBZ)
  AC_MSG_RESULT($ac_cv_lib_z)
else
  AC_MSG_RESULT(no)
  LIBZ=""
  AC_SUBST(LIBZ)
fi
])

AC_DEFUN([KDE_TRY_TIFFLIB],
[
AC_MSG_CHECKING([for libtiff $1])

AC_CACHE_VAL(kde_cv_libtiff_$1,
[
AC_LANG_CPLUSPLUS
kde_save_LIBS="$LIBS"
LIBS="$all_libraries -l$1 $LIBJPEG $LIBZ -lX11 $LIBSOCKET -lm"
kde_save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $all_includes"

AC_TRY_LINK(dnl
[
#include<tiffio.h>
],
    [return (TIFFOpen( "", "r") == 0); ],
[
    kde_cv_libtiff_$1="-l$1 $LIBJPEG $LIBZ"
], [
    kde_cv_libtiff_$1=no
])

LIBS="$kde_save_LIBS"
CXXFLAGS="$kde_save_CXXFLAGS"

])

if test "$kde_cv_libtiff_$1" = "no"; then
    AC_MSG_RESULT(no)
    LIBTIFF=""
    $3
else
    LIBTIFF="$kde_cv_libtiff_$1"
    AC_MSG_RESULT(yes)
    AC_DEFINE_UNQUOTED(HAVE_LIBTIFF, 1, [Define if you have libtiff])
    $2
fi

])

AC_DEFUN([AC_FIND_TIFF],
[
AC_REQUIRE([K_PATH_X])
AC_REQUIRE([AC_FIND_ZLIB])
AC_REQUIRE([AC_FIND_JPEG])
AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])

KDE_TRY_TIFFLIB(tiff, [],
   KDE_TRY_TIFFLIB(tiff34))

AC_SUBST(LIBTIFF)
])


AC_DEFUN([AC_FIND_PNG],
[
AC_REQUIRE([AC_FIND_ZLIB])
AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])
AC_MSG_CHECKING([for libpng])
AC_CACHE_VAL(ac_cv_lib_png,
[
kde_save_LIBS="$LIBS"
LIBS="$LIBS $all_libraries -lpng $LIBZ -lm -lX11 $LIBSOCKET"
kde_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes"
AC_LANG_C
AC_TRY_LINK(dnl
    [
    #include<png.h>
    ],
    [
    png_structp png_ptr = png_create_read_struct(  /* image ptr */
		PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    return( png_ptr != 0 );
    ],
    eval "ac_cv_lib_png='-lpng $LIBZ -lm'",
    eval "ac_cv_lib_png=no"
)
LIBS="$kde_save_LIBS"
CFLAGS="$kde_save_CFLAGS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_png`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG, 1, [Define if you have libpng])
  LIBPNG="$ac_cv_lib_png"
  AC_SUBST(LIBPNG)
  AC_MSG_RESULT($ac_cv_lib_png)
else
  AC_MSG_RESULT(no)
  LIBPNG=""
  AC_SUBST(LIBPNG)
fi
])

AC_DEFUN([AC_CHECK_BOOL],
[
	AC_MSG_CHECKING([for bool])
        AC_CACHE_VAL(ac_cv_have_bool,
        [
		AC_LANG_CPLUSPLUS
          	AC_TRY_COMPILE([],
                 [bool aBool = true;],
                 [ac_cv_have_bool="yes"],
                 [ac_cv_have_bool="no"])
        ]) dnl end AC_CHECK_VAL
        AC_MSG_RESULT($ac_cv_have_bool)
        if test "$ac_cv_have_bool" = "yes"; then
        	AC_DEFINE(HAVE_BOOL, 1, [Define if the C++ compiler supports BOOL])
        fi
])

AC_DEFUN([AC_CHECK_GNU_EXTENSIONS],
[
AC_MSG_CHECKING(if you need GNU extensions)
AC_CACHE_VAL(ac_cv_gnu_extensions,
[
cat > conftest.c << EOF
#include <features.h>

#ifdef __GNU_LIBRARY__
yes
#endif
EOF

if (eval "$ac_cpp conftest.c") 2>&5 |
  egrep "yes" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_gnu_extensions=yes
else
  ac_cv_gnu_extensions=no
fi
])

AC_MSG_RESULT($ac_cv_gnu_extensions)
if test "$ac_cv_gnu_extensions" = "yes"; then
  AC_DEFINE_UNQUOTED(_GNU_SOURCE, 1, [Define if you need to use the GNU extensions])
fi
])

AC_DEFUN([KDE_CHECK_COMPILER_FLAG],
[
AC_REQUIRE([AC_CHECK_COMPILERS])
AC_MSG_CHECKING(whether $CXX supports -$1)
kde_cache=`echo $1 | sed 'y%.=/+-%___p_%'`
AC_CACHE_VAL(kde_cv_prog_cxx_$kde_cache,
[
echo 'void f(){}' >conftest.cc
if test -z "`$CXX -$1 -c conftest.cc 2>&1`"; then
  eval "kde_cv_prog_cxx_$kde_cache=yes"
else
  eval "kde_cv_prog_cxx_$kde_cache=no"
fi
rm -f conftest*
])
if eval "test \"`echo '$kde_cv_prog_cxx_'$kde_cache`\" = yes"; then
 AC_MSG_RESULT(yes)
 :
 $2
else
 AC_MSG_RESULT(no)
 :
 $3
fi
])

AC_DEFUN([AC_CHECK_COMPILERS],
[
  dnl this is somehow a fat lie, but prevents other macros from double checking
  AC_PROVIDE([AC_PROG_CC])
  AC_PROVIDE([AC_PROG_CPP])
  AC_ARG_ENABLE(debug,[  --enable-debug          creates debugging code [default=no]],
  [
   if test $enableval = "no"; dnl
     then
       kde_use_debug_code="no"
       kde_use_debug_define=yes
     else
       kde_use_debug_code="yes"
       kde_use_debug_define=no
   fi
  ], [kde_use_debug_code="no"
      kde_use_debug_define=no
    ])

  AC_ARG_ENABLE(strict,[  --enable-strict         compiles with strict compiler options (may not work!)],
   [
    if test $enableval = "no"; then
         kde_use_strict_options="no"
       else
         kde_use_strict_options="yes"
    fi
   ], [kde_use_strict_options="no"])

dnl this was AC_PROG_CC. I had to include it manualy, since I had to patch it
  AC_MSG_CHECKING(for a C-Compiler)
  dnl if there is one, print out. if not, don't matter
  AC_MSG_RESULT($CC)

  if test -z "$CC"; then AC_CHECK_PROG(CC, gcc, gcc) fi
  if test -z "$CC"; then AC_CHECK_PROG(CC, cc, cc, , , /usr/ucb/cc) fi
  if test -z "$CC"; then AC_CHECK_PROG(CC, xlc, xlc) fi
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])

  AC_PROG_CC_WORKS
  AC_PROG_CC_GNU

  if test $ac_cv_prog_gcc = yes; then
    GCC=yes
  else
    GCC=
  fi

  if test -z "$CFLAGS"; then
    if test "$kde_use_debug_code" = "yes"; then
      AC_PROG_CC_G
      if test $ac_cv_prog_cc_g = yes; then
        CFLAGS="-g"
      fi
    else
      if test "$GCC" = "yes"; then
        CFLAGS="-O2"
      else
        CFLAGS=""
      fi
      if test "$kde_use_debug_define" = "yes"; then
         CFLAGS="$CFLAGS -DNDEBUG"
      fi
    fi

    if test "$GCC" = "yes"; then
     CFLAGS="$CFLAGS"

     if test "$kde_use_strict_options" = "yes"; then
	CFLAGS="$CFLAGS -W -Wall -ansi -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings"
     fi
    fi

  fi

  case "$host" in
  *-*-sysv4.2uw*) CFLAGS="$CFLAGS -D_UNIXWARE";;
  esac

  if test -z "$LDFLAGS" && test "$kde_use_debug_code" = "no" && test "$GCC" = "yes"; then
     LDFLAGS="-s"
  fi


dnl this is AC_PROG_CPP. I had to include it here, since autoconf checks
dnl dependecies between AC_PROG_CPP and AC_PROG_CC (or is it automake?)

  AC_MSG_CHECKING(how to run the C preprocessor)
  # On Suns, sometimes $CPP names a directory.
  if test -n "$CPP" && test -d "$CPP"; then
    CPP=
  fi
  if test -z "$CPP"; then
  AC_CACHE_VAL(ac_cv_prog_CPP,
  [  # This must be in double quotes, not single quotes, because CPP may get
    # substituted into the Makefile and "${CC-cc}" will confuse make.
    CPP="${CC-cc} -E"
    # On the NeXT, cc -E runs the code through the compiler's parser,
    # not just through cpp.
    dnl Use a header file that comes with gcc, so configuring glibc
    dnl with a fresh cross-compiler works.
    AC_TRY_CPP([#include <assert.h>
    Syntax Error], ,
    CPP="${CC-cc} -E -traditional-cpp"
    AC_TRY_CPP([#include <assert.h>
    Syntax Error], , CPP=/lib/cpp))
    ac_cv_prog_CPP="$CPP"])dnl
    CPP="$ac_cv_prog_CPP"
  else
    ac_cv_prog_CPP="$CPP"
  fi
  AC_MSG_RESULT($CPP)
  AC_SUBST(CPP)dnl


  AC_MSG_CHECKING(for a C++-Compiler)
  dnl if there is one, print out. if not, don't matter
  AC_MSG_RESULT($CXX)

  if test -z "$CXX"; then AC_CHECK_PROG(CXX, g++, g++) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, CC, CC) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, xlC, xlC) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, DCC, DCC) fi
  test -z "$CXX" && AC_MSG_ERROR([no acceptable C++-compiler found in \$PATH])

  AC_PROG_CXX_WORKS
  AC_PROG_CXX_GNU

  if test $ac_cv_prog_gxx = yes; then
    GXX=yes
  fi

  if test -z "$CXXFLAGS"; then
    if test "$kde_use_debug_code" = "yes"; then
      AC_PROG_CXX_G
      if test $ac_cv_prog_cxx_g = yes; then
        CXXFLAGS="-g"
      fi
    else
      if test "$GXX" = "yes"; then
         CXXFLAGS="-O2"
      fi
      if test "$kde_use_debug_define" = "yes"; then
         CXXFLAGS="$CXXFLAGS -DNDEBUG"
      fi
    fi

    KDE_CHECK_COMPILER_FLAG(fno-exceptions,
	[
	  CXXFLAGS="$CXXFLAGS -fno-exceptions"
	])

    KDE_CHECK_COMPILER_FLAG(fno-rtti,
	[
	  CXXFLAGS="$CXXFLAGS -fno-rtti"
	])

    KDE_CHECK_COMPILER_FLAG(fno-check-new,
	[
	  CXXFLAGS="$CXXFLAGS -fno-check-new"
	])

    if test "$GXX" = "yes"; then
       CXXFLAGS="$CXXFLAGS"

       if test true || test "$kde_use_debug_code" = "yes"; then
	 CXXFLAGS="$CXXFLAGS -Wall -pedantic -W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings"

         KDE_CHECK_COMPILER_FLAG(Wno-long-long,
	 [
	   CXXFLAGS="$CXXFLAGS -Wno-long-long"
	 ])
         KDE_CHECK_COMPILER_FLAG(fno-builtin,
         [
           CXXFLAGS="$CXXFLAGS -fno-builtin"
         ]) 
	
       fi

       if test "$kde_use_strict_options" = "yes"; then
	CXXFLAGS="$CXXFLAGS -Wcast-qual -Wbad-function-cast -Wshadow -Wcast-align -Woverloaded-virtual"
       fi

       if test "$kde_very_strict" = "yes"; then
         CXXFLAGS="$CXXFLAGS -Wold-style-cast -Wredundant-decls -Wconversion"
       fi
    fi
  fi

    KDE_CHECK_COMPILER_FLAG(fexceptions,
	[
	  USE_EXCEPTIONS="-fexceptions"
	],
	  USE_EXCEPTIONS=
	)
    AC_SUBST(USE_EXCEPTIONS)

    KDE_CHECK_COMPILER_FLAG(frtti,
	[
	  USE_RTTI="-frtti"
	],
	  USE_RTTI=
	)
    AC_SUBST(USE_RTTI)

    case "$host" in
      *-*-sysv4.2uw*) CXXFLAGS="$CXXFLAGS -D_UNIXWARE";;
    esac

    AC_PROVIDE(AC_PROG_CXXCPP)

    AC_MSG_CHECKING(how to run the C++ preprocessor)
    if test -z "$CXXCPP"; then
      AC_CACHE_VAL(ac_cv_prog_CXXCPP,
      [
         AC_LANG_SAVE[]dnl
         AC_LANG_CPLUSPLUS[]dnl
         CXXCPP="${CXX-g++} -E"
         AC_TRY_CPP([#include <stdlib.h>], , CXXCPP=/lib/cpp)
         ac_cv_prog_CXXCPP="$CXXCPP"
         AC_LANG_RESTORE[]dnl
     ])dnl
     CXXCPP="$ac_cv_prog_CXXCPP"
     fi
    AC_MSG_RESULT($CXXCPP)
    AC_SUBST(CXXCPP)dnl

    # the following is to allow programs, that are known to
    # have problems when compiled with -O2
    if test -n "$CXXFLAGS"; then
      kde_safe_IFS=$IFS
      IFS=" "
      NOOPT_CXXFLAGS=""
      for i in $CXXFLAGS; do
        if test ! "$i" = "-O2"; then
          NOOPT_CXXFLAGS="$NOOPT_CXXFLAGS $i"
        fi
      done
      IFS=$kde_safe_IFS
    fi
    AC_SUBST(NOOPT_CXXFLAGS)

    KDE_CHECK_FINAL

    ifdef([AM_DEPENDENCIES], AC_REQUIRE([KDE_ADD_DEPENDENCIES]), [])

    KDE_CXXFLAGS=
    AC_SUBST(KDE_CXXFLAGS)
])

AC_DEFUN([KDE_ADD_DEPENDENCIES],
[
   [A]M_DEPENDENCIES(CC)
   [A]M_DEPENDENCIES(CXX)
])

dnl just a wrapper to clean up configure.in
AC_DEFUN([KDE_PROG_LIBTOOL],
[
AC_REQUIRE([AC_CHECK_COMPILERS])
AC_REQUIRE([AC_ENABLE_SHARED])
AC_REQUIRE([AC_ENABLE_STATIC])

if test -z "$LIBTOOL"; then
  dnl libtool is only for C, so I must force him
  dnl to find the correct flags for C++
  kde_save_cc=$CC
  kde_save_cflags="$CFLAGS"
  CC=$CXX
  CFLAGS="$CXXFLAGS"
  AC_LANG_SAVE
  AC_LANG_C
  AC_REQUIRE([AC_LIBTOOL_DLOPEN])
  AC_REQUIRE([AM_PROG_LIBTOOL])
  AC_REQUIRE([AC_PROG_LIBTOOL]) dnl for libraries
  LIBTOOL="$LIBTOOL --silent"
  dnl AC_SUBST(LIBTOOL)
  AC_LANG_RESTORE
  CC=$kde_save_cc
  CFLAGS="$kde_save_cflags"
fi
])

AC_DEFUN([KDE_DO_IT_ALL],
[
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM
AM_INIT_AUTOMAKE($1, $2)
AM_DISABLE_LIBRARIES
AC_PREFIX_DEFAULT(${KDEDIR:-/usr/local/kde})
AC_CHECK_COMPILERS
KDE_PROG_LIBTOOL
AM_KDE_WITH_NLS
AC_PATH_KDE
])

AC_DEFUN([AC_CHECK_RPATH],
[
AC_MSG_CHECKING(for rpath)
AC_ARG_ENABLE(rpath,
      [  --disable-rpath         do not use the rpath feature of ld],
      USE_RPATH=$enableval, USE_RPATH=yes)

if test -z "$SHOWEQ_RPATH" && test "$USE_RPATH" = "yes"; then

  SHOWEQ_RPATH=

  if test -n "$qt_libraries"; then
    SHOWEQ_RPATH="$SHOWEQ_RPATH -rpath \$(qt_libraries)"
  fi

  dnl $x_libraries is set to /usr/lib in case
  if test -n "$X_LDFLAGS"; then
    SHOWEQ_RPATH="$SHOWEQ_RPATH -rpath \$(x_libraries)"
  fi
  if test -n "$SHOWEQ_EXTRA_RPATH"; then
    SHOWEQ_RPATH="$SHOWEQ_RPATH \$(SHOWEQ_EXTRA_RPATH)"
  fi
fi
AC_SUBST(SHOWEQ_EXTRA_RPATH)
AC_SUBST(SHOWEQ_RPATH)
AC_MSG_RESULT($USE_RPATH)
])

dnl Check for the type of the third argument of getsockname
AC_DEFUN([AC_CHECK_KSIZE_T],
[AC_MSG_CHECKING(for the third argument of getsockname)
AC_CACHE_VAL(ac_cv_ksize_t,
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
[AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
],[
socklen_t a=0;
getsockname(0,(struct sockaddr*)0, &a);
],
ac_cv_ksize_t=socklen_t,
ac_cv_ksize_t=)
if test -z "$ac_cv_ksize_t"; then
ac_safe_cxxflags="$CXXFLAGS"
if test "$GCC" = "yes"; then
  CXXFLAGS="-Werror $CXXFLAGS"
fi
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
],[
int a=0;
getsockname(0,(struct sockaddr*)0, &a);
],
ac_cv_ksize_t=int,
ac_cv_ksize_t=size_t)
CXXFLAGS="$ac_safe_cxxflags"
fi
AC_LANG_RESTORE
])

if test -z "$ac_cv_ksize_t"; then
  ac_cv_ksize_t=int
fi

AC_MSG_RESULT($ac_cv_ksize_t)
AC_DEFINE_UNQUOTED(ksize_t, $ac_cv_ksize_t,
      [Define the type of the third argument for getsockname]
)

])

dnl This is a merge of some macros out of the gettext aclocal.m4
dnl since we don't need anything, I took the things we need
dnl the copyright for them is:
dnl >
dnl Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
dnl This Makefile.in is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.
dnl >
dnl for this file it is relicensed under LGPL

AC_DEFUN([AM_KDE_WITH_NLS],
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS, 1, [Define if NLS is requested])

      AM_PATH_PROG_WITH_TEST_KDE(MSGFMT, msgfmt,
         [test -n "`$ac_dir/$ac_word --version 2>&1 | grep 'GNU gettext'`"], msgfmt)
      AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)

      if test -z "`$GMSGFMT --version 2>&1 | grep 'GNU gettext'`"; then
        AC_MSG_RESULT([found msgfmt program is not GNU msgfmt; ignore it])
        GMSGFMT=":"
      fi
      MSGFMT=$GMSGFMT
      AC_SUBST(GMSGFMT)
      AC_SUBST(MSGFMT)

      AM_PATH_PROG_WITH_TEST_KDE(XGETTEXT, xgettext,
	[test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext programs is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi
     AC_SUBST(XGETTEXT)
    fi

  ])

# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.

# serial 1
# Stephan Kulow: I appended a _KDE against name conflicts

dnl AM_PATH_PROG_WITH_TEST_KDE(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST_KDE],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])


# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.

# serial 1

AC_DEFUN([AM_LC_MESSAGES],
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES, 1, [Define if your locale.h file contains LC_MESSAGES])
    fi
  fi])

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.

# serial 1
# Stephan Kulow: I put a KDE in it to avoid name conflicts

AC_DEFUN([AM_KDE_GNU_GETTEXT],
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl
   AC_REQUIRE([AM_KDE_WITH_NLS])dnl
   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h values.h alloca.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
__argz_count __argz_stringify __argz_next])

   AC_MSG_CHECKING(for stpcpy)
   AC_CACHE_VAL(kde_cv_func_stpcpy,
   [
   kde_safe_cxxflags=$CXXFLAGS
   CXXFLAGS="-Wmissing-prototypes -Werror"
   AC_LANG_SAVE
   AC_LANG_CPLUSPLUS
   AC_TRY_COMPILE([
   #include <string.h>
   ],
   [
   char buffer[200];
   stpcpy(buffer, buffer);
   ],
   kde_cv_func_stpcpy=yes,
   kde_cv_func_stpcpy=no)
   AC_LANG_RESTORE
   CXXFLAGS=$kde_safe_cxxflags
   ])
   AC_MSG_RESULT($kde_cv_func_stpcpy)
   if eval "test \"`echo $kde_cv_func_stpcpy`\" = yes"; then
     AC_DEFINE(HAVE_STPCPY, 1, [Define if you have stpcpy])
   fi

   AM_LC_MESSAGES

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

  ])

AC_DEFUN([AC_HAVE_XPM],
 [AC_REQUIRE_CPP()dnl
  AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])

 test -z "$XPM_LDFLAGS" && XPM_LDFLAGS=
 test -z "$XPM_INCLUDE" && XPM_INCLUDE=

 AC_ARG_WITH(xpm, [  --without-xpm           disable color pixmap XPM tests],
	xpm_test=$withval, xpm_test="yes")
 if test "x$xpm_test" = xno; then
   ac_cv_have_xpm=no
 else
   AC_MSG_CHECKING(for XPM)
   AC_CACHE_VAL(ac_cv_have_xpm,
   [
    AC_LANG_C
    ac_save_ldflags="$LDFLAGS"
    ac_save_cflags="$CFLAGS"
    LDFLAGS="$LDFLAGS $XPM_LDFLAGS $all_libraries -lXpm -lX11 -lXext $LIBSOCKET"
    CFLAGS="$CFLAGS $X_INCLUDES"
    test ! -z "$XPM_INCLUDE" && CFLAGS="-I$XPM_INCLUDE $CFLAGS"
    AC_TRY_LINK([#include <X11/xpm.h>],[],
	ac_cv_have_xpm="yes",ac_cv_have_xpm="no")
    LDFLAGS="$ac_save_ldflags"
    CFLAGS="$ac_save_cflags"
   ])dnl

  if test "$ac_cv_have_xpm" = no; then
    AC_MSG_RESULT(no)
    XPM_LDFLAGS=""
    XPMINC=""
    $2
  else
    AC_DEFINE(HAVE_XPM, 1, [Define if you have XPM support])
    if test "$XPM_LDFLAGS" = ""; then
       XPMLIB='-lXpm $(LIB_X11)'
    else
       XPMLIB="-L$XPM_LDFLAGS -lXpm "'$(LIB_X11)'
    fi
    if test "$XPM_INCLUDE" = ""; then
       XPMINC=""
    else
       XPMINC="-I$XPM_INCLUDE"
    fi
    AC_MSG_RESULT(yes)
    $1
  fi
 fi
 AC_SUBST(XPMINC)
 AC_SUBST(XPMLIB)
])

AC_DEFUN([AC_HAVE_DPMS],
 [AC_REQUIRE_CPP()dnl
  AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])

 test -z "$DPMS_LDFLAGS" && DPMS_LDFLAGS=
 test -z "$DPMS_INCLUDE" && DPMS_INCLUDE=

 AC_ARG_WITH(dpms, [  --without-dpms          disable DPMS power saving],
	dpms_test=$withval, dpms_test="yes")
 if test "x$dpms_test" = xno; then
   ac_cv_have_dpms=no
 else
   AC_MSG_CHECKING(for DPMS)
   AC_CACHE_VAL(ac_cv_have_dpms,
   [
    AC_LANG_C
    ac_save_ldflags="$LDFLAGS"
    ac_save_cflags="$CFLAGS"
    LDFLAGS="$LDFLAGS $DPMS_LDFLAGS $all_libraries -lXpm -lX11 -lXext $LIBSOCKET"
    CFLAGS="$CFLAGS $X_INCLUDES"
    test ! -z "$DPMS_INCLUDE" && CFLAGS="-I$DPMS_INCLUDE $CFLAGS"
    AC_TRY_LINK([#include <X11/extensions/dpms.h>],[],
	ac_cv_have_dpms="yes",ac_cv_have_dpms="no")
    LDFLAGS="$ac_save_ldflags"
    CFLAGS="$ac_save_cflags"
   ])dnl

  if test "$ac_cv_have_dpms" = no; then
    AC_MSG_RESULT(no)
    DPMS_LDFLAGS=""
    DPMSINC=""
    $2
  else
    AC_DEFINE(HAVE_DPMS, 1, [Define if you have DPMS support])
    if test "$DPMS_LDFLAGS" = ""; then
       DPMSLIB='-lXdpms $(LIB_X11)'
    else
       DPMSLIB="-L$DPMS_LDFLAGS -lXdpms "'$(LIB_X11)'
    fi
    if test "$DPMS_INCLUDE" = ""; then
       DPMSINC=""
    else
       DPMSINC="-I$DPMS_INCLUDE"
    fi
    AC_MSG_RESULT(yes)
    $1
  fi
 fi
 AC_SUBST(DPMSINC)
 AC_SUBST(DPMSLIB)
])

AC_DEFUN([AC_HAVE_GL],
 [AC_REQUIRE_CPP()dnl
  AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])

 test -z "$GL_LDFLAGS" && GL_LDFLAGS=
 test -z "$GL_INCLUDE" && GL_INCLUDE=

 AC_ARG_WITH(gl, [  --without-gl            disable 3D GL modes],
	gl_test=$withval, gl_test="yes")
 if test "x$gl_test" = xno; then
   ac_cv_have_gl=no
 else
   AC_MSG_CHECKING(for GL)
   AC_CACHE_VAL(ac_cv_have_gl,
   [
    AC_LANG_C
    ac_save_ldflags="$LDFLAGS"
    ac_save_cflags="$CFLAGS"
    ac_gl_is_mesagl="yes"
    LDFLAGS="$LDFLAGS $GL_LDFLAGS $all_libraries -lMesaGL -lMesaGLU -lX11 -lXext -lm $LIBSOCKET"
    CFLAGS="$CFLAGS $X_INCLUDES"
    test ! -z "$GL_INCLUDE" && CFLAGS="-I$GL_INCLUDE $CFLAGS"
    AC_TRY_LINK([],[],
	ac_cv_have_gl="yes",ac_cv_have_gl="no")
    if test $ac_cv_have_gl = "no"; then
      ac_gl_is_mesagl="no"
      LDFLAGS="$ac_save_ldflags $GL_LDFLAGS $all_libraries -lGL -lGLU -lX11 -lXext -lm $LIBSOCKET"
      CFLAGS="$ac_save_cflags $X_INCLUDES"
      test ! -z "$GL_INCLUDE" && CFLAGS="-I$GL_INCLUDE $CFLAGS"
      AC_TRY_LINK([],[],
	  ac_cv_have_gl="yes",ac_cv_have_gl="no")
    fi
    LDFLAGS="$ac_save_ldflags"
    CFLAGS="$ac_save_cflags"
   ])dnl

  if test "$ac_cv_have_gl" = no; then
    AC_MSG_RESULT(no)
    GL_LDFLAGS=""
    GLINC=""
    $2
  else
    AC_DEFINE(HAVE_GL, 1, [Defines if you have GL (Mesa, OpenGL, ...)])
    if test "$GL_LDFLAGS" = ""; then
       if test "$ac_gl_is_mesagl" = "yes"; then
          GLLIB='-lMesaGL -lMesaGLU $(LIB_X11)'
       else
          GLLIB='-lGL -lGLU $(LIB_X11)'
       fi
    else
       if test "$ac_gl_is_mesagl" = "yes"; then
          GLLIB="$GL_LDFLAGS -lMesaGL -lMesaGLU "'$(LIB_X11)'
       else
          GLLIB="$GL_LDFLAGS -lGL -lGLU "'$(LIB_X11)'
       fi
    fi
    if test "$GL_INCLUDE" = ""; then
       GLINC=""
    else
       GLINC="-I$GL_INCLUDE"
    fi
    AC_MSG_RESULT(yes)
    $1
  fi
 fi
 AC_SUBST(GLINC)
 AC_SUBST(GLLIB)
])

 dnl PAM pam

 dnl Should test for PAM (Pluggable Authentication Modules)
 AC_DEFUN([AC_PATH_PAM_DIRECT],
 [
 test -z "$pam_direct_test_library" && pam_direct_test_library=pam
 test -z "$pam_direct_test_include" && pam_direct_test_include=security/pam_appl.h

   for ac_dir in               \
                               \
     /usr/local/include        \
     /usr/include              \
     /usr/unsupported/include  \
     /opt/include              \
     /usr/pam/include          \
     /usr/local/pam/include    \
     /usr/lib/pam/include      \
 			      \
     $extra_include            \
     ; \
   do
     if test -r "$ac_dir/$pam_direct_test_include"; then
       no_pam= ac_pam_includes=$ac_dir
       break
     fi
   done

 # First see if replacing the include by lib works.
 for ac_dir in `echo "$ac_pam_includes" | sed s/include/lib/` \
                           \
     /lib                  \
     /usr/lib              \
     /usr/local/lib        \
     /usr/unsupported/lib  \
     /lib/security         \
     /usr/security/lib     \
     $extra_lib            \
     ; \
 do
   for ac_extension in a so sl; do
     if test -r $ac_dir/lib${pam_direct_test_library}.$ac_extension; then
       no_pam= ac_pam_libraries=$ac_dir
       break 2
     fi
   done
 done
])

AC_DEFUN([AC_PATH_PAM],
 [
  AC_REQUIRE([KDE_CHECK_LIBDL])
  AC_REQUIRE_CPP()dnl

  AC_CHECK_LIB(pam_misc, main, [PAM_MISC_LIB="-lpam_misc"
               AC_DEFINE_UNQUOTED(HAVE_PAM_MISC, 1, [Define if you have a PAM implementation with the pam_misc library])], [], [-lpam $LIBDL])

 AC_MSG_CHECKING(for PAM)
 AC_ARG_WITH(pam,
[  --with-pam[=ARG]        enable support for PAM: ARG=[yes|no|service name]],
  [
    if test "x$withval" = "xyes"; then
      no_pam=
      default_pam=yes
    elif test "x$withval" = "xno"; then
      no_pam=yes
    else
      no_pam=
      pam_service="$withval"
        if test -z "$pam_service"; then
        default_pam=yes
        else
        default_pam=
        fi
      fi
  ], no_pam=yes
 )

 if test ! "$no_pam" = yes; then

 AC_CACHE_VAL(ac_cv_path_pam,
 [
 ac_pam_includes=NONE
 ac_pam_libraries=NONE
 if test -z "$pam_libraries"; then
   pam_libraries=NONE
 fi
 if test -z "$pam_includes"; then
   pam_includes=NONE
 fi

 AC_PATH_PAM_DIRECT

 test "x$pam_includes" = xNONE && pam_includes=$ac_pam_includes
 test "x$pam_libraries" = xNONE && pam_libraries=$ac_pam_libraries

 if test ! "x$pam_includes" = xNONE && test ! "x$pam_libraries" = xNONE; then
   ac_pam_libs="-lpam $PAM_MISC_LIB $LIBDL"
   ac_cv_path_pam="no_pam= ac_pam_includes=$ac_pam_includes ac_pam_libraries=$ac_pam_libraries ac_pam_libs=\"$ac_pam_libs\""
 else
   ac_cv_path_pam="no_pam=yes"
 fi
 ])

 eval "$ac_cv_path_pam"

 fi

 if test "$no_pam" = yes; then
   AC_MSG_RESULT(no)
 else
   AC_DEFINE(HAVE_PAM, 1, [Defines if you have PAM (Pluggable Authentication Modules); Redhat-Users!])
   PAMLIBS="$ac_pam_libs"
   test "x$pam_includes" = xNONE && pam_includes=$ac_pam_includes
   test "x$pam_libraries" = xNONE && pam_libraries=$ac_pam_libraries
   AC_MSG_RESULT([libraries $pam_libraries, headers $pam_includes])
 if test "$default_pam" = yes; then
   AC_MSG_RESULT(["default pam service name will be used"])
 else
   AC_DEFINE_UNQUOTED(KDE_PAM_SERVICE,"$pam_service", [Define to change the default name of the PAM service used by KDE])
   AC_MSG_RESULT(["pam service name will be: " $pam_service])
 fi
dnl test whether struct pam_message is const (Linux) or not (Sun)
   pam_appl_h="$ac_pam_includes/security/pam_appl.h"
   AC_MSG_CHECKING(for const pam_message)
   AC_EGREP_HEADER([struct pam_message],
      $pam_appl_h,
      [ AC_EGREP_HEADER([const struct pam_message],
                        $pam_appl_h,
                        [AC_MSG_RESULT(["const: Linux-type PAM"]) ],
                        [AC_MSG_RESULT(["nonconst: Sun-type PAM"])
                        AC_DEFINE(PAM_MESSAGE_NONCONST, 1, [Define if your PAM support takes non-const arguments (Solaris)])]
                        )],
       [AC_MSG_RESULT(["not found - assume const, Linux-type PAM"])]
       )
 fi

 if test "x$pam_libraries" != x && test "x$pam_libraries" != xNONE ; then
     PAMLIBPATHS="-L$pam_libraries"
 fi
 if test "x$pam_includes" != x && test "x$pam_includes" != xNONE ; then
     PAMINC="-I$pam_includes"
 fi

 AC_SUBST(PAMINC)
 AC_SUBST(PAMLIBS)
 AC_SUBST(PAMLIBPATHS)

])

AC_DEFUN([KDE_CHECK_LIBDL],
[
AC_CHECK_LIB(dl, dlopen, [
LIBDL="-ldl"
ac_cv_have_dlfcn=yes
])

AC_CHECK_LIB(dld, shl_unload, [
LIBDL="-ldld"
ac_cv_have_shload=yes
])

AC_SUBST(LIBDL)
])

AC_DEFUN([KDE_CHECK_DLOPEN],
[
KDE_CHECK_LIBDL
AC_CHECK_HEADERS(dlfcn.h dl.h)
if test "$ac_cv_header_dlfcn_h" = "no"; then
  ac_cv_have_dlfcn=no
fi

if test "$ac_cv_header_dl_h" = "no"; then
  ac_cv_have_shload=no
fi

enable_dlopen=no
AC_ARG_ENABLE(dlopen,
[  --disable-dlopen        link staticly [default=no]] ,
[if test "$enableval" = yes; then
  enable_dlopen=yes
fi],
enable_dlopen=yes)

# override the user's opinion, if we know it better ;)
if test "$ac_cv_have_dlfcn" = "no" && test "$ac_cv_have_shload" = "no"; then
  enable_dlopen=no
fi

if test "$ac_cv_have_dlfcn" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_DLFCN, 1, [Define if you have dlfcn])
fi

if test "$ac_cv_have_shload" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_SHLOAD, 1, [Define if you have shload])
fi

if test "$enable_dlopen" = no ; then
  test -n "$1" && eval $1
else
  test -n "$2" && eval $2
fi

])

AC_DEFUN([KDE_CHECK_DYNAMIC_LOADING],
[
KDE_CHECK_DLOPEN(libtool_enable_shared=yes, libtool_enable_static=no)
KDE_PROG_LIBTOOL
AC_MSG_CHECKING([dynamic loading])
eval "`egrep '^build_libtool_libs=' libtool`"
if test "$build_libtool_libs" = "yes" && test "$enable_dlopen" = "yes"; then
  dynamic_loading=yes
  AC_DEFINE_UNQUOTED(HAVE_DYNAMIC_LOADING)
else
  dynamic_loading=no
fi
AC_MSG_RESULT($dynamic_loading)
if test "$dynamic_loading" = "yes"; then
  $1
else
  $2
fi
])

AC_DEFUN([KDE_ADD_INCLUDES],
[
if test -z "$1"; then
  test_include="Pix.h"
else
  test_include="$1"
fi

AC_MSG_CHECKING([for libg++ ($test_include)])

AC_CACHE_VAL(kde_cv_libgpp_includes,
[
kde_cv_libgpp_includes=no

   for ac_dir in               \
                               \
     /usr/include/g++          \
     /usr/include              \
     /usr/unsupported/include  \
     /opt/include              \
     $extra_include            \
     ; \
   do
     if test -r "$ac_dir/$test_include"; then
       kde_cv_libgpp_includes=$ac_dir
       break
     fi
   done
])

AC_MSG_RESULT($kde_cv_libgpp_includes)
if test "$kde_cv_libgpp_includes" != "no"; then
  all_includes="-I$kde_cv_libgpp_includes $all_includes"
fi
])
])


AC_DEFUN([KDE_CHECK_MICO],
[
AC_REQUIRE([KDE_CHECK_LIBDL])
AC_REQUIRE([KDE_MISC_TESTS])
AC_MSG_CHECKING(for MICO)

if test -z "$MICODIR"; then
    kde_micodir=/usr/local
 else
    kde_micodir="$MICODIR"
fi

AC_ARG_WITH(micodir,
  [  --with-micodir=micodir  where mico is installed ],
  kde_micodir=$withval,
  kde_micodir=$kde_micodir
)

AC_CACHE_VAL(kde_cv_mico_incdir,
[
  mico_incdirs="$kde_micodir/include /usr/include /usr/local/include /usr/local/include /opt/local/include $kde_extra_includes"
AC_FIND_FILE(CORBA.h, $mico_incdirs, kde_cv_mico_incdir)

])
kde_micodir=`echo $kde_cv_mico_incdir | sed -e 's#/include##'`

if test ! -r  $kde_micodir/include/CORBA.h; then
  AC_MSG_ERROR([No CORBA.h found, specify another micodir])
fi

AC_MSG_RESULT($kde_micodir)

MICO_INCLUDES=-I$kde_micodir/include
AC_SUBST(MICO_INCLUDES)
MICO_LDFLAGS=-L$kde_micodir/lib
AC_SUBST(MICO_LDFLAGS)
micodir=$kde_micodir
AC_SUBST(micodir)

AC_MSG_CHECKING([for MICO version])
AC_CACHE_VAL(kde_cv_mico_version,
[
AC_LANG_C
cat >conftest.$ac_ext <<EOF
#include <stdio.h>
#include <mico/version.h>
int main() {

   printf("MICO_VERSION=%s\n",MICO_VERSION);
   return (0);
}
EOF
ac_compile='${CC-gcc} $CFLAGS $MICO_INCLUDES conftest.$ac_ext -o conftest'
if AC_TRY_EVAL(ac_compile); then
  if eval `./conftest 2>&5`; then
    kde_cv_mico_version=$MICO_VERSION
  else
    AC_MSG_ERROR([your system is not able to execute a small application to
    find MICO version! Check $kde_micodir/include/mico/version.h])
  fi
else
  AC_MSG_ERROR([your system is not able to compile a small application to
  find MICO version! Check $kde_micodir/include/mico/version.h])
fi
])

dnl installed MICO version
mico_v_maj=`echo $kde_cv_mico_version | sed -e 's/^\(.*\)\..*\..*$/\1/'`
mico_v_mid=`echo $kde_cv_mico_version | sed -e 's/^.*\.\(.*\)\..*$/\1/'`
mico_v_min=`echo $kde_cv_mico_version | sed -e 's/^.*\..*\.\(.*\)$/\1/'`

if test "x$1" = "x"; then
 req_version="2.3.0"
else
 req_version=$1
fi

dnl required MICO version
req_v_maj=`echo $req_version | sed -e 's/^\(.*\)\..*\..*$/\1/'`
req_v_mid=`echo $req_version | sed -e 's/^.*\.\(.*\)\..*$/\1/'`
req_v_min=`echo $req_version | sed -e 's/^.*\..*\.\(.*\)$/\1/'`

if test "$mico_v_maj" -lt "$req_v_maj" || \
   ( test "$mico_v_maj" -eq "$req_v_maj" && \
        test "$mico_v_mid" -lt "$req_v_mid" ) || \
   ( test "$mico_v_mid" -eq "$req_v_mid" && \
        test "$mico_v_min" -lt "$req_v_min" )

then
  AC_MSG_ERROR([found MICO version $kde_cv_mico_version but version $req_version \
at least is required. You should upgrade MICO.])
else
  AC_MSG_RESULT([$kde_cv_mico_version (minimum version $req_version, ok)])
fi

LIBMICO="-lmico$kde_cv_mico_version $LIBCRYPT $LIBSOCKET $LIBDL"
AC_SUBST(LIBMICO)
if test -z "$IDL"; then
  IDL='$(kde_bindir)/cuteidl'
fi
AC_SUBST(IDL)
IDL_DEPENDENCIES='$(kde_includes)/CUTE.h'
AC_SUBST(IDL_DEPENDENCIES)

idldir="\$(includedir)/idl"
AC_SUBST(idldir)

])

AC_DEFUN([KDE_CHECK_MINI_STL],
[
AC_REQUIRE([KDE_CHECK_MICO])

AC_MSG_CHECKING(if we use mico's mini-STL)
AC_CACHE_VAL(kde_cv_have_mini_stl,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
kde_save_cxxflags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $MICO_INCLUDES"
AC_TRY_COMPILE(
[
#include <mico/config.h>
],
[
#ifdef HAVE_MINI_STL
#error "nothing"
#endif
],
kde_cv_have_mini_stl=no,
kde_cv_have_mini_stl=yes)
CXXFLAGS="$kde_save_cxxflags"
AC_LANG_RESTORE
])

if test "x$kde_cv_have_mini_stl" = "xyes"; then
   AC_MSG_RESULT(yes)
   $1
else
   AC_MSG_RESULT(no)
   $2
fi
])

])


AC_DEFUN([KDE_CHECK_LIBPTHREAD],
[
AC_CHECK_LIB(pthread, pthread_create, [LIBPTHREAD="-lpthread"], LIBPTHREAD= )
AC_SUBST(LIBPTHREAD)
])

AC_DEFUN([KDE_TRY_LINK_PYTHON],
[
if test "$kde_python_link_found" = no; then

if test "$1" = normal; then
  AC_MSG_CHECKING(if a Python application links)
else
  AC_MSG_CHECKING(if Python depends on $2)
fi

AC_CACHE_VAL(kde_cv_try_link_python_$1,
[
AC_LANG_SAVE
AC_LANG_C
kde_save_cflags="$CFLAGS"
CFLAGS="$CFLAGS $PYTHONINC"
kde_save_libs="$LIBS"
LIBS="$LIBS $LIBPYTHON $2 $LIBDL $LIBSOCKET"
kde_save_ldflags="$LDFLAGS"
LDFLAGS="$LDFLAGS $PYTHONLIB"

AC_TRY_LINK(
[
#include <Python.h>
],[
	PySys_SetArgv(1, 0);
],
	[kde_cv_try_link_python_$1=yes],
	[kde_cv_try_link_python_$1=no]
)
CFLAGS="$kde_save_cflags"
LIBS="$kde_save_libs"
LDFLAGS="$kde_save_ldflags"
])

if test "$kde_cv_try_link_python_$1" = "yes"; then
  AC_MSG_RESULT(yes)
  kde_python_link_found=yes
  if test ! "$1" = normal; then
    LIBPYTHON="$LIBPYTHON $2"
  fi
  $3
else
  AC_MSG_RESULT(no)
  $4
fi
AC_LANG_RESTORE

fi

])


AC_DEFUN([KDE_CHECK_STL_SGI],
[
    AC_MSG_CHECKING([if STL implementation is SGI like])
    AC_CACHE_VAL(kde_cv_stl_type_sgi,
    [
      AC_TRY_COMPILE([
#include <string>
using namespace std;
],[
  string astring="Hallo Welt.";
  astring.erase(0, 6); // now astring is "Welt"
  return 0;
], kde_cv_stl_type_sgi=yes,
   kde_cv_stl_type_sgi=no)
])

   AC_MSG_RESULT($kde_cv_stl_type_sgi)

   if test "$kde_cv_stl_type_sgi" = "yes"; then
	AC_DEFINE_UNQUOTED(HAVE_SGI_STL, 1, [Define if you have a STL implementation by SGI])
   fi
])

AC_DEFUN([KDE_CHECK_STL_HP],
[
    AC_MSG_CHECKING([if STL implementation is HP like])
    AC_CACHE_VAL(kde_cv_stl_type_hp,
    [
      AC_TRY_COMPILE([
#include <string>
using namespace std;
],[
  string astring="Hello World";
  astring.remove(0, 6); // now astring is "World"
  return 0;
], kde_cv_stl_type_hp=yes,
   kde_cv_stl_type_hp=no)
])
   AC_MSG_RESULT($kde_cv_stl_type_hp)

   if test "$kde_cv_stl_type_hp" = "yes"; then
	AC_DEFINE_UNQUOTED(HAVE_HP_STL, 1, [Define if you have a STL implementation by SGI])
   fi
])

AC_DEFUN([KDE_CHECK_STL],
[
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    KDE_CHECK_STL_SGI

    if test "$kde_cv_stl_type_sgi" = "no"; then
       KDE_CHECK_STL_HP

       if test "$kde_cv_stl_type_hp" = "no"; then
         AC_MSG_ERROR("no known STL type found")
       fi
    fi

    AC_LANG_RESTORE
])

AC_DEFUN([AC_FIND_QIMGIO],
   [AC_REQUIRE([AC_FIND_JPEG])
AC_REQUIRE([SHOWEQ_CHECK_EXTRA_LIBS])
AC_MSG_CHECKING([for qimgio])
AC_CACHE_VAL(ac_cv_lib_qimgio,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
ac_save_LIBS="$LIBS"
ac_save_CXXFLAGS="$CXXFLAGS"
LIBS="$all_libraries -lqimgio -lpng -lz $LIBJPEG $LIBQT"
CXXFLAGS="$CXXFLAGS -I$qt_incdir $all_includes"
AC_TRY_RUN(dnl
[
#include <qimageio.h>
#include <qstring.h>
int main() {
		QString t = "hallo";
		t.fill('t');
		qInitImageIO();
}
],
            ac_cv_lib_qimgio=yes,
            ac_cv_lib_qimgio=no,
	    ac_cv_lib_qimgio=no)
LIBS="$ac_save_LIBS"
CXXFLAGS="$ac_save_CXXFLAGS"
AC_LANG_RESTORE
])dnl
if eval "test \"`echo $ac_cv_lib_qimgio`\" = yes"; then
  LIBQIMGIO="-lqimgio -lpng -lz $LIBJPEG"
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_QIMGIO, 1, [Define if you have the Qt extension qimgio available])
  AC_SUBST(LIBQIMGIO)
else
  AC_MSG_RESULT(not found)
fi
])

AC_DEFUN([KDE_CHECK_ANSI],
[
])

AC_DEFUN([KDE_CHECK_INSURE],
[
  AC_ARG_ENABLE(insure, [  --enable-insure             use insure++ for debugging [default=no]],
  [
  if test $enableval = "no"; dnl
	then ac_use_insure="no"
	else ac_use_insure="yes"
   fi
  ], [ac_use_insure="no"])

  AC_MSG_CHECKING(if we will use Insure++ to debug)
  AC_MSG_RESULT($ac_use_insure)
  if test "$ac_use_insure" = "yes"; dnl
       then CC="insure"; CXX="insure"; dnl CFLAGS="$CLAGS -fno-rtti -fno-exceptions "????
   fi
])

AC_DEFUN([AM_DISABLE_LIBRARIES],
[
    AC_PROVIDE([AM_ENABLE_STATIC])
    AC_PROVIDE([AM_ENABLE_SHARED])
    enable_static=no
    enable_shared=yes
])


AC_DEFUN([AC_CHECK_UTMP_FILE],
[
    AC_MSG_CHECKING([for utmp file])

    AC_CACHE_VAL(kde_cv_utmp_file,
    [
    kde_cv_utmp_file=no

    for ac_file in    \
                      \
	/var/run/utmp \
	/var/adm/utmp \
	/etc/utmp     \
     ; \
    do
     if test -r "$ac_file"; then
       kde_cv_utmp_file=$ac_file
       break
     fi
    done
    ])

    AC_MSG_RESULT($kde_cv_utmp_file)
    if test "$kde_cv_utmp_file" != "no"; then
	AC_DEFINE_UNQUOTED(UTMP, "$kde_cv_utmp_file", [Define the file for utmp entries])
    fi
])


AC_DEFUN([KDE_CREATE_SUBDIRSLIST],
[

DO_NOT_COMPILE="$DO_NOT_COMPILE CVS debian bsd-port admin"

if test ! -s $srcdir/inst-apps && test -s $srcdir/subdirs; then
 cp $srcdir/subdirs $srcdir/inst-apps
fi

if test ! -s $srcdir/inst-apps; then
  TOPSUBDIRS=""
  files=`cd $srcdir && ls -1`
  dirs=`for i in $files; do if test -d $i; then echo $i; fi; done`
  for i in $dirs; do
    echo $i >> $srcdir/subdirs
  done
  cp $srcdir/subdirs $srcdir/inst-apps
fi

if test -s $srcdir/inst-apps; then
  ac_topsubdirs="`cat $srcdir/inst-apps`"
fi

for i in $ac_topsubdirs; do
  AC_MSG_CHECKING([if $i should be compiled])
  if test -d $srcdir/$i; then
    install_it="yes"
    for j in $DO_NOT_COMPILE; do
      if test $i = $j; then
        install_it="no"
      fi
    done
  else
    install_it="no"
  fi
  AC_MSG_RESULT($install_it)
  if test $install_it = "yes"; then
    TOPSUBDIRS="$TOPSUBDIRS $i"
  fi
done

AC_SUBST(TOPSUBDIRS)
])

AC_DEFUN([KDE_CHECK_NAMESPACES],
[
AC_MSG_CHECKING(whether C++ compiler supports namespaces)
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE([
],
[
namespace Foo {
  extern int i;
  namespace Bar {
    extern int i;
  }
}

int Foo::i = 0;
int Foo::Bar::i = 1;
],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_NAMESPACES)
], [
AC_MSG_RESULT(no)
])
AC_LANG_RESTORE
])

AC_DEFUN([KDE_CHECK_NEWLIBS],
[

])

dnl ------------------------------------------------------------------------
dnl Check for S_ISSOCK macro. Doesn't exist on Unix SCO. faure@kde.org
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_CHECK_S_ISSOCK],
[
AC_MSG_CHECKING(for S_ISSOCK)
AC_CACHE_VAL(ac_cv_have_s_issock,
[
AC_LANG_SAVE
AC_LANG_C
AC_TRY_LINK(
[
#include <sys/stat.h>
],
[
struct stat buff;
int b = S_ISSOCK( buff.st_mode );
],
ac_cv_have_s_issock=yes,
ac_cv_have_s_issock=no)
AC_LANG_RESTORE
])
AC_MSG_RESULT($ac_cv_have_s_issock)
if test "$ac_cv_have_s_issock" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_S_ISSOCK, 1, [Define if sys/stat.h declares S_ISSOCK.])
fi
])

dnl ------------------------------------------------------------------------
dnl Check for MAXPATHLEN macro, defines KDEMAXPATHLEN. faure@kde.org
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_CHECK_KDEMAXPATHLEN],
[
AC_MSG_CHECKING(for MAXPATHLEN)
AC_CACHE_VAL(ac_cv_maxpathlen,
[
AC_LANG_C
cat > conftest.$ac_ext <<EOF
#include <stdio.h>
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

KDE_HELLO MAXPATHLEN

EOF

ac_try="$ac_cpp conftest.$ac_ext 2>/dev/null | grep '^KDE_HELLO' >conftest.out"

if AC_TRY_EVAL(ac_try) && test -s conftest.out; then
    ac_cv_maxpathlen=`sed 's#KDE_HELLO ##' conftest.out`
else
    ac_cv_maxpathlen=1024
fi

rm conftest.*

])
AC_MSG_RESULT($ac_cv_maxpathlen)
AC_DEFINE_UNQUOTED(KDEMAXPATHLEN,$ac_cv_maxpathlen, [Define a safe value for MAXLENPATH] )
])

dnl -------------------------------------------------------------------------
dnl See if the compiler supports a template repository         bero@redhat.de
dnl -------------------------------------------------------------------------
AC_DEFUN([KDE_COMPILER_REPO],
[
  REPO=""
  NOREPO=""

  KDE_CHECK_COMPILER_FLAG(frepo,
   [
     REPO="-frepo"
     NOREPO="-fno-repo"
   ])

  if test -z "$REPO"; then
  KDE_CHECK_COMPILER_FLAG(instances=explicit,
  [
     REPO="-instances=explicit"
     NOREPO="-instances=extern"
  ])
  fi

  if test -n "$REPO"; then
     AC_DEFINE_UNQUOTED(HAVE_TEMPLATE_REPOSITORY, 1, 
		[C++ compiler supports template repository])
     $1
  fi

  AC_SUBST(REPO)
  AC_SUBST(NOREPO)
])

AC_DEFUN([KDE_CHECK_HEADER],
[
   AC_LANG_SAVE
   kde_safe_cxxflags=$CXXFLAGS
   CXXFLAGS="$CXXFLAGS $all_includes"
   AC_LANG_CPLUSPLUS
   AC_CHECK_HEADER($1, $2, $3)
   CXXFLAGS=$kde_safe_cxxflags
   AC_LANG_RESTORE
])

AC_DEFUN([KDE_CHECK_QWSPRITEFIELD],
[
  KDE_CHECK_HEADER(QwSpriteField.h, ,
  [
    AC_MSG_WARN([you don't have QwSpriteField.h somewhere. Please install
       QwSpriteField out of kdesupport.])
      $1
  ])
])

AC_DEFUN([KDE_SET_PREFIX],
[
  dnl make $KDEDIR the default for the installation
  AC_PREFIX_DEFAULT(${KDEDIR:-/usr/local/kde})

  if test "x$prefix" = "xNONE"; then
    prefix=$ac_default_prefix
    ac_configure_args="$ac_configure_args --prefix $prefix"
  fi

])
