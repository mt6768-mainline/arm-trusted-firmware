/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_PRIVATE_H
#define MTSPMC_PRIVATE_H

#include <lib/utils_def.h>
#include <platform_def.h>
#include <stdint.h>

/* per_cpu/cluster helper */
struct per_cpu_reg {
	uintptr_t cluster_addr;
	uint32_t cpu_stride;
};

#define per_cpu(cluster, cpu, reg) \
	(reg[cluster].cluster_addr + ((cpu) << reg[cluster].cpu_stride))

#define per_cluster(cluster, reg) (reg[cluster].cluster_addr)

/* === SPMC related registers === */
#define SPM_POWERON_CONFIG_EN		(SPM_BASE + 0x000)
#define PROJECT_CODE			(U(0xb16) << 16)
#define BCLK_CG_EN			BIT(0)

#define SPM_PWR_STATUS			(SPM_BASE + 0x180)
#define SPM_PWR_STATUS_2ND		(SPM_BASE + 0x184)
#define SPM_CPU_PWR_STATUS		(SPM_BASE + 0x188)

#define SPM_MCUSYS_PWR_CON		(SPM_BASE + 0x200)
#define SPM_MP0_CPUTOP_PWR_CON		(SPM_BASE + 0x204)
#define SPM_MP0_CPU0_PWR_CON		(SPM_BASE + 0x208)

/* Bit fields of SPM_*_PWR_CON */
#define PWR_ON_ACK			BIT(31)
#define VPROC_EXT_OFF			BIT(7)
#define DORMANT_EN			BIT(6)
#define RESETPWRON_CONFIG		BIT(5)
#define PWR_CLK_DIS			BIT(4)
#define PWR_ON				BIT(2)
#define PWR_RST_B			BIT(0)

static const struct per_cpu_reg SPM_CPU_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2U },
};

static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPUTOP_PWR_CON, .cpu_stride = 0U },
};

/* === MCUCFG related registers === */
/* aa64naa32 */
#define MCUCFG_MP0_CLUSTER_CFG5		(MCUCFG_BASE + 0xc8e4)
/* reset vectors */
#define MCUCFG_MP0_CLUSTER_CFG8		(MCUCFG_BASE + 0xc900)

static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_CLUSTER_CFG8, .cpu_stride = 3U },
};

static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_CLUSTER_CFG5, .cpu_stride = 0U },
};

/* === CPC control === */
#define MCUCFG_CPC_FLOW_CTRL_CFG	(MCUCFG_BASE + 0xa814)

/* Bit fields of CPC_FLOW_CTRL_CFG */
#define CPC_CTRL_ENABLE			BIT(16)
#define SSPM_ALL_PWR_CTRL_EN		BIT(13)
#define GIC_WAKEUP_IGNORE(cpu)		BIT(21 + (cpu))

#endif /* MTSPMC_PRIVATE_H */
