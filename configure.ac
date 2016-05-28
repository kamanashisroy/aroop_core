# please refer to https://wiki.gnome.org/Projects/Vala/GameDevelopmentSeries/Setup

m4_define([aroop_core_major], [0])
m4_define([aroop_core_minor], [2])
m4_define([aroop_core_micro], [0])

m4_define([aroop_core_version], [aroop_core_major.aroop_core_minor.aroop_core_micro])
m4_define([aroop_core_suffix], [-aroop_core_version])
m4_define([aroop_core_api_version], [0.0])

m4_define([aroop_core_name], [aroop_core])

AC_PREREQ([2.64])
AC_INIT([AROOP_CORE], [aroop_core_version], [kamanashisroy@github.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE
AM_PATH_CHECK()

AROOP_CORE_MAJOR_VERSION=aroop_core_major
AROOP_CORE_MINOR_VERSION=aroop_core_minor
AROOP_CORE_MICRO_VERSION=aroop_core_micro
AROOP_CORE_VERSION=aroop_core_version
AROOP_CORE_API_VERSION=aroop_core_api_version
PACKAGE_SUFFIX=aroop_core_suffix
AC_SUBST(PACKAGE_SUFFIX)
AC_SUBST(AROOP_CORE_MAJOR_VERSION)
AC_SUBST(AROOP_CORE_MINOR_VERSION)
AC_SUBST(AROOP_CORE_MICRO_VERSION)
AC_SUBST(AROOP_CORE_VERSION)
AC_SUBST(AROOP_CORE_API_VERSION)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_RANLIB


AC_CONFIG_FILES([
		factory/Makefile
		factory/tests/Makefile
		Makefile
		aroop_core.pc
           ])

AC_OUTPUT
