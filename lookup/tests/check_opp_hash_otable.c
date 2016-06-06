
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
#include "aroop/core/xtring.h"

C_CAPSULE_START


START_TEST (test_opp_hash_otable)
{
	opp_map_pointer_t*arr[10];
	opp_hash_otable_t otable;
	opp_hash_otable_create(&otable, arr, 10, 10, 0, aroop_txt_get_hash_cb, aroop_txt_equals_cb);
	
	aroop_txt_t some_text = {};
	aroop_txt_embeded_set_static_string(&some_text, "fine");
	aroop_txt_t*content = aroop_txt_new_copy_deep(&some_text, NULL); // We must create xtring not extring ..
	opp_hash_otable_set(&otable, content, content);
	aroop_txt_t*output = opp_hash_otable_get(&otable, content);

	ck_assert_int_eq(content, output);
	ck_assert_int_eq(otable.collision, 0);

	opp_hash_otable_destroy(&otable);
}
END_TEST

Suite * opp_hash_otable_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("opp_hash_otable.c");

	/* Core test case */
	tc_core = tcase_create("Core");

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

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

C_CAPSULE_END

