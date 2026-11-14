/*
 * Copyright (c) 2026, Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <mtk_sip_svc.h>
#include <platform_def.h>

#include "emi_mpu.h"

#define EMIMPU_SET		U(0)
#define EMIMPU_READ		U(2)
#define EMIMPU_READ_SA		U(0)
#define EMIMPU_READ_EA		U(1)
#define EMIMPU_READ_APC		U(2)

#define EMI_MPU_REGIONS		U(32)
#define EMI_MPU_GROUPS		U(2)
#define EMI_MPU_ALIGN_BITS	U(16)
#define EMI_MPU_DRAM_BASE	ULL(0x40000000)
#define EMI_MPU_POOL_LIMIT	ULL(0xc0000000)
#define EMI_MPU_POOL_ALIGN	ULL(0x1000000)
#define EMI_MPU_ADDR_MASK	U(0x00ffffff)
#define EMI_MPU_LOCK		BIT_32(31)
#define EMI_MPU_PERM_MASK	U(7)
#define EMI_MPU_NO_PROTECTION	U(0)
#define EMI_MPU_SEC_R_NSEC_RW	U(6)

#define EMI_MPU_SA(r)		(EMI_MPU_BASE + U(0x100) + (r) * U(4))
#define EMI_MPU_EA(r)		(EMI_MPU_BASE + U(0x200) + (r) * U(4))
#define EMI_MPU_APC(r, g)		(EMI_MPU_BASE + U(0x300) + \
				 (r) * U(4) + (g) * U(0x100))

/* D0 (AP) and D2 (CONN) may read/write; all other domains are forbidden. */
#define EMI_MPU_APC0		U(0x00b6da28)
#define EMI_MPU_APC1		U(0x00b6db6d)

struct emi_mpu_pool {
	uint32_t region;
	uint32_t size;
	uint32_t start;
	uint32_t end;
	bool prepared;
	bool locked;
};

/* Match the dynamically allocated connectivity reservations in the kernel. */
static struct emi_mpu_pool emi_mpu_pools[] = {
	{ .region = 26U, .size = U(0x400000) },
	{ .region = 29U, .size = U(0x410000) },
};

static spinlock_t emi_mpu_lock;

static bool ranges_overlap(uint64_t start, uint64_t end,
			   uint64_t other_start, uint64_t other_end)
{
	return (start < other_end) && (other_start < end);
}

static bool emi_mpu_range_protected(uint32_t region, uint64_t start,
				  uint64_t end)
{
	uint32_t r;

	if (ranges_overlap(start, end, TZRAM_BASE, TZRAM_BASE + TZRAM_SIZE)) {
		return true;
	}

	/* Preserve inherited regions that deny AP non-secure read/write. */
	for (r = 0U; r < EMI_MPU_REGIONS; r++) {
		uint32_t apc0 = mmio_read_32(EMI_MPU_APC(r, 0));
		uint32_t ap_perm = apc0 & EMI_MPU_PERM_MASK;
		bool ap_restricted = (ap_perm != EMI_MPU_NO_PROTECTION) &&
				     (ap_perm != EMI_MPU_SEC_R_NSEC_RW);
		uint64_t sa, ea;

		/* Do not repurpose a secure region, even at another address. */
		if ((r == region) && ap_restricted) {
			return true;
		}
		if (!ap_restricted) {
			/* The two connectivity pools must also remain disjoint. */
			if (((r != 26U) && (r != 29U)) || (r == region) ||
			    ((apc0 | mmio_read_32(EMI_MPU_APC(r, 1))) == 0U)) {
				continue;
			}
		}
		sa = EMI_MPU_DRAM_BASE +
		     ((uint64_t)mmio_read_32(EMI_MPU_SA(r)) <<
		      EMI_MPU_ALIGN_BITS);
		ea = EMI_MPU_DRAM_BASE +
		     (((uint64_t)mmio_read_32(EMI_MPU_EA(r)) + 1ULL) <<
		      EMI_MPU_ALIGN_BITS);
		if ((sa < ea) && ranges_overlap(start, end, sa, ea)) {
			return true;
		}
	}
	return false;
}

static u_register_t emi_mpu_read(uint32_t type, uint32_t region,
			       uint32_t group)
{
	if ((region >= EMI_MPU_REGIONS) || (group >= EMI_MPU_GROUPS)) {
		return MTK_SIP_E_INVALID_RANGE;
	}

	switch (type) {
	case EMIMPU_READ_SA:
		if (group != 0U) {
			return MTK_SIP_E_INVALID_PARAM;
		}
		return mmio_read_32(EMI_MPU_SA(region));
	case EMIMPU_READ_EA:
		if (group != 0U) {
			return MTK_SIP_E_INVALID_PARAM;
		}
		return mmio_read_32(EMI_MPU_EA(region));
	case EMIMPU_READ_APC:
		return mmio_read_32(EMI_MPU_APC(region, group));
	default:
		return MTK_SIP_E_INVALID_PARAM;
	}
}

