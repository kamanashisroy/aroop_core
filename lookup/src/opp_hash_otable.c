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

#ifndef AROOP_CONCATENATED_FILE
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_list.h"
#include "aroop/opp/opp_hash_otable.h"
#include "aroop/opp/opp_hash.h"
#include "aroop/core/logger.h"
#endif

C_CAPSULE_START

OPP_CB(hash_otable_item) {
	opp_map_pointer_t*item = (opp_map_pointer_t*)data;

	switch(callback) {
	case OPPN_ACTION_INITIALIZE:
		item->key = cb_data;
		OPPREF(item->key);
		item->ptr = va_arg(ap, void*);
		OPPREF(item->ptr);
		break;
	case OPPN_ACTION_FINALIZE:
		OPPUNREF(item->key);
		OPPUNREF(item->ptr);
		break;
	}


	return 0;
}

#define OPP_KEY_HASH(h,x) ({h->hfunc.aroop_cb(h->hfunc.aroop_closure_data,x);})
OPP_INLINE static unsigned int opp_hash_otable_get_index(opp_hash_otable_t*ht, void*key) {
	//unsigned long hash = aroop_txt_get_hash(key);
	unsigned long hash = OPP_KEY_HASH(ht,key);
	unsigned int index = hash%(ht->max_slots);
	int times = ht->max_slots;
	while(times--) {
		opp_map_pointer_t*x = (*(ht->slots))[index];
		if(!x) {
			return -1;
		} else if(x->key && ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
			return index;
		}
		index = (index+times)%ht->max_slots;
	}
	return -1;
}

void*opp_hash_otable_get_no_ref(opp_hash_otable_t*ht, void*key) {
	unsigned int index = opp_hash_otable_get_index(ht, key);
	if(index == -1)
		return NULL;
	return (*(ht->slots))[index]->ptr;
}

void*opp_hash_otable_get(opp_hash_otable_t*ht, void*key) {
	void*data = opp_hash_otable_get_no_ref(ht, key);
	if(data)OPPREF(data);
	return data;
}

static opp_map_pointer_t empty_slot = {.key = NULL, .ptr = NULL};
int opp_hash_otable_set(opp_hash_otable_t*ht, void*key, void*obj_data) {
	//unsigned long hash = aroop_txt_get_hash(key);
	if(obj_data == NULL) { /* we are doing unset */
		unsigned int index = opp_hash_otable_get_index(ht, key);
		if(index == -1)
			return 0;
		OPPUNREF((*(ht->slots))[index]);
		(*(ht->slots))[index] = &empty_slot;
		return 0;
	}
	unsigned long hash = OPP_KEY_HASH(ht,key);
	unsigned int index = hash%(ht->max_slots);
	int times = ht->max_slots;
	unsigned int collision = 0;
	while(times--) {
		opp_map_pointer_t*x = (*(ht->slots))[index];
		if(!x || !x->key) {
			// empty
			(*(ht->slots))[index] = opp_alloc4(&ht->fac, 0, 0, 0, key, obj_data);
			break;
		} else if(ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
			OPPUNREF((*(ht->slots))[index]); // delete old
			(*(ht->slots))[index] = opp_alloc4(&ht->fac, 0, 0, 0, key, obj_data);
			break;
		}
		index = (index+times)%ht->max_slots;
		collision++;
	}
	if(collision)
		ht->collision++;
	return 0;
}

int opp_hash_otable_traverse(opp_hash_otable_t*ht, int (*cb)(void*cb_data, void*key, void*data), void*cb_data, int flags, int if_not_flags, int matchhash) {
	int index = ht->max_slots;
	while(index--) {
		opp_map_pointer_t*x = (*(ht->slots))[index];
		if(x && x->key)
			cb(cb_data, x->key, x->ptr);
	}
	return 0;
}

int opp_hash_otable_create_and_profile(opp_hash_otable_t*ht, opp_map_pointer_t*(* const arr)[], unsigned int arr_size, int pool_size, unsigned int flag, opp_hash_function_t hfunc, opp_equals_t efunc
		, char*source_file, int source_line, char*module_name) {
	opp_hash_otable_t temp = {
		.hfunc = hfunc,
		.efunc = efunc,
		.slots = arr,
		.max_slots = arr_size,
		.collision = 0
	};
	memset(*arr, 0, sizeof(opp_map_pointer_t)*arr_size);
	memcpy(ht, &temp, sizeof(temp));
	return opp_factory_create_full_and_profile(&ht->fac, pool_size
			, sizeof(opp_map_pointer_ext_t)
			, 1, flag | OPPF_SWEEP_ON_UNREF | OPPF_EXTENDED | OPPF_SEARCHABLE
			, OPP_CB_FUNC(hash_otable_item)
			, source_file, source_line, module_name);
}

C_CAPSULE_END
