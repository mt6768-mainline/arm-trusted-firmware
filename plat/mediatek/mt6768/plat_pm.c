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
#include <mcucfg.h>
#include <mtspmc.h>
#include <platform_def.h>
#include <pmic.h>
#include <rtc.h>

#define MTK_MCDI_MODE			U(1)
#define PLAT_MT_CPU_SUSPEND_CLUSTER	MPIDR_AFFLVL1

#define IS_CLUSTER_OFF_STATE(s) \
	((s)->pwr_domain_state[MPIDR_AFFLVL1] == PLAT_MAX_OFF_STATE)

static uintptr_t secure_entrypoint;
static unsigned int plat_power_state[PLATFORM_CORE_COUNT];

static void circular_buffer_setup(void)
{
	/* Clear DBG_CONTROL[0] to enable circular buffer */
	mmio_clrbits_32(CA75_CPU6_DBG_CONTROL, BIT_CA75_LASTPC_DIS);
	mmio_clrbits_32(CA75_CPU7_DBG_CONTROL, BIT_CA75_LASTPC_DIS);
}

static void sram_delsel_setup(void)
{
	/* Adjust CPU6 and CPU7 SRAM DELSEL matching after cluster powerup */
	mmio_clrsetbits_32(CA75_CPU6_SRAM_DELSEL, CPU_SRAM_DELSEL_MASK,
			   CPU_SRAM_DELSEL_VAL);
	mmio_clrsetbits_32(CA75_CPU7_SRAM_DELSEL, CPU_SRAM_DELSEL_MASK,
			   CPU_SRAM_DELSEL_VAL);
}

static void plat_cpu_pwrdwn_common(unsigned int cpu,
				   const psci_power_state_t *state,
				   unsigned int req_pstate)
{
	assert(cpu < PLATFORM_CORE_COUNT);

	if ((psci_get_pstate_pwrlvl(req_pstate) >= PLAT_MT_CPU_SUSPEND_CLUSTER) ||
	    IS_CLUSTER_OFF_STATE(state) || (req_pstate == 0U)) {
		write_clusterpwrdn_el1(0);
	}

	/* Prevent interrupts from spuriously waking up this CPU */
	write_dbgprcr_el1(0ULL);
	gicv3_cpuif_disable(cpu);
	mt_gic_rdistif_save();
	gicv3_rdistif_off(cpu);
}

static void plat_cpu_pwron_common(unsigned int cpu,
				  const psci_power_state_t *state,
				  unsigned int req_pstate)
{
	assert(cpu < PLATFORM_CORE_COUNT);

	if (IS_CLUSTER_OFF_STATE(state)) {
		circular_buffer_setup();
		sram_delsel_setup();
	}

	write_clusterpwrdn_el1(1);

	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));

	gicv3_rdistif_on(cpu);
	gicv3_cpuif_enable(cpu);
	mt_gic_rdistif_init();
	mt_gic_rdistif_restore();

	plat_power_state[cpu] = 0U;
}

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

	mcucfg_enable_gic_wakeup(0U, cpu);
	plat_cpu_pwron_common(cpu, state, 0U);
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	plat_cpu_pwrdwn_common(cpu, state, 0U);

	spm_poweroff_cpu(0U, cpu);
	mcucfg_disable_gic_wakeup(0U, cpu);
}

static void plat_power_domain_suspend(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	plat_cpu_pwrdwn_common(cpu, state, plat_power_state[cpu]);
}

static void plat_power_domain_suspend_finish(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	plat_cpu_pwron_common(cpu, state, plat_power_state[cpu]);
}

static int plat_validate_power_state(unsigned int power_state,
				    psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int aff_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int state_id = psci_get_pstate_id(power_state);
	unsigned int cpu = plat_my_core_pos();

	if (aff_lvl > PLAT_MT_CPU_SUSPEND_CLUSTER) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	} else {
		if (state_id > MTK_MCDI_MODE) {
			return PSCI_E_INVALID_PARAMS;
		}

		for (unsigned int i = 0U; i <= aff_lvl; i++) {
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
		}
	}

	plat_power_state[cpu] = power_state;

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
	rtc_power_off_sequence();
	wk_pmic_enable_sdn_delay();
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
	.pwr_domain_suspend		= plat_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_power_domain_suspend_finish,
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

	circular_buffer_setup();
	sram_delsel_setup();
	spmc_init();

	return 0;
}
