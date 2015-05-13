// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/* tmp1 value for ini/inir/outi/otir for [C.1-0][io.1-0] */
static const UINT8 irep_tmp1[4][4] = {
	{0,0,1,0},{0,1,0,1},{1,0,1,1},{0,1,1,0}
};

/* tmp1 value for ind/indr/outd/otdr for [C.1-0][io.1-0] */
static const UINT8 drep_tmp1[4][4] = {
	{0,1,0,0},{1,0,0,1},{0,0,1,0},{0,1,0,1}
};

/* tmp2 value for all in/out repeated opcodes for B.7-0 */
static const UINT8 breg_tmp2[256] = {
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,
	0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,
	1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1
};

static const UINT8 cc_op[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	3, 9, 7, 4, 4, 4, 6, 3, 4, 7, 6, 4, 4, 4, 6, 3,
	7, 9, 7, 4, 4, 4, 6, 3, 8, 7, 6, 4, 4, 4, 6, 3,
	6, 9,16, 4, 4, 4, 6, 4, 6, 7,15, 4, 4, 4, 6, 3,
	6, 9,13, 4,10,10, 9, 3, 6, 7,12, 4, 4, 4, 6, 3,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	7, 7, 7, 7, 7, 7, 3, 7, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 6, 4,
	5, 9, 6, 9, 6,11, 6,11, 5, 9, 6, 0, 6,16, 6,11,
	5, 9, 6,10, 6,11, 6,11, 5, 3, 6, 9, 6, 0, 6,11,
	5, 9, 6,16, 6,11, 6,11, 5, 3, 6, 3, 6, 0, 6,11,
	5, 9, 6, 3, 6,11, 6,11, 5, 4, 6, 3, 6, 0, 6,11
};

static const UINT8 cc_cb[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	6, 6, 6, 6, 6, 6, 9, 6, 6, 6, 6, 6, 6, 6, 9, 6,
	6, 6, 6, 6, 6, 6, 9, 6, 6, 6, 6, 6, 6, 6, 9, 6,
	6, 6, 6, 6, 6, 6, 9, 6, 6, 6, 6, 6, 6, 6, 9, 6,
	6, 6, 6, 6, 6, 6, 9, 6, 6, 6, 6, 6, 6, 6, 9, 6,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7,
	7, 7, 7, 7, 7, 7,13, 7, 7, 7, 7, 7, 7, 7,13, 7
};

static const UINT8 cc_ed[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	12,13, 6, 6, 9, 6, 6, 6,12,13, 6, 6, 9, 6, 6, 6,
	12,13, 6, 6, 9, 6, 6, 6,12,13, 6, 6, 9, 6, 6, 6,
	12,13, 6, 6, 9, 6, 6, 6,12,13, 6, 6,10, 6, 6, 6,
	12,13, 6, 6, 9, 6, 6, 6,12,13, 6, 6, 9, 6, 6, 6,
	9,10,10,19, 6,12, 6, 6, 9,10,10,18,17,12, 6, 6,
	9,10,10,19, 6,12, 6, 6, 9,10,10,18,17,12, 6, 6,
	9,10,10,19, 6,12, 6,16, 9,10,10,18,17,12, 6,16,
	9,10,10,19,12,12, 8, 6, 9,10,10,18,17,12, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	12,12,12,12, 6, 6, 6, 6,12,12,12,12, 6, 6, 6, 6,
	12,12,12,12, 6, 6, 6, 6,12,12,12,12, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};

static const UINT8 cc_xy[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4,
	4,12,19, 7, 9, 9,15, 4, 4,10,18, 7, 9, 9, 9, 4,
	4, 4, 4, 4,18,18,15, 4, 4,10, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	9, 9, 9, 9, 9, 9,14, 9, 9, 9, 9, 9, 9, 9,14, 9,
	15,15,15,15,15,15, 4,15, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 9, 9,14, 4, 4, 4, 4, 4, 9, 9,14, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4,12, 4,19, 4,14, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4
};

static const UINT8 cc_xycb[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* DJNZ */
	2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,   /* JR NZ/JR Z */
	2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,   /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0,   /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
	5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
	5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
	5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2
};

static const UINT8 *const cc_default[6] = { cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex };

#define Z180_TABLE_dd    Z180_TABLE_xy
#define Z180_TABLE_fd    Z180_TABLE_xy