static u_register_t emi_mpu_set(uint32_t packed_start, uint32_t packed_end,
			      uint32_t apc)
{
	uint32_t region = packed_start >> 24;
	uint32_t group = packed_end >> 24;
	uint32_t start = packed_start & EMI_MPU_ADDR_MASK;
	uint32_t end = packed_end & EMI_MPU_ADDR_MASK;
	uint32_t expected_apc;
	uint64_t pa_start = (uint64_t)start << EMI_MPU_ALIGN_BITS;
	uint64_t pa_end = ((uint64_t)end + 1ULL) << EMI_MPU_ALIGN_BITS;
	struct emi_mpu_pool *pool = NULL;
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(emi_mpu_pools); i++) {
		if (emi_mpu_pools[i].region == region) {
			pool = &emi_mpu_pools[i];
			break;
		}
	}
	if (pool == NULL) {
		return MTK_SIP_E_PERMISSION_DENY;
	}
	if ((group >= EMI_MPU_GROUPS) || (pa_start < EMI_MPU_DRAM_BASE) ||
	    (pa_end > EMI_MPU_POOL_LIMIT) || (pa_end <= pa_start) ||
	    ((pa_start & (EMI_MPU_POOL_ALIGN - 1ULL)) != 0ULL) ||
	    ((pa_end - pa_start) != pool->size)) {
		return MTK_SIP_E_INVALID_RANGE;
	}
	expected_apc = (group == 0U) ? EMI_MPU_APC0 : EMI_MPU_APC1;
	if (((apc & ~EMI_MPU_LOCK) != expected_apc) ||
	    (((apc & EMI_MPU_LOCK) != 0U) !=
	     ((region == 29U) && (group == 0U)))) {
		return MTK_SIP_E_PERMISSION_DENY;
	}
	if (pool->locked ||
	    ((mmio_read_32(EMI_MPU_APC(region, 0)) & EMI_MPU_LOCK) != 0U) ||
	    emi_mpu_range_protected(region, pa_start, pa_end)) {
		return MTK_SIP_E_PERMISSION_DENY;
	}
	start -= (uint32_t)(EMI_MPU_DRAM_BASE >> EMI_MPU_ALIGN_BITS);
	end -= (uint32_t)(EMI_MPU_DRAM_BASE >> EMI_MPU_ALIGN_BITS);

	/* Group 1 must be installed and verified before group 0 can lock it. */
	if ((group == 0U) &&
	    (!pool->prepared || (pool->start != start) || (pool->end != end) ||
	     (mmio_read_32(EMI_MPU_SA(region)) != start) ||
	     (mmio_read_32(EMI_MPU_EA(region)) != end) ||
	     (mmio_read_32(EMI_MPU_APC(region, 1)) != EMI_MPU_APC1))) {
		return MTK_SIP_E_PERMISSION_DENY;
	}
	pool->prepared = false;

	/* MT6768 uses a firmware software lock; preserve any inherited HW lock. */
	mmio_write_32(EMI_MPU_SA(region), start);
	mmio_write_32(EMI_MPU_EA(region), end);
	mmio_write_32(EMI_MPU_APC(region, group), expected_apc);
	if ((mmio_read_32(EMI_MPU_SA(region)) != start) ||
	    (mmio_read_32(EMI_MPU_EA(region)) != end) ||
	    (mmio_read_32(EMI_MPU_APC(region, group)) != expected_apc)) {
		ERROR("EMI MPU: region %u group %u readback failed\n",
		      region, group);
		return MTK_SIP_E_PERMISSION_DENY;
	}
	if (group == 1U) {
		pool->start = start;
		pool->end = end;
		pool->prepared = true;
	} else if ((apc & EMI_MPU_LOCK) != 0U) {
		pool->locked = true;
	}
	return MTK_SIP_E_SUCCESS;
}

u_register_t mt6768_emi_mpu_sip_handler(u_register_t op, u_register_t arg2,
				      u_register_t arg3, u_register_t arg4)
{
	u_register_t ret;

	/* The ABI uses packed 32-bit fields, including for SMC64 callers. */
	if ((op > UINT32_MAX) || (arg2 > UINT32_MAX) ||
	    (arg3 > UINT32_MAX) || (arg4 > UINT32_MAX)) {
		return MTK_SIP_E_INVALID_PARAM;
	}
	spin_lock(&emi_mpu_lock);
	switch (op) {
	case EMIMPU_SET:
		ret = emi_mpu_set((uint32_t)arg2, (uint32_t)arg3,
				  (uint32_t)arg4);
		break;
	case EMIMPU_READ:
		ret = emi_mpu_read((uint32_t)arg2, (uint32_t)arg3,
				   (uint32_t)arg4);
		break;
	default:
		ret = MTK_SIP_E_NOT_SUPPORTED;
		break;
	}
	spin_unlock(&emi_mpu_lock);
	return ret;
}
