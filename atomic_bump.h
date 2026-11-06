/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2007-2026 John R. Smythe Jr. */
/*******************************************************************************
 * 
 *	Atomic Inline Bump Routines
 ******************************************************************************/
#ifndef	ATOMIC_BUMP_H
#define	ATOMIC_BUMP_H

#if	defined( __cplusplus )
extern	"C"	{
#endif

#include	<common.h>

typedef struct { volatile u32 counter; } bump_32;

#ifndef	LOCK
#define	LOCK	"lock\t"
#endif

/**
 * atomic_bump - bump and return
 *
 * Atomically adds 1 to @v and returns 1 + @v
 */

static __inline__	__attribute__((always_inline))	u32 Atomic_Bump( u32 * v )
{
	u32	__i = 1;

	__asm__ __volatile__(
	LOCK	"xaddl %0,%1"
		:"=r"(__i), "+m"(((bump_32*)v)->counter)
		:"m"(((bump_32*)v)->counter), "0"(__i)
		:"memory" );

	return( 1 + __i );
}

/**
 * atomic_bump - bump and return
 *
 * Atomically adds 1 to @v and returns 1 + @v
 */

static __inline__	__attribute__((always_inline))	u32 Atomic_Bump_And_Mask( u32 * v, u32 mask )
{
	u32	__i;

	__i = 1;

	__asm__ __volatile__(
	LOCK	"xaddl %0,%1"
		:"=r"(__i), "+m"(((bump_32*)v)->counter)
		:"m"(((bump_32*)v)->counter), "0"(__i)
		:"memory" );

	if( ( __i += 1 ) > mask ) {

		__asm__ __volatile__(
		LOCK	"andl %0,%1"
			:
			: "r"(mask), "m"(((bump_32*)v)->counter)
			: "memory" );

	}
	return( __i & mask );
}

#include	<cmpxchg.h>		/* LOCK_PREFIX, __xchg_dummy, __xg: one definition */

/**
 * Atomic_Exchange - exchange and return
 *
 * Atomically stores @n in @v and returns the old @v (xchg: locked by
 * the processor, never blocks, a full memory barrier).
 */

static __inline__	__attribute__((always_inline))	u32 Atomic_Exchange( volatile u32 * v, u32 n )
{
	__asm__ __volatile__(
		"xchgl %0,%1"
		:"+r"(n), "+m"(((bump_32*)v)->counter)
		:
		:"memory" );
	return( n );
}

/**
 * Atomic_Store - store
 *
 * Atomically stores @n in @v, ordered with every earlier load and store
 * (sequentially consistent, as xchg).
 */

static __inline__	__attribute__((always_inline))	void Atomic_Store( volatile u32 * v, u32 n )
{
	(void)Atomic_Exchange( v, n );
}

static inline u32 cmpxchg32( volatile void *mem, u32 old, u32 new )
{
	u32 prev;
	__asm__ __volatile__( LOCK "cmpxchgl %1,%2"
				: "=a"(prev)
				: "q"(new), "m"(*__xg(mem)), "0"(old)
				: "memory");
	return prev;
}

/**
 * atomic_bump - bump and return
 *
 * Atomically adds 1 to @v and returns 1 + @v
 */

#ifndef	OLD_BUMP

static __inline__	__attribute__((always_inline))	u32 Atomic_Bump_With_Max( u32 * v, u32 max )
{
	register u32	__result;
	register u32	__i = 1;

	__asm__ __volatile__(
	LOCK	"xaddl %[i],%[mem]\n\t"
	LOCK	"cmpxchgl %[new],%[mem]"
		:[i]"=r"(__i), [result]"=a"(__result)
		:"[i]"(__i), [mem]"m"(((bump_32*)v)->counter), [new]"q"((u32)0), "[result]"(max)
		:"memory" );

	if( __result == max ) return( 0 );
	return( __i + 1 );
}

#else

static __inline__	__attribute__((always_inline))	u32 Atomic_Bump_With_Max( u32 * v, u32 max )
{
	u32	__prev;
	u32	__new = 0;
	u32	__i = 1;

	__asm__ __volatile__(
	LOCK	"xaddl %0,%1\n\t"
		:"=r"(__i), "+m"(((bump_32*)v)->counter)
		:"m"(((bump_32*)v)->counter), "0"(__i)
		:"memory" );

	__asm__ __volatile__(
	LOCK	"cmpxchgl %1,%2"
		: "=a"(__prev)
		: "q"(__new), "m"(((bump_32*)v)->counter), "0"(max)
		: "memory" );

	if( __prev == max ) return( 0 );

	return( __i + 1 );
}

#endif

#if	defined( __cplusplus )
	}
#endif

#endif
