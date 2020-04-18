// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

const nec_common_device::nec_ophandler nec_common_device::s_nec_instruction[256] =
{
	&nec_common_device::i_add_br8,      /* 0x00 */
	&nec_common_device::i_add_wr16,     /* 0x01 */
	&nec_common_device::i_add_r8b,      /* 0x02 */
	&nec_common_device::i_add_r16w,     /* 0x03 */
	&nec_common_device::i_add_ald8,     /* 0x04 */
	&nec_common_device::i_add_axd16,    /* 0x05 */
	&nec_common_device::i_push_es,      /* 0x06 */
	&nec_common_device::i_pop_es,       /* 0x07 */
	&nec_common_device::i_or_br8,       /* 0x08 */
	&nec_common_device::i_or_wr16,      /* 0x09 */
	&nec_common_device::i_or_r8b,       /* 0x0a */
	&nec_common_device::i_or_r16w,      /* 0x0b */
	&nec_common_device::i_or_ald8,      /* 0x0c */
	&nec_common_device::i_or_axd16,     /* 0x0d */
	&nec_common_device::i_push_cs,      /* 0x0e */
	&nec_common_device::i_pre_nec,      /* 0x0f */
	&nec_common_device::i_adc_br8,      /* 0x10 */
	&nec_common_device::i_adc_wr16,     /* 0x11 */
	&nec_common_device::i_adc_r8b,      /* 0x12 */
	&nec_common_device::i_adc_r16w,     /* 0x13 */
	&nec_common_device::i_adc_ald8,     /* 0x14 */
	&nec_common_device::i_adc_axd16,    /* 0x15 */
	&nec_common_device::i_push_ss,      /* 0x16 */
	&nec_common_device::i_pop_ss,       /* 0x17 */
	&nec_common_device::i_sbb_br8,      /* 0x18 */
	&nec_common_device::i_sbb_wr16,     /* 0x19 */
	&nec_common_device::i_sbb_r8b,      /* 0x1a */
	&nec_common_device::i_sbb_r16w,     /* 0x1b */
	&nec_common_device::i_sbb_ald8,     /* 0x1c */
	&nec_common_device::i_sbb_axd16,    /* 0x1d */
	&nec_common_device::i_push_ds,      /* 0x1e */
	&nec_common_device::i_pop_ds,       /* 0x1f */
	&nec_common_device::i_and_br8,      /* 0x20 */
	&nec_common_device::i_and_wr16,     /* 0x21 */
	&nec_common_device::i_and_r8b,      /* 0x22 */
	&nec_common_device::i_and_r16w,     /* 0x23 */
	&nec_common_device::i_and_ald8,     /* 0x24 */
	&nec_common_device::i_and_axd16,    /* 0x25 */
	&nec_common_device::i_es,           /* 0x26 */
	&nec_common_device::i_daa,          /* 0x27 */
	&nec_common_device::i_sub_br8,      /* 0x28 */
	&nec_common_device::i_sub_wr16,     /* 0x29 */
	&nec_common_device::i_sub_r8b,      /* 0x2a */
	&nec_common_device::i_sub_r16w,     /* 0x2b */
	&nec_common_device::i_sub_ald8,     /* 0x2c */
	&nec_common_device::i_sub_axd16,    /* 0x2d */
	&nec_common_device::i_cs,           /* 0x2e */
	&nec_common_device::i_das,          /* 0x2f */
	&nec_common_device::i_xor_br8,      /* 0x30 */
	&nec_common_device::i_xor_wr16,     /* 0x31 */
	&nec_common_device::i_xor_r8b,      /* 0x32 */
	&nec_common_device::i_xor_r16w,     /* 0x33 */
	&nec_common_device::i_xor_ald8,     /* 0x34 */
	&nec_common_device::i_xor_axd16,    /* 0x35 */
	&nec_common_device::i_ss,           /* 0x36 */
	&nec_common_device::i_aaa,          /* 0x37 */
	&nec_common_device::i_cmp_br8,      /* 0x38 */
	&nec_common_device::i_cmp_wr16,     /* 0x39 */
	&nec_common_device::i_cmp_r8b,      /* 0x3a */
	&nec_common_device::i_cmp_r16w,     /* 0x3b */
	&nec_common_device::i_cmp_ald8,     /* 0x3c */
	&nec_common_device::i_cmp_axd16,    /* 0x3d */
	&nec_common_device::i_ds,           /* 0x3e */
	&nec_common_device::i_aas,          /* 0x3f */
	&nec_common_device::i_inc_ax,       /* 0x40 */
	&nec_common_device::i_inc_cx,       /* 0x41 */
	&nec_common_device::i_inc_dx,       /* 0x42 */
	&nec_common_device::i_inc_bx,       /* 0x43 */
	&nec_common_device::i_inc_sp,       /* 0x44 */
	&nec_common_device::i_inc_bp,       /* 0x45 */
	&nec_common_device::i_inc_si,       /* 0x46 */
	&nec_common_device::i_inc_di,       /* 0x47 */
	&nec_common_device::i_dec_ax,       /* 0x48 */
	&nec_common_device::i_dec_cx,       /* 0x49 */
	&nec_common_device::i_dec_dx,       /* 0x4a */
	&nec_common_device::i_dec_bx,       /* 0x4b */
	&nec_common_device::i_dec_sp,       /* 0x4c */
	&nec_common_device::i_dec_bp,       /* 0x4d */
	&nec_common_device::i_dec_si,       /* 0x4e */
	&nec_common_device::i_dec_di,       /* 0x4f */
	&nec_common_device::i_push_ax,      /* 0x50 */
	&nec_common_device::i_push_cx,      /* 0x51 */
	&nec_common_device::i_push_dx,      /* 0x52 */
	&nec_common_device::i_push_bx,      /* 0x53 */
	&nec_common_device::i_push_sp,      /* 0x54 */
	&nec_common_device::i_push_bp,      /* 0x55 */
	&nec_common_device::i_push_si,      /* 0x56 */
	&nec_common_device::i_push_di,      /* 0x57 */
	&nec_common_device::i_pop_ax,       /* 0x58 */
	&nec_common_device::i_pop_cx,       /* 0x59 */
	&nec_common_device::i_pop_dx,       /* 0x5a */
	&nec_common_device::i_pop_bx,       /* 0x5b */
	&nec_common_device::i_pop_sp,       /* 0x5c */
	&nec_common_device::i_pop_bp,       /* 0x5d */
	&nec_common_device::i_pop_si,       /* 0x5e */
	&nec_common_device::i_pop_di,       /* 0x5f */
	&nec_common_device::i_pusha,        /* 0x60 */
	&nec_common_device::i_popa,         /* 0x61 */
	&nec_common_device::i_chkind,       /* 0x62 */
	&nec_common_device::i_invalid,      /* 0x63 */
	&nec_common_device::i_repnc,        /* 0x64 */
	&nec_common_device::i_repc,         /* 0x65 */
	&nec_common_device::i_invalid,      /* 0x66 */
	&nec_common_device::i_invalid,      /* 0x67 */
	&nec_common_device::i_push_d16,     /* 0x68 */
	&nec_common_device::i_imul_d16,     /* 0x69 */
	&nec_common_device::i_push_d8,      /* 0x6a */
	&nec_common_device::i_imul_d8,      /* 0x6b */
	&nec_common_device::i_insb,         /* 0x6c */
	&nec_common_device::i_insw,         /* 0x6d */
	&nec_common_device::i_outsb,        /* 0x6e */
	&nec_common_device::i_outsw,        /* 0x6f */
	&nec_common_device::i_jo,           /* 0x70 */
	&nec_common_device::i_jno,          /* 0x71 */
	&nec_common_device::i_jc,           /* 0x72 */
	&nec_common_device::i_jnc,          /* 0x73 */
	&nec_common_device::i_jz,           /* 0x74 */
	&nec_common_device::i_jnz,          /* 0x75 */
	&nec_common_device::i_jce,          /* 0x76 */
	&nec_common_device::i_jnce,         /* 0x77 */
	&nec_common_device::i_js,           /* 0x78 */
	&nec_common_device::i_jns,          /* 0x79 */
	&nec_common_device::i_jp,           /* 0x7a */
	&nec_common_device::i_jnp,          /* 0x7b */
	&nec_common_device::i_jl,           /* 0x7c */
	&nec_common_device::i_jnl,          /* 0x7d */
	&nec_common_device::i_jle,          /* 0x7e */
	&nec_common_device::i_jnle,         /* 0x7f */
	&nec_common_device::i_80pre,        /* 0x80 */
	&nec_common_device::i_81pre,        /* 0x81 */
	&nec_common_device::i_82pre,        /* 0x82 */
	&nec_common_device::i_83pre,        /* 0x83 */
	&nec_common_device::i_test_br8,     /* 0x84 */
	&nec_common_device::i_test_wr16,    /* 0x85 */
	&nec_common_device::i_xchg_br8,     /* 0x86 */
	&nec_common_device::i_xchg_wr16,    /* 0x87 */
	&nec_common_device::i_mov_br8,      /* 0x88 */
	&nec_common_device::i_mov_wr16,     /* 0x89 */
	&nec_common_device::i_mov_r8b,      /* 0x8a */
	&nec_common_device::i_mov_r16w,     /* 0x8b */
	&nec_common_device::i_mov_wsreg,    /* 0x8c */
	&nec_common_device::i_lea,          /* 0x8d */
	&nec_common_device::i_mov_sregw,    /* 0x8e */
	&nec_common_device::i_popw,         /* 0x8f */
	&nec_common_device::i_nop,          /* 0x90 */
	&nec_common_device::i_xchg_axcx,    /* 0x91 */
	&nec_common_device::i_xchg_axdx,    /* 0x92 */
	&nec_common_device::i_xchg_axbx,    /* 0x93 */
	&nec_common_device::i_xchg_axsp,    /* 0x94 */
	&nec_common_device::i_xchg_axbp,    /* 0x95 */
	&nec_common_device::i_xchg_axsi,    /* 0x96 */
	&nec_common_device::i_xchg_axdi,    /* 0x97 */
	&nec_common_device::i_cbw,          /* 0x98 */
	&nec_common_device::i_cwd,          /* 0x99 */
	&nec_common_device::i_call_far,     /* 0x9a */
	&nec_common_device::i_wait,         /* 0x9b */
	&nec_common_device::i_pushf,        /* 0x9c */
	&nec_common_device::i_popf,         /* 0x9d */
	&nec_common_device::i_sahf,         /* 0x9e */
	&nec_common_device::i_lahf,         /* 0x9f */
	&nec_common_device::i_mov_aldisp,   /* 0xa0 */
	&nec_common_device::i_mov_axdisp,   /* 0xa1 */
	&nec_common_device::i_mov_dispal,   /* 0xa2 */
	&nec_common_device::i_mov_dispax,   /* 0xa3 */
	&nec_common_device::i_movsb,        /* 0xa4 */
	&nec_common_device::i_movsw,        /* 0xa5 */
	&nec_common_device::i_cmpsb,        /* 0xa6 */
	&nec_common_device::i_cmpsw,        /* 0xa7 */
	&nec_common_device::i_test_ald8,    /* 0xa8 */
	&nec_common_device::i_test_axd16,   /* 0xa9 */
	&nec_common_device::i_stosb,        /* 0xaa */
	&nec_common_device::i_stosw,        /* 0xab */
	&nec_common_device::i_lodsb,        /* 0xac */
	&nec_common_device::i_lodsw,        /* 0xad */
	&nec_common_device::i_scasb,        /* 0xae */
	&nec_common_device::i_scasw,        /* 0xaf */
	&nec_common_device::i_mov_ald8,     /* 0xb0 */
	&nec_common_device::i_mov_cld8,     /* 0xb1 */
	&nec_common_device::i_mov_dld8,     /* 0xb2 */
	&nec_common_device::i_mov_bld8,     /* 0xb3 */
	&nec_common_device::i_mov_ahd8,     /* 0xb4 */
	&nec_common_device::i_mov_chd8,     /* 0xb5 */
	&nec_common_device::i_mov_dhd8,     /* 0xb6 */
	&nec_common_device::i_mov_bhd8,     /* 0xb7 */
	&nec_common_device::i_mov_axd16,    /* 0xb8 */
	&nec_common_device::i_mov_cxd16,    /* 0xb9 */
	&nec_common_device::i_mov_dxd16,    /* 0xba */
	&nec_common_device::i_mov_bxd16,    /* 0xbb */
	&nec_common_device::i_mov_spd16,    /* 0xbc */
	&nec_common_device::i_mov_bpd16,    /* 0xbd */
	&nec_common_device::i_mov_sid16,    /* 0xbe */
	&nec_common_device::i_mov_did16,    /* 0xbf */
	&nec_common_device::i_rotshft_bd8,  /* 0xc0 */
	&nec_common_device::i_rotshft_wd8,  /* 0xc1 */
	&nec_common_device::i_ret_d16,      /* 0xc2 */
	&nec_common_device::i_ret,          /* 0xc3 */
	&nec_common_device::i_les_dw,       /* 0xc4 */
	&nec_common_device::i_lds_dw,       /* 0xc5 */
	&nec_common_device::i_mov_bd8,      /* 0xc6 */
	&nec_common_device::i_mov_wd16,     /* 0xc7 */
	&nec_common_device::i_enter,        /* 0xc8 */
	&nec_common_device::i_leave,        /* 0xc9 */
	&nec_common_device::i_retf_d16,     /* 0xca */
	&nec_common_device::i_retf,         /* 0xcb */
	&nec_common_device::i_int3,         /* 0xcc */
	&nec_common_device::i_int,          /* 0xcd */
	&nec_common_device::i_into,         /* 0xce */
	&nec_common_device::i_iret,         /* 0xcf */
	&nec_common_device::i_rotshft_b,    /* 0xd0 */
	&nec_common_device::i_rotshft_w,    /* 0xd1 */
	&nec_common_device::i_rotshft_bcl,  /* 0xd2 */
	&nec_common_device::i_rotshft_wcl,  /* 0xd3 */
	&nec_common_device::i_aam,          /* 0xd4 */
	&nec_common_device::i_aad,          /* 0xd5 */
	&nec_common_device::i_setalc,       /* 0xd6 */
	&nec_common_device::i_trans,        /* 0xd7 */
	&nec_common_device::i_fpo,          /* 0xd8 */
	&nec_common_device::i_fpo,          /* 0xd9 */
	&nec_common_device::i_fpo,          /* 0xda */
	&nec_common_device::i_fpo,          /* 0xdb */
	&nec_common_device::i_fpo,          /* 0xdc */
	&nec_common_device::i_fpo,          /* 0xdd */
	&nec_common_device::i_fpo,          /* 0xde */
	&nec_common_device::i_fpo,          /* 0xdf */
	&nec_common_device::i_loopne,       /* 0xe0 */
	&nec_common_device::i_loope,        /* 0xe1 */
	&nec_common_device::i_loop,         /* 0xe2 */
	&nec_common_device::i_jcxz,         /* 0xe3 */
	&nec_common_device::i_inal,         /* 0xe4 */
	&nec_common_device::i_inax,         /* 0xe5 */
	&nec_common_device::i_outal,        /* 0xe6 */
	&nec_common_device::i_outax,        /* 0xe7 */
	&nec_common_device::i_call_d16,     /* 0xe8 */
	&nec_common_device::i_jmp_d16,      /* 0xe9 */
	&nec_common_device::i_jmp_far,      /* 0xea */
	&nec_common_device::i_jmp_d8,       /* 0xeb */
	&nec_common_device::i_inaldx,       /* 0xec */
	&nec_common_device::i_inaxdx,       /* 0xed */
	&nec_common_device::i_outdxal,      /* 0xee */
	&nec_common_device::i_outdxax,      /* 0xef */
	&nec_common_device::i_lock,         /* 0xf0 */
	&nec_common_device::i_invalid,      /* 0xf1 */
	&nec_common_device::i_repne,        /* 0xf2 */
	&nec_common_device::i_repe,         /* 0xf3 */
	&nec_common_device::i_hlt,          /* 0xf4 */
	&nec_common_device::i_cmc,          /* 0xf5 */
	&nec_common_device::i_f6pre,        /* 0xf6 */
	&nec_common_device::i_f7pre,        /* 0xf7 */
	&nec_common_device::i_clc,          /* 0xf8 */
	&nec_common_device::i_stc,          /* 0xf9 */
	&nec_common_device::i_di,           /* 0xfa */
	&nec_common_device::i_ei,           /* 0xfb */
	&nec_common_device::i_cld,          /* 0xfc */
	&nec_common_device::i_std,          /* 0xfd */
	&nec_common_device::i_fepre,        /* 0xfe */
	&nec_common_device::i_ffpre         /* 0xff */
};

