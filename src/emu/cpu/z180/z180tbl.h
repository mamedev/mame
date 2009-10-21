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

static const UINT8 *const cc_default[6] = { cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex };

#define Z180_TABLE_dd	 Z180_TABLE_xy
#define Z180_TABLE_fd	 Z180_TABLE_xy

static void take_interrupt(z180_state *cpustate, int irqline);

#define PROTOTYPES(tablename,prefix) \
	INLINE void prefix##_00(z180_state *cpustate); INLINE void prefix##_01(z180_state *cpustate); INLINE void prefix##_02(z180_state *cpustate); INLINE void prefix##_03(z180_state *cpustate); \
	INLINE void prefix##_04(z180_state *cpustate); INLINE void prefix##_05(z180_state *cpustate); INLINE void prefix##_06(z180_state *cpustate); INLINE void prefix##_07(z180_state *cpustate); \
	INLINE void prefix##_08(z180_state *cpustate); INLINE void prefix##_09(z180_state *cpustate); INLINE void prefix##_0a(z180_state *cpustate); INLINE void prefix##_0b(z180_state *cpustate); \
	INLINE void prefix##_0c(z180_state *cpustate); INLINE void prefix##_0d(z180_state *cpustate); INLINE void prefix##_0e(z180_state *cpustate); INLINE void prefix##_0f(z180_state *cpustate); \
	INLINE void prefix##_10(z180_state *cpustate); INLINE void prefix##_11(z180_state *cpustate); INLINE void prefix##_12(z180_state *cpustate); INLINE void prefix##_13(z180_state *cpustate); \
	INLINE void prefix##_14(z180_state *cpustate); INLINE void prefix##_15(z180_state *cpustate); INLINE void prefix##_16(z180_state *cpustate); INLINE void prefix##_17(z180_state *cpustate); \
	INLINE void prefix##_18(z180_state *cpustate); INLINE void prefix##_19(z180_state *cpustate); INLINE void prefix##_1a(z180_state *cpustate); INLINE void prefix##_1b(z180_state *cpustate); \
	INLINE void prefix##_1c(z180_state *cpustate); INLINE void prefix##_1d(z180_state *cpustate); INLINE void prefix##_1e(z180_state *cpustate); INLINE void prefix##_1f(z180_state *cpustate); \
	INLINE void prefix##_20(z180_state *cpustate); INLINE void prefix##_21(z180_state *cpustate); INLINE void prefix##_22(z180_state *cpustate); INLINE void prefix##_23(z180_state *cpustate); \
	INLINE void prefix##_24(z180_state *cpustate); INLINE void prefix##_25(z180_state *cpustate); INLINE void prefix##_26(z180_state *cpustate); INLINE void prefix##_27(z180_state *cpustate); \
	INLINE void prefix##_28(z180_state *cpustate); INLINE void prefix##_29(z180_state *cpustate); INLINE void prefix##_2a(z180_state *cpustate); INLINE void prefix##_2b(z180_state *cpustate); \
	INLINE void prefix##_2c(z180_state *cpustate); INLINE void prefix##_2d(z180_state *cpustate); INLINE void prefix##_2e(z180_state *cpustate); INLINE void prefix##_2f(z180_state *cpustate); \
	INLINE void prefix##_30(z180_state *cpustate); INLINE void prefix##_31(z180_state *cpustate); INLINE void prefix##_32(z180_state *cpustate); INLINE void prefix##_33(z180_state *cpustate); \
	INLINE void prefix##_34(z180_state *cpustate); INLINE void prefix##_35(z180_state *cpustate); INLINE void prefix##_36(z180_state *cpustate); INLINE void prefix##_37(z180_state *cpustate); \
	INLINE void prefix##_38(z180_state *cpustate); INLINE void prefix##_39(z180_state *cpustate); INLINE void prefix##_3a(z180_state *cpustate); INLINE void prefix##_3b(z180_state *cpustate); \
	INLINE void prefix##_3c(z180_state *cpustate); INLINE void prefix##_3d(z180_state *cpustate); INLINE void prefix##_3e(z180_state *cpustate); INLINE void prefix##_3f(z180_state *cpustate); \
	INLINE void prefix##_40(z180_state *cpustate); INLINE void prefix##_41(z180_state *cpustate); INLINE void prefix##_42(z180_state *cpustate); INLINE void prefix##_43(z180_state *cpustate); \
	INLINE void prefix##_44(z180_state *cpustate); INLINE void prefix##_45(z180_state *cpustate); INLINE void prefix##_46(z180_state *cpustate); INLINE void prefix##_47(z180_state *cpustate); \
	INLINE void prefix##_48(z180_state *cpustate); INLINE void prefix##_49(z180_state *cpustate); INLINE void prefix##_4a(z180_state *cpustate); INLINE void prefix##_4b(z180_state *cpustate); \
	INLINE void prefix##_4c(z180_state *cpustate); INLINE void prefix##_4d(z180_state *cpustate); INLINE void prefix##_4e(z180_state *cpustate); INLINE void prefix##_4f(z180_state *cpustate); \
	INLINE void prefix##_50(z180_state *cpustate); INLINE void prefix##_51(z180_state *cpustate); INLINE void prefix##_52(z180_state *cpustate); INLINE void prefix##_53(z180_state *cpustate); \
	INLINE void prefix##_54(z180_state *cpustate); INLINE void prefix##_55(z180_state *cpustate); INLINE void prefix##_56(z180_state *cpustate); INLINE void prefix##_57(z180_state *cpustate); \
	INLINE void prefix##_58(z180_state *cpustate); INLINE void prefix##_59(z180_state *cpustate); INLINE void prefix##_5a(z180_state *cpustate); INLINE void prefix##_5b(z180_state *cpustate); \
	INLINE void prefix##_5c(z180_state *cpustate); INLINE void prefix##_5d(z180_state *cpustate); INLINE void prefix##_5e(z180_state *cpustate); INLINE void prefix##_5f(z180_state *cpustate); \
	INLINE void prefix##_60(z180_state *cpustate); INLINE void prefix##_61(z180_state *cpustate); INLINE void prefix##_62(z180_state *cpustate); INLINE void prefix##_63(z180_state *cpustate); \
	INLINE void prefix##_64(z180_state *cpustate); INLINE void prefix##_65(z180_state *cpustate); INLINE void prefix##_66(z180_state *cpustate); INLINE void prefix##_67(z180_state *cpustate); \
	INLINE void prefix##_68(z180_state *cpustate); INLINE void prefix##_69(z180_state *cpustate); INLINE void prefix##_6a(z180_state *cpustate); INLINE void prefix##_6b(z180_state *cpustate); \
	INLINE void prefix##_6c(z180_state *cpustate); INLINE void prefix##_6d(z180_state *cpustate); INLINE void prefix##_6e(z180_state *cpustate); INLINE void prefix##_6f(z180_state *cpustate); \
	INLINE void prefix##_70(z180_state *cpustate); INLINE void prefix##_71(z180_state *cpustate); INLINE void prefix##_72(z180_state *cpustate); INLINE void prefix##_73(z180_state *cpustate); \
	INLINE void prefix##_74(z180_state *cpustate); INLINE void prefix##_75(z180_state *cpustate); INLINE void prefix##_76(z180_state *cpustate); INLINE void prefix##_77(z180_state *cpustate); \
	INLINE void prefix##_78(z180_state *cpustate); INLINE void prefix##_79(z180_state *cpustate); INLINE void prefix##_7a(z180_state *cpustate); INLINE void prefix##_7b(z180_state *cpustate); \
	INLINE void prefix##_7c(z180_state *cpustate); INLINE void prefix##_7d(z180_state *cpustate); INLINE void prefix##_7e(z180_state *cpustate); INLINE void prefix##_7f(z180_state *cpustate); \
	INLINE void prefix##_80(z180_state *cpustate); INLINE void prefix##_81(z180_state *cpustate); INLINE void prefix##_82(z180_state *cpustate); INLINE void prefix##_83(z180_state *cpustate); \
	INLINE void prefix##_84(z180_state *cpustate); INLINE void prefix##_85(z180_state *cpustate); INLINE void prefix##_86(z180_state *cpustate); INLINE void prefix##_87(z180_state *cpustate); \
	INLINE void prefix##_88(z180_state *cpustate); INLINE void prefix##_89(z180_state *cpustate); INLINE void prefix##_8a(z180_state *cpustate); INLINE void prefix##_8b(z180_state *cpustate); \
	INLINE void prefix##_8c(z180_state *cpustate); INLINE void prefix##_8d(z180_state *cpustate); INLINE void prefix##_8e(z180_state *cpustate); INLINE void prefix##_8f(z180_state *cpustate); \
	INLINE void prefix##_90(z180_state *cpustate); INLINE void prefix##_91(z180_state *cpustate); INLINE void prefix##_92(z180_state *cpustate); INLINE void prefix##_93(z180_state *cpustate); \
	INLINE void prefix##_94(z180_state *cpustate); INLINE void prefix##_95(z180_state *cpustate); INLINE void prefix##_96(z180_state *cpustate); INLINE void prefix##_97(z180_state *cpustate); \
	INLINE void prefix##_98(z180_state *cpustate); INLINE void prefix##_99(z180_state *cpustate); INLINE void prefix##_9a(z180_state *cpustate); INLINE void prefix##_9b(z180_state *cpustate); \
	INLINE void prefix##_9c(z180_state *cpustate); INLINE void prefix##_9d(z180_state *cpustate); INLINE void prefix##_9e(z180_state *cpustate); INLINE void prefix##_9f(z180_state *cpustate); \
	INLINE void prefix##_a0(z180_state *cpustate); INLINE void prefix##_a1(z180_state *cpustate); INLINE void prefix##_a2(z180_state *cpustate); INLINE void prefix##_a3(z180_state *cpustate); \
	INLINE void prefix##_a4(z180_state *cpustate); INLINE void prefix##_a5(z180_state *cpustate); INLINE void prefix##_a6(z180_state *cpustate); INLINE void prefix##_a7(z180_state *cpustate); \
	INLINE void prefix##_a8(z180_state *cpustate); INLINE void prefix##_a9(z180_state *cpustate); INLINE void prefix##_aa(z180_state *cpustate); INLINE void prefix##_ab(z180_state *cpustate); \
	INLINE void prefix##_ac(z180_state *cpustate); INLINE void prefix##_ad(z180_state *cpustate); INLINE void prefix##_ae(z180_state *cpustate); INLINE void prefix##_af(z180_state *cpustate); \
	INLINE void prefix##_b0(z180_state *cpustate); INLINE void prefix##_b1(z180_state *cpustate); INLINE void prefix##_b2(z180_state *cpustate); INLINE void prefix##_b3(z180_state *cpustate); \
	INLINE void prefix##_b4(z180_state *cpustate); INLINE void prefix##_b5(z180_state *cpustate); INLINE void prefix##_b6(z180_state *cpustate); INLINE void prefix##_b7(z180_state *cpustate); \
	INLINE void prefix##_b8(z180_state *cpustate); INLINE void prefix##_b9(z180_state *cpustate); INLINE void prefix##_ba(z180_state *cpustate); INLINE void prefix##_bb(z180_state *cpustate); \
	INLINE void prefix##_bc(z180_state *cpustate); INLINE void prefix##_bd(z180_state *cpustate); INLINE void prefix##_be(z180_state *cpustate); INLINE void prefix##_bf(z180_state *cpustate); \
	INLINE void prefix##_c0(z180_state *cpustate); INLINE void prefix##_c1(z180_state *cpustate); INLINE void prefix##_c2(z180_state *cpustate); INLINE void prefix##_c3(z180_state *cpustate); \
	INLINE void prefix##_c4(z180_state *cpustate); INLINE void prefix##_c5(z180_state *cpustate); INLINE void prefix##_c6(z180_state *cpustate); INLINE void prefix##_c7(z180_state *cpustate); \
	INLINE void prefix##_c8(z180_state *cpustate); INLINE void prefix##_c9(z180_state *cpustate); INLINE void prefix##_ca(z180_state *cpustate); INLINE void prefix##_cb(z180_state *cpustate); \
	INLINE void prefix##_cc(z180_state *cpustate); INLINE void prefix##_cd(z180_state *cpustate); INLINE void prefix##_ce(z180_state *cpustate); INLINE void prefix##_cf(z180_state *cpustate); \
	INLINE void prefix##_d0(z180_state *cpustate); INLINE void prefix##_d1(z180_state *cpustate); INLINE void prefix##_d2(z180_state *cpustate); INLINE void prefix##_d3(z180_state *cpustate); \
	INLINE void prefix##_d4(z180_state *cpustate); INLINE void prefix##_d5(z180_state *cpustate); INLINE void prefix##_d6(z180_state *cpustate); INLINE void prefix##_d7(z180_state *cpustate); \
	INLINE void prefix##_d8(z180_state *cpustate); INLINE void prefix##_d9(z180_state *cpustate); INLINE void prefix##_da(z180_state *cpustate); INLINE void prefix##_db(z180_state *cpustate); \
	INLINE void prefix##_dc(z180_state *cpustate); INLINE void prefix##_dd(z180_state *cpustate); INLINE void prefix##_de(z180_state *cpustate); INLINE void prefix##_df(z180_state *cpustate); \
	INLINE void prefix##_e0(z180_state *cpustate); INLINE void prefix##_e1(z180_state *cpustate); INLINE void prefix##_e2(z180_state *cpustate); INLINE void prefix##_e3(z180_state *cpustate); \
	INLINE void prefix##_e4(z180_state *cpustate); INLINE void prefix##_e5(z180_state *cpustate); INLINE void prefix##_e6(z180_state *cpustate); INLINE void prefix##_e7(z180_state *cpustate); \
	INLINE void prefix##_e8(z180_state *cpustate); INLINE void prefix##_e9(z180_state *cpustate); INLINE void prefix##_ea(z180_state *cpustate); INLINE void prefix##_eb(z180_state *cpustate); \
	INLINE void prefix##_ec(z180_state *cpustate); INLINE void prefix##_ed(z180_state *cpustate); INLINE void prefix##_ee(z180_state *cpustate); INLINE void prefix##_ef(z180_state *cpustate); \
	INLINE void prefix##_f0(z180_state *cpustate); INLINE void prefix##_f1(z180_state *cpustate); INLINE void prefix##_f2(z180_state *cpustate); INLINE void prefix##_f3(z180_state *cpustate); \
	INLINE void prefix##_f4(z180_state *cpustate); INLINE void prefix##_f5(z180_state *cpustate); INLINE void prefix##_f6(z180_state *cpustate); INLINE void prefix##_f7(z180_state *cpustate); \
	INLINE void prefix##_f8(z180_state *cpustate); INLINE void prefix##_f9(z180_state *cpustate); INLINE void prefix##_fa(z180_state *cpustate); INLINE void prefix##_fb(z180_state *cpustate); \
	INLINE void prefix##_fc(z180_state *cpustate); INLINE void prefix##_fd(z180_state *cpustate); INLINE void prefix##_fe(z180_state *cpustate); INLINE void prefix##_ff(z180_state *cpustate); \
