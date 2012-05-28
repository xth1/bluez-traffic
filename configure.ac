AC_PREREQ(2.50)
AC_INIT()

AM_INIT_AUTOMAKE(bluez-traffic, 0.1)
AM_CONFIG_HEADER(config.h)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr/local)

AC_DEFINE_UNQUOTED(LOCALEDIR, "${prefix}/share/locale",
				[Directory for the localization files])

COMPILER_FLAGS

AC_LANG_C

AC_PROG_CC
AC_PROG_CC_PIE
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug],
			[enable compiling with debugging information]), [
	if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_g}" = "yes"); then
		CFLAGS="$CFLAGS -g -O0 -DDEBUG"
	fi
])

AC_ARG_ENABLE(pie, AC_HELP_STRING([--enable-pie],
			[enable position independent executables flag]), [
	if (test "${enableval}" = "yes" &&
				test "${ac_cv_prog_cc_pie}" = "yes"); then
		CFLAGS="$CFLAGS -fPIE"
		LDFLAGS="$LDFLAGS -pie"
	fi
])

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.8, dummy=yes,
				AC_MSG_ERROR(gtk+ >= 2.8 is required))

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0, dummy=yes,
				AC_MSG_ERROR(glib >= 2.0 is required))
PKG_CHECK_MODULES(BLUEZ, bluez >= 4.99, dummy=yes,
				AC_MSG_ERROR(bluez >= 4.99 is required))

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_OUTPUT(Makefile src/Makefile)
