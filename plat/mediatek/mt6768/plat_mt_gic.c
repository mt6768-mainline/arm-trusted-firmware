/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include "plat_private.h"

void clear_sec_pol_ctl_en(void)
{
	unsigned int i;

	/* Enable direct access to INT_POL_CTL registers */
	mmio_write_32(INT_CFG_DIRECT_ACCESS_EN, INT_CFG_DIRECT_ACCESS_ENABLE);

	/*
	 * Clear security polarity control enable to allow Non-Secure world (Linux)
	 * to configure interrupt polarity in SYSIRQ.
	 */
	for (i = 0; i <= NR_INT_POL_CTL - 1; i++) {
		mmio_write_32(SEC_POL_CTL_EN0 + (i * 4), 0);
	}

	/* Enable IRQ latency reduction */
	mmio_write_32(GIC_ACAO_CTL0, mmio_read_32(GIC_ACAO_CTL0) |
				     GIC_ACAO_CTL0_IRQ_LATENCY_REDUCTION);

	dsb();
}
