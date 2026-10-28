/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <mt_spm_vcorefs.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>
#include <spm.h>
#include <spm_pmic_wrap.h>

#define SPM_DVFS_TIMEOUT       1000     /* 1ms */

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS),

	/* SPM_AP_STANDBY_CON */
	.wfi_op = 0,
	.mcusys_idle_mask = 0,
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 0,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0,
	.sspm_mask_b = 0,
	.srcclkeni_mask_b = 0,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 0,
	.conn_mask_b = 0,
	.conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.spm_apsrc_req = 0,
	.spm_f26m_req = 0,
	.spm_infra_req = 0,
	.spm_vrf18_req = 0,
	.spm_ddren_req = 0,
	.spm_rsv_src_req = 0,
	.spm_ddren_2_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
	.csyspwreq_mask = 0,
	.ccif0_md_event_mask_b = 0,
	.ccif0_ap_event_mask_b = 0,
	.ccif1_md_event_mask_b = 0,
	.ccif1_ap_event_mask_b = 0,
	.md_srcclkena_0_infra_mask_b = 0,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0,
	.md_apsrc_req_1_infra_mask_b = 0,
	.conn_apsrcreq_infra_mask_b = 0,
	.md_ddr_en_0_mask_b = 0,
	.md_ddr_en_1_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,

	/* SPM_SRC2_MASK */
	.vdec_req_mask_b = 0,
	.conn_ddr_en_mask_b = 0,
	.mfg_req_mask_b = 0,
	.ufs_srcclkena_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0,
	.gce_vrf18_req_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,
};

struct spm_lp_scen {
	struct pwr_ctrl *pwrctrl;
};

static struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static uint32_t __spmfw_idx = SPMFW_LP4_2CH_3200;
static int is_freq_hopping;
static int dvfs_enable_done;

static int vcore_opp_0_uv = 800000;
static int vcore_opp_1_uv = 700000;
static int vcore_opp_2_uv = 700000;
static int vcore_opp_3_uv = 650000;
static int dram_freq[3] = {-1, -1, -1};
static const unsigned int opp_drams_step[] = {2, 1, 0};

static void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
{
	if (cmd < NR_IDX_ALL) {
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, cmd, val);
	} else {
		INFO("cmd out of range!\n");
	}
}

void spm_request_dvfs_opp(uint64_t id, uint64_t reg)
{
	int i = 0;

	switch (id) {
	case 0: /* ZQTX */
		mmio_write_32(DVFSRC_SEC_SW_REQ, reg);
		if (reg != 0x0) {
			while ((mmio_read_32(DVFSRC_LEVEL) >> 16) < 0x4) {
				if (i >= SPM_DVFS_TIMEOUT)
					break;
				udelay(1);
				i++;
			}
		}
		break;
	default:
		break;
	}
}

static void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue)
{
	mmio_write_32(SPM_DVS_LEVEL, (0x1 << 18) | (0x1 << 3));
	mmio_write_32(SPM_DFS_LEVEL, (0x1 << 2));
	mmio_write_32(SPM_SW_RSV_9, (1U << 31) | (1U << 14));
	mmio_write_32(DVFSRC_EVENT_MASK_CON, 0x1FFFF);
	mmio_write_32(DVFSRC_EVENT_SEL, 0x0);
}

static void spm_go_to_vcorefs(uint64_t spm_flags)
{
	struct pwr_ctrl *pwrctrl = __spm_vcorefs.pwrctrl;

	pwrctrl->pcm_flags = spm_flags;
	spm_set_power_control(pwrctrl);
	spm_set_wakeup_event(pwrctrl);
	spm_set_pcm_flags(pwrctrl);
	spm_send_cpu_wakeup_event();
}

