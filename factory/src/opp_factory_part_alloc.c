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

OPP_INLINE struct opp_pool*opp_factory_create_pool_donot_use(struct opp_factory*obuff, struct opp_pool*addpoint, void*nofreememory) {
	// allocate a block of memory
	struct opp_pool*pool = (struct opp_pool*)nofreememory;
	opp_factory_profiler_checkleak();
	if(!pool && !(pool = (struct opp_pool*)profiler_replace_malloc(obuff->memory_chunk_size))) {
		SYNC_LOG(SYNC_ERROR, "Out of memory\n");
		return NULL;
	}

	if(!obuff->pools) {
		obuff->pools = pool;
		pool->idx = 0;
		pool->next = NULL;
	} else {
		SYNC_ASSERT(addpoint);
		// insert the pool in appropriate place
		pool->next = addpoint->next;
		addpoint->next = pool;
		pool->idx = addpoint->idx+1;
	}
	if(pool->idx > 64) {
		opp_watchdog_report(WATCHDOG_ALERT, "pool->idx > 64");
	}

	obuff->pool_count++;
	opp_factory_profiler_checkleak();

	SYNC_UWORD8_T*ret = (SYNC_UWORD8_T*)(pool+1);
	/* clear bitstring */
	memset(ret, 0, obuff->bitstring_size);
	// setup pool
	pool->bitstring = (BITSTRING_TYPE*)ret;
#ifdef OPP_HAS_RECYCLING
	pool->recycled = NULL;
#endif
	pool->head = ret + (obuff->bitstring_size);
	pool->end = ret + obuff->memory_chunk_size - sizeof(struct opp_pool);
	pool->flags = nofreememory?0:OPP_POOL_FREEABLE;
	CHECK_POOL(pool);
	return pool;
}

#ifdef OPP_HAS_RECYCLING
OPP_INLINE static SYNC_UWORD8_T*opp_alloc4_recycle(const SYNC_UWORD8_T slot_count, struct opp_pool*pool) {
	if(slot_count == 1) {
		/* traverse the pool link-list */
		for(;pool; pool = pool->next) {
			CHECK_POOL(pool);
			if(pool->recycled) {
				SYNC_ASSERT(!pool->recycled->refcount);
				const SYNC_UWORD8_T*ret = (SYNC_UWORD8_T*)pool->recycled;
				pool->recycled = pool->recycled->recycled;
				return ret;
			}
		}
	}
	return NULL;
}
#endif

OPP_INLINE static SYNC_UWORD8_T opp_alloc4_count_slots(SYNC_UWORD16_T size, const SYNC_UWORD16_T obj_size, const SYNC_UWORD16_T pool_size) {
	SYNC_UWORD8_T slot_count = 1;
	if(!size) /* the default size is one slot */
		return 1;
	size += sizeof(struct opp_object); /* we need to add object header */
	OPP_NORMALIZE_SIZE(size); /* we need to convert the size into a chunk size */
	/* TODO optimize the following line */
	slot_count = size / obj_size + ((size % obj_size)?1:0); /* find the number of slot_count needed */

	/* number of slot_count must be smaller than bit per string */
	if(slot_count > BIT_PER_STRING) {
		SYNC_LOG(SYNC_ERROR, "Alloc failed:solt %d cannot fit in %d-bit header for request size %d\n", slot_count, BIT_PER_STRING, size);
		return 0;
	}
	/* number of slot_count must be smaller than object pool size */
	if(slot_count > pool_size) {
		SYNC_LOG(SYNC_ERROR, "Alloc failed:Cannot fit solts %d in pool of %d\n", slot_count, pool_size);
		return 0;
	}
	/* apply some invariant testing */
	SYNC_ASSERT(slot_count != 0);
	SYNC_ASSERT(slot_count <= BIT_PER_STRING && slot_count <= pool_size);
	return slot_count;
}


OPP_INLINE static void opp_alloc4_init_object_bit_index(struct opp_object*obj, const SYNC_UWORD8_T bit_idx, BITSTRING_TYPE* bitstring, const SYNC_UWORD8_T slot_count) {
	obj->bit_idx = bit_idx;
	obj->bitstring = bitstring;
	obj->slots = slot_count;
}


