
SUBDIRS = tests

TARGET_AR=libopp_factory.a libopp_debug_factory.a libopp_profiled_factory.a
TARGET_LAR=libopp_factory.la libopp_debug_factory.la libopp_profiled_factory.la
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)

OPP_FACTORY_SOURCE_FILES = \
src/opp_factory_create.c  src/opp_factory_part_alloc.c  src/opp_factory_part_iterator.c  src/opp_factory_part_property.c src/opp_rbtree_internal.c

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -Iinc

# opp_factory
libopp_factory_la_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_factory_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_OPT_CFLAG)
libopp_factory_a_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_factory_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_OPT_CFLAG)

# opp_debug_factory
libopp_debug_factory_la_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_debug_factory_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_DEBUG_CFLAG)
libopp_debug_factory_a_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_debug_factory_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_DEBUG_CFLAG)

# opp_profiled_factory
libopp_profiled_factory_la_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_profiled_factory_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_PROFILED_CFLAG)
libopp_profiled_factory_a_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_profiled_factory_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(AROOP_PROFILED_CFLAG)



