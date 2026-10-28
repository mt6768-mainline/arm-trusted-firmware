/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <mt_gic_v3.h>
#include <mtspmc.h>
#include <platform_def.h>
#include <pmic.h>

static uintptr_t secure_entrypoint;

static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	uint64_t scr = read_scr_el3();

	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();
	dsb();
	wfi();
	write_scr_el3(scr);
}

static int plat_power_domain_on(u_register_t mpidr)
{
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);
	unsigned int cluster = 0U;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	mcucfg_init_archstate(cluster, cpu, true);
	mcucfg_set_bootaddr(cluster, cpu, secure_entrypoint);
	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	assert(cpu < PLATFORM_CORE_COUNT);

	write_clusterpwrdn_el1(1);

	mcucfg_enable_gic_wakeup(0U, cpu);

	gicv3_rdistif_on(cpu);
	gicv3_cpuif_enable(cpu);
	mt_gic_rdistif_init();
	mt_gic_rdistif_restore();
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	assert(cpu < PLATFORM_CORE_COUNT);

	/* Prevent interrupts from spuriously waking up this CPU */
	mt_gic_rdistif_save();
	gicv3_cpuif_disable(cpu);
	gicv3_rdistif_off(cpu);

	spm_poweroff_cpu(0U, cpu);
	mcucfg_disable_gic_wakeup(0U, cpu);

	write_clusterpwrdn_el1(0);
}

static int plat_validate_power_state(unsigned int power_state,
				    psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int aff_lvl = psci_get_pstate_pwrlvl(power_state);

	if (aff_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	} else {
		for (unsigned int i = 0U; i <= aff_lvl; i++) {
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
		}
	}

	return PSCI_E_SUCCESS;
}

static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	for (unsigned int i = PSCI_CPU_PWR_LVL; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

static void __dead2 plat_system_off(void)
{
	INFO("MTK System Off\n");
	pmic_power_off();
	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	INFO("MTK System Reset\n");
	mmio_write_32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

static const plat_psci_ops_t plat_psci_ops = {
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_power_domain_on,
	.pwr_domain_on_finish		= plat_power_domain_on_finish,
	.pwr_domain_off			= plat_power_domain_off,
	.validate_power_state		= plat_validate_power_state,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_psci_ops;
	secure_entrypoint = sec_entrypoint;

	for (unsigned int i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		mcucfg_init_archstate(0U, i, true);
		mcucfg_set_bootaddr(0U, i, secure_entrypoint);
	}

	spmc_init();

	return 0;
}
