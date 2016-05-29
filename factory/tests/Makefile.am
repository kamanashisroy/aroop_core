TESTS = check_opp_factory_create check_opp_factory_part_alloc check_avl_tree_partition_test
check_PROGRAMS = check_opp_factory_create check_opp_factory_part_alloc check_avl_tree_partition_test


AROOP_CORE_INCLUDES=-I$(top_builddir)/includes -I$(top_builddir)/factory/inc
OPP_FACTORY_LIBS=$(top_builddir)/factory/libopp_factory.la

check_opp_factory_create_SOURCES = check_opp_factory_create.c
check_opp_factory_create_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_factory_create_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@


check_opp_factory_part_alloc_SOURCES = check_opp_factory_part_alloc.c
check_opp_factory_part_alloc_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_opp_factory_part_alloc_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@

check_avl_tree_partition_test_SOURCES = check_avl_tree_partition_test.c
check_avl_tree_partition_test_CFLAGS = @CHECK_CFLAGS@ $(AROOP_CORE_INCLUDES)
check_avl_tree_partition_test_LDADD = $(OPP_FACTORY_LIBS) @CHECK_LIBS@
