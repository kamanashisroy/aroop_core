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


#define CHECK_WEAK_OBJ(x) ({if(!x->refcount){bsv &= ~( 1 << bit_idx);continue;}})
void*opp_search(struct opp_factory*obuff
	, opp_hash_t hash, obj_comp_t compare_func, const void*compare_data, void**rval) {
	void*ret = NULL;
	SYNC_ASSERT(obuff->property & OPPF_SEARCHABLE);
	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if((ret = opp_lookup_table_search(&obuff->tree, hash, compare_func, compare_data))) {
		OPPREF(ret);
	}
	obuff->internal_flags &= ~gcflag;
	OPP_UNLOCK(obuff);
	if(rval != NULL) {
		*rval = ret;
	}
	return ret;
}

void*opp_find_list_full_donot_use(struct opp_factory*obuff, obj_comp_t compare_func
		, const void*compare_data, unsigned int if_flag, unsigned int if_not_flag, opp_hash_t hash, int shouldref) {
	int k;
	BITSTRING_TYPE*bitstring,bsv;
	SYNC_UWORD16_T oflag;
	SYNC_UWORD16_T bit_idx, obj_idx;
	void *retval = NULL;
	const struct opp_object *obj = NULL;
	const opp_pointer_ext_t*item = NULL;
#ifdef OPTIMIZE_OBJ_LOOP
	int use_count = 0, iteration_count = 0;
#endif
	struct opp_pool*pool;
	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if(!obuff->use_count) {
		goto exit_point;
	}
#ifdef OPTIMIZE_OBJ_LOOP
	use_count = obuff->use_count;
#endif
	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
#ifdef OPTIMIZE_OBJ_LOOP
		if(iteration_count >= use_count) {
			break;
		}
#endif
		// traverse the bitset
		k = 0;
		bitstring = pool->bitstring;
		for(;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {

			bsv = *bitstring;
#ifdef OPTIMIZE_OBJ_LOOP
			if(iteration_count >= use_count) {
				break;
			}
			iteration_count += SYNC_OBJ_POPCOUNT(bsv);
#endif
			// get the bits to finalize
			while(bsv) {
				CHECK_POOL(pool);
				bit_idx = SYNC_OBJ_CTZ(bsv);
				obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < obuff->pool_size) {
					obj = (struct opp_object*)(pool->head + obj_idx*obuff->obj_size);
					CHECK_OBJ(obj);
					CHECK_WEAK_OBJ(obj);
					item = (opp_pointer_ext_t*)(obj+1);
					obj = data_to_opp_object(item->obj_data);
					CHECK_OBJ(obj);
					CHECK_WEAK_OBJ(obj);
					if(obuff->property & OPPF_EXTENDED) {
						oflag = ((struct opp_object_ext*)(obj+1))->flag;
						if(!(oflag & if_flag) || (oflag & if_not_flag) || (hash != 0 && ((struct opp_object_ext*)(obj+1))->hash != hash)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					if(compare_func(compare_data, item->obj_data)) {
						retval = item->obj_data;
						if(shouldref) {
							OPPREF(retval);
						}
						goto exit_point;
					}
					CHECK_OBJ(obj);
					// clear
					bsv &= ~( 1 << bit_idx);
				}
			}
		}
	}
	exit_point:
	obuff->internal_flags &= ~gcflag;
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	return retval;
}


void*opp_find_full(struct opp_factory*obuff, obj_comp_t compare_func, const void*compare_data
		, unsigned int if_flag, unsigned int if_not_flag, opp_hash_t hash, unsigned int shouldref) {
	int k;
	BITSTRING_TYPE*bitstring,bsv;
	void*data;
	SYNC_UWORD16_T oflag;
	SYNC_UWORD16_T bit_idx,obj_idx;
	void *retval = NULL;
	const struct opp_object *obj = NULL;
#ifdef OPTIMIZE_OBJ_LOOP
	int use_count = 0, iteration_count = 0;
#endif
	struct opp_pool*pool;
	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if(!obuff->use_count) {
		goto exit_point;
	}
#ifdef OPTIMIZE_OBJ_LOOP
	use_count = obuff->use_count;
#endif

	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
#ifdef OPTIMIZE_OBJ_LOOP
		if(iteration_count >= use_count) {
			break;
		}
#endif
		// traverse the bitset
		k = 0;
		bitstring = pool->bitstring;
		for(;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {

			bsv = *bitstring;
#ifdef OPTIMIZE_OBJ_LOOP
			if(iteration_count >= use_count) {
				break;
			}
			iteration_count += SYNC_OBJ_POPCOUNT(bsv);
#endif
#ifndef OPP_NO_FLAG_OPTIMIZATION
			if(obuff->property & OPPF_EXTENDED) {
				if((if_flag & OPPN_PERFORMANCE) && !(if_flag & ~OPPN_PERFORMANCE)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE);
				}
				if(if_not_flag & OPPN_PERFORMANCE) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE));
				}
				if((if_flag & OPPN_PERFORMANCE1) && !(if_flag & ~OPPN_PERFORMANCE1)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE1);
				}
				if(if_not_flag & OPPN_PERFORMANCE1) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE1));
				}
				if((if_flag & OPPN_PERFORMANCE2) && !(if_flag & ~OPPN_PERFORMANCE2)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE2);
				}
				if(if_not_flag & OPPN_PERFORMANCE2) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE2));
				}
				if((if_flag & OPPN_PERFORMANCE3) && !(if_flag & ~OPPN_PERFORMANCE3)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE3);
				}
				if(if_not_flag & OPPN_PERFORMANCE3) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE3));
				}
			}
