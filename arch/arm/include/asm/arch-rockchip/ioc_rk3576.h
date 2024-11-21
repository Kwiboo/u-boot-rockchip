/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd.
 */
#ifndef _ASM_ARCH_IOC_RK3576_H
#define _ASM_ARCH_IOC_RK3576_H

/* pmu0_ioc register structure define */
struct rk3576_pmu0_ioc_reg {
	unsigned int gpio0a_iomux_sel_l;  /* address offset: 0x0000 */
	unsigned int gpio0a_iomux_sel_h;  /* address offset: 0x0004 */
	unsigned int gpio0b_iomux_sel_l;  /* address offset: 0x0008 */
	unsigned int reserved000c;        /* address offset: 0x000c */
	unsigned int gpio0a_ds_l;         /* address offset: 0x0010 */
	unsigned int gpio0a_ds_h;         /* address offset: 0x0014 */
	unsigned int gpio0b_ds_l;         /* address offset: 0x0018 */
	unsigned int reserved001c;        /* address offset: 0x001c */
	unsigned int gpio0a_pull;         /* address offset: 0x0020 */
	unsigned int gpio0b_pull_l;       /* address offset: 0x0024 */
	unsigned int gpio0a_ie;           /* address offset: 0x0028 */
	unsigned int gpio0b_ie_l;         /* address offset: 0x002c */
	unsigned int gpio0a_smt;          /* address offset: 0x0030 */
	unsigned int gpio0b_smt_l;        /* address offset: 0x0034 */
	unsigned int gpio0a_pdis;         /* address offset: 0x0038 */
	unsigned int gpio0b_pdis_l;       /* address offset: 0x003c */
	unsigned int osc_con;             /* address offset: 0x0040 */
};

check_member(rk3576_pmu0_ioc_reg, osc_con, 0x0040);

/* pmu1_ioc register structure define */
struct rk3576_pmu1_ioc_reg {
	unsigned int gpio0b_iomux_sel_h;  /* address offset: 0x0000 */
	unsigned int gpio0c_iomux_sel_l;  /* address offset: 0x0004 */
	unsigned int gpio0c_iomux_sel_h;  /* address offset: 0x0008 */
	unsigned int gpio0d_iomux_sel_l;  /* address offset: 0x000c */
	unsigned int gpio0d_iomux_sel_h;  /* address offset: 0x0010 */
	unsigned int gpio0b_ds_h;         /* address offset: 0x0014 */
	unsigned int gpio0c_ds_l;         /* address offset: 0x0018 */
	unsigned int gpio0c_ds_h;         /* address offset: 0x001c */
	unsigned int gpio0d_ds_l;         /* address offset: 0x0020 */
	unsigned int gpio0d_ds_h;         /* address offset: 0x0024 */
	unsigned int gpio0b_pull_h;       /* address offset: 0x0028 */
	unsigned int gpio0c_pull;         /* address offset: 0x002c */
	unsigned int gpio0d_pull;         /* address offset: 0x0030 */
	unsigned int gpio0b_ie_h;         /* address offset: 0x0034 */
	unsigned int gpio0c_ie;           /* address offset: 0x0038 */
	unsigned int gpio0d_ie;           /* address offset: 0x003c */
	unsigned int gpio0b_smt_h;        /* address offset: 0x0040 */
	unsigned int gpio0c_smt;          /* address offset: 0x0044 */
	unsigned int gpio0d_smt;          /* address offset: 0x0048 */
	unsigned int gpio0b_pdis_h;       /* address offset: 0x004c */
	unsigned int gpio0c_pdis;         /* address offset: 0x0050 */
	unsigned int gpio0d_pdis;         /* address offset: 0x0054 */
};

check_member(rk3576_pmu1_ioc_reg, gpio0d_pdis, 0x0054);