static void (*const tablename[0x100])(z180_state *cpustate) = {	\
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
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(z180_state *cpustate)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) cpustate->icount -= cpustate->cc[Z180_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(prefix,opcode)                                     \
{                                                               \
    unsigned op = opcode;                                       \
    CC(prefix,op);                                              \
    (*Z180##prefix[op])(cpustate);                              \
}

#if BIG_SWITCH
#define EXEC_INLINE(prefix,opcode)                              \
{                                                               \
    unsigned op = opcode;                                       \
    CC(prefix,op);                                              \
    switch(op)                                                  \
    {                                                           \
    case 0x00:prefix##_##00(cpustate);break; case 0x01:prefix##_##01(cpustate);break; case 0x02:prefix##_##02(cpustate);break; case 0x03:prefix##_##03(cpustate);break; \
    case 0x04:prefix##_##04(cpustate);break; case 0x05:prefix##_##05(cpustate);break; case 0x06:prefix##_##06(cpustate);break; case 0x07:prefix##_##07(cpustate);break; \
    case 0x08:prefix##_##08(cpustate);break; case 0x09:prefix##_##09(cpustate);break; case 0x0a:prefix##_##0a(cpustate);break; case 0x0b:prefix##_##0b(cpustate);break; \
    case 0x0c:prefix##_##0c(cpustate);break; case 0x0d:prefix##_##0d(cpustate);break; case 0x0e:prefix##_##0e(cpustate);break; case 0x0f:prefix##_##0f(cpustate);break; \
    case 0x10:prefix##_##10(cpustate);break; case 0x11:prefix##_##11(cpustate);break; case 0x12:prefix##_##12(cpustate);break; case 0x13:prefix##_##13(cpustate);break; \
    case 0x14:prefix##_##14(cpustate);break; case 0x15:prefix##_##15(cpustate);break; case 0x16:prefix##_##16(cpustate);break; case 0x17:prefix##_##17(cpustate);break; \
    case 0x18:prefix##_##18(cpustate);break; case 0x19:prefix##_##19(cpustate);break; case 0x1a:prefix##_##1a(cpustate);break; case 0x1b:prefix##_##1b(cpustate);break; \
    case 0x1c:prefix##_##1c(cpustate);break; case 0x1d:prefix##_##1d(cpustate);break; case 0x1e:prefix##_##1e(cpustate);break; case 0x1f:prefix##_##1f(cpustate);break; \
    case 0x20:prefix##_##20(cpustate);break; case 0x21:prefix##_##21(cpustate);break; case 0x22:prefix##_##22(cpustate);break; case 0x23:prefix##_##23(cpustate);break; \
    case 0x24:prefix##_##24(cpustate);break; case 0x25:prefix##_##25(cpustate);break; case 0x26:prefix##_##26(cpustate);break; case 0x27:prefix##_##27(cpustate);break; \
    case 0x28:prefix##_##28(cpustate);break; case 0x29:prefix##_##29(cpustate);break; case 0x2a:prefix##_##2a(cpustate);break; case 0x2b:prefix##_##2b(cpustate);break; \
    case 0x2c:prefix##_##2c(cpustate);break; case 0x2d:prefix##_##2d(cpustate);break; case 0x2e:prefix##_##2e(cpustate);break; case 0x2f:prefix##_##2f(cpustate);break; \
    case 0x30:prefix##_##30(cpustate);break; case 0x31:prefix##_##31(cpustate);break; case 0x32:prefix##_##32(cpustate);break; case 0x33:prefix##_##33(cpustate);break; \
    case 0x34:prefix##_##34(cpustate);break; case 0x35:prefix##_##35(cpustate);break; case 0x36:prefix##_##36(cpustate);break; case 0x37:prefix##_##37(cpustate);break; \
    case 0x38:prefix##_##38(cpustate);break; case 0x39:prefix##_##39(cpustate);break; case 0x3a:prefix##_##3a(cpustate);break; case 0x3b:prefix##_##3b(cpustate);break; \
    case 0x3c:prefix##_##3c(cpustate);break; case 0x3d:prefix##_##3d(cpustate);break; case 0x3e:prefix##_##3e(cpustate);break; case 0x3f:prefix##_##3f(cpustate);break; \
    case 0x40:prefix##_##40(cpustate);break; case 0x41:prefix##_##41(cpustate);break; case 0x42:prefix##_##42(cpustate);break; case 0x43:prefix##_##43(cpustate);break; \
    case 0x44:prefix##_##44(cpustate);break; case 0x45:prefix##_##45(cpustate);break; case 0x46:prefix##_##46(cpustate);break; case 0x47:prefix##_##47(cpustate);break; \
    case 0x48:prefix##_##48(cpustate);break; case 0x49:prefix##_##49(cpustate);break; case 0x4a:prefix##_##4a(cpustate);break; case 0x4b:prefix##_##4b(cpustate);break; \
    case 0x4c:prefix##_##4c(cpustate);break; case 0x4d:prefix##_##4d(cpustate);break; case 0x4e:prefix##_##4e(cpustate);break; case 0x4f:prefix##_##4f(cpustate);break; \
    case 0x50:prefix##_##50(cpustate);break; case 0x51:prefix##_##51(cpustate);break; case 0x52:prefix##_##52(cpustate);break; case 0x53:prefix##_##53(cpustate);break; \
    case 0x54:prefix##_##54(cpustate);break; case 0x55:prefix##_##55(cpustate);break; case 0x56:prefix##_##56(cpustate);break; case 0x57:prefix##_##57(cpustate);break; \
    case 0x58:prefix##_##58(cpustate);break; case 0x59:prefix##_##59(cpustate);break; case 0x5a:prefix##_##5a(cpustate);break; case 0x5b:prefix##_##5b(cpustate);break; \
    case 0x5c:prefix##_##5c(cpustate);break; case 0x5d:prefix##_##5d(cpustate);break; case 0x5e:prefix##_##5e(cpustate);break; case 0x5f:prefix##_##5f(cpustate);break; \
    case 0x60:prefix##_##60(cpustate);break; case 0x61:prefix##_##61(cpustate);break; case 0x62:prefix##_##62(cpustate);break; case 0x63:prefix##_##63(cpustate);break; \
    case 0x64:prefix##_##64(cpustate);break; case 0x65:prefix##_##65(cpustate);break; case 0x66:prefix##_##66(cpustate);break; case 0x67:prefix##_##67(cpustate);break; \
    case 0x68:prefix##_##68(cpustate);break; case 0x69:prefix##_##69(cpustate);break; case 0x6a:prefix##_##6a(cpustate);break; case 0x6b:prefix##_##6b(cpustate);break; \
    case 0x6c:prefix##_##6c(cpustate);break; case 0x6d:prefix##_##6d(cpustate);break; case 0x6e:prefix##_##6e(cpustate);break; case 0x6f:prefix##_##6f(cpustate);break; \
    case 0x70:prefix##_##70(cpustate);break; case 0x71:prefix##_##71(cpustate);break; case 0x72:prefix##_##72(cpustate);break; case 0x73:prefix##_##73(cpustate);break; \
    case 0x74:prefix##_##74(cpustate);break; case 0x75:prefix##_##75(cpustate);break; case 0x76:prefix##_##76(cpustate);break; case 0x77:prefix##_##77(cpustate);break; \
    case 0x78:prefix##_##78(cpustate);break; case 0x79:prefix##_##79(cpustate);break; case 0x7a:prefix##_##7a(cpustate);break; case 0x7b:prefix##_##7b(cpustate);break; \
    case 0x7c:prefix##_##7c(cpustate);break; case 0x7d:prefix##_##7d(cpustate);break; case 0x7e:prefix##_##7e(cpustate);break; case 0x7f:prefix##_##7f(cpustate);break; \
    case 0x80:prefix##_##80(cpustate);break; case 0x81:prefix##_##81(cpustate);break; case 0x82:prefix##_##82(cpustate);break; case 0x83:prefix##_##83(cpustate);break; \
    case 0x84:prefix##_##84(cpustate);break; case 0x85:prefix##_##85(cpustate);break; case 0x86:prefix##_##86(cpustate);break; case 0x87:prefix##_##87(cpustate);break; \
    case 0x88:prefix##_##88(cpustate);break; case 0x89:prefix##_##89(cpustate);break; case 0x8a:prefix##_##8a(cpustate);break; case 0x8b:prefix##_##8b(cpustate);break; \
    case 0x8c:prefix##_##8c(cpustate);break; case 0x8d:prefix##_##8d(cpustate);break; case 0x8e:prefix##_##8e(cpustate);break; case 0x8f:prefix##_##8f(cpustate);break; \
    case 0x90:prefix##_##90(cpustate);break; case 0x91:prefix##_##91(cpustate);break; case 0x92:prefix##_##92(cpustate);break; case 0x93:prefix##_##93(cpustate);break; \
    case 0x94:prefix##_##94(cpustate);break; case 0x95:prefix##_##95(cpustate);break; case 0x96:prefix##_##96(cpustate);break; case 0x97:prefix##_##97(cpustate);break; \
    case 0x98:prefix##_##98(cpustate);break; case 0x99:prefix##_##99(cpustate);break; case 0x9a:prefix##_##9a(cpustate);break; case 0x9b:prefix##_##9b(cpustate);break; \
    case 0x9c:prefix##_##9c(cpustate);break; case 0x9d:prefix##_##9d(cpustate);break; case 0x9e:prefix##_##9e(cpustate);break; case 0x9f:prefix##_##9f(cpustate);break; \
    case 0xa0:prefix##_##a0(cpustate);break; case 0xa1:prefix##_##a1(cpustate);break; case 0xa2:prefix##_##a2(cpustate);break; case 0xa3:prefix##_##a3(cpustate);break; \
    case 0xa4:prefix##_##a4(cpustate);break; case 0xa5:prefix##_##a5(cpustate);break; case 0xa6:prefix##_##a6(cpustate);break; case 0xa7:prefix##_##a7(cpustate);break; \
    case 0xa8:prefix##_##a8(cpustate);break; case 0xa9:prefix##_##a9(cpustate);break; case 0xaa:prefix##_##aa(cpustate);break; case 0xab:prefix##_##ab(cpustate);break; \
    case 0xac:prefix##_##ac(cpustate);break; case 0xad:prefix##_##ad(cpustate);break; case 0xae:prefix##_##ae(cpustate);break; case 0xaf:prefix##_##af(cpustate);break; \
    case 0xb0:prefix##_##b0(cpustate);break; case 0xb1:prefix##_##b1(cpustate);break; case 0xb2:prefix##_##b2(cpustate);break; case 0xb3:prefix##_##b3(cpustate);break; \
    case 0xb4:prefix##_##b4(cpustate);break; case 0xb5:prefix##_##b5(cpustate);break; case 0xb6:prefix##_##b6(cpustate);break; case 0xb7:prefix##_##b7(cpustate);break; \
    case 0xb8:prefix##_##b8(cpustate);break; case 0xb9:prefix##_##b9(cpustate);break; case 0xba:prefix##_##ba(cpustate);break; case 0xbb:prefix##_##bb(cpustate);break; \
    case 0xbc:prefix##_##bc(cpustate);break; case 0xbd:prefix##_##bd(cpustate);break; case 0xbe:prefix##_##be(cpustate);break; case 0xbf:prefix##_##bf(cpustate);break; \
    case 0xc0:prefix##_##c0(cpustate);break; case 0xc1:prefix##_##c1(cpustate);break; case 0xc2:prefix##_##c2(cpustate);break; case 0xc3:prefix##_##c3(cpustate);break; \
    case 0xc4:prefix##_##c4(cpustate);break; case 0xc5:prefix##_##c5(cpustate);break; case 0xc6:prefix##_##c6(cpustate);break; case 0xc7:prefix##_##c7(cpustate);break; \
    case 0xc8:prefix##_##c8(cpustate);break; case 0xc9:prefix##_##c9(cpustate);break; case 0xca:prefix##_##ca(cpustate);break; case 0xcb:prefix##_##cb(cpustate);break; \
    case 0xcc:prefix##_##cc(cpustate);break; case 0xcd:prefix##_##cd(cpustate);break; case 0xce:prefix##_##ce(cpustate);break; case 0xcf:prefix##_##cf(cpustate);break; \
    case 0xd0:prefix##_##d0(cpustate);break; case 0xd1:prefix##_##d1(cpustate);break; case 0xd2:prefix##_##d2(cpustate);break; case 0xd3:prefix##_##d3(cpustate);break; \
    case 0xd4:prefix##_##d4(cpustate);break; case 0xd5:prefix##_##d5(cpustate);break; case 0xd6:prefix##_##d6(cpustate);break; case 0xd7:prefix##_##d7(cpustate);break; \
    case 0xd8:prefix##_##d8(cpustate);break; case 0xd9:prefix##_##d9(cpustate);break; case 0xda:prefix##_##da(cpustate);break; case 0xdb:prefix##_##db(cpustate);break; \
    case 0xdc:prefix##_##dc(cpustate);break; case 0xdd:prefix##_##dd(cpustate);break; case 0xde:prefix##_##de(cpustate);break; case 0xdf:prefix##_##df(cpustate);break; \
    case 0xe0:prefix##_##e0(cpustate);break; case 0xe1:prefix##_##e1(cpustate);break; case 0xe2:prefix##_##e2(cpustate);break; case 0xe3:prefix##_##e3(cpustate);break; \
    case 0xe4:prefix##_##e4(cpustate);break; case 0xe5:prefix##_##e5(cpustate);break; case 0xe6:prefix##_##e6(cpustate);break; case 0xe7:prefix##_##e7(cpustate);break; \
    case 0xe8:prefix##_##e8(cpustate);break; case 0xe9:prefix##_##e9(cpustate);break; case 0xea:prefix##_##ea(cpustate);break; case 0xeb:prefix##_##eb(cpustate);break; \
    case 0xec:prefix##_##ec(cpustate);break; case 0xed:prefix##_##ed(cpustate);break; case 0xee:prefix##_##ee(cpustate);break; case 0xef:prefix##_##ef(cpustate);break; \
    case 0xf0:prefix##_##f0(cpustate);break; case 0xf1:prefix##_##f1(cpustate);break; case 0xf2:prefix##_##f2(cpustate);break; case 0xf3:prefix##_##f3(cpustate);break; \
    case 0xf4:prefix##_##f4(cpustate);break; case 0xf5:prefix##_##f5(cpustate);break; case 0xf6:prefix##_##f6(cpustate);break; case 0xf7:prefix##_##f7(cpustate);break; \
    case 0xf8:prefix##_##f8(cpustate);break; case 0xf9:prefix##_##f9(cpustate);break; case 0xfa:prefix##_##fa(cpustate);break; case 0xfb:prefix##_##fb(cpustate);break; \
    case 0xfc:prefix##_##fc(cpustate);break; case 0xfd:prefix##_##fd(cpustate);break; case 0xfe:prefix##_##fe(cpustate);break; case 0xff:prefix##_##ff(cpustate);break; \
    }                                                                                                                                   \
}
#else
#define EXEC_INLINE EXEC
#endif
