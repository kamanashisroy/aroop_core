TESTS = check_opp_factory_create
check_PROGRAMS = check_opp_factory_create
AROOP_CORE_INCLUDES=-I$(top_builddir)/aroop -I$(top_builddir)/factory/inc
check_opp_factory_create_SOURCES = check_opp_factory_create.c
check_opp_factory_create_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_factory_create_LDADD = $(top_builddir)/factory/libopp_factory.la @CHECK_LIBS@