/* top_ioc register structure define */
struct rk3576_top_ioc_reg {
	unsigned int reserved0000[2];     /* address offset: 0x0000 */
	unsigned int gpio0b_iomux_sel_l;  /* address offset: 0x0008 */
	unsigned int gpio0b_iomux_sel_h;  /* address offset: 0x000c */
	unsigned int gpio0c_iomux_sel_l;  /* address offset: 0x0010 */
	unsigned int gpio0c_iomux_sel_h;  /* address offset: 0x0014 */
	unsigned int gpio0d_iomux_sel_l;  /* address offset: 0x0018 */
	unsigned int gpio0d_iomux_sel_h;  /* address offset: 0x001c */
	unsigned int gpio1a_iomux_sel_l;  /* address offset: 0x0020 */
	unsigned int gpio1a_iomux_sel_h;  /* address offset: 0x0024 */
	unsigned int gpio1b_iomux_sel_l;  /* address offset: 0x0028 */
	unsigned int gpio1b_iomux_sel_h;  /* address offset: 0x002c */
	unsigned int gpio1c_iomux_sel_l;  /* address offset: 0x0030 */
	unsigned int gpio1c_iomux_sel_h;  /* address offset: 0x0034 */
	unsigned int gpio1d_iomux_sel_l;  /* address offset: 0x0038 */
	unsigned int gpio1d_iomux_sel_h;  /* address offset: 0x003c */
	unsigned int gpio2a_iomux_sel_l;  /* address offset: 0x0040 */
	unsigned int gpio2a_iomux_sel_h;  /* address offset: 0x0044 */
	unsigned int gpio2b_iomux_sel_l;  /* address offset: 0x0048 */
	unsigned int gpio2b_iomux_sel_h;  /* address offset: 0x004c */
	unsigned int gpio2c_iomux_sel_l;  /* address offset: 0x0050 */
	unsigned int gpio2c_iomux_sel_h;  /* address offset: 0x0054 */
	unsigned int gpio2d_iomux_sel_l;  /* address offset: 0x0058 */
	unsigned int gpio2d_iomux_sel_h;  /* address offset: 0x005c */
	unsigned int gpio3a_iomux_sel_l;  /* address offset: 0x0060 */
	unsigned int gpio3a_iomux_sel_h;  /* address offset: 0x0064 */
	unsigned int gpio3b_iomux_sel_l;  /* address offset: 0x0068 */
	unsigned int gpio3b_iomux_sel_h;  /* address offset: 0x006c */
	unsigned int gpio3c_iomux_sel_l;  /* address offset: 0x0070 */
	unsigned int gpio3c_iomux_sel_h;  /* address offset: 0x0074 */
	unsigned int gpio3d_iomux_sel_l;  /* address offset: 0x0078 */
	unsigned int gpio3d_iomux_sel_h;  /* address offset: 0x007c */
	unsigned int gpio4a_iomux_sel_l;  /* address offset: 0x0080 */
	unsigned int gpio4a_iomux_sel_h;  /* address offset: 0x0084 */
	unsigned int gpio4b_iomux_sel_l;  /* address offset: 0x0088 */
	unsigned int gpio4b_iomux_sel_h;  /* address offset: 0x008c */
	unsigned int reserved0090[24];    /* address offset: 0x0090 */
	unsigned int ioc_misc_con;        /* address offset: 0x00f0 */
	unsigned int sdmmc_detn_flt;      /* address offset: 0x00f4 */
};

check_member(rk3576_top_ioc_reg, sdmmc_detn_flt, 0x00f4);

