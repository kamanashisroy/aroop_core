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
#endif

C_CAPSULE_START
void opp_factory_gc_nolock(struct opp_factory*obuff);
#ifdef SYNC_BIT64
#warning "Building 64 bit binary"
#else
#define SYNC_BIT32
#endif

#ifdef SYNC_BIT32
enum {
	BITSTRING_MASK = 0xFFFFFFFF
};
#define BITSTRING_TYPE SYNC_UWORD32_T
#define OPP_NORMALIZE_SIZE(x) ({(x+3)&~3;}) // multiple of 4
#define BIT_PER_STRING 32
#define OPP_INDEX_TO_BITSTRING_INDEX(x) (x>>5)
#define BITSTRING_IDX_TO_BITS(x) (x<<5)
#else
#ifdef SYNC_BIT64
//enum {
//	BITSTRING_MASK = 0xFFFFFFFFFFFFFFFF
//};

typedef uint64_t SYNC_UWORD64_T;
static const uint64_t BITSTRING_MASK = 0xFFFFFFFFFFFFFFFF;

#define BITSTRING_TYPE SYNC_UWORD64_T
#define OPP_NORMALIZE_SIZE(x) ({(x+7)&~7;})
#define BIT_PER_STRING 64
#define OPP_INDEX_TO_BITSTRING_INDEX(x) (x>>6)
#define BITSTRING_IDX_TO_BITS(x) (x<<6)
#endif
#endif

#ifndef SYNC_ASSERT
#define SYNC_ASSERT(x) assert(x)
#endif

#define AUTO_GC	// recommended
#define OPTIMIZE_OBJ_LOOP // recommended
#define OPP_HAS_TOKEN // recommended
#ifdef TEST_OBJ_FACTORY_UTILS
#define OPP_DEBUG
#endif

#ifdef OPP_DEBUG
#define OPP_DEBUG_REFCOUNT // only for testing
#endif
#ifdef OPP_DEBUG_REFCOUNT
enum {
	OPP_TRACE_SIZE = 32,
};
#endif

#ifdef OPP_DEBUG
#define FACTORY_OBJ_SIGN 0x24
#else
#undef FACTORY_OBJ_SIGN
#endif

#ifdef OPP_BUFFER_HAS_LOCK

#ifdef TEST_OBJ_FACTORY_UTILS
#define OPP_LOCK(x) do { \
	if((x)->property & OPPF_HAS_LOCK) while(sync_mutex_trylock(&(x)->lock)) { \
		sync_usleep(1); \
	} \
}while(0)
#else
#define OPP_LOCK(x) do { \
	int lock_alert = 0; \
	if((x)->property & OPPF_HAS_LOCK) while(sync_mutex_trylock(&(x)->lock)) { \
		lock_alert++; \
		if(lock_alert > 20) { \
			/*abort();*/ \
			opp_watchdog_report(WATCHDOG_ALERT, "lock failed 20 times ..\n"); \
		} \
		sync_usleep(1); \
	} \
}while(0)
#endif // TEST_OBJ_FACTORY_UTILS

#ifdef SYNC_HAS_ATOMIC_OPERATION
#define BINARY_AND_HELPER(xtype,x,y) ({volatile xtype old,new;do{old=*x;new=old&y;}while(!sync_do_compare_and_swap(x,old,new));})
#define BINARY_OR_HELPER(xtype,x,y) ({volatile xtype old,new;do{old=*x;new=old|y;}while(!sync_do_compare_and_swap(x,old,new));})
#else
#define BINARY_AND_HELPER(xtype,x,y) ({*x &= y;})
#define BINARY_OR_HELPER(xtype,x,y) ({*x |= y;})
#endif // SYNC_HAS_ATOMIC_OPERATION


