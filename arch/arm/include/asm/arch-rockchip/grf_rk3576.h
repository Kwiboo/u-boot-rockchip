/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd.
 */
#ifndef _ASM_ARCH_GRF_RK3576_H
#define _ASM_ARCH_GRF_RK3576_H

/* usb2phy_grf register structure define */
struct rk3576_usb2phygrf {
	unsigned int con[6];                  /* address offset: 0x0000 */
	unsigned int reserved0018[2];         /* address offset: 0x0018 */
	unsigned int ls_con;                  /* address offset: 0x0020 */
	unsigned int dis_con;                 /* address offset: 0x0024 */
	unsigned int bvalid_con;              /* address offset: 0x0028 */
	unsigned int id_con;                  /* address offset: 0x002c */
	unsigned int vbusvalid_con;           /* address offset: 0x0030 */
	unsigned int reserved0034[3];         /* address offset: 0x0034 */
	unsigned int dbg_con[1];              /* address offset: 0x0040 */
	unsigned int linest_timeout;          /* address offset: 0x0044 */
	unsigned int linest_deb;              /* address offset: 0x0048 */
	unsigned int rx_timeout;              /* address offset: 0x004c */
	unsigned int seq_limt;                /* address offset: 0x0050 */
	unsigned int linest_cnt_st;           /* address offset: 0x0054 */
	unsigned int dbg_st;                  /* address offset: 0x0058 */
	unsigned int rx_cnt_st;               /* address offset: 0x005c */
	unsigned int reserved0060[8];         /* address offset: 0x0060 */
	unsigned int st[1];                   /* address offset: 0x0080 */
	unsigned int reserved0084[15];        /* address offset: 0x0084 */
	unsigned int int_en;                  /* address offset: 0x00c0 */
	unsigned int int_st;                  /* address offset: 0x00c4 */
	unsigned int int_st_clr;              /* address offset: 0x00c8 */
	unsigned int reserved00cc;            /* address offset: 0x00cc */
	unsigned int detclk_sel;              /* address offset: 0x00d0 */
};

check_member(rk3576_usb2phygrf, detclk_sel, 0x00d0);

/* php_grf register structure define */
struct rk3576_phpgrf {
	unsigned int mmubp_st;                /* address offset: 0x0000 */
	unsigned int mmubp_con[1];            /* address offset: 0x0004 */
	unsigned int mmu0_con;                /* address offset: 0x0008 */
	unsigned int mmu1_con;                /* address offset: 0x000c */
	unsigned int mem_con[3];              /* address offset: 0x0010 */
	unsigned int sata0_con;               /* address offset: 0x001c */
	unsigned int sata1_con;               /* address offset: 0x0020 */
	unsigned int usb3otg1_status_lat[2];  /* address offset: 0x0024 */
	unsigned int usb3otg1_status_cb;      /* address offset: 0x002c */
	unsigned int usb3otg1_status;         /* address offset: 0x0030 */
	unsigned int usb3otg1_con[2];         /* address offset: 0x0034 */
	unsigned int reserved003c[3];         /* address offset: 0x003c */
	unsigned int pciepipe_con[1];         /* address offset: 0x0048 */
	unsigned int reserved004c[2];         /* address offset: 0x004c */
	unsigned int pcie_clkreq_st;          /* address offset: 0x0054 */
	unsigned int reserved0058;            /* address offset: 0x0058 */
	unsigned int mmu0_st[5];              /* address offset: 0x005c */
	unsigned int mmu1_st[5];              /* address offset: 0x0070 */
};

check_member(rk3576_phpgrf, mmu1_st, 0x0070);

/* pmu0_grf register structure define */
struct rk3576_pmu0grf {
	unsigned int soc_con[7];              /* address offset: 0x0000 */
	unsigned int reserved001c;            /* address offset: 0x001c */
	unsigned int io_ret_con[2];           /* address offset: 0x0020 */
	unsigned int reserved0028[2];         /* address offset: 0x0028 */
	unsigned int mem_con;                 /* address offset: 0x0030 */
	unsigned int reserved0034[3];         /* address offset: 0x0034 */
	unsigned int os_reg[8];               /* address offset: 0x0040 */
};

check_member(rk3576_pmu0grf, os_reg, 0x0040);

