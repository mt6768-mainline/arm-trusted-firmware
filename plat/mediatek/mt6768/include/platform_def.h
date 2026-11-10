/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <drivers/arm/gic_common.h>

#define PLAT_PRIMARY_CPU   0x0

#define IO_PHYS            0x10000000
#define INFRACFG_AO_BASE   (IO_PHYS + 0x1000)
#define PERI_BASE          (IO_PHYS + 0x3000)
#define GPIO_BASE          (IO_PHYS + 0x5000)
#define SPM_BASE           (IO_PHYS + 0x6000)
#define RGU_BASE           (IO_PHYS + 0x7000)
#define DVFSRC_BASE        (IO_PHYS + 0x12000)
#define PMIC_WRAP_BASE     (IO_PHYS + 0xd000)
#define SLEEP_REG_MD_BASE  (IO_PHYS + 0xf000)
#define TOPCKGEN_BASE      (IO_PHYS + 0x0)
#define APMIXEDSYS         (IO_PHYS + 0xc000)

#define MCUCFG_BASE        0x0c530000
#define MT_GIC_BASE        0x0c000000

/* Aggregate of all devices */
#define MTK_DEV_RNG0_BASE  IO_PHYS
#define MTK_DEV_RNG0_SIZE  0x400000
#define MTK_DEV_RNG1_BASE  (IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE  0x4000000
#define MTK_DEV_RNG2_BASE  MT_GIC_BASE
#define MTK_DEV_RNG2_SIZE  0x600000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE         (IO_PHYS + 0x01002000)
#define UART1_BASE         (IO_PHYS + 0x01003000)

#define UART_BAUDRATE      921600
#define UART_CLOCK         26000000

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS    13000000
#define SYS_COUNTER_FREQ_IN_MHZ      13

/*******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
#define BASE_GICD_BASE        MT_GIC_BASE
#define BASE_GICC_BASE        (MT_GIC_BASE + 0x400000)
#define MT_GIC_RDIST_BASE     (MT_GIC_BASE + 0x40000)
#define BASE_GICR_BASE        MT_GIC_RDIST_BASE
#define BASE_GICH_BASE        (MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE        (MT_GIC_BASE + 0x6000)
#define GIC_PRIVATE_SIGNALS   32

#define PLAT_ARM_GICD_BASE    BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE    BASE_GICC_BASE

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(MT_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G0_IRQ_PROPS(grp)

#define PLAT_MTK_G1S_IRQ_PROPS(grp)

/* SGI numbers */
#define MT_IRQ_SEC_SGI_0      8
#define MT_IRQ_SEC_SGI_1      9
#define MT_IRQ_SEC_SGI_2      10
#define MT_IRQ_SEC_SGI_3      11
#define MT_IRQ_SEC_SGI_4      12
#define MT_IRQ_SEC_SGI_5      13
#define MT_IRQ_SEC_SGI_6      14
#define MT_IRQ_SEC_SGI_7      15

#define INT_POL_CTL0                           (MCUCFG_BASE + 0xa650)
#define SEC_POL_CTL_EN0                        (MCUCFG_BASE + 0xa600)
#define INT_MSK_CTL0                           (MCUCFG_BASE + 0xa6a0)
#define INT_MSK_ALL                            (MCUCFG_BASE + 0xa6f4)
#define INT_CFG_INDIRECT_ACCESS                (MCUCFG_BASE + 0xa6f8)
#define INT_CFG_DIRECT_ACCESS_EN               (MCUCFG_BASE + 0xa6fc)
#define INT_CFG_DIRECT_ACCESS_ENABLE           BIT(0)
#define GIC_ACAO_CTL0                          (MCUCFG_BASE + 0xaa80)
#define GIC_ACAO_CTL0_IRQ_LATENCY_REDUCTION    BIT(6)

#define NR_INT_POL_CTL                         (20)

/*******************************************************************************
 * WDT Registers
 ******************************************************************************/
#define MTK_WDT_BASE          (IO_PHYS + 0x00007000)
#define MTK_WDT_SIZE          0x1000
#define MTK_WDT_MODE          (MTK_WDT_BASE + 0x0000)
#define MTK_WDT_LENGTH        (MTK_WDT_BASE + 0x0004)
#define MTK_WDT_RESTART       (MTK_WDT_BASE + 0x0008)
#define MTK_WDT_STATUS        (MTK_WDT_BASE + 0x000C)
#define MTK_WDT_INTERVAL      (MTK_WDT_BASE + 0x0010)
#define MTK_WDT_SWRST         (MTK_WDT_BASE + 0x0014)
#define MTK_WDT_SWSYSRST      (MTK_WDT_BASE + 0x0018)
#define MTK_WDT_NONRST_REG    (MTK_WDT_BASE + 0x0020)
#define MTK_WDT_NONRST_REG2   (MTK_WDT_BASE + 0x0024)
#define MTK_WDT_LATCH_CTL     (MTK_WDT_BASE + 0x0044)
#define MTK_WDT_DEBUG_CTL2    (MTK_WDT_BASE + 0x00A0)

#define MTK_WDT_MODE_KEY      0x22000000
#define MTK_WDT_MODE_EXTEN    0x0004
#define MTK_WDT_SWRST_KEY     0x1209
#define MTK_WDT_RESTART_KEY   0x1971

#define MTK_WDT_DEBUG_CTL2_KEY      0x55000000
#define MTK_WDT_DEBUG_CTL_DVFSRC_EN (1 << 9)
#define MTK_WDT_LATCH_CTL_KEY       0x95000000
#define MTK_WDT_LATCH_CTL_DVFSRC    (1 << 13)

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE             0x800

#define FIRMWARE_WELCOME_STR            "Booting Trusted Firmware\n"
#define PLAT_MAX_PWR_LVL                U(2)
#define PLAT_MAX_RET_STATE              U(1)
#define PLAT_MAX_OFF_STATE              U(2)

#define PLATFORM_CACHE_LINE_SIZE        64
#define PLATFORM_SYSTEM_COUNT           U(1)
#define PLATFORM_MCUSYS_COUNT           U(1)
#define PLATFORM_CLUSTER_COUNT          U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT    U(8)
#define PLATFORM_CLUSTER1_CORE_COUNT    U(0)
#define PLATFORM_CORE_COUNT             (PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER   U(8)
#define PLATFORM_NUM_AFFS               (PLATFORM_SYSTEM_COUNT + \
					 PLATFORM_MCUSYS_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define SOC_CHIP_ID                     U(0x6768)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE          0x4CE00000
#define TZRAM_SIZE          0x0004F000

#define BL31_BASE           (TZRAM_BASE + 0x1000)
#define BL31_LIMIT          (TZRAM_BASE + TZRAM_SIZE)
#define BL33_BASE           0x4c400000

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE    (1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE   (1ULL << 32)
#define MAX_XLAT_TABLES             16
#define MAX_MMAP_REGIONS            16

#define CACHE_WRITEBACK_SHIFT       6
#define CACHE_WRITEBACK_GRANULE     (1 << CACHE_WRITEBACK_SHIFT)

#endif /* PLATFORM_DEF_H */