#endif
			// get the bits to finalize
			while(bsv) {
				CHECK_POOL(pool);
				bit_idx = SYNC_OBJ_CTZ(bsv);
				obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < obuff->pool_size) {
					obj = (const struct opp_object*)(pool->head + obj_idx*obuff->obj_size);
					data = (void*)(obj+1);
					CHECK_OBJ(obj);
					CHECK_WEAK_OBJ(obj);
					if(obuff->property & OPPF_EXTENDED) {
						oflag = ((struct opp_object_ext*)(data))->flag;
						if(!(oflag & if_flag) || (oflag & if_not_flag) || (hash != 0 && ((struct opp_object_ext*)(data))->hash != hash)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					if((compare_func?compare_func(compare_data, data):1)) {
						retval = data;
						if(shouldref) {
							OPPREF(retval);
						}
						goto exit_point;
					}
					CHECK_OBJ(obj);
					// clear
					bsv &= ~( 1 << bit_idx);
				}
			}
		}
	}
	exit_point:
	obuff->internal_flags &= ~gcflag;
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	return retval;
}

int opp_count_donot_use(const struct opp_factory*obuff) {
	return obuff->use_count;
}

void opp_factory_gc_nolock(struct opp_factory*obuff) {
	if(obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW)
		return;
	int k;
	BITSTRING_TYPE*bitstring,bsv;
	int use_count;
	SYNC_UWORD16_T bit_idx,obj_idx;
	struct opp_object*obj;
	struct opp_pool*pool,*prev_pool,*next;
	opp_factory_profiler_checkleak();
	for(pool = obuff->pools, prev_pool = NULL;pool;pool = next) {
		use_count = 0;

		CHECK_POOL(pool);
		next = pool->next;
		// traverse the bitset
		k = 0;
		bitstring = pool->bitstring;
		for(;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {

			use_count += SYNC_OBJ_POPCOUNT((*bitstring) & BITSTRING_MASK);
			// get the bits to finalize
			while((bsv = (~(*bitstring)) & (*(bitstring+BITFIELD_FINALIZE)))) {
				CHECK_POOL(pool);
				bit_idx = SYNC_OBJ_CTZ(bsv);
				obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < obuff->pool_size) {
					obj = (struct opp_object*)(pool->head + obj_idx*obuff->obj_size);
					OPP_FINALIZE_NOW(obuff,obj);
				}
			}
		}

		// destroy memory chunk
		if(use_count == 0) {
#ifdef FACTORY_OBJ_SIGN
//			obj = (struct sync_object*)pool->head;
//			CHECK_OBJ(obj);
#endif
			if(prev_pool) {
				prev_pool->next = pool->next;
			} else {
				obuff->pools = pool->next;
			}
			if(pool->flags & OPP_POOL_FREEABLE) {
				//sync_free(pool);
				profiler_replace_free(pool, obuff->memory_chunk_size);
			}
			pool = NULL;
			obuff->pool_count--;
			opp_factory_profiler_checkleak();
		} else {
			prev_pool = pool;
		}
	}
	opp_factory_profiler_checkleak();
}

