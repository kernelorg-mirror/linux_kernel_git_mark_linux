/*
 * This file provides wrappers with KASAN instrumentation for atomic operations.
 * To use this functionality an arch's atomic.h file needs to define all
 * atomic operations with arch_ prefix (e.g. arch_atomic_read()) and include
 * this file at the end. This file provides atomic_read() that forwards to
 * arch_atomic_read() for actual atomic operation.
 * Note: if an arch atomic operation is implemented by means of other atomic
 * operations (e.g. atomic_read()/atomic_cmpxchg() loop), then it needs to use
 * arch_ variants (i.e. arch_atomic_read()/arch_atomic_cmpxchg()) to avoid
 * double instrumentation.
 */

#ifndef _LINUX_ATOMIC_INSTRUMENTED_H
#define _LINUX_ATOMIC_INSTRUMENTED_H

#include <linux/build_bug.h>
#include <linux/kasan-checks.h>

static __always_inline int atomic_read(const atomic_t *v)
{
	kasan_check_read(v, sizeof(*v));
	return arch_atomic_read(v);
}

static __always_inline s64 atomic64_read(const atomic64_t *v)
{
	kasan_check_read(v, sizeof(*v));
	return arch_atomic64_read(v);
}

static __always_inline void atomic_set(atomic_t *v, int i)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_set(v, i);
}

static __always_inline void atomic64_set(atomic64_t *v, s64 i)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_set(v, i);
}

#define INSTR_ATOMIC_XCHG(order)					\
static __always_inline int						\
atomic_xchg##order(atomic_t *v, int i)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_xchg##order(v, i);				\
}

INSTR_ATOMIC_XCHG()

#ifdef arch_atomic_xchg_relaxed
INSTR_ATOMIC_XCHG(_relaxed)
#define atomic_xchg_relaxed atomic_xchg_relaxed
#endif

#ifdef arch_atomic_xchg_acquire
INSTR_ATOMIC_XCHG(_acquire)
#define atomic_xchg_acquire atomic_xchg_acquire
#endif

#ifdef arch_atomic_xchg_release
INSTR_ATOMIC_XCHG(_release)
#define atomic_xchg_release atomic_xchg_release
#endif

#define INSTR_ATOMIC64_XCHG(order)					\
static __always_inline s64						\
atomic64_xchg##order(atomic64_t *v, s64 i)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_xchg##order(v, i);				\
}

INSTR_ATOMIC64_XCHG()

#ifdef arch_atomic64_xchg_relaxed
INSTR_ATOMIC64_XCHG(_relaxed)
#define atomic64_xchg_relaxed atomic64_xchg_relaxed
#endif

#ifdef arch_atomic64_xchg_acquire
INSTR_ATOMIC64_XCHG(_acquire)
#define atomic64_xchg_acquire atomic64_xchg_acquire
#endif

#ifdef arch_atomic64_xchg_release
INSTR_ATOMIC64_XCHG(_release)
#define atomic64_xchg_release atomic64_xchg_release
#endif

#define INSTR_ATOMIC_CMPXCHG(order)					\
static __always_inline int						\
atomic_cmpxchg##order(atomic_t *v, int old, int new)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_cmpxchg##order(v, old, new);			\
}

INSTR_ATOMIC_CMPXCHG()

#ifdef arch_atomic_cmpxchg_relaxed
INSTR_ATOMIC_CMPXCHG(_relaxed)
#define atomic_cmpxchg_relaxed atomic_cmpxchg_relaxed
#endif

#ifdef arch_atomic_cmpxchg_acquire
INSTR_ATOMIC_CMPXCHG(_acquire)
#define atomic_cmpxchg_acquire atomic_cmpxchg_acquire
#endif

#ifdef arch_atomic_cmpxchg_release
INSTR_ATOMIC_CMPXCHG(_release)
#define atomic_cmpxchg_release atomic_cmpxchg_release
#endif

#define INSTR_ATOMIC64_CMPXCHG(order)					\
static __always_inline s64						\
atomic64_cmpxchg##order(atomic64_t *v, s64 old, s64 new)		\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_cmpxchg##order(v, old, new);		\
}

INSTR_ATOMIC64_CMPXCHG()

#ifdef arch_atomic64_cmpxchg_relaxed
INSTR_ATOMIC64_CMPXCHG(_relaxed)
#define atomic64_cmpxchg_relaxed atomic64_cmpxchg_relaxed
#endif

#ifdef arch_atomic64_cmpxchg_acquire
INSTR_ATOMIC64_CMPXCHG(_acquire)
#define atomic64_cmpxchg_acquire atomic64_cmpxchg_acquire
#endif

#ifdef arch_atomic64_cmpxchg_release
INSTR_ATOMIC64_CMPXCHG(_release)
#define atomic64_cmpxchg_release atomic64_cmpxchg_release
#endif

#define INSTR_ATOMIC_TRY_CMPXCHG(order)					\
static __always_inline bool						\
atomic_try_cmpxchg##order(atomic_t *v, int *old, int new)		\
{									\
	kasan_check_write(v, sizeof(*v));				\
	kasan_check_read(old, sizeof(*old));				\
	return arch_atomic_try_cmpxchg##order(v, old, new);		\
}

