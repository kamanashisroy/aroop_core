
SUBDIRS = tests

#TARGET_HAS_THREAD_O=libaroop_core_has_thread.o
TARGET_LAR=libaroop_core_static.la libaroop_core_debug.la libaroop_core_profiled.la
lib_LTLIBRARIES=$(TARGET_LAR)

AROOP_CORE_SOURCE_FILES = \
src/aroop_core.c	src/aroop_factory.c	src/aroop_txt.c

#OPP_COLLECTION_LIBS=-Wl,--whole-archive $(top_builddir)/factory/libopp_factory.a $(top_builddir)/collection/libopp_collection.a  -Wl,--no-whole-archive
#OPP_DEBUG_COLLECTION_LIBS=-Wl,--whole-archive $(top_builddir)/factory/libopp_debug_factory.a $(top_builddir)/collection/libopp_debug_collection.a -Wl,--no-whole-archive
#OPP_PROFILED_COLLECTION_LIBS=-Wl,--whole-archive $(top_builddir)/factory/libopp_profiled_factory.a $(top_builddir)/collection/libopp_profiled_collection.a -Wl,--no-whole-archive

OPP_COLLECTION_LIBS=$(top_builddir)/factory/libopp_factory.la $(top_builddir)/collection/libopp_collection.la $(top_builddir)/lookup/libopp_lookup.la
OPP_DEBUG_COLLECTION_LIBS=$(top_builddir)/factory/libopp_debug_factory.la $(top_builddir)/collection/libopp_debug_collection.la $(top_builddir)/lookup/libopp_debug_lookup.la
OPP_PROFILED_COLLECTION_LIBS=$(top_builddir)/factory/libopp_profiled_factory.la $(top_builddir)/collection/libopp_profiled_collection.la $(top_builddir)/lookup/libopp_profiled_lookup.la

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes/ -Ifactory/inc

# static linking
libaroop_core_static_la_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_static_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_OPT_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_static_la_LDFLAGS = --static $(OPP_COLLECTION_LIBS)

# debug linking
libaroop_core_debug_la_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_debug_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_DEBUG_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_debug_la_LDFLAGS = --static $(OPP_DEBUG_COLLECTION_LIBS)

# profiled linking
libaroop_core_profiled_la_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_profiled_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_PROFILED_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_profiled_la_LDFLAGS = --static $(OPP_PROFILED_COLLECTION_LIBS)


#libaroop_core_has_thread_o_SOURCES = $(AROOP_CORE_SOURCE_FILES)
#libaroop_core_has_thread_o_CPPFLAGS= $(AROOP_CORE_CFLAGS)-DHAS_THREAD $(AROOP_CORE_INCLUDES)

