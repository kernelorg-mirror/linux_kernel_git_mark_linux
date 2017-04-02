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
#define atomic64_add_unless_relaxed __atomic64_add_unless_relaxed
#endif

#ifdef arch_atomic64_add_unless_acquire
INSTR_ATOMIC64_ADD_UNLESS(_acquire)
#define atomic64_add_unless_acquire __atomic64_add_unless_acquire
#endif

#ifdef arch_atomic64_add_unless_release
INSTR_ATOMIC64_ADD_UNLESS(_release)
#define atomic64_add_unless_release __atomic64_add_unless_release
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

static __always_inline void atomic64_inc(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_inc(v);
}

static __always_inline void atomic_dec(atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_dec(v);
}

static __always_inline void atomic64_dec(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_dec(v);
}

static __always_inline void atomic_add(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_add(i, v);
}

static __always_inline void atomic64_add(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_add(i, v);
}

static __always_inline void atomic_sub(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_sub(i, v);
}

static __always_inline void atomic64_sub(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_sub(i, v);
}

static __always_inline void atomic_and(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_and(i, v);
}

static __always_inline void atomic64_and(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_and(i, v);
}

static __always_inline void atomic_or(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_or(i, v);
}

static __always_inline void atomic64_or(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_or(i, v);
}

static __always_inline void atomic_xor(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic_xor(i, v);
}

static __always_inline void atomic64_xor(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	arch_atomic64_xor(i, v);
}

static __always_inline int atomic_inc_return(atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_inc_return(v);
}

static __always_inline s64 atomic64_inc_return(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_inc_return(v);
}

static __always_inline int atomic_dec_return(atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_dec_return(v);
}

static __always_inline s64 atomic64_dec_return(atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_dec_return(v);
}

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

static __always_inline int atomic_add_return(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_add_return(i, v);
}

static __always_inline s64 atomic64_add_return(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_add_return(i, v);
}

static __always_inline int atomic_sub_return(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_sub_return(i, v);
}

static __always_inline s64 atomic64_sub_return(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_sub_return(i, v);
}

static __always_inline int atomic_fetch_add(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_fetch_add(i, v);
}

static __always_inline s64 atomic64_fetch_add(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_fetch_add(i, v);
}

static __always_inline int atomic_fetch_sub(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_fetch_sub(i, v);
}

static __always_inline s64 atomic64_fetch_sub(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_fetch_sub(i, v);
}

static __always_inline int atomic_fetch_and(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_fetch_and(i, v);
}

static __always_inline s64 atomic64_fetch_and(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_fetch_and(i, v);
}

static __always_inline int atomic_fetch_or(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_fetch_or(i, v);
}

static __always_inline s64 atomic64_fetch_or(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_fetch_or(i, v);
}

static __always_inline int atomic_fetch_xor(int i, atomic_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic_fetch_xor(i, v);
}

static __always_inline s64 atomic64_fetch_xor(s64 i, atomic64_t *v)
{
	kasan_check_write(v, sizeof(*v));
	return arch_atomic64_fetch_xor(i, v);
}

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

/*
 * In the following macros we need to be careful to not clash with arch_ macros.
 * arch_xchg() can be defined as an extended statement expression as well,
 * if we define a __ptr variable, and arch_xchg() also defines __ptr variable,
 * and we pass __ptr as an argument to arch_xchg(), it will use own __ptr
 * instead of ours. This leads to unpleasant crashes. To avoid the problem
 * the following macros declare variables with lots of underscores.
 */

#define cmpxchg(ptr, old, new)				\
({							\
	__typeof__(ptr) ___ptr = (ptr);			\
	kasan_check_write(___ptr, sizeof(*___ptr));	\
	arch_cmpxchg((ptr), (old), (new));		\
})

#define sync_cmpxchg(ptr, old, new)			\
({							\
	__typeof__(ptr) ___ptr = (ptr);			\
	kasan_check_write(___ptr, sizeof(*___ptr));	\
	arch_sync_cmpxchg(___ptr, (old), (new));	\
})

#define cmpxchg_local(ptr, old, new)			\
({							\
	__typeof__(ptr) ____ptr = (ptr);		\
	kasan_check_write(____ptr, sizeof(*____ptr));	\
	arch_cmpxchg_local(____ptr, (old), (new));	\
})

#define cmpxchg64(ptr, old, new)			\
({							\
	__typeof__(ptr) ____ptr = (ptr);		\
	kasan_check_write(____ptr, sizeof(*____ptr));	\
	arch_cmpxchg64(____ptr, (old), (new));		\
})

#define cmpxchg64_local(ptr, old, new)			\
({							\
	__typeof__(ptr) ____ptr = (ptr);		\
	kasan_check_write(____ptr, sizeof(*____ptr));	\
	arch_cmpxchg64_local(____ptr, (old), (new));	\
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
