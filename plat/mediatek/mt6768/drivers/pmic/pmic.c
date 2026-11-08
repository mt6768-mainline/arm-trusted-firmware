/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <pmic.h>
#include <pmic_wrap_init.h>

uint32_t pmic_get_hwcid(void)
{
	uint32_t val = 0;

	pwrap_read(PMIC_RG_HWCID_ADDR, &val);

	return val;
}

void wk_pmic_enable_sdn_delay(void)
{
	uint32_t con = 0;

	pwrap_write(PMIC_TMA_KEY, 0x9CA7);
	pwrap_read(PMIC_PSEQ_ELR11, &con);
	con &= ~PMIC_RG_SDN_DLY_ENB;
	pwrap_write(PMIC_PSEQ_ELR11, con);
	pwrap_write(PMIC_TMA_KEY, 0);
}

void pmic_power_off(void)
{
	uint32_t con = 0;

	pwrap_read(PMIC_STRUP_CON12, &con);
	con &= ~(PMIC_RG_SMART_RST_MODE | PMIC_RG_SMART_RST_SDN_EN);
	pwrap_write(PMIC_STRUP_CON12, con);

	pwrap_write(PMIC_PWRHOLD, 0x0);
}
