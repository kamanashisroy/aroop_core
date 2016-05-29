
SUBDIRS = tests

TARGET_BIN=
#TARGET_HAS_THREAD_O=libaroop_core_has_thread.o
TARGET_LAR=libaroop_core.la
TARGET_AR=libaroop_core_static.a libaroop_core_debug.a libaroop_core_nginz.a
bin_PROGRAMS=$(TARGET_O) $(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)

AROOP_CORE_SOURCE_FILES = \
src/aroop_core.c	src/aroop_factory.c	src/aroop_txt.c

OPP_COLLECTION_LIBS=$(top_builddir)/factory/libopp_factory.la $(top_builddir)/collection/libopp_collection.la
OPP_DEBUG_COLLECTION_LIBS=$(top_builddir)/factory/libopp_debug_factory.la $(top_builddir)/collection/libopp_debug_collection.la
OPP_PROFILED_COLLECTION_LIBS=$(top_builddir)/factory/libopp_profiled_factory.la $(top_builddir)/collection/libopp_profiled_collection.la

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes/ -Ifactory/inc

libaroop_core_la_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(OPP_COLLECTION_LIBS) $(AROOP_OPT_CFLAG)
libaroop_core_static_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_static_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_OPT_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_static_a_LIBADD = $(OPP_COLLECTION_LIBS)
libaroop_core_debug_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_debug_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_DEBUG_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_debug_a_LIBADD = $(OPP_DEBUG_COLLECTION_LIBS)
libaroop_core_nginz_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_nginz_a_CPPFLAGS= $(AROOP_CORE_CFLAGS) $(AROOP_PROFILED_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_nginz_a_LIBADD = $(OPP_PROFILED_COLLECTION_LIBS)
#libaroop_core_has_thread_o_SOURCES = $(AROOP_CORE_SOURCE_FILES)
#libaroop_core_has_thread_o_CPPFLAGS= $(AROOP_CORE_CFLAGS)-DHAS_THREAD $(AROOP_CORE_INCLUDES)

