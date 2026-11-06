/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Atomic Compare And Exchange Routine (8, 16, 32 bit; non-blocking)
 ******************************************************************************/
#ifndef	CompareAndExchange

//#if	defined( __cplusplus )
//extern	"C"	{
//#endif

#include	<common.h>

/*
 * Shared with atomic_bump.h (which includes this header): defined once,
 * so both headers can be included in any order.
 */

#ifndef	LOCK_PREFIX
#define	LOCK_PREFIX	"lock "
#endif

#ifndef	XCHG_DUMMY_DEFINED
#define	XCHG_DUMMY_DEFINED
struct __xchg_dummy { u32 a[100]; };
#define __xg(x) ((struct __xchg_dummy *)(x))
#endif

/*
 * Atomic compare and exchange.  Compare OLD with MEM, if identical,
 * store NEW in MEM.  Return the initial value in MEM.  Success is
 * indicated by comparing RETURN with OLD.
 */

static inline u32 __cmpxchg(volatile void *mem, u32 old,
				      u32 new, int size)
{
	u32 prev;
	switch (size) {
	case 1:
		__asm__ __volatile__(LOCK_PREFIX "cmpxchgb %b1,%2"
				     : "=a"(prev)
				     : "q"(new), "m"(*__xg(mem)), "0"(old)
				     : "memory");
		return prev;
	case 2:
		__asm__ __volatile__(LOCK_PREFIX "cmpxchgw %w1,%2"
				     : "=a"(prev)
				     : "q"(new), "m"(*__xg(mem)), "0"(old)
				     : "memory");
		return prev;
	case 4:
		__asm__ __volatile__(LOCK_PREFIX "cmpxchgl %1,%2"
				     : "=a"(prev)
				     : "q"(new), "m"(*__xg(mem)), "0"(old)
				     : "memory");
		return prev;
	}
	return old;
}

#define CompareAndExchange(mem,o,n)\
	((__typeof__(*(mem)))__cmpxchg((mem),(u32)(o),\
					(u32)(n),sizeof(*(mem))))

//#if	defined( __cplusplus )
//	}
//#endif

#endif
