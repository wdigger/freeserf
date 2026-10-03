#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([freeserf], [0.1], [https://github.com/freeserf/freeserf/issues])
AC_CONFIG_SRCDIR([src/freeserf.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES([SDL3], [sdl3])
PKG_CHECK_MODULES([SDL3_mixer], [sdl3-mixer], [have_sdl3_mixer=yes], [have_sdl3_mixer=no])

# Checks for header files.
AC_HEADER_ASSERT
AC_HEADER_STDC
AC_CHECK_HEADERS([byteswap.h endian.h fcntl.h stdint.h stdlib.h string.h unistd.h sys/endian.h sys/mman.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN
AC_SYS_LARGEFILE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([atexit memset munmap strtol])

# Check SDL3_mixer support
AC_MSG_CHECKING([whether to enable SDL3_mixer support])
AC_ARG_ENABLE([sdl3-mixer], [AC_HELP_STRING([--enable-sdl3-mixer],
			   [enable SDL3_mixer support (music/sound effects)])],
			   [enable_sdl3_mixer=$enableval], [enable_sdl3_mixer=maybe])
AS_IF([test "x$enable_sdl3_mixer" != xno], [
	AS_IF([test $have_sdl3_mixer = yes], [
		AC_DEFINE([ENABLE_SDL3_MIXER], 1, [Define to 1 to enable SDL3_mixer support])
		AC_MSG_RESULT([yes])
		enable_sdl3_mixer=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_sdl3_mixer" = xyes], [
			AC_MSG_ERROR([missing dependencies for SDL3_mixer])
		])
		enable_sdl3_mixer=no
	])
], [
	AC_MSG_RESULT([no])
	enable_sdl3_mixer=no
])
AM_CONDITIONAL([ENABLE_SDL3_MIXER], [test "x$enable_sdl3_mixer" = xyes])

# Check debug mode
AC_MSG_CHECKING([whether to enable debug mode])
debug_default="no"
AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
	[enable debug mode])],,[enable_debug=$debug_default])
AS_IF([test "x$enable_debug" != xno], [
	CFLAGS="$CFLAGS -pedantic -Wall -ggdb"
	CXXFLAGS="$CXXFLAGS -ggdb -Wall"
	AC_SUBST([CFLAGS])
	AC_SUBST([CXXFLAGS])
	AC_MSG_RESULT([yes])
	], [
	CFLAGS="$CFLAGS -DNDEBUG"
	CXXFLAGS="$CXXFLAGS -DNDEBUG"
	AC_SUBST([CFLAGS])
	AC_SUBST([CXXFLAGS])
	AC_MSG_RESULT([no])])

# Check profiling mode
AC_MSG_CHECKING([whether to enable profile mode])
profile_default="no"
AC_ARG_ENABLE([profile], [AC_HELP_STRING([--enable-profile],
	[enable profile mode])],,[enable_profile=$profile_default])
AS_IF([test "x$enable_profile" != xno], [
	CFLAGS="$CFLAGS -p"
	CXXFLAGS="$CXXFLAGS -p"
	LDFLAGS="$LDFLAGS -pg"
	AC_SUBST([CFLAGS])
	AC_SUBST([CXXFLAGS])
	AC_MSG_RESULT([yes])
	], [
	AC_MSG_RESULT([no])])

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT


echo "
 $PACKAGE_NAME $VERSION

    prefix:		${prefix}
    compiler:		${CC}
    compiler (c++):	${CXX}
    cflags:		${CFLAGS}
    cxxflags:		${CXXFLAGS}
    ldflags:		${LDFLAGS}

    SDL3_mixer:		${enable_sdl3_mixer}
"