static void spm_vcorefs_freq_hopping(uint64_t gps_on)
{
	int i = 0;

	if (gps_on) {
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x3) | 0x3);
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
		while ((mmio_read_32(SPM2SW_MAILBOX_0) & 0x1) != 0x1) {
			if (i >= SPM_DVFS_TIMEOUT)
				break;
			udelay(1);
			i++;
		}
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x1));
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
		mmio_write_32(SPM2SW_MAILBOX_0, (mmio_read_32(SPM2SW_MAILBOX_0) & ~0x1));
	} else {
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x3) | 0x1);
		spm_send_cpu_wakeup_event();
		while ((mmio_read_32(SPM2SW_MAILBOX_0) & 0x1) != 0x1) {
			if (i >= SPM_DVFS_TIMEOUT)
				break;
			udelay(1);
			i++;
		}
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x1));
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
		mmio_write_32(SPM2SW_MAILBOX_0, (mmio_read_32(SPM2SW_MAILBOX_0) & ~0x1));
	}
}

static const struct reg_config dvfsrc_init_configs_common[] = {
	{ DVFSRC_EMI_REQUEST,		0x00240009 },
	{ DVFSRC_EMI_REQUEST3,		0x09000000 },
	{ DVFSRC_EMI_MD2SPM0,		0x0000003F },
	{ DVFSRC_EMI_MD2SPM1,		0x00000000 },
	{ DVFSRC_EMI_MD2SPM2,		0x000080C0 },
	{ DVFSRC_EMI_MD2SPM0_T,		0x00000007 },
	{ DVFSRC_EMI_MD2SPM1_T,		0x00000038 },
	{ DVFSRC_EMI_MD2SPM2_T,		0x000080C0 },
	{ DVFSRC_MD_SW_CONTROL,		0x20000000 },
	{ DVFSRC_TIMEOUT_NEXTREQ,	0x00000014 },
	{ DVFSRC_INT_EN,		0x00000003 },
	{ DVFSRC_LEVEL_LABEL_0_1,	0x00010000 },
	{ DVFSRC_LEVEL_LABEL_2_3,	0x00020101 },
	{ DVFSRC_LEVEL_LABEL_4_5,	0x01020012 },
	{ DVFSRC_LEVEL_LABEL_6_7,	0x02120112 },
	{ DVFSRC_LEVEL_LABEL_8_9,	0x00230013 },
	{ DVFSRC_LEVEL_LABEL_10_11,	0x01230113 },
	{ DVFSRC_LEVEL_LABEL_12_13,	0x02230213 },
	{ DVFSRC_LEVEL_LABEL_14_15,	0x03230323 },
	{ DVFSRC_FORCE,			0x40000000 },
	{ DVFSRC_RSRV_1,		0x0000000C },
	{ DVFSRC_QOS_EN,		0x0001027B },
	{ DVFSRC_BASIC_CONTROL,		0x0000407B },
	{ DVFSRC_BASIC_CONTROL,		0x0000017B },
	{ DVFSRC_FORCE,			0x00000000 },
	{ (uint32_t)-1, 0 },
};

static const struct reg_config dvfsrc_init_configs_lp4[] = {
	{ DVFSRC_EMI_HRT,		0x003E362C },
	{ DVFSRC_EMI_QOS0,		0x00000033 },
	{ DVFSRC_EMI_QOS1,		0x0000004C },
	{ DVFSRC_VCORE_HRT,		0x00000036 },
	{ (uint32_t)-1, 0 },
};

static const struct reg_config dvfsrc_init_configs_lp3[] = {
	{ DVFSRC_EMI_HRT,		0x00000020 },
	{ DVFSRC_EMI_QOS0,		0x00000026 },
	{ DVFSRC_EMI_QOS1,		0x00000033 },
	{ DVFSRC_VCORE_HRT,		0x00000020 },
	{ (uint32_t)-1, 0 },
};

static int plat_rgu_cfg_dvfsrc(int enable)
{
	unsigned int dbg_ctl, latch;

	if (enable != 1)
		return -1;

	dbg_ctl = mmio_read_32(MTK_WDT_DEBUG_CTL2);
	latch = mmio_read_32(MTK_WDT_LATCH_CTL);

	dbg_ctl |= MTK_WDT_DEBUG_CTL_DVFSRC_EN | MTK_WDT_DEBUG_CTL2_KEY;
	latch |= MTK_WDT_LATCH_CTL_DVFSRC | MTK_WDT_LATCH_CTL_KEY;

	mmio_write_32(MTK_WDT_DEBUG_CTL2, dbg_ctl);
	mmio_write_32(MTK_WDT_LATCH_CTL, latch);

	return 0;
}

