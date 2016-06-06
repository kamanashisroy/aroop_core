TESTS = check_opp_hash_otable check_opp_hash_ctable
check_PROGRAMS = check_opp_hash_otable check_opp_hash_ctable


AROOP_CORE_INCLUDES=-I$(top_builddir)/includes
OPP_COLLECTION_LIBS=$(top_builddir)/factory/libopp_debug_factory.la $(top_builddir)/collection/libopp_debug_collection.la $(top_builddir)/lookup/libopp_debug_lookup.la

check_opp_hash_otable_SOURCES = check_opp_hash_otable.c
check_opp_hash_otable_CFLAGS = -ggdb3 @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_hash_otable_LDADD = $(OPP_COLLECTION_LIBS) @CHECK_LIBS@

check_opp_hash_ctable_SOURCES = check_opp_hash_ctable.c
check_opp_hash_ctable_CFLAGS = -ggdb3 @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_hash_ctable_LDADD = $(OPP_COLLECTION_LIBS) @CHECK_LIBS@