int opp_exists(struct opp_factory*obuff, const void*data) {
	struct opp_pool*pool = NULL;
	int found = 0;
	OPP_LOCK(obuff);

	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
		// TODO check the bitflag not the refcount
		if(data >= (void*)pool->head && data < (void*)pool->end) {
			found = 1;
			CHECK_OBJ((struct opp_object*)((unsigned char*)data - sizeof(struct opp_object)));
			break;
		}
	}
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
	return found;
}

void opp_factory_lock_donot_use(struct opp_factory*obuff) {
	OPP_LOCK(obuff);
}

void opp_factory_unlock_donot_use(struct opp_factory*obuff) {
	OPP_UNLOCK(obuff);
}

void opp_factory_gc_donot_use(struct opp_factory*obuff) {
	OPP_LOCK(obuff);
	opp_factory_gc_nolock(obuff);
	OPP_UNLOCK(obuff);
}

void opp_factory_list_do_full(struct opp_factory*obuff, obj_do_t obj_do, void*func_data
		, unsigned int if_list_flag, unsigned int if_not_list_flag, unsigned int if_flag, unsigned int if_not_flag
		, opp_hash_t list_hash, opp_hash_t hash) {
	int k;
	BITSTRING_TYPE*bitstring,bsv;
	SYNC_UWORD16_T oflag;
	SYNC_UWORD16_T bit_idx, obj_idx;
	const struct opp_object *obj = NULL;
	opp_pointer_ext_t*item = NULL;
#ifdef OPTIMIZE_OBJ_LOOP
	int use_count = 0, iteration_count = 0;
#endif
	struct opp_pool*pool;
	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if(!obuff->use_count) {
		goto exitpoint;
	}
#ifdef OPTIMIZE_OBJ_LOOP
	use_count = obuff->use_count;
#endif
	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
#ifdef OPTIMIZE_OBJ_LOOP
		if(iteration_count >= use_count) {
			break;
		}
#endif

		// traverse the bitset
		k = 0;
		bitstring = pool->bitstring;
		for(;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {

			bsv = *bitstring;
#ifdef OPTIMIZE_OBJ_LOOP
			if(iteration_count >= use_count) {
				break;
			}
			iteration_count += SYNC_OBJ_POPCOUNT(bsv);
#endif
#ifndef OPP_NO_FLAG_OPTIMIZATION
			if(obuff->property & OPPF_EXTENDED) {
				if((if_list_flag & OPPN_PERFORMANCE) && !(if_list_flag & ~OPPN_PERFORMANCE)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE);
				}
				if(if_not_list_flag & OPPN_PERFORMANCE) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE));
				}
				if((if_list_flag & OPPN_PERFORMANCE1) && !(if_list_flag & ~OPPN_PERFORMANCE1)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE1);
				}
				if(if_not_list_flag & OPPN_PERFORMANCE1) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE1));
				}
				if((if_list_flag & OPPN_PERFORMANCE2) && !(if_list_flag & ~OPPN_PERFORMANCE2)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE2);
				}
				if(if_not_list_flag & OPPN_PERFORMANCE2) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE2));
				}
				if((if_list_flag & OPPN_PERFORMANCE3) && !(if_list_flag & ~OPPN_PERFORMANCE3)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE3);
				}
				if(if_not_list_flag & OPPN_PERFORMANCE3) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE3));
				}
			}
#endif
			// get the bits to finalize
			while(bsv) {
				CHECK_POOL(pool);
				bit_idx = SYNC_OBJ_CTZ(bsv);
				obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < obuff->pool_size) {
					obj = (struct opp_object*)(pool->head + obj_idx*obuff->obj_size);
					CHECK_WEAK_OBJ(obj);
					CHECK_OBJ(obj);
					item = (opp_pointer_ext_t*)(obj+1);
					if(obuff->property & OPPF_EXTENDED) {
						oflag = ((struct opp_object_ext*)(obj+1))->flag;
						if(!(oflag & if_flag) || (oflag & if_not_flag)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					obj = data_to_opp_object(item->obj_data);
					CHECK_WEAK_OBJ(obj);
					CHECK_OBJ(obj);
					if(obj->obuff->property & OPPF_EXTENDED) {
						oflag = ((struct opp_object_ext*)(obj+1))->flag;
						if(!(oflag & if_flag) || (oflag & if_not_flag) || (hash != 0 && ((struct opp_object_ext*)(obj+1))->hash != hash)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					if(obj_do && obj_do(func_data, item)) {
						goto exitpoint;
					}
					CHECK_OBJ(obj);
					// clear
					bsv &= ~( 1 << bit_idx);
				}
			}
		}
	}
exitpoint:
	obuff->internal_flags &= ~gcflag;
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
}

