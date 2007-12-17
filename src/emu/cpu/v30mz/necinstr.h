static void i_add_br8(void);
static void i_add_wr16(void);
static void i_add_r8b(void);
static void i_add_r16w(void);
static void i_add_ald8(void);
static void i_add_axd16(void);
static void i_push_es(void);
static void i_pop_es(void);
static void i_or_br8(void);
static void i_or_r8b(void);
static void i_or_wr16(void);
static void i_or_r16w(void);
static void i_or_ald8(void);
static void i_or_axd16(void);
static void i_push_cs(void);
static void i_pre_nec(void);
static void i_adc_br8(void);
static void i_adc_wr16(void);
static void i_adc_r8b(void);
static void i_adc_r16w(void);
static void i_adc_ald8(void);
static void i_adc_axd16(void);
static void i_push_ss(void);
static void i_pop_ss(void);
static void i_sbb_br8(void);
static void i_sbb_wr16(void);
static void i_sbb_r8b(void);
static void i_sbb_r16w(void);
static void i_sbb_ald8(void);
static void i_sbb_axd16(void);
static void i_push_ds(void);
static void i_pop_ds(void);
static void i_and_br8(void);
static void i_and_r8b(void);
static void i_and_wr16(void);
static void i_and_r16w(void);
static void i_and_ald8(void);
static void i_and_axd16(void);
static void i_es(void);
static void i_daa(void);
static void i_sub_br8(void);
static void i_sub_wr16(void);
static void i_sub_r8b(void);
static void i_sub_r16w(void);
static void i_sub_ald8(void);
static void i_sub_axd16(void);
static void i_cs(void);
static void i_das(void);
static void i_xor_br8(void);
static void i_xor_r8b(void);
static void i_xor_wr16(void);
static void i_xor_r16w(void);
static void i_xor_ald8(void);
static void i_xor_axd16(void);
static void i_ss(void);
static void i_aaa(void);
static void i_cmp_br8(void);
static void i_cmp_wr16(void);
static void i_cmp_r8b(void);
static void i_cmp_r16w(void);
static void i_cmp_ald8(void);
static void i_cmp_axd16(void);
static void i_ds(void);
static void i_aas(void);
static void i_inc_ax(void);
static void i_inc_cx(void);
static void i_inc_dx(void);
static void i_inc_bx(void);
static void i_inc_sp(void);
static void i_inc_bp(void);
static void i_inc_si(void);
static void i_inc_di(void);
static void i_dec_ax(void);
static void i_dec_cx(void);
static void i_dec_dx(void);
static void i_dec_bx(void);
static void i_dec_sp(void);
static void i_dec_bp(void);
static void i_dec_si(void);
static void i_dec_di(void);
static void i_push_ax(void);
static void i_push_cx(void);
static void i_push_dx(void);
static void i_push_bx(void);
static void i_push_sp(void);
static void i_push_bp(void);
static void i_push_si(void);
static void i_push_di(void);
static void i_pop_ax(void);
static void i_pop_cx(void);
static void i_pop_dx(void);
static void i_pop_bx(void);
static void i_pop_sp(void);
static void i_pop_bp(void);
static void i_pop_si(void);
static void i_pop_di(void);
static void i_pusha(void);
static void i_popa(void);
static void i_chkind(void);
static void i_repnc(void);
static void i_repc(void);
static void i_push_d16(void);
static void i_imul_d16(void);
static void i_push_d8(void);
static void i_imul_d8(void);
static void i_insb(void);
static void i_insw(void);
static void i_outsb(void);
static void i_outsw(void);
static void i_jo(void);
static void i_jno(void);
static void i_jc(void);
static void i_jnc(void);
static void i_jz(void);
static void i_jnz(void);
static void i_jce(void);
static void i_jnce(void);
static void i_js(void);
static void i_jns(void);
static void i_jp(void);
static void i_jnp(void);
static void i_jl(void);
static void i_jnl(void);
static void i_jle(void);
static void i_jnle(void);
static void i_80pre(void);
static void i_82pre(void);
static void i_81pre(void);
static void i_83pre(void);
static void i_test_br8(void);
static void i_test_wr16(void);
static void i_xchg_br8(void);
static void i_xchg_wr16(void);
static void i_mov_br8(void);
static void i_mov_r8b(void);
static void i_mov_wr16(void);
static void i_mov_r16w(void);
static void i_mov_wsreg(void);
static void i_lea(void);
static void i_mov_sregw(void);
static void i_invalid(void);
static void i_popw(void);
static void i_nop(void);
static void i_xchg_axcx(void);
static void i_xchg_axdx(void);
static void i_xchg_axbx(void);
static void i_xchg_axsp(void);
static void i_xchg_axbp(void);
static void i_xchg_axsi(void);
static void i_xchg_axdi(void);
static void i_cbw(void);
static void i_cwd(void);
static void i_call_far(void);
static void i_pushf(void);
static void i_popf(void);
static void i_sahf(void);
static void i_lahf(void);
static void i_mov_aldisp(void);
static void i_mov_axdisp(void);
static void i_mov_dispal(void);
static void i_mov_dispax(void);
static void i_movsb(void);
static void i_movsw(void);
static void i_cmpsb(void);
static void i_cmpsw(void);
static void i_test_ald8(void);
static void i_test_axd16(void);
static void i_stosb(void);
static void i_stosw(void);
static void i_lodsb(void);
static void i_lodsw(void);
static void i_scasb(void);
static void i_scasw(void);
static void i_mov_ald8(void);
static void i_mov_cld8(void);
static void i_mov_dld8(void);
static void i_mov_bld8(void);
static void i_mov_ahd8(void);
static void i_mov_chd8(void);
static void i_mov_dhd8(void);
static void i_mov_bhd8(void);
static void i_mov_axd16(void);
static void i_mov_cxd16(void);
static void i_mov_dxd16(void);
static void i_mov_bxd16(void);
static void i_mov_spd16(void);
static void i_mov_bpd16(void);
static void i_mov_sid16(void);
static void i_mov_did16(void);
static void i_rotshft_bd8(void);
static void i_rotshft_wd8(void);
static void i_ret_d16(void);
static void i_ret(void);
static void i_les_dw(void);
static void i_lds_dw(void);
static void i_mov_bd8(void);
static void i_mov_wd16(void);
static void i_enter(void);
static void i_leave(void);
static void i_retf_d16(void);
static void i_retf(void);
static void i_int3(void);
static void i_int(void);
static void i_into(void);
static void i_iret(void);
static void i_rotshft_b(void);
static void i_rotshft_w(void);
static void i_rotshft_bcl(void);
static void i_rotshft_wcl(void);
static void i_aam(void);
static void i_aad(void);
static void i_setalc(void);
static void i_trans(void);
static void i_fpo(void);
static void i_loopne(void);
static void i_loope(void);
static void i_loop(void);
static void i_jcxz(void);
static void i_inal(void);
static void i_inax(void);
static void i_outal(void);
static void i_outax(void);
static void i_call_d16(void);
static void i_jmp_d16(void);
static void i_jmp_far(void);
static void i_jmp_d8(void);
static void i_inaldx(void);
static void i_inaxdx(void);
static void i_outdxal(void);
static void i_outdxax(void);
static void i_lock(void);
static void i_repne(void);
static void i_repe(void);
static void i_hlt(void);
static void i_cmc(void);
static void i_f6pre(void);
static void i_f7pre(void);
static void i_clc(void);
static void i_stc(void);
static void i_di(void);
static void i_ei(void);
static void i_cld(void);
static void i_std(void);
static void i_fepre(void);
static void i_ffpre(void);

