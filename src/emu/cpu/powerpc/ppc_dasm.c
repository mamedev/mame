// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 * disasm.c
 *
 * PowerPC 603e disassembler.
 *
 * When possible, invalid forms of instructions are checked for. To the best
 * of my knowledge, all appropriate load/store instructions are checked. I'm
 * not sure whether any other kinds of instructions need checking.
 */

/* Originally written by Bart Trzynadlowski for Supermodel project
 *
 * PowerPC 403 opcodes and MAME conversion by Ville Linde
 */

#include "emu.h"
#include "debugger.h"
#include "ppccom.h"

/*
 * Operand Formats
 *
 * These convey information on what operand fields are present and how they
 * ought to be printed.
 *
 * I'm fairly certain all of these are used, but that is not guaranteed.
 */

enum
{
	F_NONE,         // <no operands>
	F_LI,           // LI*4+PC if AA=0 else LI*4
	F_BCx,          // BO, BI, target_addr  used only by BCx
	F_RT_RA_0_SIMM, // rT, rA|0, SIMM       rA|0 means if rA == 0, print 0
	F_ADDIS,        // rT, rA, SIMM (printed as unsigned)   only used by ADDIS
	F_RT_RA_SIMM,   // rT, rA, SIMM
	F_RA_RT_UIMM,   // rA, rT, UIMM
	F_CMP_SIMM,     // crfD, L, A, SIMM
	F_CMP_UIMM,     // crfD, L, A, UIMM
	F_RT_RA_0_RB,   // rT, rA|0, rB
	F_RT_RA_RB,     // rT, rA, rB
	F_RT_D_RA_0,    // rT, d(rA|0)
	F_RT_D_RA,      // rT, d(rA)
	F_RA_RT_RB,     // rA, rT, rB
	F_FRT_D_RA_0,   // frT, d(RA|0)
	F_FRT_D_RA,     // frT, d(RA)
	F_FRT_RA_0_RB,  // frT, rA|0, rB
	F_FRT_RA_RB,    // frT, rA, rB
	F_TWI,          // TO, rA, SIMM         only used by TWI instruction
	F_CMP,          // crfD, L, rA, rB
	F_RA_RT,        // rA, rT
	F_RA_0_RB,      // rA|0, rB
	F_FRT_FRB,      // frT, frB
	F_FCMP,         // crfD, frA, frB
	F_CRFD_CRFS,    // crfD, crfS
	F_MCRXR,        // crfD                 only used by MCRXR
	F_RT,           // rT
	F_MFSR,         // rT, SR               only used by MFSR
	F_MTSR,         // SR, rT               only used by MTSR
	F_MFFSx,        // frT                  only used by MFFSx
	F_FCRBD,        // crbD                 FPSCR[crbD]
	F_MTFSFIx,      // crfD, IMM            only used by MTFSFIx
	F_RB,           // rB
	F_TW,           // TO, rA, rB           only used by TW
	F_RT_RA_0_NB,   // rT, rA|0, NB         print 32 if NB == 0
	F_SRAWIx,       // rA, rT, SH           only used by SRAWIx
	F_BO_BI,        // BO, BI
	F_CRBD_CRBA_CRBB,   // crbD, crbA, crbB
	F_RT_SPR,       // rT, SPR              and TBR
	F_MTSPR,        // SPR, rT              only used by MTSPR
	F_MTCRF,        // CRM, rT              only used by MTCRF
	F_MTFSFx,       // FM, frB              only used by MTFSFx
	F_RT_DCR,       // rT, DCR
	F_MTDCR,        // DCR, rT
	F_RT_RA,        // rT, rA
	F_FRT_FRA_FRC_FRB,  // frT, frA, frC, frB
	F_FRT_FRA_FRB,  // frT, frA, frB
	F_FRT_FRA_FRC,  // frT, frA, frC
	F_RA_RT_SH_MB_ME,   // rA, rT, SH, MB, ME
	F_RLWNMx,       // rT, rA, rB, MB, ME   only used by RLWNMx
	F_RT_RB         // rT, rB
};

/*
 * Flags
 */

#define FL_OE           (1 << 0)    // if there is an OE field
#define FL_RC           (1 << 1)    // if there is an RC field
#define FL_LK           (1 << 2)    // if there is an LK field
#define FL_AA           (1 << 3)    // if there is an AA field
#define FL_CHECK_RA_RT  (1 << 4)    // assert rA!=0 and rA!=rT
#define FL_CHECK_RA     (1 << 5)    // assert rA!=0
#define FL_CHECK_LSWI   (1 << 6)    // specific check for LSWI validity
#define FL_CHECK_LSWX   (1 << 7)    // specific check for LSWX validity
#define FL_SO           (1 << 8)    // use DASMFLAG_STEP_OUT


/*
 * Instruction Descriptor
 *
 * Describes the layout of an instruction.
 */

struct IDESCR
{
	char    mnem[32];   // mnemonic
	UINT32  match;      // bit pattern of instruction after it has been masked
	UINT32  mask;       // mask of variable fields (AND with ~mask to compare w/
						// bit pattern to determine a match)
	int format;         // operand format
	int flags;          // flags
};

/*
 * Instruction Table
 *
 * Table of instruction descriptors which allows the disassembler to decode
 * and print instructions.
 */