#ifdef arch_atomic_try_cmpxchg
INSTR_ATOMIC_TRY_CMPXCHG()
#define atomic_try_cmpxchg atomic_try_cmpxchg
#endif

#ifdef arch_atomic_try_cmpxchg_relaxed
INSTR_ATOMIC_TRY_CMPXCHG(_relaxed)
#define atomic_try_cmpxchg_relaxed atomic_try_cmpxchg_relaxed
#endif

#ifdef arch_atomic_try_cmpxchg_acquire
INSTR_ATOMIC_TRY_CMPXCHG(_acquire)
#define atomic_try_cmpxchg_acquire atomic_try_cmpxchg_acquire
#endif

#ifdef arch_atomic_try_cmpxchg_release
INSTR_ATOMIC_TRY_CMPXCHG(_release)
#define atomic_try_cmpxchg_release atomic_try_cmpxchg_release
#endif

#define INSTR_ATOMIC64_TRY_CMPXCHG(order)				\
static __always_inline bool						\
atomic64_try_cmpxchg##order(atomic64_t *v, s64 *old, s64 new)		\
{									\
	kasan_check_write(v, sizeof(*v));				\
	kasan_check_read(old, sizeof(*old));				\
	return arch_atomic64_try_cmpxchg##order(v, old, new);		\
}

#ifdef arch_atomic64_try_cmpxchg
INSTR_ATOMIC64_TRY_CMPXCHG()
#define atomic_try_cmpxchg atomic_try_cmpxchg
#endif

#ifdef arch_atomic64_try_cmpxchg_relaxed
INSTR_ATOMIC64_TRY_CMPXCHG(_relaxed)
#define atomic_try_cmpxchg_relaxed atomic_try_cmpxchg_relaxed
#endif

#ifdef arch_atomic64_try_cmpxchg_acquire
INSTR_ATOMIC64_TRY_CMPXCHG(_acquire)
#define atomic_try_cmpxchg_acquire atomic_try_cmpxchg_acquire
#endif

#ifdef arch_atomic64_try_cmpxchg_release
INSTR_ATOMIC64_TRY_CMPXCHG(_release)
#define atomic_try_cmpxchg_release atomic_try_cmpxchg_release
#endif

#define __INSTR_ATOMIC_ADD_UNLESS(order)				\
static __always_inline int						\
__atomic_add_unless##order(atomic_t *v, int a, int u)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return __arch_atomic_add_unless##order(v, a, u);		\
}

__INSTR_ATOMIC_ADD_UNLESS()

#ifdef __arch_atomic_add_unless_relaxed
__INSTR_ATOMIC_ADD_UNLESS(_relaxed)
#define __atomic_add_unless_relaxed __atomic_add_unless_relaxed
#endif

#ifdef __arch_atomic_add_unless_acquire
__INSTR_ATOMIC_ADD_UNLESS(_acquire)
#define __atomic_add_unless_acquire __atomic_add_unless_acquire
#endif

#ifdef __arch_atomic_add_unless_release
__INSTR_ATOMIC_ADD_UNLESS(_release)
#define __atomic_add_unless_release __atomic_add_unless_release
#endif

#define INSTR_ATOMIC64_ADD_UNLESS(order)				\
static __always_inline bool						\
atomic64_add_unless##order(atomic64_t *v, s64 a, s64 u)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_add_unless##order(v, a, u);		\
}

INSTR_ATOMIC64_ADD_UNLESS()

#ifdef arch_atomic64_add_unless_relaxed
INSTR_ATOMIC64_ADD_UNLESS(_relaxed)
#define atomic64_add_unless_relaxed atomic64_add_unless_relaxed
#endif

#ifdef arch_atomic64_add_unless_acquire
INSTR_ATOMIC64_ADD_UNLESS(_acquire)
#define atomic64_add_unless_acquire atomic64_add_unless_acquire
#endif

#ifdef arch_atomic64_add_unless_release
INSTR_ATOMIC64_ADD_UNLESS(_release)
#define atomic64_add_unless_release atomic64_add_unless_release
#endif

#define INSTR_ATOMIC_INC(order)						\
static __always_inline void						\
atomic_inc##order(atomic_t *v)						\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_inc##order(v);					\
}

INSTR_ATOMIC_INC()

#ifdef arch_atomic_inc_relaxed
INSTR_ATOMIC_INC(_relaxed)
#define atomic_inc_relaxed atomic_inc_relaxed
#endif

#ifdef arch_atomic_inc_acquire
INSTR_ATOMIC_INC(_acquire)
#define atomic_inc_acquire atomic_inc_acquire
#endif

#ifdef arch_atomic_inc_release
INSTR_ATOMIC_INC(_release)
#define atomic_inc_release atomic_inc_release
#endif

#define INSTR_ATOMIC64_INC(order)					\
static __always_inline void						\
atomic64_inc##order(atomic64_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_inc##order(v);					\
}

INSTR_ATOMIC64_INC()

#ifdef arch_atomic64_inc_relaxed
INSTR_ATOMIC64_INC(_relaxed)
#define atomic64_inc_relaxed atomic64_inc_relaxed
#endif

#ifdef arch_atomic64_inc_acquire
INSTR_ATOMIC64_INC(_acquire)
#define atomic64_inc_acquire atomic64_inc_acquire
#endif

