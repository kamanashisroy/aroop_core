
pkgincludedir=$(includedir)/aroop_core@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
aroop/aroop_array.h       aroop/aroop_core.h                  aroop/aroop_core_type_info.h aroop/aroop_int_type.h  aroop/aroop_factory.h \
aroop/aroop_assignment.h  aroop/aroop_core_type_conversion.h  aroop/aroop_error.h           aroop/aroop_memory_profiler.h  \
aroop/opp/opp_any_obj.h  aroop/opp/opp_factory_profiler.h  aroop/opp/opp_hash.h        aroop/opp/opp_indexed_list.h  aroop/opp/opp_iterator.h  aroop/opp/opp_queue.h            aroop/opp/opp_salt.h  aroop/opp/opp_thread_main.h  aroop/opp/opp_watchdog.h \
aroop/opp/opp_factory.h  aroop/opp/opp_factory_types.h     aroop/opp/opp_hash_table.h  aroop/opp/opp_hash_otable.h	aroop/opp/opp_io.h            aroop/opp/opp_list.h      aroop/opp/opp_rbtree_internal.h  aroop/opp/opp_str2.h  aroop/opp/opp_type.h \
aroop/core/config.h  aroop/core/decorator.h  aroop/core/logger.h  aroop/core/memory.h  aroop/core/thread.h  aroop/core/xtring.h \
aroop/platform/linux/linux_config.h  aroop/platform/linux/linux_graphics.h  aroop/platform/linux/linux_logger.h \
aroop/platform/raspberrypi/raspberry_pi_bare_metal_config.h aroop/platform/raspberrypi/raspberry_pi_bare_metal_logger.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = aroop_core@PACKAGE_SUFFIX@.pc

aroop_core@PACKAGE_SUFFIX@.pc: aroop_core.pc
	cp $< $@



#MAINTAINERCLEANFILES = aroop_core.pc
