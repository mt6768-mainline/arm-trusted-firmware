/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCUCFG_H
#define MCUCFG_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif /* __ASSEMBLER__ */

#include <lib/utils_def.h>
#include <platform_def.h>

#define MCUCFG_REG(ofs)			(MCUCFG_BASE + (ofs))

#define MP0_AXI_CONFIG			MCUCFG_REG(0x002c)
#define MPx_AXI_CONFIG_ACINACTM		BIT(4)

#define MP0_RW_RSVD0			MCUCFG_REG(0x006c)

#define CA75_CPU6_SRAM_DELSEL		MCUCFG_REG(0x3240)
#define CA75_CPU6_DBG_CONTROL		MCUCFG_REG(0x3308)
#define CA75_CPU7_SRAM_DELSEL		MCUCFG_REG(0x3a40)
#define CA75_CPU7_DBG_CONTROL		MCUCFG_REG(0x3b08)
#define CPU_SRAM_DELSEL_MASK		U(0xffffff)
#define CPU_SRAM_DELSEL_VAL		U(0x249249)
#define BIT_CA75_LASTPC_DIS		BIT(0)

/* CPC related registers */
#define CPC_MCUSYS_CPC_OFF_THRES	MCUCFG_REG(0xa820)
#define CPC_MCUSYS_PWR_CTRL		MCUCFG_REG(0xa804)
#define CPC_MCUSYS_CPC_FLOW_CTRL_CFG	MCUCFG_REG(0xa814)
#define CPC_MCUSYS_LAST_CORE_REQ	MCUCFG_REG(0xa818)
#define CPC_MCUSYS_MP_LAST_CORE_RESP	MCUCFG_REG(0xa81c)
#define CPC_MCUSYS_LAST_CORE_RESP	MCUCFG_REG(0xa824)
#define CPC_MCUSYS_PWR_ON_MASK		MCUCFG_REG(0xa828)
#define CPC_MCUSYS_CPU_ON_SW_HINT_SET	MCUCFG_REG(0xa8a8)
#define CPC_MCUSYS_CPU_ON_SW_HINT_CLR	MCUCFG_REG(0xa8ac)

#define MP0_CLUSTER_CFG0		MCUCFG_REG(0xC8D0)

#endif /* MCUCFG_H */