/* vccio_ioc register structure define */
struct rk3576_vccio_ioc_reg {
	unsigned int reserved0000[8];     /* address offset: 0x0000 */
	unsigned int gpio1a_ds_l;         /* address offset: 0x0020 */
	unsigned int gpio1a_ds_h;         /* address offset: 0x0024 */
	unsigned int gpio1b_ds_l;         /* address offset: 0x0028 */
	unsigned int gpio1b_ds_h;         /* address offset: 0x002c */
	unsigned int gpio1c_ds_l;         /* address offset: 0x0030 */
	unsigned int gpio1c_ds_h;         /* address offset: 0x0034 */
	unsigned int gpio1d_ds_l;         /* address offset: 0x0038 */
	unsigned int gpio1d_ds_h;         /* address offset: 0x003c */
	unsigned int gpio2a_ds_l;         /* address offset: 0x0040 */
	unsigned int gpio2a_ds_h;         /* address offset: 0x0044 */
	unsigned int gpio2b_ds_l;         /* address offset: 0x0048 */
	unsigned int gpio2b_ds_h;         /* address offset: 0x004c */
	unsigned int gpio2c_ds_l;         /* address offset: 0x0050 */
	unsigned int gpio2c_ds_h;         /* address offset: 0x0054 */
	unsigned int gpio2d_ds_l;         /* address offset: 0x0058 */
	unsigned int gpio2d_ds_h;         /* address offset: 0x005c */
	unsigned int gpio3a_ds_l;         /* address offset: 0x0060 */
	unsigned int gpio3a_ds_h;         /* address offset: 0x0064 */
	unsigned int gpio3b_ds_l;         /* address offset: 0x0068 */
	unsigned int gpio3b_ds_h;         /* address offset: 0x006c */
	unsigned int gpio3c_ds_l;         /* address offset: 0x0070 */
	unsigned int gpio3c_ds_h;         /* address offset: 0x0074 */
	unsigned int gpio3d_ds_l;         /* address offset: 0x0078 */
	unsigned int gpio3d_ds_h;         /* address offset: 0x007c */
	unsigned int gpio4a_ds_l;         /* address offset: 0x0080 */
	unsigned int gpio4a_ds_h;         /* address offset: 0x0084 */
	unsigned int gpio4b_ds_l;         /* address offset: 0x0088 */
	unsigned int gpio4b_ds_h;         /* address offset: 0x008c */
	unsigned int reserved0090[32];    /* address offset: 0x0090 */
	unsigned int gpio1a_pull;         /* address offset: 0x0110 */
	unsigned int gpio1b_pull;         /* address offset: 0x0114 */
	unsigned int gpio1c_pull;         /* address offset: 0x0118 */
	unsigned int gpio1d_pull;         /* address offset: 0x011c */
	unsigned int gpio2a_pull;         /* address offset: 0x0120 */
	unsigned int gpio2b_pull;         /* address offset: 0x0124 */
	unsigned int gpio2c_pull;         /* address offset: 0x0128 */
	unsigned int gpio2d_pull;         /* address offset: 0x012c */
	unsigned int gpio3a_pull;         /* address offset: 0x0130 */
	unsigned int gpio3b_pull;         /* address offset: 0x0134 */
	unsigned int gpio3c_pull;         /* address offset: 0x0138 */
	unsigned int gpio3d_pull;         /* address offset: 0x013c */
	unsigned int gpio4a_pull;         /* address offset: 0x0140 */
	unsigned int gpio4b_pull;         /* address offset: 0x0144 */
	unsigned int reserved0148[14];    /* address offset: 0x0148 */
	unsigned int gpio1a_ie;           /* address offset: 0x0180 */
	unsigned int gpio1b_ie;           /* address offset: 0x0184 */
	unsigned int gpio1c_ie;           /* address offset: 0x0188 */
	unsigned int gpio1d_ie;           /* address offset: 0x018c */
	unsigned int gpio2a_ie;           /* address offset: 0x0190 */
	unsigned int gpio2b_ie;           /* address offset: 0x0194 */
	unsigned int gpio2c_ie;           /* address offset: 0x0198 */
	unsigned int gpio2d_ie;           /* address offset: 0x019c */
	unsigned int gpio3a_ie;           /* address offset: 0x01a0 */
	unsigned int gpio3b_ie;           /* address offset: 0x01a4 */
	unsigned int gpio3c_ie;           /* address offset: 0x01a8 */
	unsigned int gpio3d_ie;           /* address offset: 0x01ac */
	unsigned int gpio4a_ie;           /* address offset: 0x01b0 */
	unsigned int gpio4b_ie;           /* address offset: 0x01b4 */
	unsigned int reserved01b8[22];    /* address offset: 0x01b8 */
	unsigned int gpio1a_smt;          /* address offset: 0x0210 */
	unsigned int gpio1b_smt;          /* address offset: 0x0214 */
	unsigned int gpio1c_smt;          /* address offset: 0x0218 */
	unsigned int gpio1d_smt;          /* address offset: 0x021c */
	unsigned int gpio2a_smt;          /* address offset: 0x0220 */
	unsigned int gpio2b_smt;          /* address offset: 0x0224 */
	unsigned int gpio2c_smt;          /* address offset: 0x0228 */
	unsigned int gpio2d_smt;          /* address offset: 0x022c */
	unsigned int gpio3a_smt;          /* address offset: 0x0230 */
	unsigned int gpio3b_smt;          /* address offset: 0x0234 */
	unsigned int gpio3c_smt;          /* address offset: 0x0238 */
	unsigned int gpio3d_smt;          /* address offset: 0x023c */
	unsigned int gpio4a_smt;          /* address offset: 0x0240 */
	unsigned int gpio4b_smt;          /* address offset: 0x0244 */
	unsigned int reserved0248[14];    /* address offset: 0x0248 */
	unsigned int gpio1a_pdis;         /* address offset: 0x0280 */
	unsigned int gpio1b_pdis;         /* address offset: 0x0284 */
	unsigned int gpio1c_pdis;         /* address offset: 0x0288 */
	unsigned int gpio1d_pdis;         /* address offset: 0x028c */
	unsigned int gpio2a_pdis;         /* address offset: 0x0290 */
	unsigned int gpio2b_pdis;         /* address offset: 0x0294 */
	unsigned int gpio2c_pdis;         /* address offset: 0x0298 */
	unsigned int gpio2d_pdis;         /* address offset: 0x029c */
	unsigned int gpio3a_pdis;         /* address offset: 0x02a0 */
	unsigned int gpio3b_pdis;         /* address offset: 0x02a4 */
	unsigned int gpio3c_pdis;         /* address offset: 0x02a8 */
	unsigned int gpio3d_pdis;         /* address offset: 0x02ac */
	unsigned int gpio4a_pdis;         /* address offset: 0x02b0 */
	unsigned int gpio4b_pdis;         /* address offset: 0x02b4 */
	unsigned int reserved02b8[82];    /* address offset: 0x02b8 */
	unsigned int misc_con[9];         /* address offset: 0x0400 */
};

