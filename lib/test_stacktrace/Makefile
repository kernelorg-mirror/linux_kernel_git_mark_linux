# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_TEST_STACKTRACE) += test_stacktrace.o

test_stacktrace-objs	+= funcs.o \
			   kunit.o

# Inhibit tail calls so we can ensure that functions should be in the trace.
CFLAGS_funcs.o		+= -fno-optimize-sibling-calls