static void dvfsrc_init(void)
{
	const struct reg_config *config_diff;
	uint32_t i = 0;
	uint32_t j = 0;

	if (dvfs_enable_done)
		return;

	if (__spmfw_idx == SPMFW_LP3_1CH_1866)
		config_diff = dvfsrc_init_configs_lp3;
	else
		config_diff = dvfsrc_init_configs_lp4;

	while (config_diff[j].offset != (uint32_t)-1) {
		mmio_write_32(config_diff[j].offset, config_diff[j].val);
		j++;
	}

	while (dvfsrc_init_configs_common[i].offset != (uint32_t)-1) {
		mmio_write_32(dvfsrc_init_configs_common[i].offset,
			      dvfsrc_init_configs_common[i].val);
		i++;
	}

	plat_rgu_cfg_dvfsrc(1);

	dvfs_enable_done = 1;
}

static unsigned int spm_vcorefs_get_efuse_data(uint64_t idx)
{
	if (idx == 0)
		return mmio_read_32(VCORE_VB_EFUSE);

	return 0;
}

static int get_vb_volt(int vcore_opp)
{
	int ret = 0;

	if (vcore_opp == 2) {
		ret = (spm_vcorefs_get_efuse_data(0) >> 9) & 0x3;
		if (ret != 0)
			ret = ret - 1;
	}
	return ret * 25000;
}

static void spm_vcorefs_vcore_setting(uint64_t flag)
{
	vcore_opp_2_uv = 700000 - get_vb_volt(2);

	if (__spmfw_idx == SPMFW_LP3_1CH_1866) {
		vcore_opp_2_uv = 700000;
		vcore_opp_3_uv = 700000;
	}

	switch (flag) {
	case 1: /* HV */
		vcore_opp_0_uv = 843750;
		vcore_opp_1_uv = 737500;
		vcore_opp_2_uv = 737500;
		vcore_opp_3_uv = 687500;
		break;
	case 2: /* LV */
		vcore_opp_0_uv = 756250;
		vcore_opp_1_uv = 662500;
		vcore_opp_2_uv = 662500;
		vcore_opp_3_uv = 612500;
		break;
	case 3: /* AGING */
		vcore_opp_0_uv -= 12500;
		vcore_opp_1_uv -= 12500;
		vcore_opp_2_uv -= 12500;
		vcore_opp_3_uv -= 12500;
		break;
	default:
		break;
	}

	spm_vcorefs_pwarp_cmd(0, __vcore_uv_to_pmic(vcore_opp_0_uv));
	spm_vcorefs_pwarp_cmd(2, __vcore_uv_to_pmic(vcore_opp_1_uv));
	spm_vcorefs_pwarp_cmd(3, __vcore_uv_to_pmic(vcore_opp_2_uv));
	spm_vcorefs_pwarp_cmd(4, __vcore_uv_to_pmic(vcore_opp_3_uv));
}

static int spm_vcorefs_get_vcore_num(void)
{
	return 4;
}

static int spm_vcorefs_get_vcore_uv(int gear)
{
	switch (gear) {
	case 3:
		return vcore_opp_0_uv;
	case 2:
		return vcore_opp_1_uv;
	case 1:
		return vcore_opp_2_uv;
	case 0:
	default:
		return vcore_opp_3_uv;
	}
}

static int spm_vcorefs_get_dram_freq(unsigned int gear)
{
	int freq = -1;

	if (gear < ARRAY_SIZE(opp_drams_step)) {
		freq = dram_freq[opp_drams_step[gear]];
		if (freq != -1)
			return freq * 1000;
	}

	return freq;
}

static void spm_vcorefs_set_dram_freq(int opp, int freq)
{
	if (opp < ARRAY_SIZE(dram_freq))
		dram_freq[opp] = freq;
}

