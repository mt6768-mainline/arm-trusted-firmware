/*
 * Copyright (c) 2026, Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_MPU_H
#define EMI_MPU_H

#include <arch.h>

u_register_t mt6768_emi_mpu_sip_handler(u_register_t op, u_register_t arg2,
				      u_register_t arg3, u_register_t arg4);

#endif /* EMI_MPU_H */
