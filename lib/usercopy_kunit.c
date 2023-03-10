// SPDX-License-Identifier: GPL-2.0
/*
 * Test cases for usercopy faulting behaviour
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <kunit/test.h>

#include <linux/highmem.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

/*
 * Arbitrarily chosen user address for the test page.
 */
#define UBUF_ADDR_BASE	SZ_2M

struct usercopy_env {
	struct mm_struct		*mm;
	void				*kbuf;
	struct page			*ubuf_page;
	void				*ubuf;
};

struct usercopy_params {
	long		offset;
	unsigned long	size;
};

static void *usercopy_env_alloc(void)
{
	struct usercopy_env *env = kzalloc(sizeof(*env), GFP_KERNEL);
	if (!env)
		return NULL;

	env->kbuf = vmalloc(PAGE_SIZE);
	if (!env->kbuf)
		goto out_free_env;

	return env;

out_free_env:
	kfree(env);
	return NULL;
}

static void usercopy_env_free(struct usercopy_env *env)
{
	vfree(env->kbuf);
	kfree(env);
}

static void *usercopy_mm_alloc(struct usercopy_env *env)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	mm = mm_alloc();
	if (!mm)
		return NULL;

	if (mmap_write_lock_killable(mm))
		goto out_free;

	vma = vm_area_alloc(mm);
	if (!vma)
		goto out_unlock;

	vma_set_anonymous(vma);
	vma->vm_start = UBUF_ADDR_BASE;
	vma->vm_end = UBUF_ADDR_BASE + PAGE_SIZE;
	vm_flags_init(vma, VM_READ | VM_MAYREAD | VM_WRITE | VM_MAYWRITE);
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

	if (insert_vm_struct(mm, vma))
		goto out_free_vma;

	mmap_write_unlock(mm);
	return mm;

out_free_vma:
	vm_area_free(vma);
out_unlock:
	mmap_write_unlock(mm);
out_free:
	mmput(mm);
	return NULL;
}

static void usercopy_mm_free(struct mm_struct *mm)
{
	mmput(mm);
}

static struct page *usercopy_ubuf_pin(struct usercopy_env *env)
{
	struct page *p = NULL;

	kthread_use_mm(env->mm);
	pin_user_pages_unlocked(UBUF_ADDR_BASE, 1, &p, FOLL_LONGTERM);
	kthread_unuse_mm(env->mm);

	return p;
}

static void usercopy_ubuf_unpin(struct usercopy_env *env)
{
	unpin_user_page(env->ubuf_page);
}

static int usercopy_test_init(struct kunit *test)
{
	struct usercopy_env *env;

	env = usercopy_env_alloc();
	if (!env)
		return -ENOMEM;

	env->mm = usercopy_mm_alloc(env);
	if (!env->mm)
		goto out_free_env;

	env->ubuf_page = usercopy_ubuf_pin(env);
	if (!env->ubuf_page)
		goto out_free_mm;

	env->ubuf = kmap(env->ubuf_page);
	if (!env->ubuf)
		goto out_unpin_ubuf;

	test->priv = env;

	return 0;

out_unpin_ubuf:
	usercopy_ubuf_unpin(env);
out_free_mm:
	usercopy_mm_free(env->mm);
out_free_env:
	usercopy_env_free(env);
	return -ENOMEM;
}

static void usercopy_test_exit(struct kunit *test)
{
	struct usercopy_env *env = test->priv;

	kunmap(env->ubuf);

	usercopy_ubuf_unpin(env);
	usercopy_mm_free(env->mm);
	usercopy_env_free(env);
}

static char buf_zero(int offset)
{
	return 0;
}

static char buf_pattern(int offset)
{
	return offset & 0xff;
}

static void buf_init_pattern(char *buf)
{
	for (int i = 0; i < PAGE_SIZE; i++)
		buf[i] = buf_pattern(i);
}

static void buf_init_zero(char *buf)
{
	memset(buf, 0, PAGE_SIZE);
}

