// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpuhotplug.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/perf_event.h>

static enum cpuhp_state bogo_cpuhp_state;

struct bogo_cpu_state {
	bool ready;
};

static DEFINE_PER_CPU(struct bogo_cpu_state, cpu_state);

static int bogo_pmu_event_init(struct perf_event *event)
{
	if (event->attr.type != event->pmu->type)
		return -EINVAL;

	return 0;
}

static void bogo_pmu_enable(struct pmu *pmu)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static void bogo_pmu_disable(struct pmu *pmu)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static int bogo_pmu_add(struct perf_event *event, int flags)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
	return 0;
}

static void bogo_pmu_del(struct perf_event *event, int flags)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static void bogo_pmu_start(struct perf_event *event, int flags)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static void bogo_pmu_stop(struct perf_event *event, int flags)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static void bogo_pmu_read(struct perf_event *event)
{
	WARN_ON_ONCE(!this_cpu_read(cpu_state.ready));
}

static int bogo_pmu_cpu_startup(unsigned int cpu)
{
	WARN_ON_ONCE(!irqs_disabled());

	/* Extend a possible race window */
	udelay(100);

	this_cpu_write(cpu_state.ready, true);
	return 0;
}

static int bogo_pmu_cpu_teardown(unsigned int cpu)
{
	this_cpu_write(cpu_state.ready, false);
	return 0;
}

struct hlist_node	hotplug_node;

struct pmu bogo_pmu = {
	.module		= THIS_MODULE,
	.task_ctx_nr	= perf_sw_context,
	.pmu_enable	= bogo_pmu_enable,
	.pmu_disable	= bogo_pmu_disable,
	.event_init	= bogo_pmu_event_init,
	.add		= bogo_pmu_add,
	.del		= bogo_pmu_del,
	.start		= bogo_pmu_start,
	.stop		= bogo_pmu_stop,
	.read		= bogo_pmu_read,
};

static int __init bogo_pmu_driver_init(void)
{
	int ret;

	ret = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "bogo_pmu",
				bogo_pmu_cpu_startup,
				bogo_pmu_cpu_teardown);
	if (ret < 0)
		return ret;

	bogo_cpuhp_state = ret;

	ret = perf_pmu_register(&bogo_pmu, "bogo_pmu", -1);
	if (ret)
		goto out_remove_cpuhp;

	return 0;

out_remove_cpuhp:
	cpuhp_remove_multi_state(bogo_cpuhp_state);
	return ret;
}

static void __exit bogo_pmu_driver_exit(void)
{
	cpuhp_remove_multi_state(bogo_cpuhp_state);
}

module_init(bogo_pmu_driver_init);
module_exit(bogo_pmu_driver_exit);

MODULE_DESCRIPTION("Bogus PMU for testing");
MODULE_AUTHOR("Mark Rutland <mark.rutland@arm.com>");
MODULE_LICENSE("GPL v2");
