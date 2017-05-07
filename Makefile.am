ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libunlucky.la
libunlucky_la_SOURCES = src/unlucky_time.c src/override.c src/utils.c
libunlucky_la_LIBADD = -lbsd -ldl
libunlucky_la_CFLAGS = -g -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY -D OVERRIDE_TIME

TESTS = check_unlucky check_override
check_PROGRAMS = check_unlucky check_override

check_unlucky_SOURCES = ./tests/check_unlucky.c $(top_builddir)/src/unlucky_time.h
check_unlucky_CFLAGS = @CHECK_CFLAGS@
check_unlucky_LDADD = $(top_builddir)/.libs/libunlucky.la @CHECK_LIBS@

check_override_SOURCES = ./tests/check_override.c $(top_builddir)/src/unlucky_time.h
check_override_CFLAGS = @CHECK_CFLAGS@
check_override_LDADD = $(top_builddir)/.libs/libunlucky.la @CHECK_LIBS@