/* pmu0_sgrf register structure define */
struct rk3576_pmu0sgrf {
	unsigned int soc_con[3];              /* address offset: 0x0000 */
	unsigned int reserved000c[13];        /* address offset: 0x000c */
	unsigned int dcie_con[8];             /* address offset: 0x0040 */
	unsigned int dcie_wlock;              /* address offset: 0x0060 */
};

check_member(rk3576_pmu0sgrf, dcie_wlock, 0x0060);

/* pmu1_grf register structure define */
struct rk3576_pmu1grf {
	unsigned int soc_con[8];              /* address offset: 0x0000 */
	unsigned int reserved0020[12];        /* address offset: 0x0020 */
	unsigned int biu_con;                 /* address offset: 0x0050 */
	unsigned int biu_status;              /* address offset: 0x0054 */
	unsigned int reserved0058[2];         /* address offset: 0x0058 */
	unsigned int soc_status;              /* address offset: 0x0060 */
	unsigned int reserved0064[7];         /* address offset: 0x0064 */
	unsigned int mem_con[2];              /* address offset: 0x0080 */
	unsigned int reserved0088[30];        /* address offset: 0x0088 */
	unsigned int func_rst_status;         /* address offset: 0x0100 */
	unsigned int func_rst_clr;            /* address offset: 0x0104 */
	unsigned int reserved0108[2];         /* address offset: 0x0108 */
	unsigned int sd_detect_con;           /* address offset: 0x0110 */
	unsigned int sd_detect_sts;           /* address offset: 0x0114 */
	unsigned int sd_detect_clr;           /* address offset: 0x0118 */
	unsigned int sd_detect_cnt;           /* address offset: 0x011c */
	unsigned int reserved0120[56];        /* address offset: 0x0120 */
	unsigned int os_reg[16];              /* address offset: 0x0200 */
};

check_member(rk3576_pmu1grf, os_reg, 0x0200);

/* pmu1_sgrf register structure define */
struct rk3576_pmu1sgrf {
	unsigned int soc_con[18];             /* address offset: 0x0000 */
};

check_member(rk3576_pmu1sgrf, soc_con, 0x0000);

/* sdgmac_grf register structure define */
struct rk3576_sdgmacgrf {
	unsigned int mem_con[5];              /* address offset: 0x0000 */
	unsigned int reserved0014[2];         /* address offset: 0x0014 */
	unsigned int gmac_st[1];              /* address offset: 0x001c */
	unsigned int gmac0_con;               /* address offset: 0x0020 */
	unsigned int gmac1_con;               /* address offset: 0x0024 */
	unsigned int gmac0_tp[2];             /* address offset: 0x0028 */
	unsigned int gmac1_tp[2];             /* address offset: 0x0030 */
	unsigned int gmac0_cmd;               /* address offset: 0x0038 */
	unsigned int gmac1_cmd;               /* address offset: 0x003c */
	unsigned int reserved0040[2];         /* address offset: 0x0040 */
	unsigned int mem_gate_con;            /* address offset: 0x0048 */
};

check_member(rk3576_sdgmacgrf, mem_gate_con, 0x0048);

/* sys_grf register structure define */
struct rk3576_sysgrf {
	unsigned int soc_con[13];             /* address offset: 0x0000 */
	unsigned int reserved0034[3];         /* address offset: 0x0034 */
	unsigned int biu_con[6];              /* address offset: 0x0040 */
	unsigned int reserved0058[2];         /* address offset: 0x0058 */
	unsigned int biu_status[8];           /* address offset: 0x0060 */
	unsigned int mem_con[19];             /* address offset: 0x0080 */
	unsigned int reserved00cc[29];        /* address offset: 0x00cc */
	unsigned int soc_status[2];           /* address offset: 0x0140 */
	unsigned int memfault_status[2];      /* address offset: 0x0148 */
	unsigned int reserved0150[12];        /* address offset: 0x0150 */
	unsigned int soc_code;                /* address offset: 0x0180 */
	unsigned int reserved0184[3];         /* address offset: 0x0184 */
	unsigned int soc_version;             /* address offset: 0x0190 */
	unsigned int reserved0194[3];         /* address offset: 0x0194 */
	unsigned int chip_id;                 /* address offset: 0x01a0 */
	unsigned int reserved01a4[3];         /* address offset: 0x01a4 */
	unsigned int chip_version;            /* address offset: 0x01b0 */
};