static void assert_size_valid(struct kunit *test,
			      const struct usercopy_params *params,
			      unsigned long ret)
{
	const unsigned long size = params->size;
	const long offset = params->offset;

	if (ret > size) {
		KUNIT_ASSERT_FAILURE(test,
			"return value is impossibly large (offset=%ld, size=%lu, ret=%lu)",
			offset, size, ret);
	}

	/*
	 * When the user buffer starts within a faulting page, no bytes can be
	 * copied, so ret must equal size.
	 */
	if (offset < 0 || offset >= PAGE_SIZE) {
		if (ret == size)
			return;

		KUNIT_ASSERT_FAILURE(test,
			"impossible copy did not fail (offset=%ld, size=%lu, ret=%lu)",
			offset, size, ret);
	}

	/*
	 * When the user buffer fits entirely within a non-faulting page, all
	 * bytes must be copied, so ret must equal 0.
	 */
	if (offset + size <= PAGE_SIZE) {
		if (ret == 0)
			return;

		KUNIT_ASSERT_FAILURE(test,
			"completely possible copy failed (offset=%ld, size=%lu, ret=%lu)",
			offset, size, ret);
	}

	/*
	 * The buffer starts in a non-faulting page, but continues into a
	 * faulting page. At least one byte must be copied, and at most all the
	 * non-faulting bytes may be copied.
	 */
	if (ret == size) {
		KUNIT_ASSERT_FAILURE(test,
			"too few bytes consumed (offset=%ld, size=%lu, ret=%lu)",
			offset, size, ret);
	}

	if (ret < (offset + size) - PAGE_SIZE) {
		KUNIT_ASSERT_FAILURE(test,
			   "too many bytes consumed (offset=%ld, size=%lu, ret=%lu)",
			   offset, size, ret);
	}
}

static void assert_src_valid(struct kunit *test,
			     const struct usercopy_params *params,
			     char (*buf_expected)(int),
			     const char *src, long src_offset,
			     unsigned long ret)
{
	const unsigned long size = params->size;
	const long offset = params->offset;

	/*
	 * A usercopy MUST NOT modify the source buffer.
	 */
	for (int i = 0; i < PAGE_SIZE; i++) {
		char expected = buf_expected(i);
		char val = src[i];

		if (val == expected)
			continue;

		KUNIT_ASSERT_FAILURE(test,
			"source bytes modified (src[%d]=0x%x, offset=%ld, size=%lu, ret=%lu)",
			i, offset, size, ret);
	}
}

static void assert_dst_valid(struct kunit *test,
			     const struct usercopy_params *params,
			     char (*buf_expected)(int),
			     const char *dst, long dst_offset,
			     unsigned long ret)
{
	const unsigned long size = params->size;
	const long offset = params->offset;

	/*
	 * A usercopy MUST NOT modify any bytes before the destination buffer.
	 */
	for (int i = 0; i < dst_offset; i++) {
		char expected = buf_expected(i);
		char val = dst[i];

		if (val == expected)
			continue;

		KUNIT_ASSERT_FAILURE(test,
			"pre-destination bytes modified (dst_page[%d]=0x%x, offset=%ld, size=%lu, ret=%lu)",
			i, val, offset, size, ret);
	}

	/*
	 * A usercopy MUST NOT modify any bytes after the end of the destination
	 * buffer.
	 */
	for (int i = dst_offset + size - ret; i < PAGE_SIZE; i++) {
		char expected = buf_expected(i);
		char val = dst[i];

		if (val == expected)
			continue;

		KUNIT_ASSERT_FAILURE(test,
			"post-destination bytes modified (dst_page[%d]=0x%x, offset=%ld, size=%lu, ret=%lu)",
			i, val, offset, size, ret);
	}
}

static void assert_copy_valid(struct kunit *test,
			      const struct usercopy_params *params,
			      const char *dst, long dst_offset,
			      const char *src, long src_offset,
			      unsigned long ret)
{
	const unsigned long size = params->size;
	const long offset = params->offset;

	/*
	 * Have we actually copied the bytes we expected to?
	 */
	for (int i = 0; i < params->size - ret; i++) {
		char dst_val = dst[dst_offset + i];
		char src_val = src[src_offset + i];

		if (dst_val == src_val)
			continue;

		KUNIT_ASSERT_FAILURE(test,
			"copied bytes incorrect (dst_page[%ld+%d]=0x%x, src_page[%ld+%d]=0x%x, offset=%ld, size=%lu, ret=%lu",
			dst_offset, i, dst_val,
			src_offset, i, src_val,
			offset, size, ret);
	}
}

static void assert_clear_valid(struct kunit *test,
			       const struct usercopy_params *params,
			       const char *dst, long dst_offset,
			       unsigned long ret)
{
	const unsigned long size = params->size;
	const long offset = params->offset;

	/*
	 * Have we actually zeroed the bytes we expected to?
	 */
	for (int i = 0; i < params->size - ret; i++) {
		char dst_val = dst[dst_offset + i];

		if (dst_val == 0)
			continue;

		KUNIT_ASSERT_FAILURE(test,
			"zeroed bytes incorrect (dst_page[%ld+%d]=0x%x, offset=%ld, size=%lu, ret=%lu",
			dst_offset, i, dst_val,
			offset, size, ret);
	}
}
static unsigned long do_copy_to_user(const struct usercopy_env *env,
				     const struct usercopy_params *params)
{
	void __user *uptr = (void __user *)UBUF_ADDR_BASE + params->offset;
	void *kptr = env->kbuf;
	unsigned long ret;

	kthread_use_mm(env->mm);
	ret = raw_copy_to_user(uptr, kptr, params->size);
	kthread_unuse_mm(env->mm);

	return ret;
}