#ifdef arch_atomic64_inc_release
INSTR_ATOMIC64_INC(_release)
#define atomic64_inc_release atomic64_inc_release
#endif

#define INSTR_ATOMIC_DEC(order)						\
static __always_inline void						\
atomic_dec##order(atomic_t *v)						\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_dec##order(v);					\
}

INSTR_ATOMIC_DEC()

#ifdef arch_atomic_dec_relaxed
INSTR_ATOMIC_DEC(_relaxed)
#define atomic_dec_relaxed atomic_dec_relaxed
#endif

#ifdef arch_atomic_dec_acquire
INSTR_ATOMIC_DEC(_acquire)
#define atomic_dec_acquire atomic_dec_acquire
#endif

#ifdef arch_atomic_dec_release
INSTR_ATOMIC_DEC(_release)
#define atomic_dec_release atomic_dec_release
#endif

#define INSTR_ATOMIC64_DEC(order)					\
static __always_inline void						\
atomic64_dec##order(atomic64_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_dec##order(v);					\
}

INSTR_ATOMIC64_DEC()

#ifdef arch_atomic64_dec_relaxed
INSTR_ATOMIC64_DEC(_relaxed)
#define atomic64_dec_relaxed atomic64_dec_relaxed
#endif

#ifdef arch_atomic64_dec_acquire
INSTR_ATOMIC64_DEC(_acquire)
#define atomic64_dec_acquire atomic64_dec_acquire
#endif

#ifdef arch_atomic64_dec_release
INSTR_ATOMIC64_DEC(_release)
#define atomic64_dec_release atomic64_dec_release
#endif

#define INSTR_ATOMIC_ADD(order)						\
static __always_inline void						\
atomic_add##order(int i, atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_add##order(i, v);					\
}

INSTR_ATOMIC_ADD()

#ifdef arch_atomic_add_relaxed
INSTR_ATOMIC_ADD(_relaxed)
#define atomic_add_relaxed atomic_add_relaxed
#endif

#ifdef arch_atomic_add_acquire
INSTR_ATOMIC_ADD(_acquire)
#define atomic_add_acquire atomic_add_acquire
#endif

#ifdef arch_atomic_add_release
INSTR_ATOMIC_ADD(_release)
#define atomic_add_release atomic_add_release
#endif

#define INSTR_ATOMIC64_ADD(order)					\
static __always_inline void						\
atomic64_add##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_add##order(i, v);					\
}

INSTR_ATOMIC64_ADD()

#ifdef arch_atomic64_add_relaxed
INSTR_ATOMIC64_ADD(_relaxed)
#define atomic64_add_relaxed atomic64_add_relaxed
#endif

#ifdef arch_atomic64_add_acquire
INSTR_ATOMIC64_ADD(_acquire)
#define atomic64_add_acquire atomic64_add_acquire
#endif

#ifdef arch_atomic64_add_release
INSTR_ATOMIC64_ADD(_release)
#define atomic64_add_release atomic64_add_release
#endif

#define INSTR_ATOMIC_SUB(order)						\
static __always_inline void						\
atomic_sub##order(int i, atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_sub##order(i, v);					\
}

INSTR_ATOMIC_SUB()

#ifdef arch_atomic_sub_relaxed
INSTR_ATOMIC_SUB(_relaxed)
#define atomic_sub_relaxed atomic_sub_relaxed
#endif

#ifdef arch_atomic_sub_acquire
INSTR_ATOMIC_SUB(_acquire)
#define atomic_sub_acquire atomic_sub_acquire
#endif

#ifdef arch_atomic_sub_release
INSTR_ATOMIC_SUB(_release)
#define atomic_sub_release atomic_sub_release
#endif

#define INSTR_ATOMIC64_SUB(order)					\
static __always_inline void						\
atomic64_sub##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_sub##order(i, v);					\
}

INSTR_ATOMIC64_SUB()

#ifdef arch_atomic64_sub_relaxed
INSTR_ATOMIC64_SUB(_relaxed)
#define atomic64_sub_relaxed atomic64_sub_relaxed
#endif

#ifdef arch_atomic64_sub_acquire
INSTR_ATOMIC64_SUB(_acquire)
#define atomic64_sub_acquire atomic64_sub_acquire
#endif

#ifdef arch_atomic64_sub_release
INSTR_ATOMIC64_SUB(_release)
#define atomic64_sub_release atomic64_sub_release
#endif

#define INSTR_ATOMIC_AND(order)						\
static __always_inline void						\
atomic_and##order(int i, atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_and##order(i, v);					\
}

INSTR_ATOMIC_AND()

#ifdef arch_atomic_and_relaxed
INSTR_ATOMIC_AND(_relaxed)
#define atomic_and_relaxed atomic_and_relaxed
#endif

#ifdef arch_atomic_and_acquire
INSTR_ATOMIC_AND(_acquire)
#define atomic_and_acquire atomic_and_acquire
#endif

#ifdef arch_atomic_and_release
INSTR_ATOMIC_AND(_release)
#define atomic_and_release atomic_and_release
#endif

#define INSTR_ATOMIC64_AND(order)					\
static __always_inline void						\
atomic64_and##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_and##order(i, v);					\
}

INSTR_ATOMIC64_AND()

