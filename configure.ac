AC_PREREQ(2.50)

m4_define([displayconfig_major_version], [3])
m4_define([displayconfig_minor_version], [10])
m4_define([displayconfig_micro_version], [2])

m4_define([displayconfig_version],
          [displayconfig_major_version.displayconfig_minor_version.displayconfig_micro_version])

AC_INIT([displayconfig], [displayconfig_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=mutter])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/monitor.c)
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],)
AM_MAINTAINER_MODE([enable])

DISPLAYCONFIG_MAJOR_VERSION=displayconfig_major_version
DISPLAYCONFIG_MINOR_VERSION=displayconfig_minor_version
DISPLAYCONFIG_MICRO_VERSION=displayconfig_micro_version
AC_SUBST(DISPLAYCONFIG_MAJOR_VERSION)
AC_SUBST(DISPLAYCONFIG_MINOR_VERSION)
AC_SUBST(DISPLAYCONFIG_MICRO_VERSION)

# Honor aclocal flags
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

GETTEXT_PACKAGE=mutter
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Name of default gettext domain])

#IT_PROG_INTLTOOL([0.34.90])
AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

# Sets GLIB_GENMARSHAL and GLIB_MKENUMS
AM_PATH_GLIB_2_0()

#### Integer sizes

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(__int64)

## byte order
AC_C_BIGENDIAN

DISPLAYCONFIG_PC_MODULES="
   gtk+-3.0 >= 3.9.11
   gio-2.0 >= 2.25.10
   xcomposite >= 0.2 xfixes xrender xdamage xi >= 1.6.0
   upower-glib > 0.9.11
   gnome-desktop-3.0
"

GLIB_GSETTINGS

AC_ARG_ENABLE(verbose-mode,
  AC_HELP_STRING([--disable-verbose-mode],
                 [disable mutter's ability to do verbose logging, for embedded/size-sensitive custom builds]),,
  enable_verbose_mode=yes)

if test x$enable_verbose_mode = xyes; then
    AC_DEFINE(WITH_VERBOSE_MODE,1,[Build with verbose mode support])
fi

AC_ARG_ENABLE(xsync,
  AC_HELP_STRING([--disable-xsync],
                 [disable mutter's use of the XSync extension]),,
  enable_xsync=auto)

AM_GLIB_GNU_GETTEXT

## here we get the flags we'll actually use
# GRegex requires Glib-2.14.0
PKG_CHECK_MODULES(ALL, glib-2.0 >= 2.14.0)

PKG_CHECK_MODULES(DISPLAYCONFIG, $DISPLAYCONFIG_PC_MODULES)

PKG_CHECK_EXISTS([xi >= 1.6.99.1],
                 AC_DEFINE([HAVE_XI23],[1],[Define if you have support for XInput 2.3 or greater]))

AC_PATH_XTRA

ALL_X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"

RANDR_LIBS=
found_randr=no
AC_CHECK_LIB(Xrandr, XRRUpdateConfiguration,
               [AC_CHECK_HEADER(X11/extensions/Xrandr.h,
                                RANDR_LIBS=-lXrandr found_randr=yes,,
				[#include <X11/Xlib.h>])],
               , -lXrender -lXext $ALL_X_LIBS)

if test "x$found_randr" = "xyes"; then
   AC_DEFINE(HAVE_RANDR, , [Have the Xrandr extension library])
fi

XSYNC_LIBS=
found_xsync=no
AC_CHECK_LIB(Xext, XSyncQueryExtension,
               [AC_CHECK_HEADER(X11/extensions/sync.h,
                                found_xsync=yes,,
				[#include <X11/Xlib.h>])],
               , $ALL_X_LIBS)

if test x$enable_xsync = xno; then
   found_xsync=no
fi

if test x$enable_xsync = xyes; then
   if test "$found_xsync" = "no"; then
      AC_MSG_ERROR([--enable-xsync forced and XSync not found])
      exit 1
   fi
fi

if test "x$found_xsync" = "xyes"; then
   XSYNC_LIBS=-lXext
   AC_DEFINE(HAVE_XSYNC, , [Have the Xsync extension library])
fi

DISPLAYCONFIG_LIBS="$DISPLAYCONFIG_LIBS $XSYNC_LIBS $RANDR_LIBS $SHAPE_LIBS $X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS -lm"

AC_ARG_ENABLE(debug,
	[  --enable-debug		enable debugging],,
	enable_debug=no)
if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -g -O"
fi

GTK_DOC_CHECK([1.15], [--flavour no-tmpl])

#### Warnings (last since -Werror can disturb other tests)

# Stay command-line compatible with the gnome-common configure option. Here
# minimum/yes/maximum are the same, however.
AC_ARG_ENABLE(compile_warnings,
  AS_HELP_STRING([--enable-compile-warnings=@<:@no/minimum/yes/maximum/error@:>@],[Turn on compiler warnings]),,
  enable_compile_warnings=error)

changequote(,)dnl
if test "$enable_compile_warnings" != no ; then
  if test "x$GCC" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-Wall[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wall" ;;
    esac

#   case " $CFLAGS " in
#   *[\ \	]-Wshadow[\ \	]*) ;;
#   *) CFLAGS="$CFLAGS -Wshadow" ;;
#   esac

    case " $CFLAGS " in
    *[\ \	]-Wchar-subscripts[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wchar-subscripts" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wmissing-declarations[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wmissing-declarations" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wmissing-prototypes[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wmissing-prototypes" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wnested-externs[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wnested-externs" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wpointer-arith[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wpointer-arith" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wcast-align[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wcast-align" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-Wsign-compare[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wsign-compare" ;;
    esac

    if test "$enable_compile_warnings" = error; then
      case " $CFLAGS " in
      *[\ \	]-Werror[\ \	]*) ;;
      *) CFLAGS="$CFLAGS -Werror -Wno-error=deprecated-declarations" ;;
      esac
    fi
  fi
fi
changequote([,])dnl

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
])

AC_OUTPUT

if test x$enable_verbose_mode = xno; then
        echo "*** WARNING WARNING WARNING WARNING WARNING"
        echo "*** Building without verbose mode"
        echo "*** This means there's no way to debug mutter problems."
        echo "*** Please build normal desktop versions of mutter"
        echo "*** with verbose mode enabled so users can use it when they report bugs."
fi

dnl ==========================================================================
echo "
mutter-$VERSION

	prefix:                   ${prefix}
	source code location:	  ${srcdir}
	compiler:		  ${CC}

	Xsync:                    ${found_xsync}
"


DISPLAYCONFIG_MINOR_VERSION=displayconfig_minor_version
if expr $DISPLAYCONFIG_MINOR_VERSION % 2 > /dev/null ; then
   stable_version=`expr $DISPLAYCONFIG_MINOR_VERSION - 1`
   echo "This is the UNSTABLE branch of mutter"
   echo -n "Use 3.$stable_version.x for stable "
   echo    "(gnome-3-$stable_version branch in git)"
else
   echo "This is the stable branch of mutter"
fi
