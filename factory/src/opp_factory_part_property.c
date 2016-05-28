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
#endif

C_CAPSULE_START


int opp_callback(void*data, int callback, void*cb_data) {
	struct opp_object*obj = data_to_opp_object(data);
	struct opp_factory*obuff = obj->obuff;
	SYNC_ASSERT(obuff->callback);
	CHECK_OBJ(obj);
	static va_list va;
	return obuff->callback(data, callback, cb_data, va, obj->slots*obuff->obj_size - sizeof(struct opp_object));
}

int opp_callback2(void*data, int callback, void*cb_data, ...) {
	struct opp_object*obj = data_to_opp_object(data);
	struct opp_factory*obuff = obj->obuff;
	SYNC_ASSERT(obuff->callback);
	CHECK_OBJ(obj);
	va_list va;
	va_start(va, cb_data);
	int ret = obuff->callback(data, callback, cb_data, va, obj->slots*obuff->obj_size - sizeof(struct opp_object));
	va_end(va);
	return ret;
}

#ifdef OPP_HAS_TOKEN
void*opp_get(struct opp_factory*obuff, int token) {
	unsigned int idx;
	unsigned int k;
	int pool_idx;
	void*data = NULL;
	struct opp_pool*pool;

	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	do {
		/* sanity check */
		if(token < obuff->token_offset) break;
		idx = (token - obuff->token_offset);
		k = idx%obuff->pool_size;
		pool_idx = (idx - k)/obuff->pool_size;

		for(pool = obuff->pools;pool;pool = pool->next) {
			CHECK_POOL(pool);
			if(pool->idx != pool_idx) {
				continue;
			}

			struct opp_object*obj = (struct opp_object*)(pool->head + obuff->obj_size*k);
			BITSTRING_TYPE bsv = *(pool->bitstring+OPP_INDEX_TO_BITSTRING_INDEX(k)*BITFIELD_SIZE);
			int bit_idx = k % BIT_PER_STRING;
//			if(obj->refcount) {
//				SYNC_ASSERT(obj->bit_idx == bit_idx && obj->obuff == obuff && (bsv & 1<<bit_idx));
//			}
			if((bsv & (1<<bit_idx)) && (obj->bit_idx == bit_idx) && obj->refcount
					/*&& (!(obuff->property & OBJ_FACTORY_EXTENDED) || ((struct sync_object_ext_tiny*)(obj+1))->flag != OBJ_ITEM_ZOMBIE)*/) {
				CHECK_OBJ(obj);
				data = obj+1;
				OPPREF(data);
			}
			break;
		}
	} while(0);
	obuff->internal_flags &= ~gcflag;
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	return data;
}

void opp_shrink(void*data, unsigned int size) {
	struct opp_object*obj = data_to_opp_object(data);
	unsigned int slots;
	struct opp_factory*obuff = obj->obuff;
	
	size += sizeof(struct opp_object);
	slots = size / obuff->obj_size + ((size % obuff->obj_size)?1:0);
	
	if(!slots || slots > BIT_PER_STRING || slots == obj->slots) {
		return;
	}
	
	CHECK_OBJ(obj);
	OPP_LOCK(obuff);
	UNSET_PAIRED_BITS(obj);
	
	obj->slots = slots;
	
	SET_PAIRED_BITS(obj);
	if(obuff->callback){
		int new_len = slots*obuff->obj_size - sizeof(struct opp_object);
		static va_list va;
		obuff->callback(data, OPPN_ACTION_SHRINK, &new_len, va, new_len);
	}
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
}


void opp_set_flag_by_token(struct opp_factory*obuff, int token, unsigned int flag) {
	SYNC_ASSERT(obuff->property & OPPF_EXTENDED);
	OPP_LOCK(obuff);
	void*data = opp_get(obuff,token);
	if(data) {
		opp_set_flag(data,flag);
		OPPUNREF(data);
	}
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
}

void opp_unset_flag_by_token(struct opp_factory*obuff, int token, unsigned int flag) {
	SYNC_ASSERT(obuff->property & OPPF_EXTENDED);
	OPP_LOCK(obuff);
	void*data = opp_get(obuff,token);
	if(data) {
		opp_unset_flag(data,flag);
		OPPUNREF(data);
	}
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
}
#endif