#define OPP_UNLOCK(x) do { \
	if((x)->property & OPPF_HAS_LOCK) { \
		SYNC_ASSERT(!sync_mutex_unlock(&(x)->lock)); \
	} \
}while(0)
#else // OPP_BUFFER_HAS_LOCK
#define OPP_LOCK(x)
#define OPP_UNLOCK(x)
#define BINARY_AND_HELPER(xtype,x,y) ({*x &= y;})
#define BINARY_OR_HELPER(xtype,x,y) ({*x |= y;})
#endif

#ifdef OPP_DEBUG
#ifdef OPP_HAS_RECYCLING
#define CHECK_POOL(x) ({SYNC_ASSERT((x)->end > (x)->head \
	&& (x)->head > ((SYNC_UWORD8_T*)(x)->bitstring) \
	&& ((SYNC_UWORD8_T*)(x)->bitstring) > ((SYNC_UWORD8_T*)(x)) \
	&& (!(x)->recycled || ((SYNC_UWORD8_T*)(x)->recycled) < (x)->end) \
	&& (((BITSTRING_TYPE*)((x)+1)) == (x)->bitstring));})
#else
#define CHECK_POOL(x) ({SYNC_ASSERT((x)->end > (x)->head \
	&& (x)->head > ((SYNC_UWORD8_T*)(x)->bitstring) \
	&& ((SYNC_UWORD8_T*)(x)->bitstring) > ((SYNC_UWORD8_T*)(x)) \
	&& (((BITSTRING_TYPE*)((x)+1)) == (x)->bitstring));})
#endif
#else
#define CHECK_POOL(x)
#endif

#ifdef OPP_DEBUG
#define CHECK_OBJ(x) ({SYNC_ASSERT((x)->signature == FACTORY_OBJ_SIGN);})
#else
#define CHECK_OBJ(x)
#endif

#ifdef AUTO_GC
#define DO_AUTO_GC_CHECK(x) do{\
	if((x)->pool_count*(x)->pool_size - (x)->slot_use_count > ((x)->pool_size << 1)) { \
		opp_factory_gc_nolock(x); \
	} \
}while(0);
#else
#define DO_AUTO_GC_CHECK(x)
#endif

#define OPP_FINALIZE_NOW(x,y) ({\
	if((x->property & OPPF_SEARCHABLE) && (((struct opp_object_ext*)(y+1))->flag & OPPN_INTERNAL_1))\
		opp_lookup_table_delete(&x->tree, (struct opp_object_ext*)(y+1));\
	if(x->callback){static va_list va;x->callback(y+1, OPPN_ACTION_FINALIZE, NULL, va, y->slots*x->obj_size - sizeof(struct opp_object));}*(y->bitstring+BITFIELD_FINALIZE) &= ~( 1 << y->bit_idx);UNSET_PAIRED_BITS(y);\
})

enum {
	OPP_POOL_FREEABLE = 1,
};
struct opp_pool {
	SYNC_UWORD16_T idx;
	SYNC_UWORD16_T flags;
#ifdef OPP_HAS_RECYCLING
	struct opp_object*recycled;
#endif
	BITSTRING_TYPE*bitstring; // each word(doublebyte) pair contains the usage and finalization flag respectively.
	SYNC_UWORD8_T*head;
	SYNC_UWORD8_T*end;
	struct opp_pool*next;
};

// bit hacks http://graphics.stanford.edu/~seander/bithacks.html

enum {
	BITFIELD_FINALIZE = 1,
	BITFIELD_PAIRED = 2,
#ifndef OPP_NO_FLAG_OPTIMIZATION
	BITFIELD_PERFORMANCE = 3,
	BITFIELD_PERFORMANCE1 = 4,
	BITFIELD_PERFORMANCE2 = 5,
	BITFIELD_PERFORMANCE3 = 6,
	BITFIELD_SIZE = 7,
#else
	BITFIELD_SIZE = 3,
#endif
};

#ifndef OPP_NO_FLAG_OPTIMIZATION
enum {
	OPPN_PERFORMANCE = 1,
	OPPN_PERFORMANCE1 = 1<<1,
	OPPN_PERFORMANCE2 = 1<<2,
	OPPN_PERFORMANCE3 = 1<<3,
};
#endif