static int spm_vcorefs_get_freq_num(void)
{
	return 3;
}

static void spm_vcorefs_fb_action(uint64_t blank)
{
	if (blank) {
		mmio_write_32(DVFSRC_EMI_MD2SPM0_T, 0x0000003F);
		mmio_write_32(DVFSRC_EMI_MD2SPM1_T, 0x00000000);
	} else {
		mmio_write_32(DVFSRC_EMI_MD2SPM0_T, 0x00000007);
		mmio_write_32(DVFSRC_EMI_MD2SPM1_T, 0x00000038);
	}
}

uint64_t spm_vcorefs_v2_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t *x4)
{
	uint64_t ret = 0;
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_INIT:
		spm_vcorefs_vcore_setting(x3 & 0xF);
		spm_dvfsfw_init(0, 0);
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if (x2 & 0x1)
			spm_flags |= SPM_FLAG_DIS_VCORE_DVS;
		if (x2 & 0x2)
			spm_flags |= SPM_FLAG_DIS_VCORE_DFS;

		spm_go_to_vcorefs(spm_flags);
		mmio_write_32(DVFSRC_SW_BW_4, 0x000000FF);
		dvfsrc_init();
		*x4 = __spmfw_idx;
		break;
	case VCOREFS_SMC_CMD_KICK:
		mmio_write_32(DVFSRC_SW_BW_4, 0x00000000);
		break;
	case VCOREFS_SMC_CMD_OPP_TYPE:
		*x4 = 0;
		break;
	case VCOREFS_SMC_CMD_FW_TYPE:
		*x4 = __spmfw_idx;
		break;
	case VCOREFS_SMC_CMD_GET_UV:
		*x4 = spm_vcorefs_get_vcore_uv(x2);
		break;
	case VCOREFS_SMC_CMD_GET_FREQ:
		*x4 = spm_vcorefs_get_dram_freq(x2);
		break;
	case VCOREFS_SMC_CMD_GET_NUM_V:
		*x4 = spm_vcorefs_get_vcore_num();
		break;
	case VCOREFS_SMC_CMD_GET_NUM_F:
		*x4 = spm_vcorefs_get_freq_num();
		break;
	case VCOREFS_SMC_CMD_FB_ACTION:
		spm_vcorefs_fb_action(x2);
		break;
	case VCOREFS_SMC_CMD_SET_FREQ:
		spm_vcorefs_set_dram_freq(x2, x3);
		break;
	case VCOREFS_SMC_CMD_GET_EFUSE:
		*x4 = spm_vcorefs_get_efuse_data(x2);
		break;
	case VCOREFS_SMC_CMD_DVFS_HOPPING:
		if (__spmfw_idx != SPMFW_LP3_1CH_1866) {
			spm_vcorefs_freq_hopping(x2);
			is_freq_hopping = !!x2;
		}
		break;
	case VCOREFS_SMC_CMD_DVFS_HOPPING_STATE:
		*x4 = is_freq_hopping;
		break;
	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}
	return ret;
}

void spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_0:
		spm_dvfsfw_init(x2, x3);
		break;
	case VCOREFS_SMC_CMD_1:
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if (x2 & SPM_FLAG_DIS_VCORE_DVS)
			spm_flags |= SPM_FLAG_DIS_VCORE_DVS;
		if (x2 & SPM_FLAG_DIS_VCORE_DFS)
			spm_flags |= SPM_FLAG_DIS_VCORE_DFS;
		spm_go_to_vcorefs(spm_flags);
		break;
	case VCOREFS_SMC_CMD_2:
		spm_request_dvfs_opp(x2, x3);
		break;
	case VCOREFS_SMC_CMD_3:
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	case VCOREFS_SMC_CMD_4:
		spm_vcorefs_freq_hopping(x2);
		break;
	default:
		break;
	}
}

void spm_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t cmd = x1;

	switch (cmd) {
	case SPM_ARGS_SPMFW_IDX:
		__spmfw_idx = x2;
		break;
	default:
		break;
	}
}
