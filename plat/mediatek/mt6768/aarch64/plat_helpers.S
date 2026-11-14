/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <mcucfg.h>
#include <platform_def.h>

	.globl plat_is_my_cpu_primary
	.globl plat_my_core_pos
	.globl plat_mediatek_calc_core_pos
	.globl plat_reset_handler

func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_mediatek_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_mediatek_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_mediatek_calc_core_pos(u_register_t mpidr);
	 *
	 * With this function: CorePos = CoreID (AFF1)
	 * we do it with x0 = (x0 >> 8) & 0xff
	 * -----------------------------------------------------
	 */
func plat_mediatek_calc_core_pos
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_mediatek_calc_core_pos

func plat_reset_handler
	mov	x10, x30
	bl	plat_my_core_pos
	mov	x30, x10
	mov	w1, #0x1
	lsl	w1, w1, w0
	ldr	x0, =CPC_MCUSYS_CPU_ON_SW_HINT_SET
	str	w1, [x0]
	dsb	sy

	/* -----------------------------------------------------
	 * Enable DSU PMU EL1 write-access for MTK platform.
	 * Bits [12:10]: CLUSTERPMUEN, SMEN, TSIDEN
	 * -----------------------------------------------------
	 */
	mov	x0, #0x1c00
	mrs	x1, actlr_el3
	orr	x1, x1, x0
	msr	actlr_el3, x1
	mrs	x1, actlr_el2
	orr	x1, x1, x0
	msr	actlr_el2, x1

	ret
endfunc plat_reset_handler