#ifdef arch_atomic64_and_relaxed
INSTR_ATOMIC64_AND(_relaxed)
#define atomic64_and_relaxed atomic64_and_relaxed
#endif

#ifdef arch_atomic64_and_acquire
INSTR_ATOMIC64_AND(_acquire)
#define atomic64_and_acquire atomic64_and_acquire
#endif

#ifdef arch_atomic64_and_release
INSTR_ATOMIC64_AND(_release)
#define atomic64_and_release atomic64_and_release
#endif

#define INSTR_ATOMIC_OR(order)						\
static __always_inline void						\
atomic_or##order(int i, atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_or##order(i, v);					\
}

INSTR_ATOMIC_OR()

#ifdef arch_atomic_or_relaxed
INSTR_ATOMIC_OR(_relaxed)
#define atomic_or_relaxed atomic_or_relaxed
#endif

#ifdef arch_atomic_or_acquire
INSTR_ATOMIC_OR(_acquire)
#define atomic_or_acquire atomic_or_acquire
#endif

#ifdef arch_atomic_or_release
INSTR_ATOMIC_OR(_release)
#define atomic_or_release atomic_or_release
#endif

#define INSTR_ATOMIC64_OR(order)					\
static __always_inline void						\
atomic64_or##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_or##order(i, v);					\
}

INSTR_ATOMIC64_OR()

#ifdef arch_atomic64_or_relaxed
INSTR_ATOMIC64_OR(_relaxed)
#define atomic64_or_relaxed atomic64_or_relaxed
#endif

#ifdef arch_atomic64_or_acquire
INSTR_ATOMIC64_OR(_acquire)
#define atomic64_or_acquire atomic64_or_acquire
#endif

#ifdef arch_atomic64_or_release
INSTR_ATOMIC64_OR(_release)
#define atomic64_or_release atomic64_or_release
#endif

#define INSTR_ATOMIC_XOR(order)						\
static __always_inline void						\
atomic_xor##order(int i, atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic_xor##order(i, v);					\
}

INSTR_ATOMIC_XOR()

#ifdef arch_atomic_xor_relaxed
INSTR_ATOMIC_XOR(_relaxed)
#define atomic_xor_relaxed atomic_xor_relaxed
#endif

#ifdef arch_atomic_xor_acquire
INSTR_ATOMIC_XOR(_acquire)
#define atomic_xor_acquire atomic_xor_acquire
#endif

#ifdef arch_atomic_xor_release
INSTR_ATOMIC_XOR(_release)
#define atomic_xor_release atomic_xor_release
#endif

#define INSTR_ATOMIC64_XOR(order)					\
static __always_inline void						\
atomic64_xor##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	arch_atomic64_xor##order(i, v);					\
}

INSTR_ATOMIC64_XOR()

#ifdef arch_atomic64_xor_relaxed
INSTR_ATOMIC64_XOR(_relaxed)
#define atomic64_xor_relaxed atomic64_xor_relaxed
#endif

#ifdef arch_atomic64_xor_acquire
INSTR_ATOMIC64_XOR(_acquire)
#define atomic64_xor_acquire atomic64_xor_acquire
#endif

#ifdef arch_atomic64_xor_release
INSTR_ATOMIC64_XOR(_release)
#define atomic64_xor_release atomic64_xor_release
#endif

#define INSTR_ATOMIC_INC_RETURN(order)					\
static __always_inline int						\
atomic_inc_return##order(atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_inc_return##order(v);			\
}

INSTR_ATOMIC_INC_RETURN()

#ifdef arch_atomic_inc_return_relaxed
INSTR_ATOMIC_INC_RETURN(_relaxed)
#define atomic_inc_return_relaxed atomic_inc_return_relaxed
#endif

#ifdef arch_atomic_inc_return_acquire
INSTR_ATOMIC_INC_RETURN(_acquire)
#define atomic_inc_return_acquire atomic_inc_return_acquire
#endif

#ifdef arch_atomic_inc_return_release
INSTR_ATOMIC_INC_RETURN(_release)
#define atomic_inc_return_release atomic_inc_return_release
#endif

#define INSTR_ATOMIC64_INC_RETURN(order)				\
static __always_inline s64						\
atomic64_inc_return##order(atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_inc_return##order(v);			\
}

INSTR_ATOMIC64_INC_RETURN()

#ifdef arch_atomic64_inc_return_relaxed
INSTR_ATOMIC64_INC_RETURN(_relaxed)
#define atomic64_inc_return_relaxed atomic64_inc_return_relaxed
#endif

#ifdef arch_atomic64_inc_return_acquire
INSTR_ATOMIC64_INC_RETURN(_acquire)
#define atomic64_inc_return_acquire atomic64_inc_return_acquire
#endif

#ifdef arch_atomic64_inc_return_release
INSTR_ATOMIC64_INC_RETURN(_release)
#define atomic64_inc_return_release atomic64_inc_return_release
#endif

#define INSTR_ATOMIC_DEC_RETURN(order)					\
static __always_inline int						\
atomic_dec_return##order(atomic_t *v)					\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_dec_return##order(v);			\
}

INSTR_ATOMIC_DEC_RETURN()

