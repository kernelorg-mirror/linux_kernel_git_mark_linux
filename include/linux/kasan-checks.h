/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_KASAN_CHECKS_H
#define _LINUX_KASAN_CHECKS_H

#ifdef __SANITIZE_ADDRESS__
void __kasan_check_read(const volatile void *p, unsigned int size);
#define kasan_check_read __kasan_check_read
void __kasan_check_write(const volatile void *p, unsigned int size);
#define kasan_check_write __kasan_check_write
#else
static inline void kasan_check_read(const volatile void *p, unsigned int size)
{ }
static inline void kasan_check_write(const volatile void *p, unsigned int size)
{ }
#endif

#endif
