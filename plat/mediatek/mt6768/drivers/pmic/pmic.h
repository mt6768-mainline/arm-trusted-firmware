/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_H
#define PMIC_H

#include <stdint.h>

#define PMIC_RG_HWCID_ADDR     0x0008
#define PMIC_TMA_KEY           0x03a8
#define PMIC_PWRHOLD           0x0a08
#define PMIC_STRUP_CON12       0x0a3c
#define PMIC_PSEQ_ELR11        0x0a62

#define PMIC_RG_SDN_DLY_ENB          (1U << 10)
#define PMIC_RG_SMART_RST_SDN_EN     (1U << 12)
#define PMIC_RG_SMART_RST_MODE       (1U << 13)

uint32_t pmic_get_hwcid(void);
void wk_pmic_enable_sdn_delay(void);
void pmic_power_off(void);

#endif /* PMIC_H */