static const IDESCR itab[] =
{
	{ "add",    D_OP(31)|D_XO(266), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "addc",   D_OP(31)|D_XO(10),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "adde",   D_OP(31)|D_XO(138), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "addi",   D_OP(14),           M_RT|M_RA|M_SIMM,           F_RT_RA_0_SIMM, 0           },
	{ "addic",  D_OP(12),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
	{ "addic.", D_OP(13),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
	{ "addis",  D_OP(15),           M_RT|M_RA|M_SIMM,           F_ADDIS,        0           },
	{ "addme",  D_OP(31)|D_XO(234), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
	{ "addze",  D_OP(31)|D_XO(202), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
	{ "and",    D_OP(31)|D_XO(28),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "andc",   D_OP(31)|D_XO(60),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "andi.",  D_OP(28),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
	{ "andis.", D_OP(29),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
	{ "b",      D_OP(18),           M_LI|M_AA|M_LK,             F_LI,           FL_AA|FL_LK },
	{ "bc",     D_OP(16),           M_BO|M_BI|M_BD|M_AA|M_LK,   F_BCx,          FL_AA|FL_LK },
	{ "bcctr",  D_OP(19)|D_XO(528), M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK       },
	{ "bclr",   D_OP(19)|D_XO(16),  M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK|FL_SO },
	{ "cmp",    D_OP(31)|D_XO(0),   M_CRFD|M_RA|M_RB,           F_CMP,          0           },
	{ "cmpd",   D_OP(31)|D_XO(0)|M_L,M_CRFD|M_RA|M_RB,          F_CMP,          0           },
	{ "cmpi",   D_OP(11),           M_CRFD|M_RA|M_SIMM,         F_CMP_SIMM,     0           },
	{ "cmpdi",  D_OP(11)|M_L,       M_CRFD|M_RA|M_SIMM,         F_CMP_SIMM,     0           },
	{ "cmpl",   D_OP(31)|D_XO(32),  M_CRFD|M_RA|M_RB,           F_CMP,          0           },
	{ "cmpld",  D_OP(31)|D_XO(32)|M_L,M_CRFD|M_RA|M_RB,         F_CMP,          0           },
	{ "cmpli",  D_OP(10),           M_CRFD|M_RA|M_UIMM,         F_CMP_UIMM,     0           },
	{ "cmpldi", D_OP(10)|M_L,       M_CRFD|M_RA|M_UIMM,         F_CMP_UIMM,     0           },
	{ "cntlzw", D_OP(31)|D_XO(26),  M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
	{ "crand",  D_OP(19)|D_XO(257), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "crandc", D_OP(19)|D_XO(129), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "creqv",  D_OP(19)|D_XO(289), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "crnand", D_OP(19)|D_XO(225), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "crnor",  D_OP(19)|D_XO(33),  M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "cror",   D_OP(19)|D_XO(449), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "crorc",  D_OP(19)|D_XO(417), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "crxor",  D_OP(19)|D_XO(193), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
	{ "dcba",   D_OP(31)|D_XO(758), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbf",   D_OP(31)|D_XO(86),  M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbi",   D_OP(31)|D_XO(470), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbst",  D_OP(31)|D_XO(54),  M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbt",   D_OP(31)|D_XO(278), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbtst", D_OP(31)|D_XO(246), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcbz",   D_OP(31)|D_XO(1014),M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dccci",  D_OP(31)|D_XO(454), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "dcread", D_OP(31)|D_XO(486), M_RA|M_RB,                  F_RT_RA_RB,     0           },
	{ "divw",   D_OP(31)|D_XO(491), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "divwu",  D_OP(31)|D_XO(459), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "dsa",    D_OP(31)|D_XO(628), 0,                          0,              0           },
	{ "eciwx",  D_OP(31)|D_XO(310), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "ecowx",  D_OP(31)|D_XO(438), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "eieio",  D_OP(31)|D_XO(854), 0,                          F_NONE,         0           },
	{ "eqv",    D_OP(31)|D_XO(284), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "esa",    D_OP(31)|D_XO(596), 0,                          0,              0           },
	{ "extsb",  D_OP(31)|D_XO(954), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
	{ "extsh",  D_OP(31)|D_XO(922), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
	{ "fabs",   D_OP(63)|D_XO(264), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fadd",   D_OP(63)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "fadds",  D_OP(59)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "fcmpo",  D_OP(63)|D_XO(32),  M_CRFD|M_RA|M_RB,           F_FCMP,         0           },
	{ "fcmpu",  D_OP(63)|D_XO(0),   M_CRFD|M_RA|M_RB,           F_FCMP,         0           },
	{ "fctiw",  D_OP(63)|D_XO(14),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fctiwz", D_OP(63)|D_XO(15),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fdiv",   D_OP(63)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "fdivs",  D_OP(59)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "fmadd",  D_OP(63)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fmadds", D_OP(59)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fmr",    D_OP(63)|D_XO(72),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fmsub",  D_OP(63)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fmsubs", D_OP(59)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fmul",   D_OP(63)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC       },
	{ "fmuls",  D_OP(59)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC       },
	{ "fnabs",  D_OP(63)|D_XO(136), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fneg",   D_OP(63)|D_XO(40),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fnmadd", D_OP(63)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fnmadds",D_OP(59)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fnmsub", D_OP(63)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fnmsubs",D_OP(59)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fres",   D_OP(59)|D_XO(24),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "frsp",   D_OP(63)|D_XO(12),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "frsqrte",D_OP(63)|D_XO(26),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fsel",   D_OP(63)|D_XO(23),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
	{ "fsqrt",  D_OP(63)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fsqrts", D_OP(59)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
	{ "fsub",   D_OP(63)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "fsubs",  D_OP(59)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
	{ "icbi",   D_OP(31)|D_XO(982), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "icbt",   D_OP(31)|D_XO(262), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "iccci",  D_OP(31)|D_XO(966), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "icread", D_OP(31)|D_XO(998), M_RA|M_RB,                  F_RA_0_RB,      0           },
	{ "isync",  D_OP(19)|D_XO(150), 0,                          F_NONE,         0           },
	{ "lbz",    D_OP(34),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "lbzu",   D_OP(35),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
	{ "lbzux",  D_OP(31)|D_XO(119), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
	{ "lbzx",   D_OP(31)|D_XO(87),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lfd",    D_OP(50),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
	{ "lfdu",   D_OP(51),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
	{ "lfdux",  D_OP(31)|D_XO(631), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
	{ "lfdx",   D_OP(31)|D_XO(599), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
	{ "lfs",    D_OP(48),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
	{ "lfsu",   D_OP(49),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
	{ "lfsux",  D_OP(31)|D_XO(567), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
	{ "lfsx",   D_OP(31)|D_XO(535), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
	{ "lha",    D_OP(42),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "lhau",   D_OP(43),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
	{ "lhaux",  D_OP(31)|D_XO(375), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
	{ "lhax",   D_OP(31)|D_XO(343), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lhbrx",  D_OP(31)|D_XO(790), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lhz",    D_OP(40),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "lhzu",   D_OP(41),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
	{ "lhzux",  D_OP(31)|D_XO(311), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
	{ "lhzx",   D_OP(31)|D_XO(279), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lmw",    D_OP(46),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "lswi",   D_OP(31)|D_XO(597), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   FL_CHECK_LSWI },
	{ "lswx",   D_OP(31)|D_XO(533), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   FL_CHECK_LSWX },
	{ "lwarx",  D_OP(31)|D_XO(20),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lwbrx",  D_OP(31)|D_XO(534), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "lwz",    D_OP(32),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "lwzu",   D_OP(33),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
	{ "lwzux",  D_OP(31)|D_XO(55),  M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
	{ "lwzx",   D_OP(31)|D_XO(23),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "mcrf",   D_OP(19)|D_XO(0),   M_CRFD|M_CRFS,              F_CRFD_CRFS,    0           },
	{ "mcrfs",  D_OP(63)|D_XO(64),  M_CRFD|M_CRFS,              F_CRFD_CRFS,    0           },
	{ "mcrxr",  D_OP(31)|D_XO(512), M_CRFD,                     F_MCRXR,        0           },
	{ "mfcr",   D_OP(31)|D_XO(19),  M_RT,                       F_RT,           0           },
	{ "mfdcr",  D_OP(31)|D_XO(323), M_RT|M_DCR,                 F_RT_DCR,       0           },
	{ "mffs",   D_OP(63)|D_XO(583), M_RT|M_RC,                  F_MFFSx,        FL_RC       },
	{ "mfmsr",  D_OP(31)|D_XO(83),  M_RT,                       F_RT,           0           },
	{ "mfspr",  D_OP(31)|D_XO(339), M_RT|M_SPR,                 F_RT_SPR,       0           },
	{ "mfsr",   D_OP(31)|D_XO(595), M_RT|M_SR,                  F_MFSR,         0           },
	{ "mfsrin", D_OP(31)|D_XO(659), M_RT|M_RB,                  F_RT_RB,        0           },
	{ "mftb",   D_OP(31)|D_XO(371), M_RT|M_TBR,                 F_RT_SPR,       0           },
	{ "mtcrf",  D_OP(31)|D_XO(144), M_RT|M_CRM,                 F_MTCRF,        0           },
	{ "mtdcr",  D_OP(31)|D_XO(451), M_RT|M_DCR,                 F_MTDCR,        0           },
	{ "mtfsb0", D_OP(63)|D_XO(70),  M_CRBD|M_RC,                F_FCRBD,        FL_RC       },
	{ "mtfsb1", D_OP(63)|D_XO(38),  M_CRBD|M_RC,                F_FCRBD,        FL_RC       },
	{ "mtfsf",  D_OP(63)|D_XO(711), M_FM|M_RB|M_RC,             F_MTFSFx,       FL_RC       },
	{ "mtfsfi", D_OP(63)|D_XO(134), M_CRFD|M_IMM|M_RC,          F_MTFSFIx,      FL_RC       },
	{ "mtmsr",  D_OP(31)|D_XO(146), M_RT,                       F_RT,           0           },
	{ "mtspr",  D_OP(31)|D_XO(467), M_RT|M_SPR,                 F_MTSPR,        0           },
	{ "mtsr",   D_OP(31)|D_XO(210), M_RT|M_SR,                  F_MTSR,         0           },
	{ "mtsrin", D_OP(31)|D_XO(242), M_RT|M_RB,                  F_RT_RB,        0           },
	{ "mulhw",  D_OP(31)|D_XO(75),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC       },
	{ "mulhwu", D_OP(31)|D_XO(11),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC       },
	{ "mulli",  D_OP(7),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
	{ "mullw",  D_OP(31)|D_XO(235), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "nand",   D_OP(31)|D_XO(476), M_RA|M_RT|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "neg",    D_OP(31)|D_XO(104), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
	{ "nor",    D_OP(31)|D_XO(124), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "or",     D_OP(31)|D_XO(444), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "orc",    D_OP(31)|D_XO(412), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "ori",    D_OP(24),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
	{ "oris",   D_OP(25),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
	{ "rfi",    D_OP(19)|D_XO(50),  0,                          F_NONE,         0           },
	{ "rfci",   D_OP(19)|D_XO(51),  0,                          F_NONE,         0           },
	{ "rlwimi", D_OP(20),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC,  F_RA_RT_SH_MB_ME,   FL_RC   },
	{ "rlwinm", D_OP(21),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC,  F_RA_RT_SH_MB_ME,   FL_RC   },
	{ "rlwnm",  D_OP(23),           M_RT|M_RA|M_RB|M_MB|M_ME|M_RC,  F_RLWNMx,   FL_RC       },
	{ "sc",     D_OP(17)|2,         0,                          F_NONE,         0           },
	{ "slw",    D_OP(31)|D_XO(24),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "sraw",   D_OP(31)|D_XO(792), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "srawi",  D_OP(31)|D_XO(824), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC       },
	{ "srw",    D_OP(31)|D_XO(536), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "stb",    D_OP(38),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "stbu",   D_OP(39),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
	{ "stbux",  D_OP(31)|D_XO(247), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
	{ "stbx",   D_OP(31)|D_XO(215), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "stfd",   D_OP(54),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
	{ "stfdu",  D_OP(55),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
	{ "stfdux", D_OP(31)|D_XO(759), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
	{ "stfdx",  D_OP(31)|D_XO(727), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
	{ "stfiwx", D_OP(31)|D_XO(983), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
	{ "stfs",   D_OP(52),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
	{ "stfsu",  D_OP(53),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
	{ "stfsux", D_OP(31)|D_XO(695), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
	{ "stfsx",  D_OP(31)|D_XO(663), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
	{ "sth",    D_OP(44),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "sthbrx", D_OP(31)|D_XO(918), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "sthu",   D_OP(45),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
	{ "sthux",  D_OP(31)|D_XO(439), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
	{ "sthx",   D_OP(31)|D_XO(407), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "stmw",   D_OP(47),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "stswi",  D_OP(31)|D_XO(725), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   0           },
	{ "stswx",  D_OP(31)|D_XO(661), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "stw",    D_OP(36),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
	{ "stwbrx", D_OP(31)|D_XO(662), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "stwcx.", D_OP(31)|D_XO(150)|1,   M_RT|M_RA|M_RB,         F_RT_RA_0_RB,   0           },
	{ "stwu",   D_OP(37),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
	{ "stwux",  D_OP(31)|D_XO(183), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
	{ "stwx",   D_OP(31)|D_XO(151), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
	{ "subf",   D_OP(31)|D_XO(40),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "subfc",  D_OP(31)|D_XO(8),   M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "subfe",  D_OP(31)|D_XO(136), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
	{ "subfic", D_OP(8),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
	{ "subfme", D_OP(31)|D_XO(232), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
	{ "subfze", D_OP(31)|D_XO(200), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
	{ "sync",   D_OP(31)|D_XO(598), 0,                          F_NONE,         0           },
	{ "tlbia",  D_OP(31)|D_XO(370), 0,                          F_NONE,         0           },
	{ "tlbie",  D_OP(31)|D_XO(306), M_RB,                       F_RB,           0           },
	{ "tlbsync",D_OP(31)|D_XO(566), 0,                          F_NONE,         0           },
	{ "tw",     D_OP(31)|D_XO(4),   M_TO|M_RA|M_RB,             F_TW,           0           },
	{ "twi",    D_OP(3),            M_TO|M_RA|M_SIMM,           F_TWI,          0           },
	{ "wrtee",  D_OP(31)|D_XO(131), M_RT,                       F_RT,           0           },
	{ "wrteei", D_OP(31)|D_XO(163), 0,                          0,              0           },
	{ "xor",    D_OP(31)|D_XO(316), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
	{ "xori",   D_OP(26),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
	{ "xoris",  D_OP(27),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },

	/*
	 * PowerPC 603e/EC603e-specific instructions
	 */

	{ "tlbld",  D_OP(31)|D_XO(978), M_RB,                       F_RB,           0           },
	{ "tlbli",  D_OP(31)|D_XO(1010),M_RB,                       F_RB,           0           }
};

/*
 * CR Bits
 *
 * Use an index of BI&3 into this table to obtain the CR field bit name.
 */

static const char *const crbit[4] = { "lt", "gt", "eq", "so" };
static const char *const crnbit[4] = { "ge", "le", "ne", "nso" };


/*
 * SPR():
 *
 * Decode the SPR (or TBR) field and append the register name to dest. If
 * no name is associated with the field value, the value itself is printed.
 */

static void SPR(char *dest, int spr_field)
{
	int spr;

	/*
	 * Construct the SPR number -- SPR field is 2 5-bit fields
	 */

	spr = (spr_field >> 5) & 0x1f;
	spr |= (spr_field & 0x1f) << 5;

	/*
	 * Append the SPR name to the destination string using strcat()
	 */

	switch (spr)
	{
		/* UISA SPR register indexes */
		case SPR_XER:       strcat(dest, "xer");    break;
		case SPR_LR:        strcat(dest, "lr");     break;
		case SPR_CTR:       strcat(dest, "ctr");    break;

		/* VEA SPR register indexes */
		case SPRVEA_TBL_R:  strcat(dest, "tbl");    break;
		case SPRVEA_TBU_R:  strcat(dest, "tbu");    break;

		/* OEA SPR register indexes */
		case SPROEA_DSISR:  strcat(dest, "dsisr");  break;
		case SPROEA_DAR:    strcat(dest, "dar");    break;
		case SPROEA_DEC:    strcat(dest, "dec");    break;
		case SPROEA_SDR1:   strcat(dest, "sdr1");   break;
		case SPROEA_SRR0:   strcat(dest, "srr0");   break;
		case SPROEA_SRR1:   strcat(dest, "srr1");   break;
		case SPROEA_SPRG0:  strcat(dest, "sprg0");  break;
		case SPROEA_SPRG1:  strcat(dest, "sprg1");  break;
		case SPROEA_SPRG2:  strcat(dest, "sprg2");  break;
		case SPROEA_SPRG3:  strcat(dest, "sprg3");  break;
		case SPROEA_ASR:    strcat(dest, "asr");    break;
		case SPROEA_EAR:    strcat(dest, "ear");    break;
		case SPROEA_PVR:    strcat(dest, "pvr");    break;
		case SPROEA_IBAT0U: strcat(dest, "ibat0u"); break;
		case SPROEA_IBAT0L: strcat(dest, "ibat0l"); break;
		case SPROEA_IBAT1U: strcat(dest, "ibat1u"); break;
		case SPROEA_IBAT1L: strcat(dest, "ibat1l"); break;
		case SPROEA_IBAT2U: strcat(dest, "ibat2u"); break;
		case SPROEA_IBAT2L: strcat(dest, "ibat2l"); break;
		case SPROEA_IBAT3U: strcat(dest, "ibat3u"); break;
		case SPROEA_IBAT3L: strcat(dest, "ibat3l"); break;
		case SPROEA_DBAT0U: strcat(dest, "dbat0u"); break;
		case SPROEA_DBAT0L: strcat(dest, "dbat0l"); break;
		case SPROEA_DBAT1U: strcat(dest, "dbat1u"); break;
		case SPROEA_DBAT1L: strcat(dest, "dbat1l"); break;
		case SPROEA_DBAT2U: strcat(dest, "dbat2u"); break;
		case SPROEA_DBAT2L: strcat(dest, "dbat2l"); break;
		case SPROEA_DBAT3U: strcat(dest, "dbat3u"); break;
		case SPROEA_DBAT3L: strcat(dest, "dbat3l"); break;
		case SPROEA_DABR:   strcat(dest, "dabr/iac2");  break;  // unsupported on 603e/EC603e

		/* PowerPC 603E SPR register indexes */
		case SPR603_HID0:   strcat(dest, "hid0/dbsr");  break;
		case SPR603_HID1:   strcat(dest, "hid1");   break;
		case SPR603_DMISS:  strcat(dest, "dmiss");  break;
		case SPR603_DCMP:   strcat(dest, "dcmp");   break;
		case SPR603_HASH1:  strcat(dest, "hash1");  break;
		case SPR603_HASH2:  strcat(dest, "hash2/icdbdr");   break;
		case SPR603_IMISS:  strcat(dest, "imiss");  break;
		case SPR603_ICMP:   strcat(dest, "icmp/dear");  break;
		case SPR603_RPA:    strcat(dest, "rpa/evpr");   break;
		case SPR603_IABR:   strcat(dest, "iabr/dbcr");  break;

		/* PowerPC 4XX SPR register indexes */
		case SPR4XX_SGR:    strcat(dest, "sgr");    break;
		case SPR4XX_DCWR:   strcat(dest, "dcwr");   break;
		case SPR4XX_PID:    strcat(dest, "pid");    break;
		case SPR4XX_TBHU:   strcat(dest, "tbhu");   break;
		case SPR4XX_TBLU:   strcat(dest, "tblu");   break;
//      case SPR4XX_ICDBDR: strcat(dest, "icdbdr"); break;  // same as SPR603E_HASH2
//      case SPR4XX_DEAR:   strcat(dest, "dear");   break;  // same as SPR603E_ICMP
//      case SPR4XX_EVPR:   strcat(dest, "evpr");   break;  // same as SPR603E_RPA
		case SPR4XX_CDBCR:  strcat(dest, "cdbcr");  break;
		case SPR4XX_TSR:    strcat(dest, "tsr");    break;
		case SPR4XX_TCR:    strcat(dest, "tcr");    break;
		case SPR4XX_PIT:    strcat(dest, "pit");    break;
		case SPR4XX_TBHI:   strcat(dest, "tbhi");   break;
		case SPR4XX_TBLO:   strcat(dest, "tblo");   break;
		case SPR4XX_SRR2:   strcat(dest, "srr2");   break;
		case SPR4XX_SRR3:   strcat(dest, "srr3");   break;
//      case SPR4XX_DBSR:   strcat(dest, "dbsr");   break;  // same as SPR603E_HID0
//      case SPR4XX_DBCR:   strcat(dest, "dbcr");   break;  // same as SPR603E_IABR
		case SPR4XX_IAC1:   strcat(dest, "iac1");   break;
//      case SPR4XX_IAC2:   strcat(dest, "iac2");   break;  // same as SPROEA_DABR
		case SPR4XX_DAC1:   strcat(dest, "dac1");   break;
		case SPR4XX_DAC2:   strcat(dest, "dac2");   break;
		case SPR4XX_DCCR:   strcat(dest, "dccr");   break;
		case SPR4XX_ICCR:   strcat(dest, "iccr");   break;
		case SPR4XX_PBL1:   strcat(dest, "pbl1");   break;
		case SPR4XX_PBU1:   strcat(dest, "pbu1");   break;
		case SPR4XX_PBL2:   strcat(dest, "pbl2");   break;
		case SPR4XX_PBU2:   strcat(dest, "pbu2");   break;

		default:            sprintf(dest + strlen(dest), "%d", spr); break;
	}
}

static void DCR(char *dest, int dcr_field)
{
	int dcr;

	/*
	 * Construct the DCR number -- DCR field is 2 5-bit fields
	 */

	dcr = (dcr_field >> 5) & 0x1f;
	dcr |= (dcr_field & 0x1f) << 5;

	/*
	 * Append the DCR name to the destination string using strcat()
	 */

	switch (dcr)
	{
		case 144:   strcat(dest, "bear"); break;
		case 145:   strcat(dest, "besr"); break;
		case 128:   strcat(dest, "br0"); break;
		case 129:   strcat(dest, "br1"); break;
		case 130:   strcat(dest, "br2"); break;
		case 131:   strcat(dest, "br3"); break;
		case 132:   strcat(dest, "br4"); break;
		case 133:   strcat(dest, "br5"); break;
		case 134:   strcat(dest, "br6"); break;
		case 135:   strcat(dest, "br7"); break;
		case 112:   strcat(dest, "brh0"); break;
		case 113:   strcat(dest, "brh1"); break;
		case 114:   strcat(dest, "brh2"); break;
		case 115:   strcat(dest, "brh3"); break;
		case 116:   strcat(dest, "brh4"); break;
		case 117:   strcat(dest, "brh5"); break;
		case 118:   strcat(dest, "brh6"); break;
		case 119:   strcat(dest, "brh7"); break;
		case 196:   strcat(dest, "dmacc0"); break;
		case 204:   strcat(dest, "dmacc1"); break;
		case 212:   strcat(dest, "dmacc2"); break;
		case 220:   strcat(dest, "dmacc3"); break;
		case 192:   strcat(dest, "dmacr0"); break;
		case 200:   strcat(dest, "dmacr1"); break;
		case 208:   strcat(dest, "dmacr2"); break;
		case 216:   strcat(dest, "dmacr3"); break;
		case 193:   strcat(dest, "dmact0"); break;
		case 201:   strcat(dest, "dmact1"); break;
		case 209:   strcat(dest, "dmact2"); break;
		case 217:   strcat(dest, "dmact3"); break;
		case 194:   strcat(dest, "dmada0"); break;
		case 202:   strcat(dest, "dmada1"); break;
		case 210:   strcat(dest, "dmada2"); break;
		case 218:   strcat(dest, "dmada3"); break;
		case 195:   strcat(dest, "dmasa0"); break;
		case 203:   strcat(dest, "dmasa1"); break;
		case 211:   strcat(dest, "dmasa2"); break;
		case 219:   strcat(dest, "dmasa3"); break;
		case 224:   strcat(dest, "dmasr"); break;
		case 66:    strcat(dest, "exier"); break;
		case 64:    strcat(dest, "exisr"); break;
		case 160:   strcat(dest, "iocr"); break;

		default:    sprintf(dest + strlen(dest), "%d", dcr); break;
	}
}


/*
 * DecodeSigned16():
 *
 * Predecodes the SIMM field for us. If do_unsigned, it is printed as an
 * unsigned 16-bit integer.
 */

static void DecodeSigned16(char *outbuf, UINT32 op, int do_unsigned)
{
	INT16 s;

	s = G_SIMM(op);
	if (do_unsigned)    // sign extend to unsigned 32-bits
		sprintf(outbuf, "0x%04X", (UINT32) s);
	else                // print as signed 16 bits
	{
		if (s < 0)
		{
			s *= -1;
			sprintf(outbuf, "-0x%04X", s);
		}
		else
			sprintf(outbuf, "0x%04X",s);
	}
}

/*
 * Mask():
 *
 * Generate a mask from bit MB through ME (PPC-style backwards bit numbering.)
 */

static UINT32 Mask(int mb, int me)
{
	UINT32  i, mask;

	mb &= 31;
	me &= 31;

	i = mb;
	mask = 0;
	while (1)
	{
		mask |= (1 << (31 - i));
		if (i == me)
			break;
		i = (i + 1) & 31;
	}

	return mask;
}

/*
 * Check():
 *
 * Perform checks on the instruction as required by the flags. Returns 1 if
 * the instruction failed.
 */

#if 0
static int Check(UINT32 op, int flags)
{
	int nb, rt, ra;

	if( !flags ) return 0;  // nothing to check for!

	rt = G_RT(op);
	ra = G_RA(op);

	if (flags & FL_CHECK_RA_RT) // invalid if rA==0 or rA==rT
	{
		if ((G_RA(op) == 0) || (G_RA(op) == G_RT(op)))
			return 1;
	}

	if (flags & FL_CHECK_RA)    // invalid if rA==0
	{
		if (G_RA(op) == 0)
			return 1;
	}

	if (flags & FL_CHECK_LSWI)
	{
		/*
		 * Check that rA is not in the range of registers to be loaded (even
		 * if rA == 0)
		 */

		nb = G_NB(op);

		if (ra >= rt && ra <= (rt + nb - 1))    return 1;
		if ((rt + nb - 1) > 31) // register wrap-around!
		{
			if (ra < ((rt + nb - 1) - 31))
				return 1;
		}
	}

	if (flags & FL_CHECK_LSWX)
	{
		/*
		 * Check that rT != rA, rT != rB, and rD and rA both do not specify
		 * R0.
		 *
		 * We cannot check fully whether rA or rB are in the range of
		 * registers specified to be loaded because that depends on XER.
		 */

		if (rt == ra || rt == G_RB(op) || ((rt == 0) && (ra == 0)))
			return 1;
	}

	return 0;   // passed checks
}
#endif
/*
 * Simplified():
 *
 * Handles all simplified instruction forms. Returns 1 if one was decoded,
 * otherwise 0 to indicate disassembly should carry on as normal.
 */

static int Simplified(UINT32 op, UINT32 vpc, char *signed16, char *mnem, char *oprs)
{
	UINT32  value, disp;

	value = G_SIMM(op); // value is fully sign-extended SIMM field
	if (value & 0x8000)
		value |= 0xffff0000;

	if (op == (D_OP(24)|D_RT(0)|D_RA(0)|D_UIMM(0)))
		strcat(mnem, "nop");        // ori r0,r0,0 -> nop
	else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(444)))
	{
		if (G_RT(op) == G_RB(op))
		{
			strcat(mnem, "mr");     // orx rA,rT,rT -> mrx rA,rT
			if (op & M_RC)  strcat(mnem, ".");
			sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
		}
		else
			return 0;
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(124)))
	{
		if (G_RT(op) == G_RB(op))
		{
			strcat(mnem, "not");    // nor rA,rT,rT -> not rA,rT
			if (op & M_RC)  strcat(mnem, ".");
			sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
		}
		else
			return 0;
	}
	else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(14))
	{
		if (G_RA(op) == 0)
		{
			strcat(mnem, "li");     // addi rT,0,value -> li rT,value
			sprintf(oprs, "r%d,0x%08X", G_RT(op), value);
		}
		else
			return 0;
	}
	else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(15))
	{
		if (G_RA(op) == 0)
		{
			strcat(mnem, "li"); // addis rT,0,value -> li rT,(value<<16)
			sprintf(oprs, "r%d,0x%08X", G_RT(op), value << 16);
		}
		else
		{
			strcat(mnem, "addi");   // addis rT,rA,SIMM -> addi rT,rA,SIMM<<16
			sprintf(oprs, "r%d,r%d,0x%08X", G_RT(op), G_RA(op), value << 16);
		}
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(29))
	{
		strcat(mnem, "andi.");  // andis. rA,rT,UIMM -> andi. rA,rT,UIMM<<16
		sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(25))
	{
		strcat(mnem, "ori");    // oris rA,rT,UIMM -> ori rA,rT,UIMM<<16
		sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(27))
	{
		strcat(mnem, "xori");   // xoris rA,rT,UIMM -> xori rA,rT,UIMM<<16
		sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_SH|M_MB|M_ME|M_RC)) == D_OP(20))
	{
		value = Mask(G_MB(op), G_ME(op));
		strcat(mnem, "rlwimi"); // rlwimi[.] rA,rT,SH,MB,ME -> rlwimi[.] rA,rT,SH,MASK
		if (op & M_RC) strcat(mnem, ".");
		sprintf(oprs, "r%d,r%d,%d,0x%08X", G_RA(op), G_RT(op), G_SH(op), value);
	}
	else if ((op & ~(M_RT|M_RA|M_SH|M_MB|M_ME|M_RC)) == D_OP(21))
	{
		value = Mask(G_MB(op), G_ME(op));
		if (G_SH(op) == 0)      // rlwinm[.] rA,rT,0,MB,ME -> and[.] rA,rT,MASK
		{
			strcat(mnem, "and");
			if (op & M_RC) strcat(mnem, ".");
			sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), value);
		}
		else                    // rlwinm[.] rA,rT,SH,MASK
		{
			strcat(mnem, "rlwinm");
			if (op & M_RC) strcat(mnem, ".");
			sprintf(oprs, "r%d,r%d,%d,0x%08X", G_RA(op), G_RT(op), G_SH(op), value);
		}
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_MB|M_ME|M_RC)) == D_OP(23))
	{
		value = Mask(G_MB(op), G_ME(op));
		strcat(mnem, "rlwnm");  // rlwnm[.] rA,rT,SH,MB,ME -> rlwnm[.] rA,rT,SH,MASK
		if (op & M_RC) strcat(mnem, ".");
		sprintf(oprs, "r%d,r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_RB(op), value);
	}
	else if ((op & ~(M_BO|M_BI|M_BD|M_AA|M_LK)) == D_OP(16))
	{
		disp = G_BD(op) * 4;
		if (disp & 0x00008000)
			disp |= 0xffff0000;

		switch (G_BO(op))
		{
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
				strcat(mnem, "b");
				strcat(mnem, crnbit[G_BI(op) & 3]);
				break;
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				strcat(mnem, "b");
				strcat(mnem, crbit[G_BI(op) & 3]);
				break;
			case 0x10:  case 0x11:  case 0x18:  case 0x19:
				strcat(mnem, "bdnz");
				break;
			case 0x12:  case 0x13:  case 0x1a:  case 0x1b:
				strcat(mnem, "bdz");
				break;
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				strcat(mnem, "b");
				break;
			default:
				return 0;
		}

		if (op & M_LK)  strcat(mnem, "l");
		if (op & M_AA)  strcat(mnem, "a");

		if (!(G_BO(op) & 0x10) && G_BI(op) / 4 != 0)
			sprintf(oprs, "cr%d,0x%08X", G_BI(op) / 4, disp + ((op & M_AA) ? 0 : vpc));
		else
			sprintf(oprs, "0x%08X", disp + ((op & M_AA) ? 0 : vpc));
	}
	else if ((op & ~(M_BO|M_BI|M_LK)) == (D_OP(19)|D_XO(528)) || (op & ~(M_BO|M_BI|M_LK)) == (D_OP(19)|D_XO(16)))
	{
		switch (G_BO(op))
		{
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
				strcat(mnem, "b");
				strcat(mnem, crnbit[G_BI(op) & 3]);
				break;
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				strcat(mnem, "b");
				strcat(mnem, crbit[G_BI(op) & 3]);
				break;
			case 0x10:  case 0x11:  case 0x18:  case 0x19:
				strcat(mnem, "bdnz");
				break;
			case 0x12:  case 0x13:  case 0x1a:  case 0x1b:
				strcat(mnem, "bdz");
				break;
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				strcat(mnem, "b");
				break;
			default:
				return 0;
		}

		strcat(mnem, (G_XO(op) == 528) ? "ctr" : "lr");
		if (op & M_LK)  strcat(mnem, "l");
		if (op & M_AA)  strcat(mnem, "a");

		if (!(G_BO(op) & 0x10) && G_BI(op) / 4 != 0)
			sprintf(oprs, "cr%d", G_BI(op) / 4);
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(40)))
	{
		strcat(mnem, "sub");
		if (op & M_OE) strcat(mnem, "o");
		if (op & M_RC) strcat(mnem, ".");
		sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(8)))
	{
		strcat(mnem, "subc");
		if (op & M_OE) strcat(mnem, "o");
		if (op & M_RC) strcat(mnem, ".");
		sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
	}
	else
		return 0;   // no match

	return 1;
}

offs_t ppc_dasm_one(char *buffer, UINT32 pc, UINT32 op)
{
	char signed16[12];
	UINT32 disp;
	int i,j;
	char mnem[200];
	char oprs[200];
	offs_t flags = DASMFLAG_SUPPORTED;

	mnem[0] = '\0'; // so we can use strcat()
	oprs[0] = '\0';

	/*
	 * Decode signed 16-bit fields (SIMM and d) to spare us the work later
	 */

	DecodeSigned16(signed16, op, 0);

	/*
	 * Try simplified forms first, then real instructions
	 */

	if( Simplified(op, pc, signed16, mnem, oprs) ) {
		buffer += sprintf(buffer, "%s", mnem);
		for( j = strlen(mnem); j < 10; j++ ) {
			buffer += sprintf(buffer, " ");
		}
		buffer += sprintf(buffer, "%s", oprs);
		return 4 | flags;
	}

	/*
	 * Search for the instruction in the list and print it if there's a match
	 */

	for (i = 0; i < sizeof(itab) / sizeof(IDESCR); i++)
	{
		if ((op & ~itab[i].mask) == itab[i].match)  // check for match
		{
			/*
			 * Base mnemonic followed be O, ., L, A
			 */

			strcat(mnem, itab[i].mnem);
			if (itab[i].flags & FL_OE)  if (op & M_OE) strcat(mnem, "o");
			if (itab[i].flags & FL_RC)  if (op & M_RC) strcat(mnem, ".");
			if (itab[i].flags & FL_LK)  if (op & M_LK) strcat(mnem, "l");
			if (itab[i].flags & FL_AA)  if (op & M_AA) strcat(mnem, "a");

			/*
			 * Print operands
			 */

			switch (itab[i].format)
			{
			case F_RT_RA_RB:
				sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_RT_RA_0_SIMM:
				if (G_RA(op))
					sprintf(oprs, "r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
				else
					sprintf(oprs, "r%d,0,%s", G_RT(op), signed16);
				break;

			case F_ADDIS:
				if (G_RA(op))
					sprintf(oprs, "r%d,r%d,0x%04X", G_RT(op), G_RA(op), G_SIMM(op));
				else
					sprintf(oprs, "r%d,0,0x%04X", G_RT(op), G_SIMM(op));
				break;

			case F_RT_RA_SIMM:
				sprintf(oprs, "r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
				break;

			case F_RT_RA:
				sprintf(oprs, "r%d,r%d", G_RT(op), G_RA(op));
				break;

			case F_RA_RT_RB:
				sprintf(oprs, "r%d,r%d,r%d", G_RA(op), G_RT(op), G_RB(op));
				break;

			case F_RA_RT_UIMM:
				sprintf(oprs, "r%d,r%d,0x%04X", G_RA(op), G_RT(op), G_UIMM(op));
				break;

			case F_LI:
				disp = G_LI(op) * 4;
				if (disp & 0x02000000)  // sign extend
					disp |= 0xfc000000;
				sprintf(oprs, "0x%08X", disp + ((op & M_AA) ? 0 : pc));
				break;

			case F_BCx:
				disp = G_BD(op) * 4;
				if (disp & 0x00008000)
					disp |= 0xffff0000;

				if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
					sprintf(oprs, "0x%02X,%d,0x%08X", G_BO(op), G_BI(op), disp + ((op & M_AA) ? 0 : pc));
				else                    // BI gives us the condition bit
					sprintf(oprs, "0x%02X,cr%d[%s],0x%08X", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3], disp + ((op & M_AA) ? 0 : pc));
				break;

			case F_BO_BI:
				if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
					sprintf(oprs, "0x%02X,%d", G_BO(op), G_BI(op));
				else
					sprintf(oprs, "0x%02X,cr%d[%s]", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3]);
				break;

			case F_CMP:
				if (G_L(op))
					strcat(mnem, "d");
				if (G_CRFD(op) == 0)
					sprintf(oprs, "r%d,r%d", G_RA(op), G_RB(op));
				else
					sprintf(oprs, "cr%d,r%d,r%d", G_CRFD(op), G_RA(op), G_RB(op));
				break;

			case F_CMP_SIMM:
				if (G_L(op))
					strcat(mnem, "d");
				if (G_CRFD(op) == 0)
					sprintf(oprs, "r%d,%s", G_RA(op), signed16);
				else
					sprintf(oprs, "cr%d,r%d,%s", G_CRFD(op), G_RA(op), signed16);
				break;

			case F_CMP_UIMM:
				if (G_L(op))
					strcat(mnem, "d");
				if (G_CRFD(op) == 0)
					sprintf(oprs, "r%d,0x%04X", G_RA(op), G_UIMM(op));
				else
					sprintf(oprs, "cr%d,r%d,0x%04X", G_CRFD(op), G_RA(op), G_UIMM(op));
				break;

			case F_RA_RT:
				sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
				break;

			case F_CRBD_CRBA_CRBB:
				sprintf(oprs, "cr%d[%s],cr%d[%s],cr%d[%s]", G_CRBD(op) / 4, crbit[G_CRBD(op) & 3], G_CRBA(op) / 4, crbit[G_CRBA(op) & 3], G_CRBB(op) / 4, crbit[G_CRBB(op) & 3]);
				break;

			case F_RA_0_RB:
				if (G_RA(op))
					sprintf(oprs, "r%d,r%d", G_RA(op), G_RB(op));
				else
					sprintf(oprs, "0,r%d", G_RB(op));
				break;

			case F_RT_RA_0_RB:
				if (G_RA(op))
					sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				else
					sprintf(oprs, "r%d,0,r%d", G_RT(op), G_RB(op));
				break;

			case F_FRT_FRB:
				sprintf(oprs, "f%d,f%d", G_RT(op), G_RB(op));
				break;

			case F_FRT_FRA_FRB:
				sprintf(oprs, "f%d,f%d,f%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_FCMP:
				sprintf(oprs, "cr%d,f%d,f%d", G_CRFD(op), G_RA(op), G_RB(op));
				break;

			case F_FRT_FRA_FRC_FRB:
				sprintf(oprs, "f%d,f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op), G_RB(op));
				break;

			case F_FRT_FRA_FRC:
				sprintf(oprs, "f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op));
				break;

			case F_RT_D_RA_0:
				if (G_RA(op))
					sprintf(oprs, "r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				else
					sprintf(oprs, "r%d,0x%08X", G_RT(op), (UINT32) ((INT16) G_D(op)));
				break;

			case F_RT_D_RA:
				sprintf(oprs, "r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				break;

			case F_FRT_D_RA_0:
				if (G_RA(op))
					sprintf(oprs, "f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				else
					sprintf(oprs, "f%d,0x%08X", G_RT(op), (UINT32) ((INT16) G_D(op)));
				break;

			case F_FRT_D_RA:
				sprintf(oprs, "f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				break;

			case F_FRT_RA_RB:
				sprintf(oprs, "f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_FRT_RA_0_RB:
				if (G_RA(op))
					sprintf(oprs, "f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				else
					sprintf(oprs, "f%d,0,r%d", G_RT(op), G_RB(op));
				break;

			case F_RT_RA_0_NB:
				if (G_RA(op))
					sprintf(oprs, "r%d,r%d,%d", G_RT(op), G_RA(op), G_NB(op) ? G_NB(op) : 32);
				else
					sprintf(oprs, "r%d,0,%d", G_RT(op), G_NB(op) ? G_NB(op) : 32);
				break;

			case F_CRFD_CRFS:
				sprintf(oprs, "cr%d,cr%d", G_CRFD(op), G_CRFS(op));
				break;

			case F_MCRXR:
				sprintf(oprs, "cr%d", G_CRFD(op));
				break;

			case F_RT:
				sprintf(oprs, "r%d", G_RT(op));
				break;

			case F_MFFSx:
				sprintf(oprs, "f%d", G_RT(op));
				break;

			case F_FCRBD:
				sprintf(oprs, "fpscr[%d]", G_CRBD(op));
				break;

			case F_RT_SPR:
				sprintf(oprs, "r%d,", G_RT(op));
				SPR(oprs, G_SPR(op));
				break;

			case F_RT_DCR:
				sprintf(oprs, "r%d,", G_RT(op));
				DCR(oprs, G_DCR(op));
				break;

			case F_MFSR:
				sprintf(oprs, "r%d,sr%d", G_RT(op), G_SR(op));
				break;

			case F_MTCRF:
				sprintf(oprs, "0x%02X,r%d", G_CRM(op), G_RT(op));
				break;

			case F_MTFSFx:
				sprintf(oprs, "0x%02X,f%d", G_FM(op), G_RB(op));
				break;

			case F_MTFSFIx:
				sprintf(oprs, "cr%d,0x%X", G_CRFD(op), G_IMM(op));
				break;

			case F_MTSPR:
				SPR(oprs, G_SPR(op));
				sprintf(oprs + strlen(oprs), ",r%d", G_RT(op));
				break;

			case F_MTDCR:
				DCR(oprs, G_DCR(op));
				sprintf(oprs + strlen(oprs), ",r%d", G_RT(op));
				break;

			case F_MTSR:
				sprintf(oprs, "sr%d,r%d", G_SR(op), G_RT(op));
				break;

			case F_RT_RB:
				sprintf(oprs, "r%d,r%d", G_RT(op), G_RB(op));
				break;

			case F_RA_RT_SH_MB_ME:
				sprintf(oprs, "r%d,r%d,%d,%d,%d", G_RA(op), G_RT(op), G_SH(op), G_MB(op), G_ME(op));
				break;

			case F_RLWNMx:
				sprintf(oprs, "r%d,r%d,r%d,%d,%d", G_RA(op), G_RT(op), G_RB(op), G_MB(op), G_ME(op));
				break;

			case F_SRAWIx:
				sprintf(oprs, "r%d,r%d,%d", G_RA(op), G_RT(op), G_SH(op));
				break;

			case F_RB:
				sprintf(oprs, "r%d", G_RB(op));
				break;

			case F_TW:
				sprintf(oprs, "%d,r%d,r%d", G_TO(op), G_RA(op), G_RB(op));
				break;

			case F_TWI:
				sprintf(oprs, "%d,r%d,%s", G_TO(op), G_RA(op), signed16);
				break;

			case F_NONE:
			default:
				break;
			}

			if ((itab[i].flags & FL_LK) && (op & M_LK))
				flags |= DASMFLAG_STEP_OVER;
			else if (itab[i].flags & FL_SO)
				flags |= DASMFLAG_STEP_OUT;

			buffer += sprintf(buffer, "%s", mnem);
			for( j = strlen(mnem); j < 10; j++ ) {
				buffer += sprintf(buffer, " ");
			}
			buffer += sprintf(buffer, "%s", oprs);
			return 4 | flags;
		}
	}

	sprintf(buffer, "?");
	return 4 | flags;
}

CPU_DISASSEMBLE( powerpc )
{
	UINT32 op = *(UINT32 *)oprom;
	op = BIG_ENDIANIZE_INT32(op);
	return ppc_dasm_one(buffer, pc, op);
}