const nec_common_device::nec_ophandler nec_common_device::s_nec80_instruction[256] =
{
	&nec_common_device::i_nop_80,
	&nec_common_device::i_lxib_80,
	&nec_common_device::i_staxb_80,
	&nec_common_device::i_inxb_80,
	&nec_common_device::i_inrb_80,
	&nec_common_device::i_dcrb_80,
	&nec_common_device::i_mvib_80,
	&nec_common_device::i_rlc_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_dadb_80,
	&nec_common_device::i_ldaxb_80,
	&nec_common_device::i_dcxb_80,
	&nec_common_device::i_inrc_80,
	&nec_common_device::i_dcrc_80,
	&nec_common_device::i_mvic_80,
	&nec_common_device::i_rrc_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_lxid_80,
	&nec_common_device::i_staxd_80,
	&nec_common_device::i_inxd_80,
	&nec_common_device::i_inrd_80,
	&nec_common_device::i_dcrd_80,
	&nec_common_device::i_mvid_80,
	&nec_common_device::i_ral_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_dadd_80,
	&nec_common_device::i_ldaxd_80,
	&nec_common_device::i_dcxd_80,
	&nec_common_device::i_inre_80,
	&nec_common_device::i_dcre_80,
	&nec_common_device::i_mvie_80,
	&nec_common_device::i_rar_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_lxih_80,
	&nec_common_device::i_shld_80,
	&nec_common_device::i_inxh_80,
	&nec_common_device::i_inrh_80,
	&nec_common_device::i_dcrh_80,
	&nec_common_device::i_mvih_80,
	&nec_common_device::i_daa_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_dadh_80,
	&nec_common_device::i_lhld_80,
	&nec_common_device::i_dcxh_80,
	&nec_common_device::i_inrl_80,
	&nec_common_device::i_dcrl_80,
	&nec_common_device::i_mvil_80,
	&nec_common_device::i_cma_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_lxis_80,
	&nec_common_device::i_sta_80,
	&nec_common_device::i_inxs_80,
	&nec_common_device::i_inrm_80,
	&nec_common_device::i_dcrm_80,
	&nec_common_device::i_mvim_80,
	&nec_common_device::i_stc_80,
	&nec_common_device::i_nop_80,
	&nec_common_device::i_dads_80,
	&nec_common_device::i_lda_80,
	&nec_common_device::i_dcxs_80,
	&nec_common_device::i_inra_80,
	&nec_common_device::i_dcra_80,
	&nec_common_device::i_mvia_80,
	&nec_common_device::i_cmc_80,
	&nec_common_device::i_movbb_80,
	&nec_common_device::i_movbc_80,
	&nec_common_device::i_movbd_80,
	&nec_common_device::i_movbe_80,
	&nec_common_device::i_movbh_80,
	&nec_common_device::i_movbl_80,
	&nec_common_device::i_movbm_80,
	&nec_common_device::i_movba_80,
	&nec_common_device::i_movcb_80,
	&nec_common_device::i_movcc_80,
	&nec_common_device::i_movcd_80,
	&nec_common_device::i_movce_80,
	&nec_common_device::i_movch_80,
	&nec_common_device::i_movcl_80,
	&nec_common_device::i_movcm_80,
	&nec_common_device::i_movca_80,
	&nec_common_device::i_movdb_80,
	&nec_common_device::i_movdc_80,
	&nec_common_device::i_movdd_80,
	&nec_common_device::i_movde_80,
	&nec_common_device::i_movdh_80,
	&nec_common_device::i_movdl_80,
	&nec_common_device::i_movdm_80,
	&nec_common_device::i_movda_80,
	&nec_common_device::i_moveb_80,
	&nec_common_device::i_movec_80,
	&nec_common_device::i_moved_80,
	&nec_common_device::i_movee_80,
	&nec_common_device::i_moveh_80,
	&nec_common_device::i_movel_80,
	&nec_common_device::i_movem_80,
	&nec_common_device::i_movea_80,
	&nec_common_device::i_movhb_80,
	&nec_common_device::i_movhc_80,
	&nec_common_device::i_movhd_80,
	&nec_common_device::i_movhe_80,
	&nec_common_device::i_movhh_80,
	&nec_common_device::i_movhl_80,
	&nec_common_device::i_movhm_80,
	&nec_common_device::i_movha_80,
	&nec_common_device::i_movlb_80,
	&nec_common_device::i_movlc_80,
	&nec_common_device::i_movld_80,
	&nec_common_device::i_movle_80,
	&nec_common_device::i_movlh_80,
	&nec_common_device::i_movll_80,
	&nec_common_device::i_movlm_80,
	&nec_common_device::i_movla_80,
	&nec_common_device::i_movmb_80,
	&nec_common_device::i_movmc_80,
	&nec_common_device::i_movmd_80,
	&nec_common_device::i_movme_80,
	&nec_common_device::i_movmh_80,
	&nec_common_device::i_movml_80,
	&nec_common_device::i_hlt_80,
	&nec_common_device::i_movma_80,
	&nec_common_device::i_movab_80,
	&nec_common_device::i_movac_80,
	&nec_common_device::i_movad_80,
	&nec_common_device::i_movae_80,
	&nec_common_device::i_movah_80,
	&nec_common_device::i_moval_80,
	&nec_common_device::i_movam_80,
	&nec_common_device::i_movaa_80,
	&nec_common_device::i_addb_80,
	&nec_common_device::i_addc_80,
	&nec_common_device::i_addd_80,
	&nec_common_device::i_adde_80,
	&nec_common_device::i_addh_80,
	&nec_common_device::i_addl_80,
	&nec_common_device::i_addm_80,
	&nec_common_device::i_adda_80,
	&nec_common_device::i_adcb_80,
	&nec_common_device::i_adcc_80,
	&nec_common_device::i_adcd_80,
	&nec_common_device::i_adce_80,
	&nec_common_device::i_adch_80,
	&nec_common_device::i_adcl_80,
	&nec_common_device::i_adcm_80,
	&nec_common_device::i_adca_80,
	&nec_common_device::i_subb_80,
	&nec_common_device::i_subc_80,
	&nec_common_device::i_subd_80,
	&nec_common_device::i_sube_80,
	&nec_common_device::i_subh_80,
	&nec_common_device::i_subl_80,
	&nec_common_device::i_subm_80,
	&nec_common_device::i_suba_80,
	&nec_common_device::i_sbbb_80,
	&nec_common_device::i_sbbc_80,
	&nec_common_device::i_sbbd_80,
	&nec_common_device::i_sbbe_80,
	&nec_common_device::i_sbbh_80,
	&nec_common_device::i_sbbl_80,
	&nec_common_device::i_sbbm_80,
	&nec_common_device::i_sbba_80,
	&nec_common_device::i_anab_80,
	&nec_common_device::i_anac_80,
	&nec_common_device::i_anad_80,
	&nec_common_device::i_anae_80,
	&nec_common_device::i_anah_80,
	&nec_common_device::i_anal_80,
	&nec_common_device::i_anam_80,
	&nec_common_device::i_anaa_80,
	&nec_common_device::i_xrab_80,
	&nec_common_device::i_xrac_80,
	&nec_common_device::i_xrad_80,
	&nec_common_device::i_xrae_80,
	&nec_common_device::i_xrah_80,
	&nec_common_device::i_xral_80,
	&nec_common_device::i_xram_80,
	&nec_common_device::i_xraa_80,
	&nec_common_device::i_orab_80,
	&nec_common_device::i_orac_80,
	&nec_common_device::i_orad_80,
	&nec_common_device::i_orae_80,
	&nec_common_device::i_orah_80,
	&nec_common_device::i_oral_80,
	&nec_common_device::i_oram_80,
	&nec_common_device::i_oraa_80,
	&nec_common_device::i_cmpb_80,
	&nec_common_device::i_cmpc_80,
	&nec_common_device::i_cmpd_80,
	&nec_common_device::i_cmpe_80,
	&nec_common_device::i_cmph_80,
	&nec_common_device::i_cmpl_80,
	&nec_common_device::i_cmpm_80,
	&nec_common_device::i_cmpa_80,
	&nec_common_device::i_rnz_80,
	&nec_common_device::i_popb_80,
	&nec_common_device::i_jnz_80,
	&nec_common_device::i_jmp_80,
	&nec_common_device::i_cnz_80,
	&nec_common_device::i_pushb_80,
	&nec_common_device::i_adi_80,
	&nec_common_device::i_rst0_80,
	&nec_common_device::i_rz_80,
	&nec_common_device::i_ret_80,
	&nec_common_device::i_jz_80,
	&nec_common_device::i_jmp_80,
	&nec_common_device::i_cz_80,
	&nec_common_device::i_call_80,
	&nec_common_device::i_aci_80,
	&nec_common_device::i_rst1_80,
	&nec_common_device::i_rnc_80,
	&nec_common_device::i_popd_80,
	&nec_common_device::i_jnc_80,
	&nec_common_device::i_out_80,
	&nec_common_device::i_cnc_80,
	&nec_common_device::i_pushd_80,
	&nec_common_device::i_sui_80,
	&nec_common_device::i_rst2_80,
	&nec_common_device::i_rc_80,
	&nec_common_device::i_ret_80,
	&nec_common_device::i_jc_80,
	&nec_common_device::i_in_80,
	&nec_common_device::i_cc_80,
	&nec_common_device::i_call_80,
	&nec_common_device::i_sbi_80,
	&nec_common_device::i_rst3_80,
	&nec_common_device::i_rpo_80,
	&nec_common_device::i_poph_80,
	&nec_common_device::i_jpo_80,
	&nec_common_device::i_xthl_80,
	&nec_common_device::i_cpo_80,
	&nec_common_device::i_pushh_80,
	&nec_common_device::i_ani_80,
	&nec_common_device::i_rst4_80,
	&nec_common_device::i_rpe_80,
	&nec_common_device::i_pchl_80,
	&nec_common_device::i_jpe_80,
	&nec_common_device::i_xchg_80,
	&nec_common_device::i_cpe_80,
	&nec_common_device::i_calln_80,
	&nec_common_device::i_xri_80,
	&nec_common_device::i_rst5_80,
	&nec_common_device::i_rp_80,
	&nec_common_device::i_popf_80,
	&nec_common_device::i_jp_80,
	&nec_common_device::i_di_80,
	&nec_common_device::i_cp_80,
	&nec_common_device::i_pushf_80,
	&nec_common_device::i_ori_80,
	&nec_common_device::i_rst6_80,
	&nec_common_device::i_rm_80,
	&nec_common_device::i_sphl_80,
	&nec_common_device::i_jm_80,
	&nec_common_device::i_ei_80,
	&nec_common_device::i_cm_80,
	&nec_common_device::i_call_80,
	&nec_common_device::i_cpi_80,
	&nec_common_device::i_rst7_80
};
