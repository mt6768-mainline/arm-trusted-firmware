#
# Copyright (c) 2026, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT     := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}

PLAT_INCLUDES := -I${MTK_PLAT_SOC}/include/                       \
                 -I${MTK_PLAT}/common/                            \
                 -I${MTK_PLAT}/common/include                     \
                 -I${MTK_PLAT}/drivers/gic600/                    \
                 -I${MTK_PLAT}/drivers/pmic_wrap/                 \
                 -I${MTK_PLAT}/drivers/timer/                     \
                 -I${MTK_PLAT}/drivers/uart/                      \
                 -I${MTK_PLAT}/include/                           \
                 -I${MTK_PLAT_SOC}/drivers/devapc/                \
                 -I${MTK_PLAT_SOC}/drivers/emi_mpu/               \
                 -I${MTK_PLAT_SOC}/drivers/pmic/                  \
                 -I${MTK_PLAT_SOC}/drivers/rtc/                   \
                 -I${MTK_PLAT_SOC}/drivers/spm/                   \
                 -I${MTK_PLAT_SOC}/drivers/spmc/

GICV3_SUPPORT_GIC600 := 1
HW_ASSISTED_COHERENCY := 1
USE_COHERENT_MEM := 0
PROGRAMMABLE_RESET_ADDRESS := 1
COLD_BOOT_SINGLE_CPU := 1

MACH_MT6768 := 1
$(eval $(call add_define,MACH_MT6768))

# Errata for Cortex-A55 and Cortex-A75
ERRATA_A55_1530923 := 1
ERRATA_A55_1221012 := 1
ERRATA_A75_764081 := 1
ERRATA_A75_790748 := 1

include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES := ${GICV3_SOURCES}                              \
                          ${XLAT_TABLES_LIB_SRCS}                       \
                          plat/common/aarch64/crash_console_helpers.S   \
                          plat/common/plat_psci_common.c

BL31_SOURCES += common/desc_image_load.c                              \
                drivers/delay_timer/delay_timer.c                     \
                drivers/delay_timer/generic_delay_timer.c             \
                drivers/ti/uart/aarch64/16550_console.S               \
                lib/bl_aux_params/bl_aux_params.c                     \
                lib/cpus/aarch64/cortex_a55.S                         \
                lib/cpus/aarch64/cortex_a75.S                         \
                plat/common/plat_gicv3.c                              \
                ${MTK_PLAT}/common/mtk_plat_common.c                  \
                ${MTK_PLAT}/common/mtk_sip_svc.c                      \
                ${MTK_PLAT}/common/params_setup.c                     \
                ${MTK_PLAT}/drivers/gic600/mt_gic_v3.c                \
                ${MTK_PLAT}/drivers/pmic_wrap/pmic_wrap_init.c        \
                ${MTK_PLAT}/drivers/rtc/rtc_common.c                  \
                ${MTK_PLAT}/drivers/timer/mt_timer.c                  \
                ${MTK_PLAT}/drivers/uart/uart.c                       \
                ${MTK_PLAT_SOC}/aarch64/platform_common.c             \
                ${MTK_PLAT_SOC}/aarch64/plat_helpers.S                \
                ${MTK_PLAT_SOC}/bl31_plat_setup.c                     \
                ${MTK_PLAT_SOC}/drivers/devapc/devapc.c               \
                ${MTK_PLAT_SOC}/drivers/emi_mpu/emi_mpu.c             \
                ${MTK_PLAT_SOC}/drivers/pmic/pmic.c                   \
                ${MTK_PLAT_SOC}/drivers/rtc/rtc.c                     \
                ${MTK_PLAT_SOC}/drivers/spm/mt_spm_vcorefs.c          \
                ${MTK_PLAT_SOC}/drivers/spm/spm.c                     \
                ${MTK_PLAT_SOC}/drivers/spm/spm_pmic_wrap.c           \
                ${MTK_PLAT_SOC}/drivers/spmc/mtspmc.c                 \
                ${MTK_PLAT_SOC}/plat_pm.c                             \
                ${MTK_PLAT_SOC}/plat_mt_gic.c                         \
                ${MTK_PLAT_SOC}/plat_sip_calls.c                      \
                ${MTK_PLAT_SOC}/plat_topology.c