int opp_iterator_create(struct opp_iterator*iterator, struct opp_factory*fac, unsigned int if_flag, unsigned int if_not_flag, opp_hash_t hash) {
	iterator->fac = fac;
	iterator->pool = NULL;
	iterator->k = 0;
	iterator->if_flag = if_flag;
	iterator->if_not_flag = if_not_flag;
	iterator->hash = hash;
	iterator->bit_idx = -1;
	iterator->data = NULL;
	return 0;
}

void*opp_iterator_next(struct opp_iterator*iterator) {
	struct opp_pool*pool = NULL;
	if(!iterator->fac->use_count) {
		return NULL;
	}
	OPP_LOCK(iterator->fac);
	const int gcflag = iterator->fac->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	iterator->fac->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if(iterator->data)OPPUNREF_UNLOCKED(iterator->data);
	for(pool = iterator->fac->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
		if(iterator->pool && pool != iterator->pool) {
			continue;
		}
		// traverse the bitset
		iterator->pool = NULL;
		int k = iterator->k;
		iterator->k = 0;
		const int pool_size = iterator->fac->pool_size;
		BITSTRING_TYPE*bitstring,bsv;
		bitstring = pool->bitstring;
		bitstring += k*BITFIELD_SIZE;

		for(;BITSTRING_IDX_TO_BITS(k) < pool_size;k++,bitstring+=BITFIELD_SIZE) {

			bsv = *bitstring;
			// test performance flag
#ifndef OPP_NO_FLAG_OPTIMIZATION
			if(bsv && (iterator->fac->property & OPPF_EXTENDED)) {
				if((iterator->if_flag & OPPN_PERFORMANCE) && !(iterator->if_flag & ~OPPN_PERFORMANCE)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE);
				}
				if(iterator->if_not_flag & OPPN_PERFORMANCE) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE));
				}
				if((iterator->if_flag & OPPN_PERFORMANCE1) && !(iterator->if_flag & ~OPPN_PERFORMANCE1)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE1);
				}
				if(iterator->if_not_flag & OPPN_PERFORMANCE1) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE1));
				}
				if((iterator->if_flag & OPPN_PERFORMANCE2) && !(iterator->if_flag & ~OPPN_PERFORMANCE2)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE2);
				}
				if(iterator->if_not_flag & OPPN_PERFORMANCE2) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE2));
				}
				if((iterator->if_flag & OPPN_PERFORMANCE3) && !(iterator->if_flag & ~OPPN_PERFORMANCE3)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE3);
				}
				if(iterator->if_not_flag & OPPN_PERFORMANCE3) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE3));
				}
			}
#endif
			if(iterator->bit_idx != -1) {
				if(iterator->bit_idx == BIT_PER_STRING-1) {
					bsv = 0;
				} else {
					bsv &= ~(( 1 << (iterator->bit_idx+1))-1);
				}
				iterator->bit_idx = -1;
			}

			// get the bits to finalize
			while(bsv) {
				CHECK_POOL(pool);
				SYNC_UWORD16_T bit_idx = SYNC_OBJ_CTZ(bsv);
				SYNC_UWORD16_T obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < pool_size) {
					struct opp_object *obj = (struct opp_object *)(pool->head + obj_idx*iterator->fac->obj_size);
					CHECK_OBJ(obj);
					if(iterator->fac->property & OPPF_EXTENDED) {
						SYNC_UWORD16_T oflag = ((struct opp_object_ext*)(obj+1))->flag;
						if(!(oflag & iterator->if_flag) || (oflag & iterator->if_not_flag) || (iterator->hash != 0 && ((struct opp_object_ext*)(obj+1))->hash != iterator->hash)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					void*itdata = OPPREF((obj+1));
					if(!itdata) {
						bsv &= ~( 1 << bit_idx);
						continue;
					}
					iterator->bit_idx = bit_idx;
					iterator->k = k;
					iterator->data = itdata;
					iterator->pool = pool;
					goto exit_point;
				}
			}
		}
	}
exit_point:
	iterator->fac->internal_flags &= ~gcflag;
	OPP_UNLOCK(iterator->fac);
	return iterator->data;
}

int opp_iterator_destroy(struct opp_iterator*iterator) {
	if(iterator->data)OPPUNREF(iterator->data);
	return 0;
}

int opp_factory_is_initialized(struct opp_factory*obuff) {
	return (obuff->sign == OPPF_INITIALIZED_INTERNAL);
}

