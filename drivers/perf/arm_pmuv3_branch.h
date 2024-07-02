/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Branch Record Buffer Extension Helpers.
 *
 * Copyright (C) 2022-2023 ARM Limited
 *
 * Author: Anshuman Khandual <anshuman.khandual@arm.com>
 */
#include <linux/perf/arm_pmu.h>

#ifdef CONFIG_ARM64_BRBE
void armv8pmu_branch_stack_add(struct perf_event *event, struct pmu_hw_events *cpuc);
void armv8pmu_branch_stack_del(struct perf_event *event, struct pmu_hw_events *cpuc);
void armv8pmu_branch_stack_reset(void);
void armv8pmu_branch_probe(struct arm_pmu *arm_pmu);
bool armv8pmu_branch_attr_valid(struct perf_event *event);
void armv8pmu_branch_enable(struct arm_pmu *arm_pmu);
void armv8pmu_branch_disable(void);
void brbe_read_filtered_entries(struct perf_branch_stack *branch_stack, struct perf_event *event);
#else
static inline void armv8pmu_branch_stack_add(struct perf_event *event, struct pmu_hw_events *cpuc)
{
}

static inline void armv8pmu_branch_stack_del(struct perf_event *event, struct pmu_hw_events *cpuc)
{
}

static inline void armv8pmu_branch_stack_reset(void)
{
}

static inline void armv8pmu_branch_probe(struct arm_pmu *arm_pmu)
{
}

static inline bool armv8pmu_branch_attr_valid(struct perf_event *event)
{
	WARN_ON_ONCE(!has_branch_stack(event));
	return false;
}

static inline void armv8pmu_branch_enable(struct arm_pmu *arm_pmu)
{
}

static inline void armv8pmu_branch_disable(void)
{
}

static void brbe_read_filtered_entries(struct perf_branch_stack *branch_stack,
				       struct perf_event *event)
{
}
#endif
