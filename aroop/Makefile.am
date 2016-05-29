
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

pkgincludedir=$(includedir)/aroop_core@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
aroop/aroop_array.h       aroop/aroop_core.h                  aroop/aroop_core_type_info.h aroop/aroop_int_type.h  aroop/aroop_factory.h \
aroop/aroop_assignment.h  aroop/aroop_core_type_conversion.h  aroop/aroop_error.h           aroop/aroop_memory_profiler.h  \
aroop/opp/opp_any_obj.h  aroop/opp/opp_factory_profiler.h  aroop/opp/opp_hash.h        aroop/opp/opp_indexed_list.h  aroop/opp/opp_iterator.h  aroop/opp/opp_queue.h            aroop/opp/opp_salt.h  aroop/opp/opp_thread_main.h  aroop/opp/opp_watchdog.h \
aroop/opp/opp_factory.h  aroop/opp/opp_factory_types.h     aroop/opp/opp_hash_table.h  aroop/opp/opp_io.h            aroop/opp/opp_list.h      aroop/opp/opp_rbtree_internal.h  aroop/opp/opp_str2.h  aroop/opp/opp_type.h \
aroop/core/config.h  aroop/core/decorator.h  aroop/core/logger.h  aroop/core/memory.h  aroop/core/thread.h  aroop/core/xtring.h \
aroop/platform/linux/linux_config.h  aroop/platform/linux/linux_graphics.h  aroop/platform/linux/linux_logger.h \
aroop/platform/raspberrypi/raspberry_pi_bare_metal_config.h aroop/platform/raspberrypi/raspberry_pi_bare_metal_logger.h

AROOP_CORE_INCLUDES=-I$(top_builddir)/includes/ -Ifactory/inc

libaroop_core_la_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_la_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_CORE_INCLUDES) $(OPP_COLLECTION_LIBS)
libaroop_core_static_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_static_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_OPT_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_static_a_LIBADD = $(OPP_COLLECTION_LIBS)
libaroop_core_debug_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_debug_a_CPPFLAGS = $(AROOP_CORE_CFLAGS) $(AROOP_DEBUG_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_debug_a_LIBADD = $(OPP_DEBUG_COLLECTION_LIBS)
libaroop_core_nginz_a_SOURCES = $(AROOP_CORE_SOURCE_FILES)
libaroop_core_nginz_a_CPPFLAGS= $(AROOP_CORE_CFLAGS) $(AROOP_PROFILER_CFLAG) $(AROOP_CORE_INCLUDES)
libaroop_core_nginz_a_LIBADD = $(OPP_PROFILED_COLLECTION_LIBS)
#libaroop_core_has_thread_o_SOURCES = $(AROOP_CORE_SOURCE_FILES)
#libaroop_core_has_thread_o_CPPFLAGS= $(AROOP_CORE_CFLAGS)-DHAS_THREAD $(AROOP_CORE_INCLUDES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = aroop_core@PACKAGE_SUFFIX@.pc

aroop_core@PACKAGE_SUFFIX@.pc: aroop_core.pc
	cp $< $@

#MAINTAINERCLEANFILES = aroop_core.pc
