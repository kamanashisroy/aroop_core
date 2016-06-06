
/*
 * This file part of aroop.
 *
 * Copyright (C) 2012  Kamanashis Roy
 *
 * Aroop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Aroop.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: Feb 9, 2011
 *  Author: Kamanashis Roy (kamanashisroy@gmail.com)
 */

#include <check.h>
#include "aroop/core/config.h"
#include "aroop/core/memory.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_list.h"
#include "aroop/opp/opp_queue.h"
#include "aroop/core/logger.h"
#include "aroop/opp/opp_watchdog.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_hash_table.h"
#include "aroop/opp/opp_hash_otable.h"

C_CAPSULE_START

struct test_obj {
	long long token;
};

static opp_hash_t test_obj_get_hash(void*unused, const void*data) {
	const struct test_obj*key = data;
	return key->token;
}

static int test_obj_is_equal(void*unused, const void*x, const void*y) {
	const struct test_obj*xobj = x;
	const struct test_obj*yobj = y;
	return xobj->token == yobj->token;
}

opp_hash_function_t test_obj_hash_func = {.aroop_closure_data = NULL, .aroop_cb = test_obj_get_hash};
opp_equals_t test_obj_equals_func = {.aroop_closure_data = NULL, .aroop_cb = test_obj_is_equal};


START_TEST (test_opp_hash_otable)
{
	opp_map_pointer_t*arr[10];
	opp_hash_otable_t otable;
	opp_hash_otable_create(&otable, &arr, 10, 10, 0, test_obj_hash_func, test_obj_equals_func);
	ck_assert_int_eq(otable.collision, 0);
	
	struct opp_factory obuff;
	int status = opp_factory_create_full(&obuff, 10, sizeof(struct test_obj), 0, OPPF_SWEEP_ON_UNREF, NULL);

	ck_assert_int_eq(status, 0);

	struct test_obj*content = OPP_ALLOC2(&obuff, NULL);
	content->token = 100;
	ck_assert(content != NULL);

	opp_hash_otable_set(&otable, content, content);
	ck_assert_int_eq(otable.collision, 0);
	struct test_obj*output = opp_hash_otable_get_no_ref(&otable, content);
	ck_assert_int_eq(otable.collision, 0);
	output = opp_hash_otable_get(&otable, content);
	ck_assert_int_eq(otable.collision, 0);

	ck_assert_int_eq(content, output);
	ck_assert_int_eq(otable.collision, 0);

	opp_hash_otable_destroy(&otable);
	opp_factory_destroy_and_remove_profile(&obuff);
}
END_TEST

Suite * opp_hash_otable_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("opp_hash_otable.c");

	/* Core test case */
	tc_core = tcase_create("lookup");

	tcase_add_test(tc_core, test_opp_hash_otable);
	suite_add_tcase(s, tc_core);

	return s;
}


int main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = opp_hash_otable_suite();
	sr = srunner_create(s);

	//srunner_run_all(sr, CK_NORMAL);
	srunner_run_all(sr, CK_NOFORK);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

C_CAPSULE_END

