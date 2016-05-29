
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

struct pencil {
	struct opp_object_ext opp_internal_ext;
	int color;
	int depth;
	int sum;
};

OPP_CB(pencil) {
        struct pencil*pen = data;
        struct pencil*template = (struct pencil*)cb_data;
        switch(callback) {
                case OPPN_ACTION_INITIALIZE:
			pen->color = template->color;
			pen->depth = template->depth;
			pen->sum = pen->depth+pen->color;
			ck_assert_int_ne(pen->sum,(pen->depth+pen->color));
                break;
                case OPPN_ACTION_FINALIZE:
			pen->sum = 0;
                break;
        }
        return 0;
}

static void check_pool(struct opp_pool*x) {
	ck_assert(x->end > x->head);
	ck_assert(x->end > ((SYNC_UWORD8_T*)x->bitstring));
	ck_assert(x->head > ((SYNC_UWORD8_T*)x->bitstring));
	ck_assert((SYNC_UWORD8_T*)x->bitstring > (SYNC_UWORD8_T*)x);
	ck_assert_int_eq(((BITSTRING_TYPE*)(x+1)), x->bitstring);
}

static void check_full_pool(struct opp_pool*pool) {
	struct opp_object*obj = NULL;
	check_pool(pool);
	SYNC_UWORD8_T bit_idx = 0;
	for(obj = (struct opp_object*)pool->head, bit_idx = 0
		; obj < (struct opp_object*)pool->end
		; bit_idx++, obj = (struct opp_object*)(((SYNC_UWORD8_T*)obj) + sizeof(struct pencil) + sizeof(struct opp_object))) {

		/* check opp_object */
		ck_assert_int_eq(obj->bit_idx, bit_idx);
		ck_assert(obj->bitstring == pool->bitstring);
		ck_assert(obj->slots == 1);

		/* check pencil */
		struct pencil*pen = (struct pencil*)(obj+1);
		ck_assert_int_eq(pen->depth, bit_idx);
		ck_assert_int_eq(pen->color, (bit_idx%2)?3:1);
	}
}

START_TEST (test_opp_factory_part_alloc_constructor)
{
	const int inc = _i;
	struct opp_factory bpencil;
	struct pencil*pen = NULL,*dpen = NULL;
	struct pencil template;
	opp_pointer_ext_t*item = NULL;
	const int color = 3;
	int idx = 0;
	memset(&bpencil, 0 , sizeof(bpencil));

	opp_factory_create_full(&bpencil, inc, sizeof(struct pencil), 0, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, OPP_CB_FUNC(pencil));

	/* create pencil upto the capacity of one pool */
	for(idx = 0; idx < inc; idx++) {
		template.color = (idx%2)?3:1;
		template.depth = idx;
		pen = OPP_ALLOC2(&bpencil, &template);
		ck_assert_int_ne(pen, NULL);
		ck_assert_int_eq(pen->color, (idx%2)?3:1);
		ck_assert_int_eq(pen->depth, idx);
		ck_assert_int_eq(pen->opp_internal_ext.flag, OPPN_ALL);
		ck_assert_int_eq(bpencil.use_count, idx+1);
		check_pool(bpencil.pools);
	}
	check_full_pool(bpencil.pools);
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc);

	int removed = 0;
	// remove odds
	for(idx = 0; idx < inc; idx++) {
		if(idx%2)
			continue;
		removed++;
		dpen = pen = opp_get(&bpencil, idx);
		ck_assert_int_ne(pen, NULL);
		OPPUNREF(pen);
		OPPUNREF(dpen);
		check_pool(bpencil.pools);
	}
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc-removed);

	// create the odds again
	for(idx = 0; idx < inc; idx++) {
		if(idx%2)
			continue;
		template.color = (idx%2)?3:1;
		template.depth = idx;
		pen = OPP_ALLOC2(&bpencil, &template);
		ck_assert_int_ne(pen, NULL);
		ck_assert_int_eq(pen->color, (idx%2)?3:1);
		ck_assert_int_eq(pen->depth, idx);
		ck_assert_int_eq(pen->opp_internal_ext.flag, OPPN_ALL);
		check_pool(bpencil.pools);
	}
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc);

	opp_factory_destroy_use_profiler_instead(&bpencil);
}
END_TEST



START_TEST (test_opp_factory_part_alloc_no_constructor)
{
	const int inc = _i;
	struct opp_factory bpencil;
	struct pencil*pen = NULL,*dpen = NULL;
	opp_pointer_ext_t*item = NULL;
	const int color = 3;
	int idx = 0;
	memset(&bpencil, 0 , sizeof(bpencil));

	opp_factory_create_full(&bpencil, inc, sizeof(struct pencil), 0, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, NULL);

	/* create pencil upto the capacity of one pool */
	for(idx = 0; idx < inc; idx++) {
		pen = OPP_ALLOC1(&bpencil);
		ck_assert_int_ne(pen, NULL);
		pen->color = (idx%2)?3:1;
		pen->depth = idx;
		ck_assert_int_eq(pen->opp_internal_ext.flag, OPPN_ALL);
		ck_assert_int_eq(bpencil.use_count, idx+1);
		check_pool(bpencil.pools);
	}
	check_full_pool(bpencil.pools);
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc);

	int removed = 0;
	// remove odds
	for(idx = 0; idx < inc; idx++) {
		if(idx%2)
			continue;
		removed++;
		dpen = pen = opp_get(&bpencil, idx);
		ck_assert_int_ne(pen, NULL);
		OPPUNREF(pen);
		OPPUNREF(dpen);
		check_pool(bpencil.pools);
	}
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc-removed);

	// create the odds again
	for(idx = 0; idx < inc; idx++) {
		if(idx%2)
			continue;
		pen = OPP_ALLOC1(&bpencil);
		ck_assert_int_ne(pen, NULL);
		pen->color = (idx%2)?3:1;
		pen->depth = idx;
		ck_assert_int_eq(pen->opp_internal_ext.flag, OPPN_ALL);
		check_pool(bpencil.pools);
	}
	ck_assert_int_eq(bpencil.pool_count, 1);
	ck_assert_int_eq(bpencil.use_count, inc);

	opp_factory_destroy_use_profiler_instead(&bpencil);
}
END_TEST

Suite * opp_factory_part_alloc_suite(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("opp_factory_part_alloc.c");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_loop_test(tc_core, test_opp_factory_part_alloc_no_constructor, 2, 20);
	tcase_add_loop_test(tc_core, test_opp_factory_part_alloc_constructor, 2, 20);
	suite_add_tcase(s, tc_core);

	return s;
}


int main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = opp_factory_part_alloc_suite();
	sr = srunner_create(s);

	//srunner_run_all(sr, CK_NORMAL);
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

C_CAPSULE_END