void opp_factory_do_pre_order(struct opp_factory*obuff, obj_do_t obj_do, void*func_data, unsigned int if_flag
		, unsigned int if_not_flag) {
	SYNC_ASSERT(obuff->property & OPPF_SEARCHABLE);
	OPP_LOCK(obuff);
	opp_lookup_table_traverse(&obuff->tree, obj_do, func_data, if_flag, if_not_flag);
	OPP_UNLOCK(obuff);
}

void opp_factory_do_full(struct opp_factory*obuff, obj_do_t obj_do, void*func_data, unsigned int if_flag
		, unsigned int if_not_flag, opp_hash_t hash) {
#if 0
	int i;
	SYNC_UWORD8_T*j;
	OPP_LOCK(obuff);
	for(i=0;i<OBJ_MAX_POOL_COUNT;i++) {
		if(pool->bitstring) {
			j = pool->head;
			for(;j<pool->end;j+=obuff->obj_size) {
				if(((struct opp_object*)(j))->refcount && (((struct opp_object*)(j))->flag & if_flag) && !(((struct opp_object*)(j))->flag & if_not_flag) && (hash == 0 || hash == ((struct opp_object*)(j))->flag)) {
					if(obj_do && obj_do(func_data, j + sizeof(struct opp_object))) {
						goto exitpoint;
					}
				}
			}
		}
	}
exitpoint:
	OPP_UNLOCK(obuff);
#else
	int k;
	BITSTRING_TYPE*bitstring,bsv;
	SYNC_UWORD16_T oflag;
	SYNC_UWORD16_T bit_idx, obj_idx;
	const struct opp_object *obj = NULL;
#ifdef OPTIMIZE_OBJ_LOOP
	int use_count = 0, iteration_count = 0;
#endif
	struct opp_pool*pool;
	OPP_LOCK(obuff);
	const int gcflag = obuff->internal_flags & OPP_INTERNAL_FLAG_NO_GC_NOW;
	obuff->internal_flags |= OPP_INTERNAL_FLAG_NO_GC_NOW;
	if(!obuff->use_count) {
		goto exitpoint;
	}
#ifdef OPTIMIZE_OBJ_LOOP
	use_count = obuff->use_count;
#endif
	for(pool = obuff->pools;pool;pool = pool->next) {
		CHECK_POOL(pool);
#ifdef OPTIMIZE_OBJ_LOOP
		if(iteration_count >= use_count) {
			break;
		}
#endif
		// traverse the bitset
		k = 0;
		bitstring = pool->bitstring;
		for(;BITSTRING_IDX_TO_BITS(k) < obuff->pool_size;k++,bitstring+=BITFIELD_SIZE) {

			bsv = *bitstring;
#ifdef OPTIMIZE_OBJ_LOOP
			if(iteration_count >= use_count) {
				break;
			}
			iteration_count += SYNC_OBJ_POPCOUNT(bsv);
#endif
#ifndef OPP_NO_FLAG_OPTIMIZATION
			if(obuff->property & OPPF_EXTENDED) {
				if((if_flag & OPPN_PERFORMANCE) && !(if_flag & ~OPPN_PERFORMANCE)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE);
				}
				if((if_flag & OPPN_PERFORMANCE1) && !(if_flag & ~OPPN_PERFORMANCE1)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE1);
				}
				if((if_flag & OPPN_PERFORMANCE2) && !(if_flag & ~OPPN_PERFORMANCE2)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE2);
				}
				if((if_flag & OPPN_PERFORMANCE3) && !(if_flag & ~OPPN_PERFORMANCE3)) {
					bsv &= *(bitstring+BITFIELD_PERFORMANCE3);
				}
				if(if_not_flag & OPPN_PERFORMANCE) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE));
				}
				if(if_not_flag & OPPN_PERFORMANCE1) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE1));
				}
				if(if_not_flag & OPPN_PERFORMANCE2) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE2));
				}
				if(if_not_flag & OPPN_PERFORMANCE3) {
					bsv &= ~(*(bitstring+BITFIELD_PERFORMANCE3));
				}
			}