#ifdef arch_atomic_dec_return_relaxed
INSTR_ATOMIC_DEC_RETURN(_relaxed)
#define atomic_dec_return_relaxed atomic_dec_return_relaxed
#endif

#ifdef arch_atomic_dec_return_acquire
INSTR_ATOMIC_DEC_RETURN(_acquire)
#define atomic_dec_return_acquire atomic_dec_return_acquire
#endif

#ifdef arch_atomic_dec_return_release
INSTR_ATOMIC_DEC_RETURN(_release)
#define atomic_dec_return_release atomic_dec_return_release
#endif

#define INSTR_ATOMIC64_DEC_RETURN(order)				\
static __always_inline s64						\
atomic64_dec_return##order(atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_dec_return##order(v);			\
}

INSTR_ATOMIC64_DEC_RETURN()

#ifdef arch_atomic64_dec_return_relaxed
INSTR_ATOMIC64_DEC_RETURN(_relaxed)
#define atomic64_dec_return_relaxed atomic64_dec_return_relaxed
#endif

#ifdef arch_atomic64_dec_return_acquire
INSTR_ATOMIC64_DEC_RETURN(_acquire)
#define atomic64_dec_return_acquire atomic64_dec_return_acquire
#endif

#ifdef arch_atomic64_dec_return_release
INSTR_ATOMIC64_DEC_RETURN(_release)
#define atomic64_dec_return_release atomic64_dec_return_release
#endif

static __always_inline s64 atomic64_inc_not_zero(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_inc_not_zero(v);
}

static __always_inline s64 atomic64_dec_if_positive(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_dec_if_positive(v);
}

static __always_inline bool atomic_dec_and_test(atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_dec_and_test(v);
}

static __always_inline bool atomic64_dec_and_test(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_dec_and_test(v);
}

static __always_inline bool atomic_inc_and_test(atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_inc_and_test(v);
}

static __always_inline bool atomic64_inc_and_test(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_inc_and_test(v);
}

#define INSTR_ATOMIC_ADD_RETURN(order)					\
static __always_inline int						\
atomic_add_return##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_add_return##order(i, v);			\
}

INSTR_ATOMIC_ADD_RETURN()

#ifdef arch_atomic_add_return_relaxed
INSTR_ATOMIC_ADD_RETURN(_relaxed)
#define atomic_add_return_relaxed atomic_add_return_relaxed
#endif

#ifdef arch_atomic_add_return_acquire
INSTR_ATOMIC_ADD_RETURN(_acquire)
#define atomic_add_return_acquire atomic_add_return_acquire
#endif

#ifdef arch_atomic_add_return_release
INSTR_ATOMIC_ADD_RETURN(_release)
#define atomic_add_return_release atomic_add_return_release
#endif

#define INSTR_ATOMIC64_ADD_RETURN(order)				\
static __always_inline s64						\
atomic64_add_return##order(s64 i, atomic64_t *v)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_add_return##order(i, v);			\
}

INSTR_ATOMIC64_ADD_RETURN()

#ifdef arch_atomic64_add_return_relaxed
INSTR_ATOMIC64_ADD_RETURN(_relaxed)
#define atomic64_add_return_relaxed atomic64_add_return_relaxed
#endif

#ifdef arch_atomic64_add_return_acquire
INSTR_ATOMIC64_ADD_RETURN(_acquire)
#define atomic64_add_return_acquire atomic64_add_return_acquire
#endif

#ifdef arch_atomic64_add_return_release
INSTR_ATOMIC64_ADD_RETURN(_release)
#define atomic64_add_return_release atomic64_add_return_release
#endif

#define INSTR_ATOMIC_SUB_RETURN(order)					\
static __always_inline int						\
atomic_sub_return##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_sub_return##order(i, v);			\
}

INSTR_ATOMIC_SUB_RETURN()

#ifdef arch_atomic_sub_return_relaxed
INSTR_ATOMIC_SUB_RETURN(_relaxed)
#define atomic_sub_return_relaxed atomic_sub_return_relaxed
#endif

#ifdef arch_atomic_sub_return_acquire
INSTR_ATOMIC_SUB_RETURN(_acquire)
#define atomic_sub_return_acquire atomic_sub_return_acquire
#endif

#ifdef arch_atomic_sub_return_release
INSTR_ATOMIC_SUB_RETURN(_release)
#define atomic_sub_return_release atomic_sub_return_release
#endif

#define INSTR_ATOMIC64_SUB_RETURN(order)				\
static __always_inline s64						\
atomic64_sub_return##order(s64 i, atomic64_t *v)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_sub_return##order(i, v);			\
}

INSTR_ATOMIC64_SUB_RETURN()

#ifdef arch_atomic64_sub_return_relaxed
INSTR_ATOMIC64_SUB_RETURN(_relaxed)
#define atomic64_sub_return_relaxed atomic64_sub_return_relaxed
#endif

#ifdef arch_atomic64_sub_return_acquire
INSTR_ATOMIC64_SUB_RETURN(_acquire)
#define atomic64_sub_return_acquire atomic64_sub_return_acquire
#endif

#ifdef arch_atomic64_sub_return_release
INSTR_ATOMIC64_SUB_RETURN(_release)
#define atomic64_sub_return_release atomic64_sub_return_release
#endif

