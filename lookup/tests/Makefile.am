TESTS = check_opp_hash_otable
check_PROGRAMS = check_opp_hash_otable


AROOP_CORE_INCLUDES=-I$(top_builddir)/includes
OPP_COLLECTION_LIBS=$(top_builddir)/factory/libopp_factory.la $(top_builddir)/collection/libopp_collection.la

check_opp_hash_otable_SOURCES = check_opp_hash_otable.c
check_opp_hash_otable_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_hash_otable_LDADD = $(OPP_COLLECTION_LIBS) @CHECK_LIBS@