#endif
			// get the bits to finalize
			while(bsv) {
				CHECK_POOL(pool);
				bit_idx = SYNC_OBJ_CTZ(bsv);
				obj_idx = BITSTRING_IDX_TO_BITS(k) + bit_idx;
				if(obj_idx < obuff->pool_size) {
					obj = (struct opp_object*)(pool->head + obj_idx*obuff->obj_size);
					CHECK_OBJ(obj);
					if(obuff->property & OPPF_EXTENDED) {
						oflag = ((struct opp_object_ext*)(obj+1))->flag;
						if(!(oflag & if_flag) || (oflag & if_not_flag) || (hash != 0 && ((struct opp_object_ext*)(obj+1))->hash != hash)) {
							// clear
							bsv &= ~( 1 << bit_idx);
							continue;
						}
					}
					if(obj_do && obj_do(func_data, (void*)(obj+1))) {
						goto exitpoint;
					}
//					CHECK_OBJ(obj);
					// clear
					bsv &= ~( 1 << bit_idx);
				}
			}
		}
	}
exitpoint:
	obuff->internal_flags &= ~gcflag;
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
#endif
}

struct dump_log {
	void (*log)(void *log_data, const char*fmt, ...);
	void*log_data;
};

#ifdef OPP_DEBUG_REFCOUNT
static int obj_debug_dump(const void*data, const void*func_data) {
	const struct dump_log*dump = (const struct dump_log*)func_data;
	const struct opp_object*obj = data_to_opp_object(data);
	int i;

	dump->log(dump->log_data, "== Ref:%d\n", obj->refcount);
	int idx = obj->rt_idx+1;
	for(i=0; i<OPP_TRACE_SIZE; i++,idx++) {
		idx = idx%OPP_TRACE_SIZE;

		if(obj->ref_trace[idx].filename) {
			dump->log(dump->log_data, "== count:%d[%c],file:%s(%d)\n"
					, obj->ref_trace[idx].refcount
					, obj->ref_trace[idx].op
					, obj->ref_trace[idx].filename
					, obj->ref_trace[idx].lineno);
		}
	}
	return 0;
}
#endif

void opp_factory_verb(struct opp_factory*obuff, opp_verb_t verb_obj, const void*func_data, void (*log)(void *log_data, const char*fmt, ...), void*log_data) {
#if 0
	int i, use_count = 0, pool_count = 0;
	SYNC_UWORD8_T*j;
	OPP_LOCK(obuff);
	for(i=0;i<OBJ_MAX_POOL_COUNT;i++) {
		if(pool->bitstring) {
			j = pool->head;
			for(;j<pool->end;j+=obuff->obj_size) {
				if(((struct opp_object*)(j))->refcount) {
					if(verb_obj)verb_obj(j + sizeof(struct opp_object), func_data);
					use_count++;
				}
			}
			pool_count++;
		}
	}

	if(log)
	log(log_data, "pool count:%d, use count: %d, total memory: %d bytes, total used: %d bytes"
//#ifdef OPP_DEBUG
	", obuff use count(internal) :%d"
	", obuff slot use count(internal) :%d"
	", bitstring size :%d"
	", pool size :%d"
//#endif
	"\n"
		, pool_count
		, use_count
		, (int)(pool_count*obuff->memory_chunk_size)
		, (int)(obuff->slot_use_count*obuff->obj_size)
//#ifdef OPP_DEBUG
		, obuff->use_count
		, obuff->slot_use_count
		, (int)obuff->bitstring_size
		, (int)obuff->pool_size
//#endif
	);
	DO_AUTO_GC_CHECK(obuff);
	OPP_UNLOCK(obuff);
#else
#ifdef OPP_DEBUG_REFCOUNT
	struct dump_log dump = {.log = log, .log_data = log_data};
	if(!verb_obj) {
		verb_obj = obj_debug_dump;
		func_data = &dump;
	}
#endif
	OPP_LOCK(obuff);
	opp_factory_do_full(obuff, (int (*)(void*, void*))verb_obj, (void*)func_data, OPPN_ALL, 0, 0);
	if(log) {
#if 0
		if((obuff->property & OPPF_SEARCHABLE) && obuff->tree.rb_count) {
			opp_lookup_table_verb(&obuff->tree, "hashtable", log, log_data);
		}
#endif
		log(log_data, "pool count:%d, total memory: %d bytes, used: %d bytes"
		", %d objs"
		", slots :%d"
		", bitstring size :%d"
		", pool size :%d"
		"\n"
			, obuff->pool_count
			, (int)(obuff->pool_count*obuff->memory_chunk_size)
			, (int)(obuff->slot_use_count*obuff->obj_size)
			, obuff->use_count
			, obuff->slot_use_count
			, (int)obuff->bitstring_size
			, (int)obuff->pool_size
		);
	}
	OPP_UNLOCK(obuff);
#endif
}
C_CAPSULE_END