#define INSTR_ATOMIC_FETCH_ADD(order)					\
static __always_inline int						\
atomic_fetch_add##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_fetch_add##order(i, v);			\
}

INSTR_ATOMIC_FETCH_ADD()

#ifdef arch_atomic_fetch_add_relaxed
INSTR_ATOMIC_FETCH_ADD(_relaxed)
#define atomic_fetch_add_relaxed atomic_fetch_add_relaxed
#endif

#ifdef arch_atomic_fetch_add_acquire
INSTR_ATOMIC_FETCH_ADD(_acquire)
#define atomic_fetch_add_acquire atomic_fetch_add_acquire
#endif

#ifdef arch_atomic_fetch_add_release
INSTR_ATOMIC_FETCH_ADD(_release)
#define atomic_fetch_add_release atomic_fetch_add_release
#endif

#define INSTR_ATOMIC64_FETCH_ADD(order)					\
static __always_inline s64						\
atomic64_fetch_add##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_fetch_add##order(i, v);			\
}

INSTR_ATOMIC64_FETCH_ADD()

#ifdef arch_atomic64_fetch_add_relaxed
INSTR_ATOMIC64_FETCH_ADD(_relaxed)
#define atomic64_fetch_add_relaxed atomic64_fetch_add_relaxed
#endif

#ifdef arch_atomic64_fetch_add_acquire
INSTR_ATOMIC64_FETCH_ADD(_acquire)
#define atomic64_fetch_add_acquire atomic64_fetch_add_acquire
#endif

#ifdef arch_atomic64_fetch_add_release
INSTR_ATOMIC64_FETCH_ADD(_release)
#define atomic64_fetch_add_release atomic64_fetch_add_release
#endif

#define INSTR_ATOMIC_FETCH_SUB(order)					\
static __always_inline int						\
atomic_fetch_sub##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_fetch_sub##order(i, v);			\
}

INSTR_ATOMIC_FETCH_SUB()

#ifdef arch_atomic_fetch_sub_relaxed
INSTR_ATOMIC_FETCH_SUB(_relaxed)
#define atomic_fetch_sub_relaxed atomic_fetch_sub_relaxed
#endif

#ifdef arch_atomic_fetch_sub_acquire
INSTR_ATOMIC_FETCH_SUB(_acquire)
#define atomic_fetch_sub_acquire atomic_fetch_sub_acquire
#endif

#ifdef arch_atomic_fetch_sub_release
INSTR_ATOMIC_FETCH_SUB(_release)
#define atomic_fetch_sub_release atomic_fetch_sub_release
#endif

#define INSTR_ATOMIC64_FETCH_SUB(order)					\
static __always_inline s64						\
atomic64_fetch_sub##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_fetch_sub##order(i, v);			\
}

INSTR_ATOMIC64_FETCH_SUB()

#ifdef arch_atomic64_fetch_sub_relaxed
INSTR_ATOMIC64_FETCH_SUB(_relaxed)
#define atomic64_fetch_sub_relaxed atomic64_fetch_sub_relaxed
#endif

#ifdef arch_atomic64_fetch_sub_acquire
INSTR_ATOMIC64_FETCH_SUB(_acquire)
#define atomic64_fetch_sub_acquire atomic64_fetch_sub_acquire
#endif

#ifdef arch_atomic64_fetch_sub_release
INSTR_ATOMIC64_FETCH_SUB(_release)
#define atomic64_fetch_sub_release atomic64_fetch_sub_release
#endif

#define INSTR_ATOMIC_FETCH_AND(order)					\
static __always_inline int						\
atomic_fetch_and##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_fetch_and##order(i, v);			\
}

INSTR_ATOMIC_FETCH_AND()

#ifdef arch_atomic_fetch_and_relaxed
INSTR_ATOMIC_FETCH_AND(_relaxed)
#define atomic_fetch_and_relaxed atomic_fetch_and_relaxed
#endif

#ifdef arch_atomic_fetch_and_acquire
INSTR_ATOMIC_FETCH_AND(_acquire)
#define atomic_fetch_and_acquire atomic_fetch_and_acquire
#endif

#ifdef arch_atomic_fetch_and_release
INSTR_ATOMIC_FETCH_AND(_release)
#define atomic_fetch_and_release atomic_fetch_and_release
#endif

#define INSTR_ATOMIC64_FETCH_AND(order)					\
static __always_inline s64						\
atomic64_fetch_and##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_fetch_and##order(i, v);			\
}

INSTR_ATOMIC64_FETCH_AND()

#ifdef arch_atomic64_fetch_and_relaxed
INSTR_ATOMIC64_FETCH_AND(_relaxed)
#define atomic64_fetch_and_relaxed atomic64_fetch_and_relaxed
#endif

#ifdef arch_atomic64_fetch_and_acquire
INSTR_ATOMIC64_FETCH_AND(_acquire)
#define atomic64_fetch_and_acquire atomic64_fetch_and_acquire
#endif

#ifdef arch_atomic64_fetch_and_release
INSTR_ATOMIC64_FETCH_AND(_release)
#define atomic64_fetch_and_release atomic64_fetch_and_release
#endif

#define INSTR_ATOMIC_FETCH_OR(order)					\
static __always_inline int						\
atomic_fetch_or##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_fetch_or##order(i, v);			\
}

