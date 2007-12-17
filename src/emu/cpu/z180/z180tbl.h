#if Z180_EXACT
/* tmp1 value for ini/inir/outi/otir for [C.1-0][io.1-0] */
static UINT8 irep_tmp1[4][4] = {
	{0,0,1,0},{0,1,0,1},{1,0,1,1},{0,1,1,0}
};

/* tmp1 value for ind/indr/outd/otdr for [C.1-0][io.1-0] */
static UINT8 drep_tmp1[4][4] = {
	{0,1,0,0},{1,0,0,1},{0,0,1,0},{0,1,0,1}
};

/* tmp2 value for all in/out repeated opcodes for B.7-0 */
static UINT8 breg_tmp2[256] = {
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
#endif

static UINT8 cc_op[0x100] = {
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

static UINT8 cc_cb[0x100] = {
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

static UINT8 cc_ed[0x100] = {
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

static UINT8 cc_xy[0x100] = {
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

static UINT8 cc_xycb[0x100] = {
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
static UINT8 cc_ex[0x100] = {
/*-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -a -b -c -d -e -f */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	  /* DJNZ */
   2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,	  /* JR NZ/JR Z */
   2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,	  /* JR NC/JR C */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0,	  /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
   5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
   5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
   5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2,
   5, 0, 3, 0,10, 0, 0, 2, 5, 0, 3, 0,10, 0, 0, 2
};

static UINT8 *cc[6] = { cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex };
#define Z180_TABLE_dd	 Z180_TABLE_xy
#define Z180_TABLE_fd	 Z180_TABLE_xy

static void take_interrupt(int irqline);

#define PROTOTYPES(tablename,prefix) \
	INLINE void prefix##_00(void); INLINE void prefix##_01(void); INLINE void prefix##_02(void); INLINE void prefix##_03(void); \
	INLINE void prefix##_04(void); INLINE void prefix##_05(void); INLINE void prefix##_06(void); INLINE void prefix##_07(void); \
	INLINE void prefix##_08(void); INLINE void prefix##_09(void); INLINE void prefix##_0a(void); INLINE void prefix##_0b(void); \
	INLINE void prefix##_0c(void); INLINE void prefix##_0d(void); INLINE void prefix##_0e(void); INLINE void prefix##_0f(void); \
	INLINE void prefix##_10(void); INLINE void prefix##_11(void); INLINE void prefix##_12(void); INLINE void prefix##_13(void); \
	INLINE void prefix##_14(void); INLINE void prefix##_15(void); INLINE void prefix##_16(void); INLINE void prefix##_17(void); \
	INLINE void prefix##_18(void); INLINE void prefix##_19(void); INLINE void prefix##_1a(void); INLINE void prefix##_1b(void); \
	INLINE void prefix##_1c(void); INLINE void prefix##_1d(void); INLINE void prefix##_1e(void); INLINE void prefix##_1f(void); \
	INLINE void prefix##_20(void); INLINE void prefix##_21(void); INLINE void prefix##_22(void); INLINE void prefix##_23(void); \
	INLINE void prefix##_24(void); INLINE void prefix##_25(void); INLINE void prefix##_26(void); INLINE void prefix##_27(void); \
	INLINE void prefix##_28(void); INLINE void prefix##_29(void); INLINE void prefix##_2a(void); INLINE void prefix##_2b(void); \
	INLINE void prefix##_2c(void); INLINE void prefix##_2d(void); INLINE void prefix##_2e(void); INLINE void prefix##_2f(void); \
	INLINE void prefix##_30(void); INLINE void prefix##_31(void); INLINE void prefix##_32(void); INLINE void prefix##_33(void); \
	INLINE void prefix##_34(void); INLINE void prefix##_35(void); INLINE void prefix##_36(void); INLINE void prefix##_37(void); \
	INLINE void prefix##_38(void); INLINE void prefix##_39(void); INLINE void prefix##_3a(void); INLINE void prefix##_3b(void); \
	INLINE void prefix##_3c(void); INLINE void prefix##_3d(void); INLINE void prefix##_3e(void); INLINE void prefix##_3f(void); \
	INLINE void prefix##_40(void); INLINE void prefix##_41(void); INLINE void prefix##_42(void); INLINE void prefix##_43(void); \
	INLINE void prefix##_44(void); INLINE void prefix##_45(void); INLINE void prefix##_46(void); INLINE void prefix##_47(void); \
	INLINE void prefix##_48(void); INLINE void prefix##_49(void); INLINE void prefix##_4a(void); INLINE void prefix##_4b(void); \
	INLINE void prefix##_4c(void); INLINE void prefix##_4d(void); INLINE void prefix##_4e(void); INLINE void prefix##_4f(void); \
	INLINE void prefix##_50(void); INLINE void prefix##_51(void); INLINE void prefix##_52(void); INLINE void prefix##_53(void); \
	INLINE void prefix##_54(void); INLINE void prefix##_55(void); INLINE void prefix##_56(void); INLINE void prefix##_57(void); \
	INLINE void prefix##_58(void); INLINE void prefix##_59(void); INLINE void prefix##_5a(void); INLINE void prefix##_5b(void); \
	INLINE void prefix##_5c(void); INLINE void prefix##_5d(void); INLINE void prefix##_5e(void); INLINE void prefix##_5f(void); \
	INLINE void prefix##_60(void); INLINE void prefix##_61(void); INLINE void prefix##_62(void); INLINE void prefix##_63(void); \
	INLINE void prefix##_64(void); INLINE void prefix##_65(void); INLINE void prefix##_66(void); INLINE void prefix##_67(void); \
	INLINE void prefix##_68(void); INLINE void prefix##_69(void); INLINE void prefix##_6a(void); INLINE void prefix##_6b(void); \
	INLINE void prefix##_6c(void); INLINE void prefix##_6d(void); INLINE void prefix##_6e(void); INLINE void prefix##_6f(void); \
	INLINE void prefix##_70(void); INLINE void prefix##_71(void); INLINE void prefix##_72(void); INLINE void prefix##_73(void); \
	INLINE void prefix##_74(void); INLINE void prefix##_75(void); INLINE void prefix##_76(void); INLINE void prefix##_77(void); \
	INLINE void prefix##_78(void); INLINE void prefix##_79(void); INLINE void prefix##_7a(void); INLINE void prefix##_7b(void); \
	INLINE void prefix##_7c(void); INLINE void prefix##_7d(void); INLINE void prefix##_7e(void); INLINE void prefix##_7f(void); \
	INLINE void prefix##_80(void); INLINE void prefix##_81(void); INLINE void prefix##_82(void); INLINE void prefix##_83(void); \
	INLINE void prefix##_84(void); INLINE void prefix##_85(void); INLINE void prefix##_86(void); INLINE void prefix##_87(void); \
	INLINE void prefix##_88(void); INLINE void prefix##_89(void); INLINE void prefix##_8a(void); INLINE void prefix##_8b(void); \
	INLINE void prefix##_8c(void); INLINE void prefix##_8d(void); INLINE void prefix##_8e(void); INLINE void prefix##_8f(void); \
	INLINE void prefix##_90(void); INLINE void prefix##_91(void); INLINE void prefix##_92(void); INLINE void prefix##_93(void); \
	INLINE void prefix##_94(void); INLINE void prefix##_95(void); INLINE void prefix##_96(void); INLINE void prefix##_97(void); \
	INLINE void prefix##_98(void); INLINE void prefix##_99(void); INLINE void prefix##_9a(void); INLINE void prefix##_9b(void); \
	INLINE void prefix##_9c(void); INLINE void prefix##_9d(void); INLINE void prefix##_9e(void); INLINE void prefix##_9f(void); \
	INLINE void prefix##_a0(void); INLINE void prefix##_a1(void); INLINE void prefix##_a2(void); INLINE void prefix##_a3(void); \
	INLINE void prefix##_a4(void); INLINE void prefix##_a5(void); INLINE void prefix##_a6(void); INLINE void prefix##_a7(void); \
	INLINE void prefix##_a8(void); INLINE void prefix##_a9(void); INLINE void prefix##_aa(void); INLINE void prefix##_ab(void); \
	INLINE void prefix##_ac(void); INLINE void prefix##_ad(void); INLINE void prefix##_ae(void); INLINE void prefix##_af(void); \
	INLINE void prefix##_b0(void); INLINE void prefix##_b1(void); INLINE void prefix##_b2(void); INLINE void prefix##_b3(void); \
	INLINE void prefix##_b4(void); INLINE void prefix##_b5(void); INLINE void prefix##_b6(void); INLINE void prefix##_b7(void); \
	INLINE void prefix##_b8(void); INLINE void prefix##_b9(void); INLINE void prefix##_ba(void); INLINE void prefix##_bb(void); \
	INLINE void prefix##_bc(void); INLINE void prefix##_bd(void); INLINE void prefix##_be(void); INLINE void prefix##_bf(void); \
	INLINE void prefix##_c0(void); INLINE void prefix##_c1(void); INLINE void prefix##_c2(void); INLINE void prefix##_c3(void); \
	INLINE void prefix##_c4(void); INLINE void prefix##_c5(void); INLINE void prefix##_c6(void); INLINE void prefix##_c7(void); \
	INLINE void prefix##_c8(void); INLINE void prefix##_c9(void); INLINE void prefix##_ca(void); INLINE void prefix##_cb(void); \
	INLINE void prefix##_cc(void); INLINE void prefix##_cd(void); INLINE void prefix##_ce(void); INLINE void prefix##_cf(void); \
	INLINE void prefix##_d0(void); INLINE void prefix##_d1(void); INLINE void prefix##_d2(void); INLINE void prefix##_d3(void); \
	INLINE void prefix##_d4(void); INLINE void prefix##_d5(void); INLINE void prefix##_d6(void); INLINE void prefix##_d7(void); \
	INLINE void prefix##_d8(void); INLINE void prefix##_d9(void); INLINE void prefix##_da(void); INLINE void prefix##_db(void); \
	INLINE void prefix##_dc(void); INLINE void prefix##_dd(void); INLINE void prefix##_de(void); INLINE void prefix##_df(void); \
	INLINE void prefix##_e0(void); INLINE void prefix##_e1(void); INLINE void prefix##_e2(void); INLINE void prefix##_e3(void); \
	INLINE void prefix##_e4(void); INLINE void prefix##_e5(void); INLINE void prefix##_e6(void); INLINE void prefix##_e7(void); \
	INLINE void prefix##_e8(void); INLINE void prefix##_e9(void); INLINE void prefix##_ea(void); INLINE void prefix##_eb(void); \
	INLINE void prefix##_ec(void); INLINE void prefix##_ed(void); INLINE void prefix##_ee(void); INLINE void prefix##_ef(void); \
	INLINE void prefix##_f0(void); INLINE void prefix##_f1(void); INLINE void prefix##_f2(void); INLINE void prefix##_f3(void); \
	INLINE void prefix##_f4(void); INLINE void prefix##_f5(void); INLINE void prefix##_f6(void); INLINE void prefix##_f7(void); \
	INLINE void prefix##_f8(void); INLINE void prefix##_f9(void); INLINE void prefix##_fa(void); INLINE void prefix##_fb(void); \
	INLINE void prefix##_fc(void); INLINE void prefix##_fd(void); INLINE void prefix##_fe(void); INLINE void prefix##_ff(void); \
static void (*tablename[0x100])(void) = {	\
	prefix##_00,prefix##_01,prefix##_02,prefix##_03,prefix##_04,prefix##_05,prefix##_06,prefix##_07, \
	prefix##_08,prefix##_09,prefix##_0a,prefix##_0b,prefix##_0c,prefix##_0d,prefix##_0e,prefix##_0f, \
	prefix##_10,prefix##_11,prefix##_12,prefix##_13,prefix##_14,prefix##_15,prefix##_16,prefix##_17, \
	prefix##_18,prefix##_19,prefix##_1a,prefix##_1b,prefix##_1c,prefix##_1d,prefix##_1e,prefix##_1f, \
	prefix##_20,prefix##_21,prefix##_22,prefix##_23,prefix##_24,prefix##_25,prefix##_26,prefix##_27, \
	prefix##_28,prefix##_29,prefix##_2a,prefix##_2b,prefix##_2c,prefix##_2d,prefix##_2e,prefix##_2f, \
	prefix##_30,prefix##_31,prefix##_32,prefix##_33,prefix##_34,prefix##_35,prefix##_36,prefix##_37, \
	prefix##_38,prefix##_39,prefix##_3a,prefix##_3b,prefix##_3c,prefix##_3d,prefix##_3e,prefix##_3f, \
	prefix##_40,prefix##_41,prefix##_42,prefix##_43,prefix##_44,prefix##_45,prefix##_46,prefix##_47, \
	prefix##_48,prefix##_49,prefix##_4a,prefix##_4b,prefix##_4c,prefix##_4d,prefix##_4e,prefix##_4f, \
	prefix##_50,prefix##_51,prefix##_52,prefix##_53,prefix##_54,prefix##_55,prefix##_56,prefix##_57, \
	prefix##_58,prefix##_59,prefix##_5a,prefix##_5b,prefix##_5c,prefix##_5d,prefix##_5e,prefix##_5f, \
	prefix##_60,prefix##_61,prefix##_62,prefix##_63,prefix##_64,prefix##_65,prefix##_66,prefix##_67, \
	prefix##_68,prefix##_69,prefix##_6a,prefix##_6b,prefix##_6c,prefix##_6d,prefix##_6e,prefix##_6f, \
	prefix##_70,prefix##_71,prefix##_72,prefix##_73,prefix##_74,prefix##_75,prefix##_76,prefix##_77, \
	prefix##_78,prefix##_79,prefix##_7a,prefix##_7b,prefix##_7c,prefix##_7d,prefix##_7e,prefix##_7f, \
	prefix##_80,prefix##_81,prefix##_82,prefix##_83,prefix##_84,prefix##_85,prefix##_86,prefix##_87, \
	prefix##_88,prefix##_89,prefix##_8a,prefix##_8b,prefix##_8c,prefix##_8d,prefix##_8e,prefix##_8f, \
	prefix##_90,prefix##_91,prefix##_92,prefix##_93,prefix##_94,prefix##_95,prefix##_96,prefix##_97, \
	prefix##_98,prefix##_99,prefix##_9a,prefix##_9b,prefix##_9c,prefix##_9d,prefix##_9e,prefix##_9f, \
	prefix##_a0,prefix##_a1,prefix##_a2,prefix##_a3,prefix##_a4,prefix##_a5,prefix##_a6,prefix##_a7, \
	prefix##_a8,prefix##_a9,prefix##_aa,prefix##_ab,prefix##_ac,prefix##_ad,prefix##_ae,prefix##_af, \
	prefix##_b0,prefix##_b1,prefix##_b2,prefix##_b3,prefix##_b4,prefix##_b5,prefix##_b6,prefix##_b7, \
	prefix##_b8,prefix##_b9,prefix##_ba,prefix##_bb,prefix##_bc,prefix##_bd,prefix##_be,prefix##_bf, \
	prefix##_c0,prefix##_c1,prefix##_c2,prefix##_c3,prefix##_c4,prefix##_c5,prefix##_c6,prefix##_c7, \
	prefix##_c8,prefix##_c9,prefix##_ca,prefix##_cb,prefix##_cc,prefix##_cd,prefix##_ce,prefix##_cf, \
	prefix##_d0,prefix##_d1,prefix##_d2,prefix##_d3,prefix##_d4,prefix##_d5,prefix##_d6,prefix##_d7, \
	prefix##_d8,prefix##_d9,prefix##_da,prefix##_db,prefix##_dc,prefix##_dd,prefix##_de,prefix##_df, \
	prefix##_e0,prefix##_e1,prefix##_e2,prefix##_e3,prefix##_e4,prefix##_e5,prefix##_e6,prefix##_e7, \
	prefix##_e8,prefix##_e9,prefix##_ea,prefix##_eb,prefix##_ec,prefix##_ed,prefix##_ee,prefix##_ef, \
	prefix##_f0,prefix##_f1,prefix##_f2,prefix##_f3,prefix##_f4,prefix##_f5,prefix##_f6,prefix##_f7, \
	prefix##_f8,prefix##_f9,prefix##_fa,prefix##_fb,prefix##_fc,prefix##_fd,prefix##_fe,prefix##_ff  \
}

PROTOTYPES(Z180op,op);
PROTOTYPES(Z180cb,cb);
PROTOTYPES(Z180dd,dd);
PROTOTYPES(Z180ed,ed);
PROTOTYPES(Z180fd,fd);
PROTOTYPES(Z180xycb,xycb);

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(void)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) z180_icount -= cc[Z180_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(prefix,opcode)                                     \
{                                                               \
    unsigned op = opcode;                                       \
    CC(prefix,op);                                              \
    (*Z180##prefix[op])();                                      \
}

#if BIG_SWITCH
#define EXEC_INLINE(prefix,opcode)                              \
{                                                               \
    unsigned op = opcode;                                       \
    CC(prefix,op);                                              \
    switch(op)                                                  \
    {                                                           \
    case 0x00:prefix##_##00();break; case 0x01:prefix##_##01();break; case 0x02:prefix##_##02();break; case 0x03:prefix##_##03();break; \
    case 0x04:prefix##_##04();break; case 0x05:prefix##_##05();break; case 0x06:prefix##_##06();break; case 0x07:prefix##_##07();break; \
    case 0x08:prefix##_##08();break; case 0x09:prefix##_##09();break; case 0x0a:prefix##_##0a();break; case 0x0b:prefix##_##0b();break; \
    case 0x0c:prefix##_##0c();break; case 0x0d:prefix##_##0d();break; case 0x0e:prefix##_##0e();break; case 0x0f:prefix##_##0f();break; \
    case 0x10:prefix##_##10();break; case 0x11:prefix##_##11();break; case 0x12:prefix##_##12();break; case 0x13:prefix##_##13();break; \
    case 0x14:prefix##_##14();break; case 0x15:prefix##_##15();break; case 0x16:prefix##_##16();break; case 0x17:prefix##_##17();break; \
    case 0x18:prefix##_##18();break; case 0x19:prefix##_##19();break; case 0x1a:prefix##_##1a();break; case 0x1b:prefix##_##1b();break; \
    case 0x1c:prefix##_##1c();break; case 0x1d:prefix##_##1d();break; case 0x1e:prefix##_##1e();break; case 0x1f:prefix##_##1f();break; \
    case 0x20:prefix##_##20();break; case 0x21:prefix##_##21();break; case 0x22:prefix##_##22();break; case 0x23:prefix##_##23();break; \
    case 0x24:prefix##_##24();break; case 0x25:prefix##_##25();break; case 0x26:prefix##_##26();break; case 0x27:prefix##_##27();break; \
    case 0x28:prefix##_##28();break; case 0x29:prefix##_##29();break; case 0x2a:prefix##_##2a();break; case 0x2b:prefix##_##2b();break; \
    case 0x2c:prefix##_##2c();break; case 0x2d:prefix##_##2d();break; case 0x2e:prefix##_##2e();break; case 0x2f:prefix##_##2f();break; \
    case 0x30:prefix##_##30();break; case 0x31:prefix##_##31();break; case 0x32:prefix##_##32();break; case 0x33:prefix##_##33();break; \
    case 0x34:prefix##_##34();break; case 0x35:prefix##_##35();break; case 0x36:prefix##_##36();break; case 0x37:prefix##_##37();break; \
    case 0x38:prefix##_##38();break; case 0x39:prefix##_##39();break; case 0x3a:prefix##_##3a();break; case 0x3b:prefix##_##3b();break; \
    case 0x3c:prefix##_##3c();break; case 0x3d:prefix##_##3d();break; case 0x3e:prefix##_##3e();break; case 0x3f:prefix##_##3f();break; \
    case 0x40:prefix##_##40();break; case 0x41:prefix##_##41();break; case 0x42:prefix##_##42();break; case 0x43:prefix##_##43();break; \
    case 0x44:prefix##_##44();break; case 0x45:prefix##_##45();break; case 0x46:prefix##_##46();break; case 0x47:prefix##_##47();break; \
    case 0x48:prefix##_##48();break; case 0x49:prefix##_##49();break; case 0x4a:prefix##_##4a();break; case 0x4b:prefix##_##4b();break; \
    case 0x4c:prefix##_##4c();break; case 0x4d:prefix##_##4d();break; case 0x4e:prefix##_##4e();break; case 0x4f:prefix##_##4f();break; \
    case 0x50:prefix##_##50();break; case 0x51:prefix##_##51();break; case 0x52:prefix##_##52();break; case 0x53:prefix##_##53();break; \
    case 0x54:prefix##_##54();break; case 0x55:prefix##_##55();break; case 0x56:prefix##_##56();break; case 0x57:prefix##_##57();break; \
    case 0x58:prefix##_##58();break; case 0x59:prefix##_##59();break; case 0x5a:prefix##_##5a();break; case 0x5b:prefix##_##5b();break; \
    case 0x5c:prefix##_##5c();break; case 0x5d:prefix##_##5d();break; case 0x5e:prefix##_##5e();break; case 0x5f:prefix##_##5f();break; \
    case 0x60:prefix##_##60();break; case 0x61:prefix##_##61();break; case 0x62:prefix##_##62();break; case 0x63:prefix##_##63();break; \
    case 0x64:prefix##_##64();break; case 0x65:prefix##_##65();break; case 0x66:prefix##_##66();break; case 0x67:prefix##_##67();break; \
    case 0x68:prefix##_##68();break; case 0x69:prefix##_##69();break; case 0x6a:prefix##_##6a();break; case 0x6b:prefix##_##6b();break; \
    case 0x6c:prefix##_##6c();break; case 0x6d:prefix##_##6d();break; case 0x6e:prefix##_##6e();break; case 0x6f:prefix##_##6f();break; \
    case 0x70:prefix##_##70();break; case 0x71:prefix##_##71();break; case 0x72:prefix##_##72();break; case 0x73:prefix##_##73();break; \
    case 0x74:prefix##_##74();break; case 0x75:prefix##_##75();break; case 0x76:prefix##_##76();break; case 0x77:prefix##_##77();break; \
    case 0x78:prefix##_##78();break; case 0x79:prefix##_##79();break; case 0x7a:prefix##_##7a();break; case 0x7b:prefix##_##7b();break; \
    case 0x7c:prefix##_##7c();break; case 0x7d:prefix##_##7d();break; case 0x7e:prefix##_##7e();break; case 0x7f:prefix##_##7f();break; \
    case 0x80:prefix##_##80();break; case 0x81:prefix##_##81();break; case 0x82:prefix##_##82();break; case 0x83:prefix##_##83();break; \
    case 0x84:prefix##_##84();break; case 0x85:prefix##_##85();break; case 0x86:prefix##_##86();break; case 0x87:prefix##_##87();break; \
    case 0x88:prefix##_##88();break; case 0x89:prefix##_##89();break; case 0x8a:prefix##_##8a();break; case 0x8b:prefix##_##8b();break; \
    case 0x8c:prefix##_##8c();break; case 0x8d:prefix##_##8d();break; case 0x8e:prefix##_##8e();break; case 0x8f:prefix##_##8f();break; \
    case 0x90:prefix##_##90();break; case 0x91:prefix##_##91();break; case 0x92:prefix##_##92();break; case 0x93:prefix##_##93();break; \
    case 0x94:prefix##_##94();break; case 0x95:prefix##_##95();break; case 0x96:prefix##_##96();break; case 0x97:prefix##_##97();break; \
    case 0x98:prefix##_##98();break; case 0x99:prefix##_##99();break; case 0x9a:prefix##_##9a();break; case 0x9b:prefix##_##9b();break; \
    case 0x9c:prefix##_##9c();break; case 0x9d:prefix##_##9d();break; case 0x9e:prefix##_##9e();break; case 0x9f:prefix##_##9f();break; \
    case 0xa0:prefix##_##a0();break; case 0xa1:prefix##_##a1();break; case 0xa2:prefix##_##a2();break; case 0xa3:prefix##_##a3();break; \
    case 0xa4:prefix##_##a4();break; case 0xa5:prefix##_##a5();break; case 0xa6:prefix##_##a6();break; case 0xa7:prefix##_##a7();break; \
    case 0xa8:prefix##_##a8();break; case 0xa9:prefix##_##a9();break; case 0xaa:prefix##_##aa();break; case 0xab:prefix##_##ab();break; \
    case 0xac:prefix##_##ac();break; case 0xad:prefix##_##ad();break; case 0xae:prefix##_##ae();break; case 0xaf:prefix##_##af();break; \
    case 0xb0:prefix##_##b0();break; case 0xb1:prefix##_##b1();break; case 0xb2:prefix##_##b2();break; case 0xb3:prefix##_##b3();break; \
    case 0xb4:prefix##_##b4();break; case 0xb5:prefix##_##b5();break; case 0xb6:prefix##_##b6();break; case 0xb7:prefix##_##b7();break; \
    case 0xb8:prefix##_##b8();break; case 0xb9:prefix##_##b9();break; case 0xba:prefix##_##ba();break; case 0xbb:prefix##_##bb();break; \
    case 0xbc:prefix##_##bc();break; case 0xbd:prefix##_##bd();break; case 0xbe:prefix##_##be();break; case 0xbf:prefix##_##bf();break; \
    case 0xc0:prefix##_##c0();break; case 0xc1:prefix##_##c1();break; case 0xc2:prefix##_##c2();break; case 0xc3:prefix##_##c3();break; \
    case 0xc4:prefix##_##c4();break; case 0xc5:prefix##_##c5();break; case 0xc6:prefix##_##c6();break; case 0xc7:prefix##_##c7();break; \
    case 0xc8:prefix##_##c8();break; case 0xc9:prefix##_##c9();break; case 0xca:prefix##_##ca();break; case 0xcb:prefix##_##cb();break; \
    case 0xcc:prefix##_##cc();break; case 0xcd:prefix##_##cd();break; case 0xce:prefix##_##ce();break; case 0xcf:prefix##_##cf();break; \
    case 0xd0:prefix##_##d0();break; case 0xd1:prefix##_##d1();break; case 0xd2:prefix##_##d2();break; case 0xd3:prefix##_##d3();break; \
    case 0xd4:prefix##_##d4();break; case 0xd5:prefix##_##d5();break; case 0xd6:prefix##_##d6();break; case 0xd7:prefix##_##d7();break; \
    case 0xd8:prefix##_##d8();break; case 0xd9:prefix##_##d9();break; case 0xda:prefix##_##da();break; case 0xdb:prefix##_##db();break; \
    case 0xdc:prefix##_##dc();break; case 0xdd:prefix##_##dd();break; case 0xde:prefix##_##de();break; case 0xdf:prefix##_##df();break; \
    case 0xe0:prefix##_##e0();break; case 0xe1:prefix##_##e1();break; case 0xe2:prefix##_##e2();break; case 0xe3:prefix##_##e3();break; \
    case 0xe4:prefix##_##e4();break; case 0xe5:prefix##_##e5();break; case 0xe6:prefix##_##e6();break; case 0xe7:prefix##_##e7();break; \
    case 0xe8:prefix##_##e8();break; case 0xe9:prefix##_##e9();break; case 0xea:prefix##_##ea();break; case 0xeb:prefix##_##eb();break; \
    case 0xec:prefix##_##ec();break; case 0xed:prefix##_##ed();break; case 0xee:prefix##_##ee();break; case 0xef:prefix##_##ef();break; \
    case 0xf0:prefix##_##f0();break; case 0xf1:prefix##_##f1();break; case 0xf2:prefix##_##f2();break; case 0xf3:prefix##_##f3();break; \
    case 0xf4:prefix##_##f4();break; case 0xf5:prefix##_##f5();break; case 0xf6:prefix##_##f6();break; case 0xf7:prefix##_##f7();break; \
    case 0xf8:prefix##_##f8();break; case 0xf9:prefix##_##f9();break; case 0xfa:prefix##_##fa();break; case 0xfb:prefix##_##fb();break; \
    case 0xfc:prefix##_##fc();break; case 0xfd:prefix##_##fd();break; case 0xfe:prefix##_##fe();break; case 0xff:prefix##_##ff();break; \
    }                                                                                                                                   \
}
#else
#define EXEC_INLINE EXEC
#endif
