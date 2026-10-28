/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SIP_CALLS_H
#define PLAT_SIP_CALLS_H

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS		(4)

/* Legacy / vendor SMC IDs */
#define MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH32		0x82000226
#define MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH64		0xC2000226

#define MTK_SIP_KERNEL_SPM_ARGS_AARCH32			0x8200022A
#define MTK_SIP_KERNEL_SPM_ARGS_AARCH64			0xC200022A

#define MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_ALT_AARCH32	0x82000216
#define MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_ALT_AARCH64	0xC2000216

#define MTK_SIP_KERNEL_SPM_ARGS_ALT_AARCH32		0x82000217
#define MTK_SIP_KERNEL_SPM_ARGS_ALT_AARCH64		0xC2000217

#endif /* PLAT_SIP_CALLS_H */
