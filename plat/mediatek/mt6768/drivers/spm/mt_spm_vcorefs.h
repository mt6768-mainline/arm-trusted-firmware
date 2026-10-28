/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_VCOREFS_H
#define MT_SPM_VCOREFS_H

#include <stdint.h>
#include <platform_def.h>

void spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3);
uint64_t spm_vcorefs_v2_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t *x4);
void spm_args(uint64_t x1, uint64_t x2, uint64_t x3);
void spm_request_dvfs_opp(uint64_t id, uint64_t reg);

enum vcorefs_smc_cmd {
	VCOREFS_SMC_CMD_0,
	VCOREFS_SMC_CMD_1,
	VCOREFS_SMC_CMD_2,
	VCOREFS_SMC_CMD_3,
	VCOREFS_SMC_CMD_4,
	NUM_VCOREFS_SMC_CMD,
};

enum vcorefs_smc_cmd_new {
	VCOREFS_SMC_CMD_INIT = 0,
	VCOREFS_SMC_CMD_KICK = 1,
	VCOREFS_SMC_CMD_OPP_TYPE = 2,
	VCOREFS_SMC_CMD_FW_TYPE = 3,
	VCOREFS_SMC_CMD_GET_UV = 4,
	VCOREFS_SMC_CMD_GET_FREQ = 5,
	VCOREFS_SMC_CMD_GET_NUM_V = 6,
	VCOREFS_SMC_CMD_GET_NUM_F = 7,
	VCOREFS_SMC_CMD_FB_ACTION = 8,
	/* Chip specific setting */
	VCOREFS_SMC_CMD_SET_FREQ = 16,
	VCOREFS_SMC_CMD_SET_EFUSE = 17,
	VCOREFS_SMC_CMD_GET_EFUSE = 18,
	VCOREFS_SMC_CMD_DVFS_HOPPING = 19,
	VCOREFS_SMC_CMD_DVFS_HOPPING_STATE = 20,
};

enum {
	SPMFW_LP4_2CH_3200 = 0,
	SPMFW_LP4X_2CH_3600,
	SPMFW_LP4X_2CH_3200,
	SPMFW_LP3_1CH_1866,
};

enum {
	SPM_ARGS_SPMFW_IDX = 0,
	SPM_ARGS_SPMFW_INIT,
	SPM_ARGS_SUSPEND,
	SPM_ARGS_SUSPEND_FINISH,
	SPM_ARGS_SODI,
	SPM_ARGS_SODI_FINISH,
};

#define _VCORE_BASE_UV		500000
#define _VCORE_STEP_UV		6250

/* PMIC */
#define __vcore_pmic_to_uv(pmic)	\
	(((pmic) * _VCORE_STEP_UV) + _VCORE_BASE_UV)

#define __vcore_uv_to_pmic(uv)	/* pmic >= uv */	\
	((((uv) - _VCORE_BASE_UV) + (_VCORE_STEP_UV - 1)) / _VCORE_STEP_UV)

struct reg_config {
	uint32_t offset;
	uint32_t val;
};

