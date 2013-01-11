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

static int take_interrupt(z180_state *cpustate, int irq);

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
	INLINE void prefix##_fc(z180_state *cpustate); INLINE void prefix##_fd(z180_state *cpustate); INLINE void prefix##_fe(z180_state *cpustate); INLINE void prefix##_ff(z180_state *cpustate);

#define TABLE(prefix) {\
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

static void (*const Z180ops[Z180_PREFIX_COUNT][0x100])(z180_state *cpustate) =
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
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(z180_state *cpustate)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) cpustate->extra_cycles += cpustate->cc[Z180_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/

#define EXEC_PROTOTYPE(prefix) \
INLINE int exec##_##prefix(z180_state *cpustate, const UINT8 opcode)    \
{                                                                       \
	(*Z180ops[Z180_PREFIX_##prefix][opcode])(cpustate);                                     \
	return cpustate->cc[Z180_TABLE_##prefix][opcode];                   \
}

EXEC_PROTOTYPE(op)
EXEC_PROTOTYPE(cb)
EXEC_PROTOTYPE(dd)
EXEC_PROTOTYPE(ed)
EXEC_PROTOTYPE(fd)
EXEC_PROTOTYPE(xycb)