OPP_INLINE int get_set_n_position_old(BITSTRING_TYPE x, const uint8_t n) {
#ifdef OPP_DEBUG
	int loop_breaker = 0;
#endif
	while(x) {
#ifdef OPP_DEBUG
		loop_breaker++;
		SYNC_ASSERT(loop_breaker < BIT_PER_STRING);
#endif
		/**
		 * check if there is enough slots available,
		 * the number of 1(empty) in the string should be bigger than or equal to slot count
		 */
		if(n > 1 && SYNC_OBJ_POPCOUNT(x) < n) {
			return -1;
		}
		/**
		 * number of trailing 0(not empty) bits in bsv
		 * if bsv = 0b11000, then bit_index = 3
		 */
		SYNC_UWORD8_T bit_idx = SYNC_OBJ_CTZ(x);
		if(n > 1 && (bit_idx + n) > BIT_PER_STRING) {
			// we cannot do it
			return -1;
		}
		
		/**
		 * if we need 2 slots, and we want to position it in 3 then,
		 * mask = 0b11000
		 */
		BITSTRING_TYPE mask = ((1 << n)-1)<<bit_idx;
		if((mask & x) != mask) { /* check if the slots are available */
			x &= ~mask;
			continue;
		}
		return bit_idx;
	}
	return -1;
}

OPP_INLINE int get_set_n_position(BITSTRING_TYPE x, const uint8_t n) {
	if(!n || n > BIT_PER_STRING) return -1;
	int i = n-1;
	while(x && i--) {
		x = x & (x << 1);
	};
	if(!x) return -1;
	//x = x - (x & (x-1));
	//if(!x) return -1;
	int pos = SYNC_OBJ_CTZ(x);
	//assert(pos >= (n-1));
	return pos - (n-1);
}


OPP_INLINE static SYNC_UWORD8_T*opp_alloc4_find_space(const struct opp_pool*pool, const SYNC_UWORD16_T pool_size, const SYNC_UWORD8_T slot_count, const SYNC_UWORD16_T obj_size, SYNC_UWORD16_T*obj_seq) {
	unsigned long k = 0;
	BITSTRING_TYPE*bitstring = pool->bitstring;
	for(;BITSTRING_IDX_TO_BITS(k) < pool_size;k++,bitstring+=BITFIELD_SIZE) {
		// find first 0
		BITSTRING_TYPE bsv = ~(*bitstring | *(bitstring+BITFIELD_PAIRED));
		const int pos = get_set_n_position(bsv, slot_count);
		//assert(pos == get_set_n_position_old(bsv, slot_count));
		if(pos == -1)
			continue;
		const SYNC_UWORD8_T bit_idx = pos;
		//assert((bsv & (((1 << slot_count)-1) << bit_idx)) == (((1 << slot_count)-1) << bit_idx));
		const SYNC_UWORD16_T obj_idx = *obj_seq = BITSTRING_IDX_TO_BITS(k) + bit_idx;
		if(obj_idx >= pool_size) { /* we are overflowing the buffer */
			continue;
		}
		SYNC_ASSERT(obj_idx < pool_size);
		SYNC_UWORD8_T*ret = (pool->head + obj_idx*obj_size);
		opp_alloc4_init_object_bit_index((struct opp_object*)ret, bit_idx, bitstring, slot_count);
		return ret;
	}
	return NULL;
}

OPP_INLINE static void opp_alloc4_init_object(struct opp_factory*obuff, const struct opp_pool*pool, SYNC_UWORD8_T*given, const SYNC_UWORD8_T obj_idx) {
	struct opp_object*obj = (struct opp_object*)given;
	struct opp_object_ext*ext = (struct opp_object_ext*)(obj+1);
#ifdef OPP_HAS_TOKEN
	if(obuff->property & OPPF_EXTENDED) {
		ext->token = obuff->token_offset + pool->idx*obuff->pool_size + obj_idx;
	}
#endif
	obj->obuff = obuff;
#ifdef FACTORY_OBJ_SIGN
	obj->signature = FACTORY_OBJ_SIGN;
#endif
	if(obuff->property & OPPF_EXTENDED) {
		ext->flag = OPPN_ALL;
		ext->hash = 0;
	}
	CHECK_POOL(pool);
}

OPP_INLINE static void opp_alloc4_object_setup(struct opp_object*obj, const SYNC_UWORD8_T doubleref) {
	struct opp_factory*obuff = obj->obuff;
	if(*(obj->bitstring+BITFIELD_FINALIZE) & ( 1 << obj->bit_idx)) {
		OPP_FINALIZE_NOW(obuff,obj);
	}
	obj->refcount = doubleref?2:1;
//		*(obj->bitstring) |= ( 1 << obj->bit_idx);
	*(obj->bitstring+BITFIELD_FINALIZE) |= ( 1 << obj->bit_idx);
#ifndef OPP_NO_FLAG_OPTIMIZATION
	*(obj->bitstring+BITFIELD_PERFORMANCE) &= ~( 1 << obj->bit_idx);
	*(obj->bitstring+BITFIELD_PERFORMANCE1) &= ~( 1 << obj->bit_idx);
	*(obj->bitstring+BITFIELD_PERFORMANCE2) &= ~( 1 << obj->bit_idx);
	*(obj->bitstring+BITFIELD_PERFORMANCE3) &= ~( 1 << obj->bit_idx);
#endif
	obuff->use_count++;
#ifdef OPP_DEBUG_REFCOUNT
	obj->rt_idx = 0;
	int i;
	for(i=0; i<OPP_TRACE_SIZE; i++) {
		obj->ref_trace[i].filename = NULL;
	}
#endif

	SET_PAIRED_BITS(obj);
	*(obj->bitstring) |= ( 1 << obj->bit_idx); // mark as used
}

