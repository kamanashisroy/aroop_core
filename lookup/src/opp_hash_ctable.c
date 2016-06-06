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
#include "aroop/opp/opp_hash_ctable.h"
#include "aroop/opp/opp_hash.h"
#include "aroop/core/logger.h"
#endif

C_CAPSULE_START

OPP_CB(hash_ctable_item) {
	opp_map_chained_pointer_t*item = (opp_map_chained_pointer_t*)data;

	switch(callback) {
	case OPPN_ACTION_INITIALIZE:
		// NOTE hashcode is not set item->hashcode = OPP_KEY_HASH(h, x);
		item->key = cb_data;
		OPPREF(item->key);
		item->ptr = va_arg(ap, void*);
		OPPREF(item->ptr);
		item->hashcode = va_arg(ap, opp_hash_t);
		item->next = NULL;
		break;
	case OPPN_ACTION_FINALIZE:
		OPPUNREF(item->key);
		OPPUNREF(item->ptr);
		if(item->next)
			OPPUNREF(item->next);
		break;
	}


	return 0;
}

#define OPP_KEY_HASH(h,x) ({h->hfunc.aroop_cb(h->hfunc.aroop_closure_data,x);})
OPP_INLINE static opp_map_chained_pointer_t*opp_hash_ctable_get_by_hashcode(opp_hash_ctable_t*const ht, const opp_hash_t hashcode) {
	const unsigned int index = hashcode%(ht->max_slots);
	opp_map_chained_pointer_t*x = (*(ht->slots))[index];
	while(x) {
		if(x->hashcode == hashcode) {
			return x;
		}
		x = x->next;
	}
	return NULL;
}

opp_map_chained_pointer_t*opp_hash_ctable_get_container(opp_hash_ctable_t*const ht, void* const key) {
	const opp_hash_t hashcode = OPP_KEY_HASH(ht,key);
	opp_map_chained_pointer_t*x = opp_hash_ctable_get_by_hashcode(ht, hashcode);
	if(!x)
		return NULL;
	while(x) {
		if(x->hashcode == hashcode && ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
			return x;
		}
		x = x->next;
	}
	return NULL;
}

void*opp_hash_ctable_get_no_ref(opp_hash_ctable_t*const ht, void* const key) {
	opp_map_chained_pointer_t*const x = opp_hash_ctable_get_container(ht, key);
	if(!x)
		return NULL;
	return x->ptr;
}

void*opp_hash_ctable_get(opp_hash_ctable_t* const ht, void* const key) {
	void*data = opp_hash_ctable_get_no_ref(ht, key);
	if(data)OPPREF(data);
	return data;
}

static opp_map_chained_pointer_t empty_slot = {.key = NULL, .ptr = NULL};
int opp_hash_ctable_set(opp_hash_ctable_t* const ht, void*const key, void* const obj_data) {
	if(obj_data == NULL) { /* we are doing unset */
		opp_map_chained_pointer_t*const x = opp_hash_ctable_get_container(ht, key);
		if(!x)
			return 0;
		OPPUNREF(x->key);
		OPPUNREF(x->ptr);
		// TODO remove the link (use doubly linked list ??)
		return 0;
	}
	const opp_hash_t hashcode = OPP_KEY_HASH(ht,key);
	const unsigned int index = hashcode%(ht->max_slots);
	opp_map_chained_pointer_t*x = (*(ht->slots))[index];
	if(!x) {
		(*(ht->slots))[index] = opp_alloc4(&ht->fac, 0, 0, 0, key, obj_data, hashcode);
		return 0;
	}
	unsigned int collision = 0;
	while(x) {
		if(!x->key) {
			// reuse empty
			x->key = OPPREF(key);
			x->ptr = OPPREF(obj_data);
			x->hashcode = hashcode;
			break;
		} else if(hashcode == x->hashcode && ht->efunc.aroop_cb(ht->efunc.aroop_closure_data,x->key,key)) {
			OPPUNREF(x->key);
			OPPUNREF(x->ptr);
			x->key = OPPREF(key);
			x->ptr = OPPREF(obj_data);
			break;
		} else if(x->next == NULL){
			x->next = opp_alloc4(&ht->fac, 0, 0, 0, key, obj_data, hashcode);
			break;
		}
		collision++;
		x = x->next;
	}
	if(collision)
		ht->collision++;
	return 0;
}

int opp_hash_ctable_traverse(opp_hash_ctable_t*ht, int (*cb)(void*cb_data, void*key, void*data), void*cb_data) {
	int index = ht->max_slots;
	while(index--) {
		opp_map_chained_pointer_t*x = (*(ht->slots))[index];
		while(x) {
			if(x->key)
				cb(cb_data, x->key, x->ptr);
			x = x->next;
		}
	}
	return 0;
}

int opp_hash_ctable_create_and_profile(opp_hash_ctable_t*ht, opp_map_chained_pointer_t*(* const arr)[], unsigned int arr_size, int pool_size, unsigned int flag, opp_hash_function_t hfunc, opp_equals_t efunc
		, char*source_file, int source_line, char*module_name) {
	opp_hash_ctable_t temp = {
		.hfunc = hfunc,
		.efunc = efunc,
		.slots = arr,
		.max_slots = arr_size,
		.collision = 0
	};
	memset(*arr, 0, sizeof(opp_map_chained_pointer_t*)*arr_size);
	memcpy(ht, &temp, sizeof(temp));
	return opp_factory_create_full_and_profile(&ht->fac, pool_size
			, sizeof(struct hash_ctable_item)
			, 1, flag | OPPF_SWEEP_ON_UNREF | OPPF_EXTENDED | OPPF_SEARCHABLE
			, OPP_CB_FUNC(hash_ctable_item)
			, source_file, source_line, module_name);
}

C_CAPSULE_END
