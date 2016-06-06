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
 *  Created on: Jan 16, 2012
 *      Author: kamanashisroy
 */

#ifndef OPP_HASH_OTABLE_H_
#define OPP_HASH_OTABLE_H_


#ifndef AROOP_CONCATENATED_FILE
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_list.h"
#endif

C_CAPSULE_START

enum {
	OPP_HASH_OTABLE_FLAG_NOREF = 1,
};

typedef struct hash_otable_item {
	opp_hash_t hashcode;
	void*ptr;
	void*key;
} opp_map_pointer_t;

typedef struct {
	const opp_hash_function_t hfunc;
	const opp_equals_t efunc;
	const unsigned int max_slots;
	const opp_property_t property;
	opp_map_pointer_t (* const slots)[];
	unsigned int collision;
} opp_hash_otable_t;

void*opp_hash_otable_get(opp_hash_otable_t*ht, void*key);
void*opp_hash_otable_get_no_ref(opp_hash_otable_t*ht, void*key);
int opp_hash_otable_set(opp_hash_otable_t*ht, void*key, void*obj_data);
int opp_hash_otable_traverse(opp_hash_otable_t*ht, int (*cb)(void*cb_data, void*key, void*data), void*cb_data);
int opp_hash_otable_create(opp_hash_otable_t*ht, opp_map_pointer_t(* const arr)[], unsigned int arr_size, opp_hash_function_t hfunc, opp_equals_t efunc, opp_property_t flags);
int opp_hash_otable_destroy(opp_hash_otable_t*ht);

C_CAPSULE_END



#endif /* OPP_HASH_OTABLE_H_ */
