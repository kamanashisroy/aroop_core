
SUBDIRS = tests

TARGET_LAR=libopp_collection.la libopp_debug_collection.la libopp_profiled_collection.la
noinst_LTLIBRARIES=$(TARGET_LAR)

OPP_COLLECTION_SOURCE_FILES = \
src/opp_any_obj.c           src/opp_list.c             src/opp_salt.c         src/opp_watchdog.c \
src/opp_queue.c            src/opp_str2.c \
src/opp_factory_profiler.c

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -Iinc

# opp_collection
libopp_collection_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_collection_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)
libopp_collection_la_LDFLAGS = --static

# opp_debug_collection
libopp_debug_collection_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_debug_collection_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_DEBUG_CFLAG)
libopp_debug_collection_la_LDFLAGS = --static

# opp_profiled_collection
libopp_profiled_collection_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_profiled_collection_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_PROFILED_CFLAG)
libopp_profiled_collection_la_LDFLAGS = --static


