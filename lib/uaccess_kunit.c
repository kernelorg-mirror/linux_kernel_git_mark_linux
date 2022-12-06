// SPDX-License-Identifier: GPL-2.0
/*
 * Test cases for uaccess helpers.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <kunit/test.h>

#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/uaccess.h>

/*
 * Some (32-bit) architectures don't support 64-bit {get,put}_user()
 *
 * 32-bit architectures which support 64-bit {get,put}_user() should provide
 * arch_has_uaccess_64() to indicate whether such support is present.
 */
#ifndef arch_has_uaccess_64
#define arch_has_uaccess_64()	(IS_ENABLED(CONFIG_64BIT))
#endif

/*
 * Some architectures have special instructions which can only access user
 * memory, and cannot access kernel memory, e.g.
 *
 * arm64: LDTR (Load Register, Unprivileged)
 *        STTR (Store Register, Unprivileged)
 *
 * s390: ???
 *
 * Where architectures (intend to) use such instructions in their lower-level
 * uaccess routines, it's worth checking that those behave correctly without
 * the access_ok() checks that are implicit in higher-level uaccess routines.
 *
 * Note that this is *NOT* the same as features where kernel accesses to user
 * memory can be disabled, e.g.
 *
 * arm64: PAN  (Privileged Access Never)
 * x86:   SMAP (Supervisor Mode Access Prevention)
 *
 * Where architectures toggle such controls in the lower-level uaccess
 * routines, and use regular loads/stores, it is expected that those routines
 * may access kernel memory if used incorrectly.
 *
 * Architectures with use special uaccess instructions should provide
 * arch_has_strict_uaccess() to indicate whether such support is present.
 */
#ifndef arch_has_strict_uaccess
#define arch_has_strict_uaccess()	(false)
#endif

#define GEN_KADDR_GU(type)						\
static void __init							\
kaddr_get_user_##type(struct kunit *test)				\
{									\
	type kval = ~(type)0;						\
	type val = ~(type)0;						\
	int err;							\
									\
	if (sizeof(type) == 8 && !arch_has_uaccess_64())		\
		kunit_skip(test, "64-bit uaccess unsupported");		\
									\
	err = get_user(val, (__force __user type *)&kval);		\
									\
	KUNIT_EXPECT_EQ(test, err, -EFAULT);				\
	KUNIT_EXPECT_EQ(test, val, 0);					\
}
GEN_KADDR_GU(u8)
GEN_KADDR_GU(u16)
GEN_KADDR_GU(u32)
GEN_KADDR_GU(u64)
#undef GEN_KADDR_GU

#define GEN_KADDR___GU(type)						\
static void __init							\
kaddr___get_user_##type(struct kunit *test)				\
{									\
	type kval = ~(type)0;						\
	type val = ~(type)0;						\
	int err;							\
									\
	if (sizeof(type) == 8 && !arch_has_uaccess_64())		\
		kunit_skip(test, "64-bit uaccess unsupported");		\
	if (!arch_has_strict_uaccess())					\
		kunit_skip(test, "strict uaccess unsupported");		\
									\
	err = __get_user(val, (__force __user type *)&kval);		\
									\
	KUNIT_EXPECT_EQ(test, err, -EFAULT);				\
	KUNIT_EXPECT_EQ(test, val, 0);					\
}
GEN_KADDR___GU(u8)
GEN_KADDR___GU(u16)
GEN_KADDR___GU(u32)
GEN_KADDR___GU(u64)
#undef GEN_KADDR___GU

#define GEN_KADDR_PU(type)						\
static void __init							\
kaddr_put_user_##type(struct kunit *test)				\
{									\
	type kval = 0;							\
	int err;							\
									\
	if (sizeof(type) == 8 && !arch_has_uaccess_64())		\
		kunit_skip(test, "64-bit uaccess unsupported");		\
									\
	err = put_user(~(type)0, (__force __user type *)&kval);		\
									\
	KUNIT_EXPECT_EQ(test, err, -EFAULT);				\
	KUNIT_EXPECT_EQ(test, kval, 0);					\
}
GEN_KADDR_PU(u8)
GEN_KADDR_PU(u16)
GEN_KADDR_PU(u32)
GEN_KADDR_PU(u64)
#undef GEN_KADDR_PU