INSTR_ATOMIC_FETCH_OR()

#ifdef arch_atomic_fetch_or_relaxed
INSTR_ATOMIC_FETCH_OR(_relaxed)
#define atomic_fetch_or_relaxed atomic_fetch_or_relaxed
#endif

#ifdef arch_atomic_fetch_or_acquire
INSTR_ATOMIC_FETCH_OR(_acquire)
#define atomic_fetch_or_acquire atomic_fetch_or_acquire
#endif

#ifdef arch_atomic_fetch_or_release
INSTR_ATOMIC_FETCH_OR(_release)
#define atomic_fetch_or_release atomic_fetch_or_release
#endif

#define INSTR_ATOMIC64_FETCH_OR(order)					\
static __always_inline s64						\
atomic64_fetch_or##order(s64 i, atomic64_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_fetch_or##order(i, v);			\
}

INSTR_ATOMIC64_FETCH_OR()

#ifdef arch_atomic64_fetch_or_relaxed
INSTR_ATOMIC64_FETCH_OR(_relaxed)
#define atomic64_fetch_or_relaxed atomic64_fetch_or_relaxed
#endif

#ifdef arch_atomic64_fetch_or_acquire
INSTR_ATOMIC64_FETCH_OR(_acquire)
#define atomic64_fetch_or_acquire atomic64_fetch_or_acquire
#endif

#ifdef arch_atomic64_fetch_or_release
INSTR_ATOMIC64_FETCH_OR(_release)
#define atomic64_fetch_or_release atomic64_fetch_or_release
#endif

#define INSTR_ATOMIC_FETCH_XOR(order)					\
static __always_inline int						\
atomic_fetch_xor##order(int i, atomic_t *v)				\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic_fetch_xor##order(i, v);			\
}

INSTR_ATOMIC_FETCH_XOR()

#ifdef arch_atomic_fetch_xor_relaxed
INSTR_ATOMIC_FETCH_XOR(_relaxed)
#define atomic_fetch_xor_relaxed atomic_fetch_xor_relaxed
#endif

#ifdef arch_atomic_fetch_xor_acquire
INSTR_ATOMIC_FETCH_XOR(_acquire)
#define atomic_fetch_xor_acquire atomic_fetch_xor_acquire
#endif

#ifdef arch_atomic_fetch_xor_release
INSTR_ATOMIC_FETCH_XOR(_release)
#define atomic_fetch_xor_release atomic_fetch_xor_release
#endif

#define INSTR_ATOMIC64_FETCH_XOR(xorder)				\
static __always_inline s64						\
atomic64_fetch_xor##xorder(s64 i, atomic64_t *v)			\
{									\
	kasan_check_write(v, sizeof(*v));				\
	return arch_atomic64_fetch_xor##xorder(i, v);			\
}

INSTR_ATOMIC64_FETCH_XOR()

#ifdef arch_atomic64_fetch_xor_relaxed
INSTR_ATOMIC64_FETCH_XOR(_relaxed)
#define atomic64_fetch_xor_relaxed atomic64_fetch_xor_relaxed
#endif

#ifdef arch_atomic64_fetch_xor_acquire
INSTR_ATOMIC64_FETCH_XOR(_acquire)
#define atomic64_fetch_xor_acquire atomic64_fetch_xor_acquire
#endif

#ifdef arch_atomic64_fetch_xor_release
INSTR_ATOMIC64_FETCH_XOR(_release)
#define atomic64_fetch_xor_release atomic64_fetch_xor_release
#endif

static __always_inline bool atomic_sub_and_test(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_sub_and_test(i, v);
}

static __always_inline bool atomic64_sub_and_test(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_sub_and_test(i, v);
}

static __always_inline bool atomic_add_negative(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_add_negative(i, v);
}

static __always_inline bool atomic64_add_negative(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_add_negative(i, v);
}

#define INSTR_CMPXCHG(order)							\
static __always_inline unsigned long						\
cmpxchg##order##_size(volatile void *ptr, unsigned long old,			\
		       unsigned long new, int size)				\
{										\
	kasan_check_write(ptr, size);						\
	switch (size) {								\
	case 1:									\
		return arch_cmpxchg##order((u8 *)ptr, (u8)old, (u8)new);	\
	case 2:									\
		return arch_cmpxchg##order((u16 *)ptr, (u16)old, (u16)new);	\
	case 4:									\
		return arch_cmpxchg##order((u32 *)ptr, (u32)old, (u32)new);	\
	case 8:									\
		BUILD_BUG_ON(sizeof(unsigned long) != 8);			\
		return arch_cmpxchg##order((u64 *)ptr, (u64)old, (u64)new);	\
	}									\
	BUILD_BUG();								\
	return 0;								\
}

INSTR_CMPXCHG()
#define cmpxchg(ptr, old, new)						\
({									\
	((__typeof__(*(ptr)))cmpxchg_size((ptr), (unsigned long)(old),	\
		(unsigned long)(new), sizeof(*(ptr))));			\
})

#ifdef arch_cmpxchg_relaxed
INSTR_CMPXCHG(_relaxed)
#define cmpxchg_relaxed(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg_relaxed_size((ptr),		\
		(unsigned long)(old), (unsigned long)(new), 		\
		sizeof(*(ptr))));					\
})
#endif

