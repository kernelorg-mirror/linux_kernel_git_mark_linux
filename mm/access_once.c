#include <linux/bug.h>
#include <linux/export.h>
#include <linux/kernel.h>

void access_once_alignment_check(const volatile void *ptr, int size)
{
	switch (size) {
	case 1:
	case 2:
	case 4:
	case 8:
		WARN_ON(!IS_ALIGNED((unsigned long)ptr, size));
	}
}
EXPORT_SYMBOL(access_once_alignment_check);
