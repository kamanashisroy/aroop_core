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
typedef struct hash_ctable_item {
	opp_hash_t hashcode;
	void*key;
	void*ptr;
	void*next;
} opp_map_chained_pointer_t;

typedef struct {
	struct opp_factory fac;
	const opp_hash_function_t hfunc;
	const opp_equals_t efunc;
	const unsigned int max_slots;
	opp_map_chained_pointer_t* (* const slots)[];
	unsigned int collision;
} opp_hash_ctable_t;

void*opp_hash_ctable_get(opp_hash_ctable_t*ht, void*key);
void*opp_hash_ctable_get_no_ref(opp_hash_ctable_t*ht, void*key);
int opp_hash_ctable_set(opp_hash_ctable_t*ht, void*key, void*obj_data);
int opp_hash_ctable_traverse(opp_hash_ctable_t*ht, int (*cb)(void*cb_data, void*key, void*data), void*cb_data);
#define opp_hash_ctable_create(x,ar,ars,y,z,a,b) ({opp_hash_ctable_create_and_profile(x,ar,ars,y,z,a,b, __FILE__, __LINE__, AROOP_MODULE_NAME);})
int opp_hash_ctable_create_and_profile(opp_hash_ctable_t*ht, opp_map_chained_pointer_t*(* const arr)[], unsigned int arr_size, int pool_size, unsigned int flag, opp_hash_function_t hfunc, opp_equals_t efunc
		, char*source_file, int source_line, char*module_name);
#define opp_hash_ctable_destroy(x) opp_factory_destroy_and_remove_profile(&(x)->fac)
C_CAPSULE_END



#endif /* OPP_HASH_OTABLE_H_ */
