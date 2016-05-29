
SUBDIRS = tests

TARGET_AR=libopp_factory.a
TARGET_LAR=libopp_factory.la
lib_LTLIBRARIES=$(TARGET_LAR)
lib_LIBRARIES=$(TARGET_AR)

OPP_FACTORY_SOURCE_FILES = \
src/opp_factory_create.c  src/opp_factory_part_alloc.c  src/opp_factory_part_iterator.c  src/opp_factory_part_property.c src/opp_rbtree_internal.c

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -Iinc
libopp_factory_la_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_factory_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)
libopp_factory_a_SOURCES = $(OPP_FACTORY_SOURCE_FILES)
libopp_factory_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES)