#define GEN_KADDR___PU(type)						\
static void __init							\
kaddr___put_user_##type(struct kunit *test)				\
{									\
	type kval = 0;							\
	int err;							\
									\
	if (sizeof(type) == 8 && !arch_has_uaccess_64())		\
		kunit_skip(test, "64-bit uaccess unsupported");		\
	if (!arch_has_strict_uaccess())					\
		kunit_skip(test, "strict uaccess unsupported");		\
									\
	err = __put_user(~(type)0, (__force __user type *)&kval);	\
									\
	KUNIT_EXPECT_EQ(test, err, -EFAULT);				\
	KUNIT_EXPECT_EQ(test, kval, 0);					\
}
GEN_KADDR___PU(u8)
GEN_KADDR___PU(u16)
GEN_KADDR___PU(u32)
GEN_KADDR___PU(u64)
#undef GEN_KADDR___PU

/*
 * TODO: figure out the best way of constructing this so that this can be done
 * within a module.
 *
 * Note: The logic is largely inspired by __bprm_mm_init(), and a bit of
 * vm_mmap_pgoff().
 */
static struct mm_struct *user_mm_init(unsigned long start,
				      unsigned long end,
				      unsigned long vm_flags)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;

	mm = mm_alloc();
	if (!mm)
		return NULL;

	vma = vm_area_alloc(mm);
	if (!vma)
		goto out_free_mm;

	if (mmap_write_lock_killable(mm))
		goto out_free_vma;

	vma_set_anonymous(vma);
	vma->vm_start = start;
	vma->vm_end = end;
	vma->vm_flags = vm_flags;
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

	if (insert_vm_struct(mm, vma))
		goto out_unlock_mm;

	mmap_write_unlock(mm);
	return mm;

out_unlock_mm:
	mmap_write_unlock(mm);
out_free_vma:
	vm_area_free(vma);
out_free_mm:
	mmdrop(mm);
	return NULL;
}

/*
 * TODO: turn this into an actual set of tests
 */
static void test_foo(struct kunit *test)
{
	struct mm_struct *mm;
	u8 val = ~(u8)0;
	int err;

	mm = user_mm_init(0, PAGE_SIZE, VM_READ);
	KUNIT_ASSERT_NOT_NULL(test, mm);

	kthread_use_mm(mm);

	err = put_user(val, (__force __user u8 *)0UL);

	KUNIT_EXPECT_EQ(test, err, 0);

	kthread_unuse_mm(mm);

	mmdrop(mm);
}

static struct kunit_case __refdata uaccess_test_cases[] = {
	KUNIT_CASE(kaddr_get_user_u8),
	KUNIT_CASE(kaddr_get_user_u16),
	KUNIT_CASE(kaddr_get_user_u32),
	KUNIT_CASE(kaddr_get_user_u64),

	KUNIT_CASE(kaddr___get_user_u8),
	KUNIT_CASE(kaddr___get_user_u16),
	KUNIT_CASE(kaddr___get_user_u32),
	KUNIT_CASE(kaddr___get_user_u64),

	KUNIT_CASE(kaddr_put_user_u8),
	KUNIT_CASE(kaddr_put_user_u16),
	KUNIT_CASE(kaddr_put_user_u32),
	KUNIT_CASE(kaddr_put_user_u64),

	KUNIT_CASE(kaddr___put_user_u8),
	KUNIT_CASE(kaddr___put_user_u16),
	KUNIT_CASE(kaddr___put_user_u32),
	KUNIT_CASE(kaddr___put_user_u64),

	KUNIT_CASE(test_foo),

	{ /* sentinel */ }
};

static struct kunit_suite uaccess_test_suite = {
	.name = "uaccess",
	.test_cases = uaccess_test_cases,
};

kunit_test_suites(&uaccess_test_suite);

MODULE_AUTHOR("Mark Rutland <mark.rutland@arm.com>");
MODULE_LICENSE("GPL v2");