check_member(rk3576_sysgrf, chip_version, 0x01b0);

/* sys_sgrf register structure define */
struct rk3576_syssgrf {
	unsigned int ddr_bank_hash_ctrl;      /* address offset: 0x0000 */
	unsigned int ddr_bank_mask[4];        /* address offset: 0x0004 */
	unsigned int ddr_rank_mask[1];        /* address offset: 0x0014 */
	unsigned int reserved0018[2];         /* address offset: 0x0018 */
	unsigned int soc_con[21];             /* address offset: 0x0020 */
	unsigned int reserved0074[3];         /* address offset: 0x0074 */
	unsigned int dmac0_con[10];           /* address offset: 0x0080 */
	unsigned int reserved00a8[22];        /* address offset: 0x00a8 */
	unsigned int dmac1_con[10];           /* address offset: 0x0100 */
	unsigned int reserved0128[22];        /* address offset: 0x0128 */
	unsigned int dmac2_con[10];           /* address offset: 0x0180 */
	unsigned int reserved01a8[22];        /* address offset: 0x01a8 */
	unsigned int key_con[2];              /* address offset: 0x0200 */
	unsigned int key_wlock;               /* address offset: 0x0208 */
	unsigned int reserved020c[13];        /* address offset: 0x020c */
	unsigned int soc_status;              /* address offset: 0x0240 */
	unsigned int reserved0244[47];        /* address offset: 0x0244 */
	unsigned int ip_info_con;             /* address offset: 0x0300 */
};

check_member(rk3576_syssgrf, ip_info_con, 0x0300);

/* ufs_grf register structure define */
struct rk3576_ufsgrf {
	unsigned int clk_ctrl;                /* address offset: 0x0000 */
	unsigned int uic_src_sel;             /* address offset: 0x0004 */
	unsigned int ufs_state_ie;            /* address offset: 0x0008 */
	unsigned int ufs_state_is;            /* address offset: 0x000c */
	unsigned int ufs_state;               /* address offset: 0x0010 */
	unsigned int reserved0014[13];        /* address offset: 0x0014 */
};

check_member(rk3576_ufsgrf, reserved0014, 0x0014);

/* usbdpphy_grf register structure define */
struct rk3576_usbdpphygrf {
	unsigned int reserved0000;            /* address offset: 0x0000 */
	unsigned int con[3];                  /* address offset: 0x0004 */
	unsigned int reserved0010[29];        /* address offset: 0x0010 */
	unsigned int status[1];               /* address offset: 0x0084 */
	unsigned int reserved0088[14];        /* address offset: 0x0088 */
	unsigned int lfps_det_con;            /* address offset: 0x00c0 */
	unsigned int int_en;                  /* address offset: 0x00c4 */
	unsigned int int_status;              /* address offset: 0x00c8 */
};

check_member(rk3576_usbdpphygrf, int_status, 0x00c8);

/* usb_grf register structure define */
struct rk3576_usbgrf {
	unsigned int mmubp_st;                /* address offset: 0x0000 */
	unsigned int mmubp_con;               /* address offset: 0x0004 */
	unsigned int mmu2_con;                /* address offset: 0x0008 */
	unsigned int mem_con0;                /* address offset: 0x000c */
	unsigned int mem_con1;                /* address offset: 0x0010 */
	unsigned int reserved0014[2];         /* address offset: 0x0014 */
	unsigned int usb3otg0_status_lat[2];  /* address offset: 0x001c */
	unsigned int usb3otg0_status_cb;      /* address offset: 0x0024 */
	unsigned int usb3otg0_status;         /* address offset: 0x0028 */
	unsigned int usb3otg0_con[2];         /* address offset: 0x002c */
	unsigned int reserved0034[4];         /* address offset: 0x0034 */
	unsigned int mmu2_st[5];              /* address offset: 0x0044 */
	unsigned int mem_con[1];              /* address offset: 0x0058 */
};

check_member(rk3576_usbgrf, mem_con, 0x0058);

#endif /*  _ASM_ARCH_GRF_RK3576_H  */