static unsigned long do_copy_from_user(const struct usercopy_env *env,
				       const struct usercopy_params *params)
{
	void __user *uptr = (void __user *)UBUF_ADDR_BASE + params->offset;
	void *kptr = env->kbuf;
	unsigned long ret;

	kthread_use_mm(env->mm);
	ret = raw_copy_from_user(kptr, uptr, params->size);
	kthread_unuse_mm(env->mm);

	return ret;
}

static unsigned long do_clear_user(const struct usercopy_env *env,
				   const struct usercopy_params *params)
{
	void __user *uptr = (void __user *)UBUF_ADDR_BASE + params->offset;
	unsigned long ret;

	kthread_use_mm(env->mm);
	ret = clear_user(uptr, params->size);
	kthread_unuse_mm(env->mm);

	return ret;
}

/*
 * Generate the size and offset combinations to test.
 *
 * Usercopies may have different paths for small/medium/large copies, but
 * typically max out at 64 byte chunks. We test sizes 0 to 128 bytes to check
 * all combinations of leading/trailing chunks and bulk copies.
 *
 * For each size, we test every offset relative to a leading and trailing page
 * boundary (i.e. [size, 0] and [PAGE_SIZE - size, PAGE_SIZE]) to check every
 * possible faulting boundary.
 */
#define for_each_size_offset(size, offset)				\
	for (unsigned long size = 0; size <= 128; size++)		\
		for (long offset = -size;				\
		     offset <= (long)PAGE_SIZE;				\
		     offset = (offset ? offset + 1: (PAGE_SIZE - size)))

static void test_copy_to_user(struct kunit *test)
{
	const struct usercopy_env *env = test->priv;

	for_each_size_offset(size, offset) {
		const struct usercopy_params params = {
			.size = size,
			.offset = offset,
		};
		unsigned long ret;

		buf_init_zero(env->ubuf);
		buf_init_pattern(env->kbuf);

		ret = do_copy_to_user(env, &params);

		assert_size_valid(test, &params, ret);
		assert_src_valid(test, &params, buf_pattern,
				 env->kbuf, 0, ret);
		assert_dst_valid(test, &params, buf_zero,
				 env->ubuf, params.offset, ret);
		assert_copy_valid(test, &params,
				  env->ubuf, params.offset,
				  env->kbuf, 0,
				  ret);
	}
}

static void test_copy_from_user(struct kunit *test)
{
	const struct usercopy_env *env = test->priv;

	for_each_size_offset(size, offset) {
		const struct usercopy_params params = {
			.size = size,
			.offset = offset,
		};
		unsigned long ret;

		buf_init_zero(env->kbuf);
		buf_init_pattern(env->ubuf);

		ret = do_copy_from_user(env, &params);

		assert_size_valid(test, &params, ret);
		assert_src_valid(test, &params, buf_pattern,
				 env->ubuf, params.offset, ret);
		assert_dst_valid(test, &params, buf_zero,
				 env->kbuf, 0, ret);
		assert_copy_valid(test, &params,
				  env->kbuf, 0,
				  env->ubuf, params.offset,
				  ret);
	}
}

static void test_clear_user(struct kunit *test)
{
	const struct usercopy_env *env = test->priv;

	for_each_size_offset(size, offset) {
		const struct usercopy_params params = {
			.size = size,
			.offset = offset,
		};
		unsigned long ret;

		buf_init_pattern(env->ubuf);

		ret = do_clear_user(env, &params);

		assert_size_valid(test, &params, ret);
		assert_dst_valid(test, &params, buf_pattern,
				 env->ubuf, params.offset, ret);
		assert_clear_valid(test, &params,
				   env->ubuf, params.offset,
				   ret);
	}
}

static struct kunit_case usercopy_cases[] = {
	KUNIT_CASE(test_copy_to_user),
	KUNIT_CASE(test_copy_from_user),
	KUNIT_CASE(test_clear_user),
	{ /* sentinel */ }
};

static struct kunit_suite usercopy_suite = {
	.name = "usercopy",
	.init = usercopy_test_init,
	.exit = usercopy_test_exit,
	.test_cases = usercopy_cases,
};
kunit_test_suites(&usercopy_suite);

MODULE_AUTHOR("Mark Rutland <mark.rutland@arm.com>");
MODULE_LICENSE("GPL v2");
