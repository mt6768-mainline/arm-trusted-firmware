/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>

#include <plat_helpers.h>
#include <platform_def.h>

const unsigned char mtk_power_domain_tree_desc[] = {
	/* Number of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* Number of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return mtk_power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if ((read_mpidr() & MPIDR_MT_MASK) != 0) {
		/* ARMv8.2 arch: AFF0 is thread (must be 0), AFF1 is core (0..7) */
		if ((mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) != 0) {
			return -1;
		}
		unsigned int pos = plat_mediatek_calc_core_pos(mpidr);

		if (pos >= PLATFORM_CORE_COUNT) {
			return -1;
		}

		return (int)pos;
	}

	mpidr &= MPIDR_AFFINITY_MASK;

	if ((mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) != 0) {
		return -1;
	}

	unsigned int cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	unsigned int cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER) {
		return -1;
	}

	return (int)cpu_id;
}
