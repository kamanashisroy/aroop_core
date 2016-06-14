
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
#include "opp_factory_part_internal.h"

C_CAPSULE_START


static unsigned int calculate_obj_size(unsigned int content_size) {
	/* we have 32 bit */
	unsigned int obj_size = sizeof(struct opp_object); 
	obj_size += content_size;
#ifdef SYNC_BIT64
	obj_size = (obj_size/8)*8 + ((obj_size%8)?8:0); 
#endif
#ifdef SYNC_BIT32
	obj_size = (obj_size/4)*4 + ((obj_size%4)?4:0); 
#endif
	return obj_size;
}

static unsigned int calculate_bitstring(unsigned int inc) {
	/* we have one bit for each location */
	unsigned int bitstring_size = (inc/8)*8 + (inc%8)?8:0; 
	/* the bitstring should be multiple of 4 for 32 bit and multiple of 8 for 64 bit */
#ifdef SYNC_BIT64
	bitstring_size = (bitstring_size/8)*8 + ((bitstring_size%8)?8:0); 
#endif
#ifdef SYNC_BIT32
	bitstring_size = (bitstring_size/4)*4 + ((bitstring_size%4)?4:0); 
#endif
	bitstring_size = bitstring_size*BITFIELD_SIZE;
	return bitstring_size;
}

static void test_opp_factory_buffer(struct opp_factory*obuff, unsigned int inc, size_t content_size, unsigned int token_offset, unsigned int properties) {
	memset(obuff, 0, sizeof(struct opp_factory));
	ck_assert_int_eq(obuff->sign, 0);
	int status = opp_factory_create_full(obuff, inc, content_size, token_offset, OPPF_SWEEP_ON_UNREF, NULL);
	ck_assert_int_eq(status, 0);
	ck_assert_int_eq(OPPF_INITIALIZED_INTERNAL,obuff->sign);
	ck_assert_int_eq(obuff->pool_size, inc);
	ck_assert_int_eq(obuff->pool_count, 0);
	ck_assert_int_eq(obuff->use_count, 0);
	ck_assert_int_eq(obuff->slot_use_count, 0);
	ck_assert_int_eq(obuff->token_offset, token_offset);
	ck_assert_int_eq(calculate_obj_size(content_size), obuff->obj_size);
	ck_assert_int_eq(obuff->bitstring_size, calculate_bitstring(inc));
	ck_assert_int_eq(obuff->memory_chunk_size, obuff->bitstring_size + obuff->obj_size*obuff->pool_size + sizeof(struct opp_pool));
	ck_assert_int_eq(obuff->internal_flags, 0);
	ck_assert_int_eq(((properties) & obuff->property), properties);
	ck_assert_int_eq(obuff->callback, NULL);
	ck_assert_int_eq(obuff->pools, NULL);
	// TODO check lookup tree
}

START_TEST (test_opp_factory_create)
{
	struct opp_factory obuff;
	unsigned int i = 0;
	for(i = 0; i < 1000; i++) {
		test_opp_factory_buffer(&obuff, rand()%64+1, rand()%256, 1, OPPF_SWEEP_ON_UNREF);
	}
}
END_TEST

Suite * opp_factory_create_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("opp_factory_create.c");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_opp_factory_create);
	suite_add_tcase(s, tc_core);

	return s;
}


int main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = opp_factory_create_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

C_CAPSULE_END

