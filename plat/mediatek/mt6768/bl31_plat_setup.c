/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <devapc.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <mt_gic_v3.h>
#include <mt_timer.h>
#include <mtspmc.h>
#include <plat_private.h>
#include <platform_def.h>
#include <spm.h>

struct mtk_bl_param_t {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t tee_info_addr;
};

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;
	assert(next_image_info->h.type == PARAM_EP);

	if (next_image_info->pc != 0U) {
		return next_image_info;
	}

	return NULL;
}

/*******************************************************************************
 * Perform any BL31 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL before they are lost.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	console_16550_register(UART0_BASE, 0, UART_BAUDRATE, &console);

	NOTICE("MT6768 bl31_setup\n");

	if (arg0 != 0U) {
		param_header_t *head = (param_header_t *)arg0;

		if (head->type == PARAM_BL_PARAMS || head->type == PARAM_BL31) {
			bl31_params_parse_helper(arg0, &bl32_ep_info, &bl33_ep_info);
		} else {
			/* Preloader parameter handoff */
			struct mtk_bl_param_t *pmtk =
				(void *)((uintptr_t)arg0 & 0xffffffff);

			SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_1, 0);
			SET_SECURITY_STATE(bl33_ep_info.h.attr, NON_SECURE);
			bl33_ep_info.pc = BL33_BASE;
			bl33_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						    DISABLE_ALL_EXCEPTIONS);
			bl33_ep_info.args.arg0 = pmtk->bootarg_loc;
			bl33_ep_info.args.arg1 = pmtk->bootarg_size;
		}
	} else {
		SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_1, 0);
		SET_SECURITY_STATE(bl33_ep_info.h.attr, NON_SECURE);
		bl33_ep_info.pc = BL33_BASE;
		bl33_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					    DISABLE_ALL_EXCEPTIONS);
	}
}

/*******************************************************************************
 * Perform any BL31 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* Initialize GIC-600 driver, CPU and distributor interfaces */
	mt_gic_driver_init();
	mt_gic_init();

	/* Enable direct access and grant non-secure access to SYSIRQ polarity registers */
	clear_sec_pol_ctl_en();

	/* Allow cluster to stay on */
	write_clusterpwrdn_el1(1);

	mt_systimer_init();
	spm_boot_init();
	devapc_init();
}

/*******************************************************************************
 * Perform the early platform specific architectural setup (MMU).
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	plat_configure_mmu_el3(BL31_START,
			       BL31_END - BL31_START,
			       BL_CODE_BASE,
			       BL_CODE_END);
}
