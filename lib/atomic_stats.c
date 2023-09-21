// SPDX-License-Identifier: GPL-2.0

#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/percpu.h>

#define __do_atomic_op_str(op)	[ATOMIC_STATS_##op] = #op,

static const char *op_strings[NR_ATOMIC_STATS_OPS] = {
	__do_atomic_stats_ops(__do_atomic_op_str)
};

enum atomic_stats_count {
	ATOMIC_STAT_USED,
	ATOMIC_STAT_FAIL,
	ATOMIC_STAT_LOOP,
	NR_ATOMIC_STATS,
};

struct atomic_stats {
	atomic_long_t count[NR_ATOMIC_STATS_OPS][NR_ATOMIC_STATS];
};

DEFINE_PER_CPU(struct atomic_stats, pcp_stats);

void __atomic_stats_inc(enum atomic_stats_op op)
{
	struct atomic_stats *stats = raw_cpu_ptr(&pcp_stats);
	raw_atomic_long_inc(&stats->count[op][ATOMIC_STAT_USED]);
}

void __atomic_stats_bool_inc(enum atomic_stats_op op, bool ret)
{
	struct atomic_stats *stats = raw_cpu_ptr(&pcp_stats);
	raw_atomic_long_inc(&stats->count[op][ATOMIC_STAT_USED]);
	if (!ret)
		raw_atomic_long_inc(&stats->count[op][ATOMIC_STAT_FAIL]);
}

static long atomic_stats_get(enum atomic_stats_op op,
			     enum atomic_stats_count count)
{
	long total = 0;
	int c;

	for_each_possible_cpu(c) {
		struct atomic_stats *stats = per_cpu_ptr(&pcp_stats, c);
		total += raw_atomic_long_read(&stats->count[op][count]);
	}

	return total;
}

static int atomic_stats_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%32s %20s %20s %20s\n\n",
		   "operation", "used", "fail", "loop");

	for (int op = 0; op < NR_ATOMIC_STATS_OPS; op++) {
		seq_printf(m, "%32s %20ld %20ld %20ld\n",
			   op_strings[op],
			   atomic_stats_get(op, ATOMIC_STAT_USED),
			   atomic_stats_get(op, ATOMIC_STAT_FAIL),
			   atomic_stats_get(op, ATOMIC_STAT_LOOP));
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(atomic_stats);

int __init atomic_stats_init(void)
{
	debugfs_create_file("atomic_stats", 0400, NULL, NULL, &atomic_stats_fops);
	return 0;

}
late_initcall(atomic_stats_init);