#define TABLE(prefix) {\
	&z180_device::prefix##_00,&z180_device::prefix##_01,&z180_device::prefix##_02,&z180_device::prefix##_03,&z180_device::prefix##_04,&z180_device::prefix##_05,&z180_device::prefix##_06,&z180_device::prefix##_07, \
	&z180_device::prefix##_08,&z180_device::prefix##_09,&z180_device::prefix##_0a,&z180_device::prefix##_0b,&z180_device::prefix##_0c,&z180_device::prefix##_0d,&z180_device::prefix##_0e,&z180_device::prefix##_0f, \
	&z180_device::prefix##_10,&z180_device::prefix##_11,&z180_device::prefix##_12,&z180_device::prefix##_13,&z180_device::prefix##_14,&z180_device::prefix##_15,&z180_device::prefix##_16,&z180_device::prefix##_17, \
	&z180_device::prefix##_18,&z180_device::prefix##_19,&z180_device::prefix##_1a,&z180_device::prefix##_1b,&z180_device::prefix##_1c,&z180_device::prefix##_1d,&z180_device::prefix##_1e,&z180_device::prefix##_1f, \
	&z180_device::prefix##_20,&z180_device::prefix##_21,&z180_device::prefix##_22,&z180_device::prefix##_23,&z180_device::prefix##_24,&z180_device::prefix##_25,&z180_device::prefix##_26,&z180_device::prefix##_27, \
	&z180_device::prefix##_28,&z180_device::prefix##_29,&z180_device::prefix##_2a,&z180_device::prefix##_2b,&z180_device::prefix##_2c,&z180_device::prefix##_2d,&z180_device::prefix##_2e,&z180_device::prefix##_2f, \
	&z180_device::prefix##_30,&z180_device::prefix##_31,&z180_device::prefix##_32,&z180_device::prefix##_33,&z180_device::prefix##_34,&z180_device::prefix##_35,&z180_device::prefix##_36,&z180_device::prefix##_37, \
	&z180_device::prefix##_38,&z180_device::prefix##_39,&z180_device::prefix##_3a,&z180_device::prefix##_3b,&z180_device::prefix##_3c,&z180_device::prefix##_3d,&z180_device::prefix##_3e,&z180_device::prefix##_3f, \
	&z180_device::prefix##_40,&z180_device::prefix##_41,&z180_device::prefix##_42,&z180_device::prefix##_43,&z180_device::prefix##_44,&z180_device::prefix##_45,&z180_device::prefix##_46,&z180_device::prefix##_47, \
	&z180_device::prefix##_48,&z180_device::prefix##_49,&z180_device::prefix##_4a,&z180_device::prefix##_4b,&z180_device::prefix##_4c,&z180_device::prefix##_4d,&z180_device::prefix##_4e,&z180_device::prefix##_4f, \
	&z180_device::prefix##_50,&z180_device::prefix##_51,&z180_device::prefix##_52,&z180_device::prefix##_53,&z180_device::prefix##_54,&z180_device::prefix##_55,&z180_device::prefix##_56,&z180_device::prefix##_57, \
	&z180_device::prefix##_58,&z180_device::prefix##_59,&z180_device::prefix##_5a,&z180_device::prefix##_5b,&z180_device::prefix##_5c,&z180_device::prefix##_5d,&z180_device::prefix##_5e,&z180_device::prefix##_5f, \
	&z180_device::prefix##_60,&z180_device::prefix##_61,&z180_device::prefix##_62,&z180_device::prefix##_63,&z180_device::prefix##_64,&z180_device::prefix##_65,&z180_device::prefix##_66,&z180_device::prefix##_67, \
	&z180_device::prefix##_68,&z180_device::prefix##_69,&z180_device::prefix##_6a,&z180_device::prefix##_6b,&z180_device::prefix##_6c,&z180_device::prefix##_6d,&z180_device::prefix##_6e,&z180_device::prefix##_6f, \
	&z180_device::prefix##_70,&z180_device::prefix##_71,&z180_device::prefix##_72,&z180_device::prefix##_73,&z180_device::prefix##_74,&z180_device::prefix##_75,&z180_device::prefix##_76,&z180_device::prefix##_77, \
	&z180_device::prefix##_78,&z180_device::prefix##_79,&z180_device::prefix##_7a,&z180_device::prefix##_7b,&z180_device::prefix##_7c,&z180_device::prefix##_7d,&z180_device::prefix##_7e,&z180_device::prefix##_7f, \
	&z180_device::prefix##_80,&z180_device::prefix##_81,&z180_device::prefix##_82,&z180_device::prefix##_83,&z180_device::prefix##_84,&z180_device::prefix##_85,&z180_device::prefix##_86,&z180_device::prefix##_87, \
	&z180_device::prefix##_88,&z180_device::prefix##_89,&z180_device::prefix##_8a,&z180_device::prefix##_8b,&z180_device::prefix##_8c,&z180_device::prefix##_8d,&z180_device::prefix##_8e,&z180_device::prefix##_8f, \
	&z180_device::prefix##_90,&z180_device::prefix##_91,&z180_device::prefix##_92,&z180_device::prefix##_93,&z180_device::prefix##_94,&z180_device::prefix##_95,&z180_device::prefix##_96,&z180_device::prefix##_97, \
	&z180_device::prefix##_98,&z180_device::prefix##_99,&z180_device::prefix##_9a,&z180_device::prefix##_9b,&z180_device::prefix##_9c,&z180_device::prefix##_9d,&z180_device::prefix##_9e,&z180_device::prefix##_9f, \
	&z180_device::prefix##_a0,&z180_device::prefix##_a1,&z180_device::prefix##_a2,&z180_device::prefix##_a3,&z180_device::prefix##_a4,&z180_device::prefix##_a5,&z180_device::prefix##_a6,&z180_device::prefix##_a7, \
	&z180_device::prefix##_a8,&z180_device::prefix##_a9,&z180_device::prefix##_aa,&z180_device::prefix##_ab,&z180_device::prefix##_ac,&z180_device::prefix##_ad,&z180_device::prefix##_ae,&z180_device::prefix##_af, \
	&z180_device::prefix##_b0,&z180_device::prefix##_b1,&z180_device::prefix##_b2,&z180_device::prefix##_b3,&z180_device::prefix##_b4,&z180_device::prefix##_b5,&z180_device::prefix##_b6,&z180_device::prefix##_b7, \
	&z180_device::prefix##_b8,&z180_device::prefix##_b9,&z180_device::prefix##_ba,&z180_device::prefix##_bb,&z180_device::prefix##_bc,&z180_device::prefix##_bd,&z180_device::prefix##_be,&z180_device::prefix##_bf, \
	&z180_device::prefix##_c0,&z180_device::prefix##_c1,&z180_device::prefix##_c2,&z180_device::prefix##_c3,&z180_device::prefix##_c4,&z180_device::prefix##_c5,&z180_device::prefix##_c6,&z180_device::prefix##_c7, \
	&z180_device::prefix##_c8,&z180_device::prefix##_c9,&z180_device::prefix##_ca,&z180_device::prefix##_cb,&z180_device::prefix##_cc,&z180_device::prefix##_cd,&z180_device::prefix##_ce,&z180_device::prefix##_cf, \
	&z180_device::prefix##_d0,&z180_device::prefix##_d1,&z180_device::prefix##_d2,&z180_device::prefix##_d3,&z180_device::prefix##_d4,&z180_device::prefix##_d5,&z180_device::prefix##_d6,&z180_device::prefix##_d7, \
	&z180_device::prefix##_d8,&z180_device::prefix##_d9,&z180_device::prefix##_da,&z180_device::prefix##_db,&z180_device::prefix##_dc,&z180_device::prefix##_dd,&z180_device::prefix##_de,&z180_device::prefix##_df, \
	&z180_device::prefix##_e0,&z180_device::prefix##_e1,&z180_device::prefix##_e2,&z180_device::prefix##_e3,&z180_device::prefix##_e4,&z180_device::prefix##_e5,&z180_device::prefix##_e6,&z180_device::prefix##_e7, \
	&z180_device::prefix##_e8,&z180_device::prefix##_e9,&z180_device::prefix##_ea,&z180_device::prefix##_eb,&z180_device::prefix##_ec,&z180_device::prefix##_ed,&z180_device::prefix##_ee,&z180_device::prefix##_ef, \
	&z180_device::prefix##_f0,&z180_device::prefix##_f1,&z180_device::prefix##_f2,&z180_device::prefix##_f3,&z180_device::prefix##_f4,&z180_device::prefix##_f5,&z180_device::prefix##_f6,&z180_device::prefix##_f7, \
	&z180_device::prefix##_f8,&z180_device::prefix##_f9,&z180_device::prefix##_fa,&z180_device::prefix##_fb,&z180_device::prefix##_fc,&z180_device::prefix##_fd,&z180_device::prefix##_fe,&z180_device::prefix##_ff  \
}


const z180_device::opcode_func z180_device::s_z180ops[Z180_PREFIX_COUNT][0x100] =
{
	TABLE(op),
	TABLE(cb),
	TABLE(dd),
	TABLE(ed),
	TABLE(fd),
	TABLE(xycb)
};

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  void z180_device::prefix##_##opcode()

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) m_extra_cycles += m_cc[Z180_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/

#define EXEC_PROTOTYPE(prefix) \
int z180_device::exec##_##prefix(const UINT8 opcode)    \
{                                                                       \
	(this->*s_z180ops[Z180_PREFIX_##prefix][opcode])();                                     \
	return m_cc[Z180_TABLE_##prefix][opcode];                   \
}

EXEC_PROTOTYPE(op)
EXEC_PROTOTYPE(cb)
EXEC_PROTOTYPE(dd)
EXEC_PROTOTYPE(ed)
EXEC_PROTOTYPE(fd)
EXEC_PROTOTYPE(xycb)