check_member(rk3576_vccio_ioc_reg, misc_con, 0x0400);

/* vccio6_ioc register structure define */
struct rk3576_vccio6_ioc_reg {
	unsigned int reserved0000[36];    /* address offset: 0x0000 */
	unsigned int gpio4c_ds_l;         /* address offset: 0x0090 */
	unsigned int gpio4c_ds_h;         /* address offset: 0x0094 */
	unsigned int reserved0098[44];    /* address offset: 0x0098 */
	unsigned int gpio4c_pull;         /* address offset: 0x0148 */
	unsigned int reserved014c[27];    /* address offset: 0x014c */
	unsigned int gpio4c_ie;           /* address offset: 0x01b8 */
	unsigned int reserved01bc[35];    /* address offset: 0x01bc */
	unsigned int gpio4c_smt;          /* address offset: 0x0248 */
	unsigned int reserved024c[27];    /* address offset: 0x024c */
	unsigned int gpio4c_pdis;         /* address offset: 0x02b8 */
	unsigned int reserved02bc[53];    /* address offset: 0x02bc */
	unsigned int gpio4c_iomux_sel_l;  /* address offset: 0x0390 */
	unsigned int gpio4c_iomux_sel_h;  /* address offset: 0x0394 */
	unsigned int reserved0398[26];    /* address offset: 0x0398 */
	unsigned int misc_con[2];         /* address offset: 0x0400 */
	unsigned int reserved0408[14];    /* address offset: 0x0408 */
	unsigned int hdmitx_hpd_status;   /* address offset: 0x0440 */
};

check_member(rk3576_vccio6_ioc_reg, hdmitx_hpd_status, 0x0440);

/* vccio7_ioc register structure define */
struct rk3576_vccio7_ioc_reg {
	unsigned int reserved0000[38];    /* address offset: 0x0000 */
	unsigned int gpio4d_ds_l;         /* address offset: 0x0098 */
	unsigned int reserved009c[44];    /* address offset: 0x009c */
	unsigned int gpio4d_pull;         /* address offset: 0x014c */
	unsigned int reserved0150[27];    /* address offset: 0x0150 */
	unsigned int gpio4d_ie;           /* address offset: 0x01bc */
	unsigned int reserved01c0[35];    /* address offset: 0x01c0 */
	unsigned int gpio4d_smt;          /* address offset: 0x024c */
	unsigned int reserved0250[27];    /* address offset: 0x0250 */
	unsigned int gpio4d_pdis;         /* address offset: 0x02bc */
	unsigned int reserved02c0[54];    /* address offset: 0x02c0 */
	unsigned int gpio4d_iomux_sel_l;  /* address offset: 0x0398 */
	unsigned int reserved039c[25];    /* address offset: 0x039c */
	unsigned int xin_ufs_con;         /* address offset: 0x0400 */
};

check_member(rk3576_vccio7_ioc_reg, xin_ufs_con, 0x0400);

#endif /* _ASM_ARCH_IOC_RK3576_H */