void opp_unset_flag(void*data, unsigned int flag) {
	struct opp_object*obj = data_to_opp_object(data);
	CHECK_OBJ(obj);
	SYNC_ASSERT(obj->refcount);
#ifndef SYNC_HAS_ATOMIC_OPERATION
	struct opp_factory*obuff = obj->obuff;
	OPP_LOCK(obuff);
#endif

	SYNC_ASSERT(obj->obuff->property & OPPF_EXTENDED);
	SYNC_ASSERT(!(flag & OPPN_ALL) && !(flag & OPPN_INTERNAL_1) && !(flag & OPPN_INTERNAL_2));
	BINARY_AND_HELPER(SYNC_UWORD16_T, &((struct opp_object_ext*)data)->flag, ~flag);
#ifndef OPP_NO_FLAG_OPTIMIZATION
	if(flag & OPPN_PERFORMANCE) {
		BINARY_AND_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE), ~( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE1) {
		BINARY_AND_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE1), ~( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE2) {
		BINARY_AND_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE2), ~( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE3) {
		BINARY_AND_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE3), ~( 1 << obj->bit_idx));
	}
#endif
#ifndef SYNC_HAS_ATOMIC_OPERATION
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
#endif
}

void opp_set_flag(void*data, unsigned int flag) {
	struct opp_object*obj = data_to_opp_object(data);
	struct opp_object_ext*ext = (struct opp_object_ext*)data;
	CHECK_OBJ(obj);
	SYNC_ASSERT(obj->refcount);
	struct opp_factory*obuff = obj->obuff;
#ifndef SYNC_HAS_ATOMIC_OPERATION
	OPP_LOCK(obuff);
#endif
	SYNC_ASSERT(obuff->property & OPPF_EXTENDED);
	SYNC_ASSERT(!(flag & OPPN_ALL) && !(flag & OPPN_INTERNAL_1) && !(flag & OPPN_INTERNAL_2));
	if((flag & OPPN_ZOMBIE) && (obuff->property & OPPF_SEARCHABLE)) {
#ifdef SYNC_HAS_ATOMIC_OPERATION
		OPP_LOCK(obuff);
#endif
		if((ext->flag & OPPN_INTERNAL_1)) {
			opp_lookup_table_delete(&obuff->tree, ext);
			BINARY_AND_HELPER(SYNC_UWORD16_T, &ext->flag, ~OPPN_INTERNAL_1);
		}
		BINARY_OR_HELPER(SYNC_UWORD16_T, &ext->flag, flag);
#ifdef SYNC_HAS_ATOMIC_OPERATION
		OPP_UNLOCK(obuff);
#endif
	} else {
		BINARY_OR_HELPER(SYNC_UWORD16_T, &ext->flag, flag);
	}
#ifndef OPP_NO_FLAG_OPTIMIZATION
	if(flag & OPPN_PERFORMANCE) {
		BINARY_OR_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE), ( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE1) {
		BINARY_OR_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE1), ( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE2) {
		BINARY_OR_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE2), ( 1 << obj->bit_idx));
	}
	if(flag & OPPN_PERFORMANCE3) {
		BINARY_OR_HELPER(SYNC_UWORD16_T, (obj->bitstring+BITFIELD_PERFORMANCE3), ( 1 << obj->bit_idx));
	}
#endif
#ifndef SYNC_HAS_ATOMIC_OPERATION
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
#endif
}


void opp_force_memclean(void*data) {
	struct opp_object*obj = data_to_opp_object(data);
	struct opp_object_ext*ext = (struct opp_object_ext*)data;
	struct opp_factory*obuff = obj->obuff;
	int sz = obj->slots * obuff->obj_size - sizeof(struct opp_object);
	if(obuff->property & OPPF_EXTENDED) {
		memset(ext+1, 0, sz - sizeof(struct opp_object_ext));
	} else {
		memset(data, 0, sz);
	}
}

struct opp_factory*opp_get_factory_donot_use(void*data) {
	struct opp_object*obj = data_to_opp_object(data);
	return obj->obuff;
}

void opp_set_hash(void*data, opp_hash_t hash) {
	struct opp_object*obj = data_to_opp_object(data);
	struct opp_object_ext*ext = (struct opp_object_ext*)data;
	struct opp_factory*obuff = obj->obuff;
	CHECK_OBJ(obj);
	SYNC_ASSERT(obj->refcount);
	SYNC_ASSERT(obuff->property & OPPF_EXTENDED);
	if(obuff->property & OPPF_SEARCHABLE) {
		OPP_LOCK(obuff);
		if((ext->flag & OPPN_INTERNAL_1)) {
			opp_lookup_table_delete(&obuff->tree, ext);
			BINARY_AND_HELPER(SYNC_UWORD16_T, &ext->flag, ~OPPN_INTERNAL_1);
		}

		ext->hash = hash;
		if(!(ext->flag & OPPN_ZOMBIE) && !opp_lookup_table_insert(&obuff->tree, ext)) {
			BINARY_OR_HELPER(SYNC_UWORD16_T, &ext->flag, OPPN_INTERNAL_1);
		}
		CHECK_OBJ(obj);
		DO_AUTO_GC_CHECK(obuff);
		OPP_UNLOCK(obuff);
	} else {
		ext->hash = hash;
	}
}

