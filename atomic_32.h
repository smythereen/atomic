/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Derived from the Linux kernel's x86 atomic header (arch/x86/include/asm/atomic.h),
 * which is GPL-2.0.  Adapted for CAYE by John R. Smythe Jr.
 * Copyright (c) 2026 John R. Smythe Jr. for the adaptations.
 */
#include	<common.h>

#ifndef ATOMIC_32_H
#define ATOMIC_32_H

/*
 * Atomic 32 bit operations that C can't guarantee us (lock prefixed,
 * non-blocking: no loops, no 64 bit operations).  Useful for
 * resource counting etc..
 */

#ifndef LOCK
#define	LOCK	"lock ; "
#endif

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct { volatile s32 counter; } atom_32;

#define ATOMIC_INIT(i)	{ (i) }

/**
 * s32_read - read s32 variable
 * @v: pointer of type s32_t
 * 
 * Atomically reads the value of @v.
 */ 
#define S32_Read( v )		(((atom_32 *)v)->counter)

/**
 * s32_set - set s32 variable
 * @v: pointer of type s32_t
 * @i: required value
 * 
 * Atomically sets the value of @v to @i.
 */ 
#define S32_Set( v, i )		((((atom_32 *)v)->counter) = (i))

/**
 * s32_add - add integer to s32 variable
 * @i: integer value to add
 * @v: pointer of type s32_t
 * 
 * Atomically adds @i to @v.
 */
static	__inline__	__attribute__((always_inline))	void	s32_add( s32 i, atom_32 * v )
{
	__asm__ __volatile__(
		LOCK "addl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

/**
 * s32_sub - subtract the s32 variable
 * @i: integer value to subtract
 * @v: pointer of type s32_t
 * 
 * Atomically subtracts @i from @v.
 */
static	__inline__	__attribute__((always_inline))	void	s32_sub( s32 i, atom_32 * v )
{
	__asm__ __volatile__(
		LOCK "subl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

/**
 * s32_add_and_test - add value to variable and test result
 * @i: integer value to subtract
 * @v: pointer of type s32
 * 
 * Atomically adds @i to @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static	__inline__	__attribute__((always_inline))	s32	s32_add_and_test( s32 i, atom_32 * v )
{
	u8	c;

	__asm__ __volatile__(
		LOCK "addl %2,%0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"ir" (i), "m" (v->counter) : "memory");
	return	c;
}

/**
 * s32_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type s32_t
 * 
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static	__inline__	__attribute__((always_inline))	s32	s32_sub_and_test( s32 i, atom_32 * v )
{
	u8	c;

	__asm__ __volatile__(
		LOCK "subl %2,%0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"ir" (i), "m" (v->counter) : "memory");
	return	c;
}

/**
 * s32_inc - increment s32 variable
 * @v: pointer of type s32_t
 * 
 * Atomically increments @v by 1.
 */ 
static	__inline__	__attribute__((always_inline))	void	s32_inc( atom_32 * v )
{
	__asm__ __volatile__(
		LOCK "incl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

/**
 * s32_dec - decrement s32 variable
 * @v: pointer of type s32_t
 * 
 * Atomically decrements @v by 1.
 */ 
static	__inline__	__attribute__((always_inline))	void	s32_dec( atom_32 * v )
{
	__asm__ __volatile__(
		LOCK "decl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

/**
 * s32_dec_and_test - decrement and test
 * @v: pointer of type s32_t
 * 
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */ 
static	__inline__	__attribute__((always_inline))	s32	s32_dec_and_test( atom_32 * v )
{
	u8	c;

	__asm__ __volatile__(
		LOCK "decl %0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"m" (v->counter) : "memory");
	return	c != 0;
}

/**
 * s32_inc_and_test - increment and test 
 * @v: pointer of type s32_t
 * 
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */ 
static	__inline__	__attribute__((always_inline))	s32	s32_inc_and_test( atom_32 * v )
{
	u8	c;

	__asm__ __volatile__(
		LOCK "incl %0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"m" (v->counter) : "memory");
	return	c != 0;
}

/**
 * s32_add_negative - add and test if negative
 * @v: pointer of type s32_t
 * @i: integer value to add
 * 
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */ 
static	__inline__	__attribute__((always_inline))	s32	s32_add_negative( s32 i, atom_32 * v )
{
	u8	c;

	__asm__ __volatile__(
		LOCK "addl %2,%0; sets %1"
		:"=m" (v->counter), "=qm" (c)
		:"ir" (i), "m" (v->counter) : "memory");
	return	c;
}

/**
 * s32_add_return - add and return
 * @v: pointer of type s32_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static	__inline__	__attribute__((always_inline))	s32	s32_add_return( s32 i, atom_32 * v )
{
	s32	__i;

	__i = i;
	__asm__ __volatile__(
		LOCK "xaddl %0, %1;"
		:"=r"(i)
		:"m"(v->counter), "0"(i));
	return	i + __i;
}

/**
 * s32_return_add - return old value then add
 * @v: pointer of type s32_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns @v
 */
static	__inline__	__attribute__((always_inline))	s32	s32_return_add( s32 i, atom_32 * v )
{
	__asm__ __volatile__(
		LOCK "xaddl %0, %1;"
		:"=r"(i)
		:"m"(v->counter), "0"(i));
	return	i ;
}


static	__inline__	__attribute__((always_inline))	s32	s32_sub_return( s32 i, atom_32 * v )
{
	return s32_add_return( -i, v );
}

static	__inline__	__attribute__((always_inline))	s32	s32_return_sub( s32 i, atom_32 * v )
{
	return	s32_return_add( -i, v );
}

#define	S32_Add_And_Test( i, v )	s32_add_and_test(	(s32)i,	(atom_32 *)v )
#define	S32_Add( i, v )			s32_add(		(s32)i,	(atom_32 *)v )
#define	S32_Add_Negative( i, v )	s32_add_negative(	(s32)i,	(atom_32 *)v )
#define	S32_Add_Return( i, v )		s32_add_return(		(s32)i,	(atom_32 *)v )

#define	S32_Dec_And_Test( v )		s32_dec_and_test(		(atom_32 *)v )
#define S32_Dec_Return( v )		s32_sub_return(		1,	(atom_32 *)v )
#define	S32_Dec( v )			s32_dec(			(atom_32 *)v )

#define	S32_Inc_And_Test( v )		s32_inc_and_test(		(atom_32 *)v )
#define S32_Inc_Return( v )		s32_add_return(		1,	(atom_32 *)v )
#define	S32_Inc( v )			s32_inc(			(atom_32 *)v )

#define	S32_Sub_And_Test( i, v )	s32_sub_and_test(	(s32)i,	(atom_32 *)v )
#define	S32_Sub( i, v )			s32_sub(		(s32)i,	(atom_32 *)v )
#define	S32_Sub_Return( i, v )		s32_sub_return(		(s32)i,	(atom_32 *)v )

#define S32_Clear_Mask(mask, addr)	__asm__ __volatile__(LOCK "andl %0,%1" : : "r" (~((s32)mask)),"m" (*(s32 *)addr) : "memory")
#define S32_Set_Mask(mask, addr)	__asm__ __volatile__(LOCK "orl %0,%1"  : : "r" ((s32)mask),"m" (*(s32 *)addr) : "memory")

#define	S32_Return_Add( i, v )		s32_return_add(		(s32)i,	(atom_32 *)v )
#define	S32_Return_Sub( i, v )		s32_return_sub(		(s32)i,	(atom_32 *)v )

#endif