static void i_wait(void);

static void (*nec_instruction[256])(void) =
{
    i_add_br8,          /* 0x00 */
    i_add_wr16,         /* 0x01 */
    i_add_r8b,          /* 0x02 */
    i_add_r16w,         /* 0x03 */
    i_add_ald8,         /* 0x04 */
    i_add_axd16,        /* 0x05 */
    i_push_es,          /* 0x06 */
    i_pop_es,           /* 0x07 */
    i_or_br8,           /* 0x08 */
    i_or_wr16,          /* 0x09 */
    i_or_r8b,           /* 0x0a */
    i_or_r16w,          /* 0x0b */
    i_or_ald8,          /* 0x0c */
    i_or_axd16,         /* 0x0d */
    i_push_cs,          /* 0x0e */
	i_pre_nec			/* 0x0f */,
    i_adc_br8,          /* 0x10 */
    i_adc_wr16,         /* 0x11 */
    i_adc_r8b,          /* 0x12 */
    i_adc_r16w,         /* 0x13 */
    i_adc_ald8,         /* 0x14 */
    i_adc_axd16,        /* 0x15 */
    i_push_ss,          /* 0x16 */
    i_pop_ss,           /* 0x17 */
    i_sbb_br8,          /* 0x18 */
    i_sbb_wr16,         /* 0x19 */
    i_sbb_r8b,          /* 0x1a */
    i_sbb_r16w,         /* 0x1b */
    i_sbb_ald8,         /* 0x1c */
    i_sbb_axd16,        /* 0x1d */
    i_push_ds,          /* 0x1e */
    i_pop_ds,           /* 0x1f */
    i_and_br8,          /* 0x20 */
    i_and_wr16,         /* 0x21 */
    i_and_r8b,          /* 0x22 */
    i_and_r16w,         /* 0x23 */
    i_and_ald8,         /* 0x24 */
    i_and_axd16,        /* 0x25 */
    i_es,               /* 0x26 */
    i_daa,              /* 0x27 */
    i_sub_br8,          /* 0x28 */
    i_sub_wr16,         /* 0x29 */
    i_sub_r8b,          /* 0x2a */
    i_sub_r16w,         /* 0x2b */
    i_sub_ald8,         /* 0x2c */
    i_sub_axd16,        /* 0x2d */
    i_cs,               /* 0x2e */
    i_das,		/* 0x2f */
    i_xor_br8,          /* 0x30 */
    i_xor_wr16,         /* 0x31 */
    i_xor_r8b,          /* 0x32 */
    i_xor_r16w,         /* 0x33 */
    i_xor_ald8,         /* 0x34 */
    i_xor_axd16,        /* 0x35 */
    i_ss,               /* 0x36 */
    i_aaa,		/* 0x37 */
    i_cmp_br8,          /* 0x38 */
    i_cmp_wr16,         /* 0x39 */
    i_cmp_r8b,          /* 0x3a */
    i_cmp_r16w,         /* 0x3b */
    i_cmp_ald8,         /* 0x3c */
    i_cmp_axd16,        /* 0x3d */
    i_ds,               /* 0x3e */
    i_aas,		/* 0x3f */
    i_inc_ax,           /* 0x40 */
    i_inc_cx,           /* 0x41 */
    i_inc_dx,           /* 0x42 */
    i_inc_bx,           /* 0x43 */
    i_inc_sp,           /* 0x44 */
    i_inc_bp,           /* 0x45 */
    i_inc_si,           /* 0x46 */
    i_inc_di,           /* 0x47 */
    i_dec_ax,           /* 0x48 */
    i_dec_cx,           /* 0x49 */
    i_dec_dx,           /* 0x4a */
    i_dec_bx,           /* 0x4b */
    i_dec_sp,           /* 0x4c */
    i_dec_bp,           /* 0x4d */
    i_dec_si,           /* 0x4e */
    i_dec_di,           /* 0x4f */
    i_push_ax,          /* 0x50 */
    i_push_cx,          /* 0x51 */
    i_push_dx,          /* 0x52 */
    i_push_bx,          /* 0x53 */
    i_push_sp,          /* 0x54 */
    i_push_bp,          /* 0x55 */
    i_push_si,          /* 0x56 */
    i_push_di,          /* 0x57 */
    i_pop_ax,           /* 0x58 */
    i_pop_cx,           /* 0x59 */
    i_pop_dx,           /* 0x5a */
    i_pop_bx,           /* 0x5b */
    i_pop_sp,           /* 0x5c */
    i_pop_bp,           /* 0x5d */
    i_pop_si,           /* 0x5e */
    i_pop_di,           /* 0x5f */
    i_pusha,            /* 0x60 */
    i_popa,             /* 0x61 */
    i_chkind,           /* 0x62 */
    i_invalid,			/* 0x63 */
    i_repnc,			/* 0x64 */
    i_repc,				/* 0x65 */
    i_invalid,			/* 0x66 */
    i_invalid,			/* 0x67 */
    i_push_d16,         /* 0x68 */
    i_imul_d16,         /* 0x69 */
    i_push_d8,          /* 0x6a */
    i_imul_d8,          /* 0x6b */
    i_insb,             /* 0x6c */
    i_insw,             /* 0x6d */
    i_outsb,            /* 0x6e */
    i_outsw,            /* 0x6f */
    i_jo,               /* 0x70 */
    i_jno,              /* 0x71 */
    i_jc,               /* 0x72 */
    i_jnc,              /* 0x73 */
    i_jz,               /* 0x74 */
    i_jnz,              /* 0x75 */
    i_jce,              /* 0x76 */
    i_jnce,             /* 0x77 */
    i_js,               /* 0x78 */
    i_jns,              /* 0x79 */
    i_jp,               /* 0x7a */
    i_jnp,              /* 0x7b */
    i_jl,               /* 0x7c */
    i_jnl,              /* 0x7d */
    i_jle,              /* 0x7e */
    i_jnle,             /* 0x7f */
    i_80pre,            /* 0x80 */
    i_81pre,            /* 0x81 */
	i_82pre,			/* 0x82 */
    i_83pre,            /* 0x83 */
    i_test_br8,         /* 0x84 */
    i_test_wr16,        /* 0x85 */
    i_xchg_br8,         /* 0x86 */
    i_xchg_wr16,        /* 0x87 */
    i_mov_br8,          /* 0x88 */
    i_mov_wr16,         /* 0x89 */
    i_mov_r8b,          /* 0x8a */
    i_mov_r16w,         /* 0x8b */
    i_mov_wsreg,        /* 0x8c */
    i_lea,              /* 0x8d */
    i_mov_sregw,        /* 0x8e */
    i_popw,             /* 0x8f */
    i_nop,              /* 0x90 */
    i_xchg_axcx,        /* 0x91 */
    i_xchg_axdx,        /* 0x92 */
    i_xchg_axbx,        /* 0x93 */
    i_xchg_axsp,        /* 0x94 */
    i_xchg_axbp,        /* 0x95 */
    i_xchg_axsi,        /* 0x97 */
    i_xchg_axdi,        /* 0x97 */
    i_cbw,              /* 0x98 */
    i_cwd,              /* 0x99 */
    i_call_far,         /* 0x9a */
    i_wait,             /* 0x9b */
    i_pushf,            /* 0x9c */
    i_popf,             /* 0x9d */
    i_sahf,             /* 0x9e */
    i_lahf,             /* 0x9f */
    i_mov_aldisp,       /* 0xa0 */
    i_mov_axdisp,       /* 0xa1 */
    i_mov_dispal,       /* 0xa2 */
    i_mov_dispax,       /* 0xa3 */
    i_movsb,            /* 0xa4 */
    i_movsw,            /* 0xa5 */
    i_cmpsb,            /* 0xa6 */
    i_cmpsw,            /* 0xa7 */
    i_test_ald8,        /* 0xa8 */
    i_test_axd16,       /* 0xa9 */
    i_stosb,            /* 0xaa */
    i_stosw,            /* 0xab */
    i_lodsb,            /* 0xac */
    i_lodsw,            /* 0xad */
    i_scasb,            /* 0xae */
    i_scasw,            /* 0xaf */
    i_mov_ald8,         /* 0xb0 */
    i_mov_cld8,         /* 0xb1 */
    i_mov_dld8,         /* 0xb2 */
    i_mov_bld8,         /* 0xb3 */
    i_mov_ahd8,         /* 0xb4 */
    i_mov_chd8,         /* 0xb5 */
    i_mov_dhd8,         /* 0xb6 */
    i_mov_bhd8,         /* 0xb7 */
    i_mov_axd16,        /* 0xb8 */
    i_mov_cxd16,        /* 0xb9 */
    i_mov_dxd16,        /* 0xba */
    i_mov_bxd16,        /* 0xbb */
    i_mov_spd16,        /* 0xbc */
    i_mov_bpd16,        /* 0xbd */
    i_mov_sid16,        /* 0xbe */
    i_mov_did16,        /* 0xbf */
    i_rotshft_bd8,      /* 0xc0 */
    i_rotshft_wd8,      /* 0xc1 */
    i_ret_d16,          /* 0xc2 */
    i_ret,              /* 0xc3 */
    i_les_dw,           /* 0xc4 */
    i_lds_dw,           /* 0xc5 */
    i_mov_bd8,          /* 0xc6 */
    i_mov_wd16,         /* 0xc7 */
    i_enter,            /* 0xc8 */
    i_leave,            /* 0xc9 */
    i_retf_d16,         /* 0xca */
    i_retf,             /* 0xcb */
    i_int3,             /* 0xcc */
    i_int,              /* 0xcd */
    i_into,             /* 0xce */
    i_iret,             /* 0xcf */
    i_rotshft_b,        /* 0xd0 */
    i_rotshft_w,        /* 0xd1 */
    i_rotshft_bcl,      /* 0xd2 */
    i_rotshft_wcl,      /* 0xd3 */
    i_aam,              /* 0xd4 */
    i_aad,              /* 0xd5 */
    i_setalc,
    i_trans,            /* 0xd7 */
    i_fpo,              /* 0xd8 */
    i_fpo,              /* 0xd9 */
    i_fpo,              /* 0xda */
    i_fpo,              /* 0xdb */
    i_fpo,              /* 0xdc */
    i_fpo,              /* 0xdd */
    i_fpo,              /* 0xde */
    i_fpo,              /* 0xdf */
    i_loopne,           /* 0xe0 */
    i_loope,            /* 0xe1 */
    i_loop,             /* 0xe2 */
    i_jcxz,             /* 0xe3 */
    i_inal,             /* 0xe4 */
    i_inax,             /* 0xe5 */
    i_outal,            /* 0xe6 */
    i_outax,            /* 0xe7 */
    i_call_d16,         /* 0xe8 */
    i_jmp_d16,          /* 0xe9 */
    i_jmp_far,          /* 0xea */
    i_jmp_d8,           /* 0xeb */
    i_inaldx,           /* 0xec */
    i_inaxdx,           /* 0xed */
    i_outdxal,          /* 0xee */
    i_outdxax,          /* 0xef */
    i_lock,             /* 0xf0 */
    i_invalid,          /* 0xf1 */
    i_repne,            /* 0xf2 */
    i_repe,             /* 0xf3 */
    i_hlt,				/* 0xf4 */
    i_cmc,              /* 0xf5 */
    i_f6pre,            /* 0xf6 */
    i_f7pre,            /* 0xf7 */
    i_clc,              /* 0xf8 */
    i_stc,              /* 0xf9 */
    i_di,              /* 0xfa */
    i_ei,              /* 0xfb */
    i_cld,              /* 0xfc */
    i_std,              /* 0xfd */
    i_fepre,            /* 0xfe */
    i_ffpre             /* 0xff */
};
