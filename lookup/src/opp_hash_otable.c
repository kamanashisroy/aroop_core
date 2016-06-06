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

#define OPP_KEY_HASH(h,x) ({h->hfunc.aroop_cb(h->hfunc.aroop_closure_data,x);})
OPP_INLINE static unsigned int opp_hash_otable_get_index(opp_hash_otable_t*ht, void*key) {
	const opp_hash_t hashcode = OPP_KEY_HASH(ht,key);
	unsigned int index = hashcode%(ht->max_slots);
	int times = ht->max_slots;
	while(times--) {
		const opp_map_pointer_t*x = &(*(ht->slots))[index];
		if(!x) {
			return -1;
		} else if(x->key && x->hashcode == hashcode && ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
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
	return (*(ht->slots))[index].ptr;
}

void*opp_hash_otable_get(opp_hash_otable_t*ht, void*key) {
	void*data = opp_hash_otable_get_no_ref(ht, key);
	if(!(ht->property & OPP_HASH_OTABLE_FLAG_NOREF) && data)OPPREF(data);
	return data;
}

OPP_INLINE static int opp_map_pointer_cleanup(opp_hash_otable_t* const ht, opp_map_pointer_t* const x) {
	if(!(ht->property & OPP_HASH_OTABLE_FLAG_NOREF)) {
		x->key = x->ptr = NULL;
	} else {
		OPPUNREF(x->key);
		OPPUNREF(x->ptr);
	}
	return 0;
}

OPP_INLINE static int opp_map_pointer_set(opp_hash_otable_t* const ht, opp_map_pointer_t* const x, void*const key, void*const obj_data) {
	if(!(ht->property & OPP_HASH_OTABLE_FLAG_NOREF)) {
		x->key = key;
		x->ptr = obj_data;
	} else {
		x->key = OPPREF(key);
		x->ptr = OPPREF(obj_data);
	}
	return 0;
}

int opp_hash_otable_set(opp_hash_otable_t*const ht, void*const key, void*const obj_data) {
	//unsigned long hash = aroop_txt_get_hash(key);
	if(obj_data == NULL) { /* we are doing unset */
		unsigned int index = opp_hash_otable_get_index(ht, key);
		if(index == -1)
			return 0;
		opp_map_pointer_t*const x = &(*(ht->slots))[index];
		return opp_map_pointer_cleanup(ht, x);
	}
	const opp_hash_t hashcode = OPP_KEY_HASH(ht,key);
	unsigned int index = hashcode%(ht->max_slots);
	int times = ht->max_slots;
	unsigned int collision = 0;
	while(times--) {
		opp_map_pointer_t* const x = &(*(ht->slots))[index];
		if(!x->key) {
			// empty
			x->hashcode = hashcode;
			opp_map_pointer_set(ht, x, key, obj_data);
			break;
		} else if(x->hashcode == hashcode && ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
			opp_map_pointer_cleanup(ht, x);
			opp_map_pointer_set(ht, x, key, obj_data);
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
		opp_map_pointer_t* const x = &(*(ht->slots))[index];
		if(x->key)
			cb(cb_data, x->key, x->ptr);
	}
	return 0;
}

int opp_hash_otable_create(opp_hash_otable_t*ht, opp_map_pointer_t(* const arr)[], unsigned int arr_size, opp_hash_function_t hfunc, opp_equals_t efunc, opp_property_t flags) {
	opp_hash_otable_t temp = {
		.hfunc = hfunc,
		.efunc = efunc,
		.slots = arr,
		.max_slots = arr_size,
		.collision = 0,
		.property = flags,
	};
	memset(*arr, 0, sizeof(opp_map_pointer_t)*arr_size);
	memcpy(ht, &temp, sizeof(temp));
	return 0;
}

int opp_hash_otable_destroy(opp_hash_otable_t*ht) {
	if((ht->property & OPP_HASH_OTABLE_FLAG_NOREF) == OPP_HASH_OTABLE_FLAG_NOREF)
		return 0; // nothing to do
	int index = ht->max_slots;
	while(index--) {
		opp_map_pointer_t* const x = &(*(ht->slots))[index];
		if(x->key)opp_map_pointer_cleanup(ht, x);
	}
	return 0;
}

C_CAPSULE_END