OPP_INLINE SYNC_UWORD8_T*opp_alloc4_build(const struct opp_factory*obuff, SYNC_UWORD8_T*given, SYNC_UWORD8_T doubleref, SYNC_UWORD8_T*require_clean, SYNC_UWORD8_T*require_init, void*init_data, const SYNC_UWORD8_T slot_count, va_list ap) {
	if(*require_clean) {
		opp_force_memclean(given);
		*require_clean = 0;
	}
	if((*require_init) && obuff->callback && obuff->callback(given, OPPN_ACTION_INITIALIZE
				, init_data
				,  ap, slot_count*obuff->obj_size - sizeof(struct opp_object))) {
		void*dup = given;
		OPPUNREF(given);
		if(doubleref) {
			OPPUNREF(dup);
		}
	}
	*require_init = 0;
	return given;
}


/**
 * @brief allocates memory in object pool
 */
void*opp_alloc4(struct opp_factory*obuff, SYNC_UWORD16_T size, SYNC_UWORD8_T doubleref, SYNC_UWORD8_T require_clean, void*init_data, ...) {
	SYNC_UWORD8_T*ret = NULL;
	SYNC_UWORD8_T slot_count = 1;
	SYNC_UWORD8_T require_init = 1;
	if(!require_clean)
		require_clean = obuff->property & OPPF_MEMORY_CLEAN;
	
	SYNC_ASSERT(obuff->sign == OPPF_INITIALIZED_INTERNAL);

	OPP_LOCK(obuff);
	do {
		/* get the number of slots we need */
		if(!(slot_count = opp_alloc4_count_slots(size, obuff->obj_size, obuff->pool_size))) {
			break;
		}
#ifdef OPP_HAS_RECYCLING
		if((ret = opp_alloc4_recycle(slot_count, obuff->pools))) {
			break;
		}
#endif
		/* find the space in some empty pool */
		struct opp_pool*pool = NULL,*addpoint = NULL;
		for(addpoint = NULL, pool = obuff->pools;pool;pool = pool->next) {
			if(!addpoint && (!pool->next || (pool->idx+1 != pool->next->idx))) {
				addpoint = pool;
			}
			CHECK_POOL(pool);
			/* find some space in the pool */
			SYNC_UWORD16_T obj_idx = 0;
			if((ret = opp_alloc4_find_space(pool, obuff->pool_size, slot_count, obuff->obj_size, &obj_idx))) {
				/* initialize the object */
				opp_alloc4_init_object(obuff, pool, ret, obj_idx);
				break;
			}
		}

		if(ret) {
#ifdef OPP_HAS_RECYCLING
			pool->recycled = NULL;
#endif
			break;
		}

		/* try to allocate new space */
		pool = opp_factory_create_pool_donot_use(obuff, addpoint, NULL);
		if(!pool) {
			ret = NULL;
			break;
		}
		ret = pool->head;
		opp_alloc4_init_object_bit_index((struct opp_object*)ret, 0, pool->bitstring, slot_count);
		opp_alloc4_init_object(obuff, pool, ret, 0);
	}while(0);

	va_list ap;
	va_start(ap, init_data);

	if(ret) {
		struct opp_object*obj = (struct opp_object*)ret;
		opp_alloc4_object_setup(obj, doubleref);
		ret = (SYNC_UWORD8_T*)(obj+1);
	}

	if(ret && !(obuff->property & OPPF_FAST_INITIALIZE) && obuff->callback) {
		ret = opp_alloc4_build(obuff, ret, doubleref, &require_clean, &require_init, init_data, slot_count, ap);
	}
#ifdef OPP_DEBUG
	if(obuff->pools && obuff->pools->bitstring) {
		BITSTRING_TYPE*bitstring;
		for(bitstring = obuff->pools->bitstring;((void*)bitstring) < ((void*)obuff->pools->head);bitstring+=BITFIELD_SIZE) {
			SYNC_ASSERT(!(*(bitstring) & *(bitstring+BITFIELD_PAIRED)));
		}
	}
#endif

	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	if(ret) {
		ret = opp_alloc4_build(obuff, ret, doubleref, &require_clean, &require_init, init_data, slot_count, ap);
	}
	va_end(ap);
//	SYNC_ASSERT(ret);
	return ret;
}

C_CAPSULE_END