#ifdef OPP_CAN_TEST_FLAG
int obj_test_flag(const void*data, unsigned int flag) {
	const struct opp_object*obj = data_to_opp_object(data);
	int ret = 0;
	CHECK_OBJ(obj);
	SYNC_ASSERT(obj->refcount);
	OPP_LOCK(obj->obuff);
	ret = (obj->flag & flag);
	DO_AUTO_GC_CHECK(obj->obuff);
	OPP_UNLOCK(obj->obuff);
	return ret;
}
#endif

void*opp_ref(void*data, const char*filename, int lineno) {
	struct opp_object*obj = data_to_opp_object(data);
	SYNC_ASSERT(data);
	CHECK_OBJ(obj);
#ifdef SYNC_HAS_ATOMIC_OPERATION
#ifdef OPP_DEBUG_REFCOUNT
	obj->rt_idx++;
	obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	if(!(obj->rt_idx >= 2 && obj->ref_trace[obj->rt_idx-2].lineno == lineno)) {
		obj->ref_trace[obj->rt_idx].filename = filename;
		obj->ref_trace[obj->rt_idx].lineno = lineno;
		obj->ref_trace[obj->rt_idx].refcount = obj->refcount;
		obj->ref_trace[obj->rt_idx].op = '+';
	} else {
		obj->rt_idx--;
		obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	}
#endif

	do {
		refcount_t oldval,newval;
		oldval = obj->refcount;
		newval = oldval+1;
		if(!oldval || newval > 40000) {
			return NULL;
		}
		if(sync_do_compare_and_swap(&(obj->refcount), oldval, newval)) {
			break;
		}
	} while(1);
#else
	OPP_LOCK(obj->obuff);

#ifdef OPP_DEBUG_REFCOUNT
	obj->rt_idx++;
	obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	obj->ref_trace[obj->rt_idx].filename = filename;
	obj->ref_trace[obj->rt_idx].lineno = lineno;
	obj->ref_trace[obj->rt_idx].refcount = obj->refcount;
	obj->ref_trace[obj->rt_idx].op = '+';
#endif

	SYNC_ASSERT(obj->refcount);
	obj->refcount++;
	DO_AUTO_GC_CHECK(obj->obuff);
	OPP_UNLOCK(obj->obuff);
#endif
	return data;
}

#if 0
int opp_assert_ref_count(void*data, int refcount, const char*filename, int lineno) {
	struct opp_object*obj = data_to_opp_object(*data);
	SYNC_ASSERT(data);
	CHECK_OBJ(obj);
	SYNC_ASSERT(obj->refcount == refcount);
	return 0;
}
#endif