enum {
	OPPF_INITIALIZED_INTERNAL = 0x3428,
};

enum {
	OPP_INTERNAL_FLAG_NO_GC_NOW = 1,
};

#ifdef AROOP_LOW_MEMORY
#define refcount_t OPP_VOLATILE_VAR SYNC_UWORD16_T
#else
#define refcount_t OPP_VOLATILE_VAR SYNC_UWORD32_T
#endif

struct opp_object {
	SYNC_UWORD8_T bit_idx;
	SYNC_UWORD8_T slots;
	refcount_t refcount;
#ifdef FACTORY_OBJ_SIGN
	SYNC_UWORD32_T signature;
#endif
	BITSTRING_TYPE*bitstring;
#ifdef OPP_HAS_RECYCLING
	struct opp_object*recycled;
#endif
#ifdef OPP_DEBUG_REFCOUNT
	struct {
		const char *filename;
		SYNC_UWORD16_T lineno;
		refcount_t refcount;
		char op;
		char flags[3];
	}ref_trace[OPP_TRACE_SIZE];
	SYNC_UWORD32_T rt_idx;
#endif
	struct opp_factory*obuff;
};

#define SET_PAIRED_BITS(x) do{\
	SYNC_ASSERT((x)->slots <= BIT_PER_STRING); \
	if((x)->slots>1) { \
		*((x)->bitstring+BITFIELD_PAIRED) |= ((1<<((x)->slots-1))-1)<<((x)->bit_idx+1); \
	} \
	SYNC_ASSERT(!(*((x)->bitstring) & *((x)->bitstring+BITFIELD_PAIRED))); \
	(x)->obuff->slot_use_count += (x)->slots; \
}while(0);

#define UNSET_PAIRED_BITS(x) do{\
	SYNC_ASSERT((x)->slots <= BIT_PER_STRING); \
	/*SYNC_ASSERT(bitfield == (bitfield & *((x)->bitstring+BITFIELD_PAIRED)));*/ \
	if((x)->slots>1) { \
		*((x)->bitstring+BITFIELD_PAIRED) &= ~(((1<<((x)->slots-1))-1)<<((x)->bit_idx+1)); \
	} \
	(x)->obuff->slot_use_count -= (x)->slots; \
	(x)->slots = 0; \
}while(0);

#if 0
#define SYNC_OBJ_CTZ(x) 
#else
#if defined(__EPOC32__) || defined(RASPBERRY_PI_BARE_METAL) // todo add symbian version
#define SYNC_OBJ_POPCOUNT(a) ({ \
    unsigned int opp_internal_pop_count = (unsigned int)a; \
    opp_internal_pop_count = opp_internal_pop_count - ((opp_internal_pop_count >> 1) & 0x55555555); \
    /* Every 2 bits holds the sum of every pair of bits */ \
    opp_internal_pop_count = ((opp_internal_pop_count >> 2) & 0x33333333) + (opp_internal_pop_count & 0x33333333); \
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) */ \
    opp_internal_pop_count = (opp_internal_pop_count + (opp_internal_pop_count >> 4)) & 0x0F0F0F0F; \
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) */ \
    opp_internal_pop_count = (opp_internal_pop_count + (opp_internal_pop_count >> 16)); \
    /* The lower 16 bits hold two 8 bit sums (5 significant bits).*/ \
    /*    Upper 16 bits are garbage */ \
    (opp_internal_pop_count + (opp_internal_pop_count >> 8)) & 0x0000003F;  /* (6 significant bits) */ \
})
#else
#define SYNC_OBJ_POPCOUNT(x) __builtin_popcount(x)
#endif
#define SYNC_OBJ_CTZ(x) __builtin_ctz(x)
#endif

#define data_to_opp_object(x) ({(struct opp_object*)pointer_arith_sub_byte(x,sizeof(struct opp_object));})

C_CAPSULE_END