#ifdef arch_cmpxchg_acquire
INSTR_CMPXCHG(_acquire)
#define cmpxchg_acquire(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg_acquire_size((ptr),		\
		(unsigned long)(old), (unsigned long)(new), 		\
		sizeof(*(ptr))));					\
})
#endif

#ifdef arch_cmpxchg_release
INSTR_CMPXCHG(_release)
#define cmpxchg_release(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg_release_size((ptr),		\
		(unsigned long)(old), (unsigned long)(new), 		\
		sizeof(*(ptr))));					\
})
#endif

static __always_inline unsigned long
sync_cmpxchg_size(volatile void *ptr, unsigned long old, unsigned long new,
		  int size)
{
	kasan_check_write(ptr, size);
	switch (size) {
	case 1:
		return arch_sync_cmpxchg((u8 *)ptr, (u8)old, (u8)new);
	case 2:
		return arch_sync_cmpxchg((u16 *)ptr, (u16)old, (u16)new);
	case 4:
		return arch_sync_cmpxchg((u32 *)ptr, (u32)old, (u32)new);
	case 8:
		BUILD_BUG_ON(sizeof(unsigned long) != 8);
		return arch_sync_cmpxchg((u64 *)ptr, (u64)old, (u64)new);
	}
	BUILD_BUG();
	return 0;
}

#define sync_cmpxchg(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))sync_cmpxchg_size((ptr),			\
		(unsigned long)(old), (unsigned long)(new),		\
		sizeof(*(ptr))));					\
})

static __always_inline unsigned long
cmpxchg_local_size(volatile void *ptr, unsigned long old, unsigned long new,
		   int size)
{
	kasan_check_write(ptr, size);
	switch (size) {
	case 1:
		return arch_cmpxchg_local((u8 *)ptr, (u8)old, (u8)new);
	case 2:
		return arch_cmpxchg_local((u16 *)ptr, (u16)old, (u16)new);
	case 4:
		return arch_cmpxchg_local((u32 *)ptr, (u32)old, (u32)new);
	case 8:
		BUILD_BUG_ON(sizeof(unsigned long) != 8);
		return arch_cmpxchg_local((u64 *)ptr, (u64)old, (u64)new);
	}
	BUILD_BUG();
	return 0;
}

#define cmpxchg_local(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg_local_size((ptr),			\
		(unsigned long)(old), (unsigned long)(new),		\
		sizeof(*(ptr))));					\
})

#define INSTR_CMPXCHG64(order)						\
static __always_inline u64						\
cmpxchg64##order##_size(volatile u64 *ptr, u64 old, u64 new)		\
{									\
	kasan_check_write(ptr, sizeof(*ptr));				\
	return arch_cmpxchg64##order(ptr, old, new);			\
}

INSTR_CMPXCHG64()
#define cmpxchg64(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg64_size((ptr), (u64)(old),		\
		(u64)(new)));						\
})

#ifdef arch_cmpxchg64_relaxed
INSTR_CMPXCHG64(_relaxed)
#define cmpxchg64_relaxed(ptr, old, new)				\
({									\
	((__typeof__(*(ptr)))cmpxchg64_relaxed_size((ptr), (u64)(old),	\
		(u64)(new)));						\
})
#endif

#ifdef arch_cmpxchg64_acquire
INSTR_CMPXCHG64(_acquire)
#define cmpxchg64_acquire(ptr, old, new)				\
({									\
	((__typeof__(*(ptr)))cmpxchg64_acquire_size((ptr), (u64)(old),	\
		(u64)(new)));						\
})
#endif

#ifdef arch_cmpxchg64_release
INSTR_CMPXCHG64(_release)
#define cmpxchg64_release(ptr, old, new)				\
({									\
	((__typeof__(*(ptr)))cmpxchg64_release_size((ptr), (u64)(old),	\
		(u64)(new)));						\
})
#endif

static __always_inline u64
cmpxchg64_local_size(volatile u64 *ptr, u64 old, u64 new)
{
	kasan_check_write(ptr, sizeof(*ptr));
	return arch_cmpxchg64_local(ptr, old, new);
}

#define cmpxchg64_local(ptr, old, new)					\
({									\
	((__typeof__(*(ptr)))cmpxchg64_local_size((ptr), (u64)(old),	\
		(u64)(new)));						\
})

/*
 * Originally we had the following code here:
 *     __typeof__(p1) ____p1 = (p1);
 *     kasan_check_write(____p1, 2 * sizeof(*____p1));
 *     arch_cmpxchg_double(____p1, (p2), (o1), (o2), (n1), (n2));
 * But it leads to compilation failures (see gcc issue 72873).
 * So for now it's left non-instrumented.
 * There are few callers of cmpxchg_double(), so it's not critical.
 */
#define cmpxchg_double(p1, p2, o1, o2, n1, n2)				\
({									\
	arch_cmpxchg_double((p1), (p2), (o1), (o2), (n1), (n2));	\
})

#define cmpxchg_double_local(p1, p2, o1, o2, n1, n2)			\
({									\
	arch_cmpxchg_double_local((p1), (p2), (o1), (o2), (n1), (n2));	\
})

#endif /* _LINUX_ATOMIC_INSTRUMENTED_H */