//#ifdef OPP_BUFFER_HAS_LOCK
void opp_unref_unlocked(void**data, const char*filename, int lineno) {
	struct opp_object*obj = data_to_opp_object(*data);
#ifdef OPP_HAS_RECYCLING
	struct opp_pool*pool;
#endif
	if(!*data)
		return;
	CHECK_OBJ(obj);
#ifdef OPP_DEBUG_REFCOUNT
	obj->rt_idx++;
	obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	if(!(obj->rt_idx >= 2 && obj->ref_trace[obj->rt_idx-2].lineno == lineno)) {
		obj->ref_trace[obj->rt_idx].filename = filename;
		obj->ref_trace[obj->rt_idx].lineno = lineno;
		obj->ref_trace[obj->rt_idx].refcount = obj->refcount;
		obj->ref_trace[obj->rt_idx].op = '-';
	} else {
		obj->rt_idx--;
		obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	}
#endif

	*data = NULL;
	SYNC_ASSERT(obj->refcount);
#ifdef SYNC_HAS_ATOMIC_OPERATION
	refcount_t oldval,newval;
	do {
		oldval = obj->refcount;
		newval = oldval - 1;
		SYNC_ASSERT(oldval);
	} while(!sync_do_compare_and_swap(&(obj->refcount), oldval, newval));
	if(!newval) {
#else
	obj->refcount--;
	if(!obj->refcount) {
#endif
#ifdef OPP_HAS_RECYCLING
		for(pool = obj->obuff->pools;pool;pool = pool->next) {
			CHECK_POOL(pool);
			if((void*)obj <= (void*)pool->end && (void*)obj >= (void*)pool->head) {
				obj->recycled = pool->recycled;
				pool->recycled = obj;
				CHECK_POOL(pool);
				break;
			}
		}
#endif
		*(obj->bitstring) &= ~( 1 << obj->bit_idx);
		if(obj->obuff->property & OPPF_SWEEP_ON_UNREF) {
			OPP_FINALIZE_NOW(obj->obuff,obj);
		}
		obj->obuff->use_count--;
	}
	DO_AUTO_GC_CHECK(obj->obuff);
}
//#endif

void opp_unref(void**data, const char*filename, int lineno) {
	struct opp_object*obj = data_to_opp_object(*data);
	if(!*data)
		return;
#ifdef OPP_DEBUG_REFCOUNT
	obj->rt_idx++;
	obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	if(!(obj->rt_idx >= 2 && obj->ref_trace[obj->rt_idx-2].lineno == lineno)) {
		obj->ref_trace[obj->rt_idx].filename = filename;
		obj->ref_trace[obj->rt_idx].lineno = lineno;
		obj->ref_trace[obj->rt_idx].refcount = obj->refcount;
		obj->ref_trace[obj->rt_idx].op = '-';
	} else {
		obj->rt_idx--;
		obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
	}
#endif

	*data = NULL;
#ifdef SYNC_HAS_ATOMIC_OPERATION
	refcount_t oldval,newval;
	struct opp_factory*obuff = NULL;
	do {
		oldval = obj->refcount;
		newval = oldval - 1;
		SYNC_ASSERT(oldval);
	} while(!sync_do_compare_and_swap(&(obj->refcount), oldval, newval));
	if(newval) {
		return;
	} else {
		obuff = obj->obuff;
		OPP_LOCK(obuff);
#else
	SYNC_ASSERT(obj->refcount);
	struct opp_factory*obuff = obj->obuff;
	OPP_LOCK(obuff);
	obj->refcount--;
	if(!obj->refcount) {
#endif
		CHECK_OBJ(obj);

#ifdef OPP_HAS_RECYCLING
		struct opp_pool*pool;
		for(pool = obj->obuff->pools;pool;pool = pool->next) {
			CHECK_POOL(pool);
			if((void*)obj <= (void*)pool->end && (void*)obj >= (void*)pool->head) {
				obj->recycled = pool->recycled;
				pool->recycled = obj;
				CHECK_POOL(pool);
				break;
			}
		}
#endif
		*(obj->bitstring) &= ~( 1 << obj->bit_idx);
		if(obuff->property & OPPF_SWEEP_ON_UNREF) {
			OPP_FINALIZE_NOW(obuff,obj);
		}
		obuff->use_count--;
	}
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
}

#ifdef OPP_HAS_HIJACK
int opp_hijack(void**src, void*dest, const char*filename, int lineno) {
	struct opp_object*obj = data_to_opp_object(*src);
	int ret = -1;
	struct opp_factory*obuff = obj->obuff;
#ifdef OPP_HAS_RECYCLING
	struct opp_pool*pool;
#endif
	OPP_LOCK(obuff);
	do {
		CHECK_OBJ(obj);
#ifdef OPP_DEBUG_REFCOUNT
		obj->rt_idx++;
		obj->rt_idx = obj->rt_idx%OPP_TRACE_SIZE;
		obj->ref_trace[obj->rt_idx].filename = filename;
		obj->ref_trace[obj->rt_idx].lineno = lineno;
		obj->ref_trace[obj->rt_idx].refcount = obj->refcount;
		obj->ref_trace[obj->rt_idx].op = 0;
#endif

#ifdef OPP_DEBUG
		SYNC_ASSERT(obj->refcount);
#endif
		// see if we can do hijack
		if(obj->refcount != 1) {
			break;
		}

		// hijack
//		memcpy(dest,*src,obuff->obj_size - sizeof(struct opp_object));
		if(obuff->callback){
			va_list ap;
			obuff->callback(*src, OPPN_ACTION_DEEP_COPY, dest, ap);
		}
		obj->refcount--;
#ifdef OPP_HAS_RECYCLING
		for(pool = obj->obuff->pools;pool;pool = pool->next) {
			CHECK_POOL(pool);
			if((void*)obj <= (void*)pool->end && (void*)obj >= (void*)pool->head) {
				obj->recycled = pool->recycled;
				pool->recycled = obj;
				CHECK_POOL(pool);
				break;
			}
		}
#endif
		// clear flags
		*(obj->bitstring) &= ~( 1 << obj->bit_idx);
		*(obj->bitstring+BITFIELD_FINALIZE) &= ~( 1 << obj->bit_idx);
		UNSET_PAIRED_BITS(obj);
		obuff->use_count--;
		*src = dest;
		ret = 0;
	} while(0);
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	return ret;
}
#endif


C_CAPSULE_END
