
SUBDIRS = tests

TARGET_LAR=libopp_lookup.la libopp_debug_lookup.la libopp_profiled_lookup.la
noinst_LTLIBRARIES=$(TARGET_LAR)

OPP_COLLECTION_SOURCE_FILES = \
src/opp_hash.c	\
src/opp_hash_table.c	\
src/opp_hash_otable.c	\
src/opp_hash_ctable.c	\
src/opp_indexed_list.c

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -Iinc

# opp_lookup
libopp_lookup_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_lookup_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)
libopp_lookup_la_LDFLAGS = --static

# opp_debug_lookup
libopp_debug_lookup_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_debug_lookup_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_DEBUG_CFLAG)
libopp_debug_lookup_la_LDFLAGS = --static

# opp_profiled_lookup
libopp_profiled_lookup_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_profiled_lookup_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_PROFILED_CFLAG)
libopp_profiled_lookup_la_LDFLAGS = --static