#define DVFSRC_BASIC_CONTROL       (DVFSRC_BASE + 0x0)
#define DVFSRC_SW_REQ              (DVFSRC_BASE + 0x4)
#define DVFSRC_SW_REQ2             (DVFSRC_BASE + 0x8)
#define DVFSRC_EMI_REQUEST         (DVFSRC_BASE + 0xC)
#define DVFSRC_EMI_REQUEST2        (DVFSRC_BASE + 0x10)
#define DVFSRC_EMI_REQUEST3        (DVFSRC_BASE + 0x14)
#define DVFSRC_EMI_HRT             (DVFSRC_BASE + 0x18)
#define DVFSRC_EMI_HRT2            (DVFSRC_BASE + 0x1C)
#define DVFSRC_EMI_HRT3            (DVFSRC_BASE + 0x20)
#define DVFSRC_EMI_QOS0            (DVFSRC_BASE + 0x24)
#define DVFSRC_EMI_QOS1            (DVFSRC_BASE + 0x28)
#define DVFSRC_EMI_QOS2            (DVFSRC_BASE + 0x2C)
#define DVFSRC_EMI_MD2SPM0         (DVFSRC_BASE + 0x30)
#define DVFSRC_EMI_MD2SPM1         (DVFSRC_BASE + 0x34)
#define DVFSRC_EMI_MD2SPM2         (DVFSRC_BASE + 0x38)
#define DVFSRC_EMI_MD2SPM0_T       (DVFSRC_BASE + 0x3C)
#define DVFSRC_EMI_MD2SPM1_T       (DVFSRC_BASE + 0x40)
#define DVFSRC_EMI_MD2SPM2_T       (DVFSRC_BASE + 0x44)
#define DVFSRC_VCORE_REQUEST       (DVFSRC_BASE + 0x48)
#define DVFSRC_VCORE_REQUEST2      (DVFSRC_BASE + 0x4C)
#define DVFSRC_VCORE_HRT           (DVFSRC_BASE + 0x50)
#define DVFSRC_VCORE_HRT2          (DVFSRC_BASE + 0x54)
#define DVFSRC_VCORE_HRT3          (DVFSRC_BASE + 0x58)
#define DVFSRC_VCORE_QOS0          (DVFSRC_BASE + 0x5C)
#define DVFSRC_VCORE_QOS1          (DVFSRC_BASE + 0x60)
#define DVFSRC_VCORE_QOS2          (DVFSRC_BASE + 0x64)
#define DVFSRC_VCORE_MD2SPM0       (DVFSRC_BASE + 0x68)
#define DVFSRC_VCORE_MD2SPM1       (DVFSRC_BASE + 0x6C)
#define DVFSRC_VCORE_MD2SPM2       (DVFSRC_BASE + 0x70)
#define DVFSRC_VCORE_MD2SPM0_T     (DVFSRC_BASE + 0x74)
#define DVFSRC_VCORE_MD2SPM1_T     (DVFSRC_BASE + 0x78)
#define DVFSRC_VCORE_MD2SPM2_T     (DVFSRC_BASE + 0x7C)
#define DVFSRC_MD_REQUEST          (DVFSRC_BASE + 0x80)
#define DVFSRC_MD_SW_CONTROL       (DVFSRC_BASE + 0x84)
#define DVFSRC_MD_VMODEM_REMAP     (DVFSRC_BASE + 0x88)
#define DVFSRC_MD_VMD_REMAP        (DVFSRC_BASE + 0x8C)
#define DVFSRC_MD_VSRAM_REMAP      (DVFSRC_BASE + 0x90)
#define DVFSRC_HALT_SW_CONTROL     (DVFSRC_BASE + 0x94)
#define DVFSRC_INT                 (DVFSRC_BASE + 0x98)
#define DVFSRC_INT_EN              (DVFSRC_BASE + 0x9C)
#define DVFSRC_INT_CLR             (DVFSRC_BASE + 0xA0)
#define DVFSRC_BW_MON_WINDOW       (DVFSRC_BASE + 0xA4)
#define DVFSRC_BW_MON_THRES_1      (DVFSRC_BASE + 0xA8)
#define DVFSRC_BW_MON_THRES_2      (DVFSRC_BASE + 0xAC)
#define DVFSRC_MD_TURBO            (DVFSRC_BASE + 0xB0)
#define DVFSRC_DEBOUNCE_FOUR       (DVFSRC_BASE + 0xD0)
#define DVFSRC_DEBOUNCE_RISE_FALL  (DVFSRC_BASE + 0xD4)
#define DVFSRC_TIMEOUT_NEXTREQ     (DVFSRC_BASE + 0xD8)
#define DVFSRC_LEVEL               (DVFSRC_BASE + 0xDC)
#define DVFSRC_LEVEL_LABEL_0_1     (DVFSRC_BASE + 0xE0)
#define DVFSRC_LEVEL_LABEL_2_3     (DVFSRC_BASE + 0xE4)
#define DVFSRC_LEVEL_LABEL_4_5     (DVFSRC_BASE + 0xE8)
#define DVFSRC_LEVEL_LABEL_6_7     (DVFSRC_BASE + 0xEC)
#define DVFSRC_LEVEL_LABEL_8_9     (DVFSRC_BASE + 0xF0)
#define DVFSRC_LEVEL_LABEL_10_11   (DVFSRC_BASE + 0xF4)
#define DVFSRC_LEVEL_LABEL_12_13   (DVFSRC_BASE + 0xF8)
#define DVFSRC_LEVEL_LABEL_14_15   (DVFSRC_BASE + 0xFC)
#define DVFSRC_MM_BW_0             (DVFSRC_BASE + 0x100)
#define DVFSRC_MM_BW_1             (DVFSRC_BASE + 0x104)
#define DVFSRC_MM_BW_2             (DVFSRC_BASE + 0x108)
#define DVFSRC_MM_BW_3             (DVFSRC_BASE + 0x10C)
#define DVFSRC_MM_BW_4             (DVFSRC_BASE + 0x110)
#define DVFSRC_MM_BW_5             (DVFSRC_BASE + 0x114)
#define DVFSRC_MM_BW_6             (DVFSRC_BASE + 0x118)
#define DVFSRC_MM_BW_7             (DVFSRC_BASE + 0x11C)
#define DVFSRC_MM_BW_8             (DVFSRC_BASE + 0x120)
#define DVFSRC_MM_BW_9             (DVFSRC_BASE + 0x124)
#define DVFSRC_MM_BW_10            (DVFSRC_BASE + 0x128)
#define DVFSRC_MM_BW_11            (DVFSRC_BASE + 0x12C)
#define DVFSRC_MM_BW_12            (DVFSRC_BASE + 0x130)
#define DVFSRC_MM_BW_13            (DVFSRC_BASE + 0x134)
#define DVFSRC_MM_BW_14            (DVFSRC_BASE + 0x138)
#define DVFSRC_MM_BW_15            (DVFSRC_BASE + 0x13C)
#define DVFSRC_MD_BW_0             (DVFSRC_BASE + 0x140)
#define DVFSRC_MD_BW_1             (DVFSRC_BASE + 0x144)
#define DVFSRC_MD_BW_2             (DVFSRC_BASE + 0x148)
#define DVFSRC_MD_BW_3             (DVFSRC_BASE + 0x14C)
#define DVFSRC_MD_BW_4             (DVFSRC_BASE + 0x150)
#define DVFSRC_MD_BW_5             (DVFSRC_BASE + 0x154)
#define DVFSRC_MD_BW_6             (DVFSRC_BASE + 0x158)
#define DVFSRC_MD_BW_7             (DVFSRC_BASE + 0x15C)
#define DVFSRC_SW_BW_0             (DVFSRC_BASE + 0x160)
#define DVFSRC_SW_BW_1             (DVFSRC_BASE + 0x164)
#define DVFSRC_SW_BW_2             (DVFSRC_BASE + 0x168)
#define DVFSRC_SW_BW_3             (DVFSRC_BASE + 0x16C)
#define DVFSRC_SW_BW_4             (DVFSRC_BASE + 0x170)
#define DVFSRC_QOS_EN              (DVFSRC_BASE + 0x180)
#define DVFSRC_ISP_HRT             (DVFSRC_BASE + 0x190)
#define DVFSRC_FORCE               (DVFSRC_BASE + 0x300)
#define DVFSRC_SEC_SW_REQ          (DVFSRC_BASE + 0x304)
#define DVFSRC_LAST                (DVFSRC_BASE + 0x308)
#define DVFSRC_LAST_L              (DVFSRC_BASE + 0x30C)
#define DVFSRC_MD_SCENARIO         (DVFSRC_BASE + 0x310)
#define DVFSRC_RSRV_0              (DVFSRC_BASE + 0x600)
#define DVFSRC_RSRV_1              (DVFSRC_BASE + 0x604)
#define DVFSRC_RSRV_2              (DVFSRC_BASE + 0x608)
#define DVFSRC_RSRV_3              (DVFSRC_BASE + 0x60C)
#define DVFSRC_RSRV_4              (DVFSRC_BASE + 0x610)
#define DVFSRC_RSRV_5              (DVFSRC_BASE + 0x614)

#define VCORE_VB_EFUSE             (0x11CE05B8)

#endif /* MT_SPM_VCOREFS_H */
