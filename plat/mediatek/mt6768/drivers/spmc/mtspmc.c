/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mcucfg.h>
#include <mtspmc.h>
#include <mtspmc_private.h>
#include <plat/common/platform.h>

void mcucfg_disable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_enable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));
}

void mcucfg_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t bootaddr)
{
	assert(cluster == 0U);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), (uint32_t)bootaddr);
}

uintptr_t mcucfg_get_bootaddr(unsigned int cluster, unsigned int cpu)
{
	assert(cluster == 0U);

	return (uintptr_t)mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(unsigned int cluster, unsigned int cpu, bool arm64)
{
	uintptr_t reg;

	assert(cluster == 0U);

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	/* aa64naa32 in bits[16:23] */
	if (arm64) {
		mmio_setbits_32(reg, BIT(16 + cpu));
	} else {
		mmio_clrbits_32(reg, BIT(16 + cpu));
	}
}

bool spm_get_powerstate(uint32_t mask)
{
	return (mmio_read_32(SPM_PWR_STATUS) & mask) != 0U;
}

bool spm_get_cluster_powerstate(unsigned int cluster)
{
	assert(cluster == 0U);

	return true;
}

bool spm_get_cpu_powerstate(unsigned int cluster, unsigned int cpu)
{
	uint32_t mask = BIT(16 + cpu);

	assert(cluster == 0U);

	return spm_get_powerstate(mask);
}

int spmc_init(void)
{
	unsigned int cpu = plat_my_core_pos();

	INFO("SPM: enable CPC mode\n");

	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	/* Release PWR_RST_B of Core 1-7 */
	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 4, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 5, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 6, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 7, SPM_CPU_PWR), PWR_RST_B);

	/* Clear resetpwron of mcusys, cputop, core0 */
	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), RESETPWRON_CONFIG);

	/* Enable GIC wakeup and clear CPC ON hint for boot CPU */
	mcucfg_enable_gic_wakeup(0U, cpu);

	/* Disable CPC reset power-on and enable CPC flow control */
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_MCUSYS_CPC_RESET_PWR_ON_EN);
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);

	return 0;
}

void spm_poweron_cpu(unsigned int cluster, unsigned int cpu)
{
	/* info CPC that CPU hotplug on */
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);

	/* Set mp0_spmc_pwr_on_cpuX = 1 */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* wait for power on ack */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* info CPC that CPU hotplug off */
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);
}

void spm_poweroff_cpu(unsigned int cluster, unsigned int cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);
}

void spm_poweroff_cluster(unsigned int cluster)
{
}

void spm_poweron_cluster(unsigned int cluster)
{
}
