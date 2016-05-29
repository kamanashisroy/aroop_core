
SUBDIRS = tests

TARGET_AR=libopp_collection.a
TARGET_LAR=libopp_collection.la
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)

OPP_COLLECTION_SOURCE_FILES = \
src/opp_any_obj.c           src/opp_hash.c          src/opp_list.c             src/opp_salt.c         src/opp_watchdog.c \
src/opp_hash_table.c    src/opp_queue.c            src/opp_str2.c \
src/opp_factory_profiler.c  src/opp_indexed_list.c

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -Iinc
libopp_collection_la_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_collection_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)
libopp_collection_a_SOURCES = $(OPP_COLLECTION_SOURCE_FILES)
libopp_collection_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)

