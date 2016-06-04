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


OPP_INLINE int opp_factory_create_full(struct opp_factory*obuff
		, SYNC_UWORD16_T inc
		, SYNC_UWORD16_T obj_size
		, int token_offset
		, unsigned char property
		, opp_callback_t callback
	) {
	SYNC_ASSERT(obj_size < (1024<<1));
	SYNC_ASSERT(inc);
#ifdef SYNC_LOW_MEMORY
	SYNC_ASSERT(inc < 1024);
#else
	SYNC_ASSERT(inc < (1024<<3));
#endif

	if(obuff->sign == OPPF_INITIALIZED_INTERNAL) {
		SYNC_LOG(SYNC_ERROR, "obj is already initiated\n");
		SYNC_ASSERT(obuff->sign != OPPF_INITIALIZED_INTERNAL);
		return -1;
	}
	/* check possible leak capacity */
	if(inc >= 0x7FFF) {
		SYNC_LOG(SYNC_ERROR, "too big allocation\n");
		SYNC_ASSERT(inc < 0x7FFF);
		return -1;
	}
	/* pool size */
	const SYNC_UWORD16_T pool_size = inc;
	/* obj size */
	SYNC_UWORD16_T fixed_obj_size = obj_size + sizeof(struct opp_object);
	fixed_obj_size = OPP_NORMALIZE_SIZE(fixed_obj_size);
	/* bitstring size */
	unsigned long long bitstring_size = (inc+7) >> 3;
	bitstring_size = OPP_NORMALIZE_SIZE(bitstring_size);
	bitstring_size = bitstring_size*BITFIELD_SIZE;
	/* check possible leak */
	if(bitstring_size >=  0x7FFF) {
		SYNC_LOG(SYNC_ERROR, "too big allocation\n");
		SYNC_ASSERT(bitstring_size < 0x7FFF);
		return -1;
	}
	const unsigned long long memory_chunk_size = sizeof(struct opp_pool) + fixed_obj_size*pool_size + bitstring_size;
	/* check possible leak */
	if(0x7FFFFFFF < memory_chunk_size) {
		SYNC_LOG(SYNC_ERROR, "too big allocation\n");
		SYNC_ASSERT(0x7FFFFFFF > memory_chunk_size);
		return -1;
	}
#ifndef OBJ_MAX_BUFFER_SIZE
#define OBJ_MAX_BUFFER_SIZE (4096<<10)
#endif
	SYNC_ASSERT(obuff->memory_chunk_size < OBJ_MAX_BUFFER_SIZE);
	struct opp_factory template = {
		.sign = OPPF_INITIALIZED_INTERNAL,
		.pool_size = pool_size,
		.pool_count = 0,
		.use_count = 0,
		.slot_use_count = 0,
		.token_offset = token_offset,
		.obj_size = fixed_obj_size,
		.bitstring_size = bitstring_size,
		.memory_chunk_size = memory_chunk_size,
		.internal_flags = 0,
		.property = property | OPPF_SWEEP_ON_UNREF,
		.callback = callback,
		.pools = NULL,
	};
	memcpy(obuff, &template, sizeof(template));
#ifdef OPP_BUFFER_HAS_LOCK
	if(property & OPPF_HAS_LOCK) {
		sync_mutex_init(&obuff->lock);
	}
#endif
	if(property & OPPF_SEARCHABLE) {
		SYNC_ASSERT(property & OPPF_EXTENDED);
		opp_lookup_table_init(&obuff->tree, 0);
	}
	return 0;
}

void opp_factory_destroy_use_profiler_instead(struct opp_factory*obuff) {
	BITSTRING_TYPE*bitstring;
	unsigned long k;
	struct opp_pool*pool;
	if(obuff->sign != OPPF_INITIALIZED_INTERNAL) {
		return;
	}

	OPP_LOCK(obuff);
	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
		bitstring = pool->bitstring;
		for(k=0;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {
			*bitstring = 0;
		}
	}
	opp_factory_gc_nolock(obuff);
	obuff->sign = 1;
	OPP_UNLOCK(obuff);
#ifdef OPP_BUFFER_HAS_LOCK
	if((obuff->property &  OPPF_HAS_LOCK)) {
		sync_mutex_destroy(&obuff->lock);
	}
#endif
}



C_CAPSULE_END
