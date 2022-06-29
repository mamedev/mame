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
#include "ppc_dasm.h"

#include "ppccom.h"


/*
 * Instruction Table
 *
 * Table of instruction descriptors which allows the disassembler to decode
 * and print instructions.
 */

constexpr int I_COMMON = powerpc_disassembler::I_POWER | powerpc_disassembler::I_POWERPC;

const powerpc_disassembler::IDESCR powerpc_disassembler::itab[] =
{
	{ "a",      D_OP(31)|D_XO(10),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "abs",    D_OP(31)|D_XO(360), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "add",    D_OP(31)|D_XO(266), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "addc",   D_OP(31)|D_XO(10),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "adde",   D_OP(31)|D_XO(138), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "addi",   D_OP(14),           M_RT|M_RA|M_SIMM,           F_RT_RA_0_SIMM, 0,              I_POWERPC },
	{ "addic",  D_OP(12),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWERPC },
	{ "addic.", D_OP(13),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWERPC },
	{ "addis",  D_OP(15),           M_RT|M_RA|M_SIMM,           F_ADDIS,        0,              I_POWERPC },
	{ "addme",  D_OP(31)|D_XO(234), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWERPC },
	{ "addze",  D_OP(31)|D_XO(202), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWERPC },
	{ "ae",     D_OP(31)|D_XO(138), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "ai",     D_OP(12),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWER },
	{ "ai.",    D_OP(13),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWER },
	{ "ame",    D_OP(31)|D_XO(234), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "and",    D_OP(31)|D_XO(28),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "andc",   D_OP(31)|D_XO(60),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "andi.",  D_OP(28),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "andil.", D_OP(28),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },
	{ "andis.", D_OP(29),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "andiu.", D_OP(29),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },
	{ "aze",    D_OP(31)|D_XO(202), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "b",      D_OP(18),           M_LI|M_AA|M_LK,             F_LI,           FL_AA|FL_LK,    I_COMMON },
	{ "bc",     D_OP(16),           M_BO|M_BI|M_BD|M_AA|M_LK,   F_BCx,          FL_AA|FL_LK,    I_COMMON },
	{ "bcc",    D_OP(19)|D_XO(528), M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK,          I_POWER },
	{ "bcctr",  D_OP(19)|D_XO(528), M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK,          I_POWERPC },
	{ "bclr",   D_OP(19)|D_XO(16),  M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK|FL_SO,    I_POWERPC },
	{ "bcr",    D_OP(19)|D_XO(16),  M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK|FL_SO,    I_POWER },
	{ "cal",    D_OP(14),           M_RT|M_RA|M_SIMM,           F_RT_D_RA_0,    0,              I_POWER },
	{ "cau",    D_OP(15),           M_RT|M_RA|M_SIMM,           F_ADDIS,        0,              I_POWER },
	{ "cax",    D_OP(31)|D_XO(266), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "clcs",   D_OP(31)|D_XO(531), M_RT|M_RA,                  F_RT_RA,        0,              I_POWER },
	{ "clf",    D_OP(31)|D_XO(118), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWER },
	{ "cli",    D_OP(31)|D_XO(502), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWER },
	{ "cmp",    D_OP(31)|D_XO(0),   M_CRFD|M_RA|M_RB,           F_CMP,          0,              I_COMMON },
	{ "cmpd",   D_OP(31)|D_XO(0)|M_L,M_CRFD|M_RA|M_RB,          F_CMP,          0,              I_POWERPC },
	{ "cmpi",   D_OP(11),           M_CRFD|M_RA|M_SIMM,         F_CMP_SIMM,     0,              I_COMMON },
	{ "cmpdi",  D_OP(11)|M_L,       M_CRFD|M_RA|M_SIMM,         F_CMP_SIMM,     0,              I_POWERPC },
	{ "cmpl",   D_OP(31)|D_XO(32),  M_CRFD|M_RA|M_RB,           F_CMP,          0,              I_COMMON },
	{ "cmpld",  D_OP(31)|D_XO(32)|M_L,M_CRFD|M_RA|M_RB,         F_CMP,          0,              I_POWERPC },
	{ "cmpli",  D_OP(10),           M_CRFD|M_RA|M_UIMM,         F_CMP_UIMM,     0,              I_COMMON },
	{ "cmpldi", D_OP(10)|M_L,       M_CRFD|M_RA|M_UIMM,         F_CMP_UIMM,     0,              I_POWERPC },
	{ "cntlz",  D_OP(31)|D_XO(26),  M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC,          I_POWER },
	{ "cntlzw", D_OP(31)|D_XO(26),  M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC,          I_POWERPC },
	{ "crand",  D_OP(19)|D_XO(257), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "crandc", D_OP(19)|D_XO(129), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "creqv",  D_OP(19)|D_XO(289), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "crnand", D_OP(19)|D_XO(225), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "crnor",  D_OP(19)|D_XO(33),  M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "cror",   D_OP(19)|D_XO(449), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "crorc",  D_OP(19)|D_XO(417), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "crxor",  D_OP(19)|D_XO(193), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0,          I_COMMON },
	{ "dcba",   D_OP(31)|D_XO(758), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbf",   D_OP(31)|D_XO(86),  M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbi",   D_OP(31)|D_XO(470), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbst",  D_OP(31)|D_XO(54),  M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbt",   D_OP(31)|D_XO(278), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbtst", D_OP(31)|D_XO(246), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcbz",   D_OP(31)|D_XO(1014),M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dclst",  D_OP(31)|D_XO(630), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWER },
	{ "dclz",   D_OP(31)|D_XO(1014),M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWER },
	{ "dcs",    D_OP(31)|D_XO(598), 0,                          F_NONE,         0,              I_POWER },
	{ "dccci",  D_OP(31)|D_XO(454), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "dcread", D_OP(31)|D_XO(486), M_RA|M_RB,                  F_RT_RA_RB,     0,              I_POWERPC },
	{ "div",    D_OP(31)|D_XO(331), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "divs",   D_OP(31)|D_XO(363), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "divw",   D_OP(31)|D_XO(491), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "divwu",  D_OP(31)|D_XO(459), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "doz",    D_OP(31)|D_XO(264), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "dozi",   D_OP(9),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWER },
	{ "dsa",    D_OP(31)|D_XO(628), 0,                          0,              0,              I_POWERPC },
	{ "eciwx",  D_OP(31)|D_XO(310), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "ecowx",  D_OP(31)|D_XO(438), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "eieio",  D_OP(31)|D_XO(854), 0,                          F_NONE,         0,              I_POWERPC },
	{ "eqv",    D_OP(31)|D_XO(284), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "esa",    D_OP(31)|D_XO(596), 0,                          0,              0,              I_POWERPC },
	{ "exts",   D_OP(31)|D_XO(922), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC,          I_POWER },
	{ "extsb",  D_OP(31)|D_XO(954), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC,          I_POWERPC },
	{ "extsh",  D_OP(31)|D_XO(922), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC,          I_POWERPC },
	{ "fa",     D_OP(63)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWER },
	{ "fabs",   D_OP(63)|D_XO(264), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_COMMON },
	{ "fadd",   D_OP(63)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "fadds",  D_OP(59)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "fcir",   D_OP(63)|D_XO(14),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWER },
	{ "fcirz",  D_OP(63)|D_XO(15),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWER },
	{ "fcmpo",  D_OP(63)|D_XO(32),  M_CRFD|M_RA|M_RB,           F_FCMP,         0,              I_COMMON },
	{ "fcmpu",  D_OP(63)|D_XO(0),   M_CRFD|M_RA|M_RB,           F_FCMP,         0,              I_COMMON },
	{ "fctiw",  D_OP(63)|D_XO(14),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "fctiwz", D_OP(63)|D_XO(15),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "fd",     D_OP(63)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWER },
	{ "fdiv",   D_OP(63)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "fdivs",  D_OP(59)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "fm",     D_OP(63)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC,          I_POWER },
	{ "fma",    D_OP(63)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWER },
	{ "fmadd",  D_OP(63)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fmadds", D_OP(59)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fmr",    D_OP(63)|D_XO(72),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_COMMON },
	{ "fms",    D_OP(63)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWER },
	{ "fmsub",  D_OP(63)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fmsubs", D_OP(59)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fmul",   D_OP(63)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC,          I_POWERPC },
	{ "fmuls",  D_OP(59)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC,          I_POWERPC },
	{ "fnabs",  D_OP(63)|D_XO(136), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_COMMON },
	{ "fneg",   D_OP(63)|D_XO(40),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_COMMON },
	{ "fnma",   D_OP(63)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWER },
	{ "fnmadd", D_OP(63)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fnmadds",D_OP(59)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fnms",   D_OP(63)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWER },
	{ "fnmsub", D_OP(63)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fnmsubs",D_OP(59)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fres",   D_OP(59)|D_XO(24),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "frsp",   D_OP(63)|D_XO(12),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_COMMON },
	{ "frsqrte",D_OP(63)|D_XO(26),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "fs",     D_OP(63)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWER },
	{ "fsel",   D_OP(63)|D_XO(23),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC,      I_POWERPC },
	{ "fsqrt",  D_OP(63)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "fsqrts", D_OP(59)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC,          I_POWERPC },
	{ "fsub",   D_OP(63)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "fsubs",  D_OP(59)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC,          I_POWERPC },
	{ "icbi",   D_OP(31)|D_XO(982), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "icbt",   D_OP(31)|D_XO(262), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "iccci",  D_OP(31)|D_XO(966), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "icread", D_OP(31)|D_XO(998), M_RA|M_RB,                  F_RA_0_RB,      0,              I_POWERPC },
	{ "ics",    D_OP(19)|D_XO(150), 0,                          F_NONE,         0,              I_POWER },
	{ "isync",  D_OP(19)|D_XO(150), 0,                          F_NONE,         0,              I_POWERPC },
	{ "l",      D_OP(32),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWER },
	{ "lbz",    D_OP(34),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_COMMON },
	{ "lbzu",   D_OP(35),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT, I_COMMON },
	{ "lbzux",  D_OP(31)|D_XO(119), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT, I_COMMON },
	{ "lbzx",   D_OP(31)|D_XO(87),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "lfd",    D_OP(50),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_COMMON },
	{ "lfdu",   D_OP(51),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA,    I_COMMON },
	{ "lfdux",  D_OP(31)|D_XO(631), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA,    I_COMMON },
	{ "lfdx",   D_OP(31)|D_XO(599), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_COMMON },
	{ "lfs",    D_OP(48),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_COMMON },
	{ "lfsu",   D_OP(49),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA,    I_COMMON },
	{ "lfsux",  D_OP(31)|D_XO(567), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA,    I_COMMON },
	{ "lfsx",   D_OP(31)|D_XO(535), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_COMMON },
	{ "lha",    D_OP(42),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_COMMON },
	{ "lhau",   D_OP(43),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT, I_COMMON },
	{ "lhaux",  D_OP(31)|D_XO(375), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT, I_COMMON },
	{ "lhax",   D_OP(31)|D_XO(343), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "lhbrx",  D_OP(31)|D_XO(790), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "lhz",    D_OP(40),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_COMMON },
	{ "lhzu",   D_OP(41),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT, I_COMMON },
	{ "lhzux",  D_OP(31)|D_XO(311), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT, I_COMMON },
	{ "lhzx",   D_OP(31)|D_XO(279), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "lm",     D_OP(46),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWER },
	{ "lmw",    D_OP(46),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWERPC },
	{ "lscbx",  D_OP(31)|D_XO(277), M_RT|M_RA|M_RB|M_RC,        F_RT_RA_0_RB,   FL_RC,          I_POWER },
	{ "lsi",    D_OP(31)|D_XO(597), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   FL_CHECK_LSWI,  I_POWER },
	{ "lswi",   D_OP(31)|D_XO(597), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   FL_CHECK_LSWI,  I_POWERPC },
	{ "lswx",   D_OP(31)|D_XO(533), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   FL_CHECK_LSWX,  I_POWERPC },
	{ "lsx",    D_OP(31)|D_XO(533), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   FL_CHECK_LSWX,  I_POWER },
	{ "lu",     D_OP(33),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT, I_POWER },
	{ "lux",    D_OP(31)|D_XO(55),  M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT, I_POWER },
	{ "lwarx",  D_OP(31)|D_XO(20),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "lwbrx",  D_OP(31)|D_XO(534), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "lwz",    D_OP(32),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWERPC },
	{ "lwzu",   D_OP(33),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT, I_POWERPC },
	{ "lwzux",  D_OP(31)|D_XO(55),  M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT, I_POWERPC },
	{ "lwzx",   D_OP(31)|D_XO(23),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "lx",     D_OP(31)|D_XO(23),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWER },
	{ "maskg",  D_OP(31)|D_XO(29),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "maskir", D_OP(31)|D_XO(541), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "mcrf",   D_OP(19)|D_XO(0),   M_CRFD|M_CRFS,              F_CRFD_CRFS,    0,              I_COMMON },
	{ "mcrfs",  D_OP(63)|D_XO(64),  M_CRFD|M_CRFS,              F_CRFD_CRFS,    0,              I_COMMON },
	{ "mcrxr",  D_OP(31)|D_XO(512), M_CRFD,                     F_MCRXR,        0,              I_COMMON },
	{ "mfcr",   D_OP(31)|D_XO(19),  M_RT,                       F_RT,           0,              I_COMMON },
	{ "mfdcr",  D_OP(31)|D_XO(323), M_RT|M_DCR,                 F_RT_DCR,       0,              I_POWERPC },
	{ "mffs",   D_OP(63)|D_XO(583), M_RT|M_RC,                  F_MFFSx,        FL_RC,          I_COMMON },
	{ "mfmsr",  D_OP(31)|D_XO(83),  M_RT,                       F_RT,           0,              I_COMMON },
	{ "mfspr",  D_OP(31)|D_XO(339), M_RT|M_SPR,                 F_RT_SPR,       0,              I_COMMON },
	{ "mfsr",   D_OP(31)|D_XO(595), M_RT|M_SR,                  F_MFSR,         0,              I_COMMON },
	{ "mfsri",  D_OP(31)|D_XO(627), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWER },
	{ "mfsrin", D_OP(31)|D_XO(659), M_RT|M_RB,                  F_RT_RB,        0,              I_POWERPC },
	{ "mftb",   D_OP(31)|D_XO(371), M_RT|M_TBR,                 F_RT_SPR,       0,              I_POWERPC },
	{ "mtcrf",  D_OP(31)|D_XO(144), M_RT|M_CRM,                 F_MTCRF,        0,              I_COMMON },
	{ "mtdcr",  D_OP(31)|D_XO(451), M_RT|M_DCR,                 F_MTDCR,        0,              I_POWERPC },
	{ "mtfsb0", D_OP(63)|D_XO(70),  M_CRBD|M_RC,                F_FCRBD,        FL_RC,          I_COMMON },
	{ "mtfsb1", D_OP(63)|D_XO(38),  M_CRBD|M_RC,                F_FCRBD,        FL_RC,          I_COMMON },
	{ "mtfsf",  D_OP(63)|D_XO(711), M_FM|M_RB|M_RC,             F_MTFSFx,       FL_RC,          I_COMMON },
	{ "mtfsfi", D_OP(63)|D_XO(134), M_CRFD|M_IMM|M_RC,          F_MTFSFIx,      FL_RC,          I_COMMON },
	{ "mtmsr",  D_OP(31)|D_XO(146), M_RT,                       F_RT,           0,              I_COMMON },
	{ "mtspr",  D_OP(31)|D_XO(467), M_RT|M_SPR,                 F_MTSPR,        0,              I_COMMON },
	{ "mtsr",   D_OP(31)|D_XO(210), M_RT|M_SR,                  F_MTSR,         0,              I_COMMON },
	{ "mtsri",  D_OP(31)|D_XO(242), M_RT|M_RB,                  F_RT_RB,        0,              I_POWER },
	{ "mtsrin", D_OP(31)|D_XO(242), M_RT|M_RB,                  F_RT_RB,        0,              I_POWERPC },
	{ "mul",    D_OP(31)|D_XO(107), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "mulhw",  D_OP(31)|D_XO(75),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC,          I_POWERPC },
	{ "mulhwu", D_OP(31)|D_XO(11),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC,          I_POWERPC },
	{ "muli",   D_OP(7),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWER },
	{ "mulli",  D_OP(7),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWERPC },
	{ "mullw",  D_OP(31)|D_XO(235), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "muls",   D_OP(31)|D_XO(235), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "nabs",   D_OP(31)|D_XO(488), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "nand",   D_OP(31)|D_XO(476), M_RA|M_RT|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "neg",    D_OP(31)|D_XO(104), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_COMMON },
	{ "nor",    D_OP(31)|D_XO(124), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "or",     D_OP(31)|D_XO(444), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "orc",    D_OP(31)|D_XO(412), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "ori",    D_OP(24),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "oril",   D_OP(24),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },
	{ "oris",   D_OP(25),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "oriu",   D_OP(25),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },
	{ "rac",    D_OP(31)|D_XO(818), M_RT|M_RA|M_RB|M_RC,        F_RT_RA_0_RB,   FL_RC,          I_POWER },
	{ "rfi",    D_OP(19)|D_XO(50),  0,                          F_NONE,         0,              I_COMMON },
	{ "rfci",   D_OP(19)|D_XO(51),  0,                          F_NONE,         0,              I_POWERPC },
	{ "rfsvc",  D_OP(19)|D_XO(82),  M_LK,                       F_NONE,         FL_LK,          I_POWER },
	{ "rlimi",  D_OP(20),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC, F_RA_RT_SH_MB_ME,   FL_RC,   I_POWER },
	{ "rlinm",  D_OP(21),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC, F_RA_RT_SH_MB_ME,   FL_RC,   I_POWER },
	{ "rlmi",   D_OP(22),           M_RT|M_RA|M_RB|M_MB|M_ME|M_RC, F_RA_RT_RB_MB_ME,   FL_RC,   I_POWER },
	{ "rlnm",   D_OP(23),           M_RT|M_RA|M_RB|M_MB|M_ME|M_RC, F_RA_RT_RB_MB_ME,   FL_RC,   I_POWER },
	{ "rlwimi", D_OP(20),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC, F_RA_RT_SH_MB_ME,   FL_RC,   I_POWERPC },
	{ "rlwinm", D_OP(21),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC, F_RA_RT_SH_MB_ME,   FL_RC,   I_POWERPC },
	{ "rlwnm",  D_OP(23),           M_RT|M_RA|M_RB|M_MB|M_ME|M_RC, F_RA_RT_RB_MB_ME,   FL_RC,   I_POWERPC },
	{ "rrib",   D_OP(31)|D_XO(537), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sc",     D_OP(17)|2,         0,                          F_NONE,         0,              I_POWERPC },
	{ "sf",     D_OP(31)|D_XO(8),   M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "sfe",    D_OP(31)|D_XO(136), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWER },
	{ "sfi",    D_OP(8),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWER },
	{ "sfme",   D_OP(31)|D_XO(232), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "sfze",   D_OP(31)|D_XO(200), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWER },
	{ "sl",     D_OP(31)|D_XO(24),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sle",    D_OP(31)|D_XO(153), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sleq",   D_OP(31)|D_XO(217), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sliq",   D_OP(31)|D_XO(184), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "slliq",  D_OP(31)|D_XO(248), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "sllq",   D_OP(31)|D_XO(216), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "slq",    D_OP(31)|D_XO(152), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "slw",    D_OP(31)|D_XO(24),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWERPC },
	{ "sr",     D_OP(31)|D_XO(536), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sra",    D_OP(31)|D_XO(792), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "srai",   D_OP(31)|D_XO(824), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "sraiq",  D_OP(31)|D_XO(952), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "sraq",   D_OP(31)|D_XO(920), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sraw",   D_OP(31)|D_XO(792), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWERPC },
	{ "srawi",  D_OP(31)|D_XO(824), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWERPC },
	{ "sre",    D_OP(31)|D_XO(665), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "srea",   D_OP(31)|D_XO(921), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sreq",   D_OP(31)|D_XO(729), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "sriq",   D_OP(31)|D_XO(696), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "srliq",  D_OP(31)|D_XO(184), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC,          I_POWER },
	{ "srlq",   D_OP(31)|D_XO(728), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "srq",    D_OP(31)|D_XO(664), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWER },
	{ "srw",    D_OP(31)|D_XO(536), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_POWERPC },
	{ "st",     D_OP(36),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWER },
	{ "stb",    D_OP(38),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_COMMON },
	{ "stbrx",  D_OP(31)|D_XO(662), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWER },
	{ "stbu",   D_OP(39),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA,    I_COMMON },
	{ "stbux",  D_OP(31)|D_XO(247), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA,    I_COMMON },
	{ "stbx",   D_OP(31)|D_XO(215), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "stfd",   D_OP(54),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_COMMON },
	{ "stfdu",  D_OP(55),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA,    I_COMMON },
	{ "stfdux", D_OP(31)|D_XO(759), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA,    I_COMMON },
	{ "stfdx",  D_OP(31)|D_XO(727), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_COMMON },
	{ "stfiwx", D_OP(31)|D_XO(983), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_POWERPC },
	{ "stfs",   D_OP(52),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_COMMON },
	{ "stfsu",  D_OP(53),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA,    I_COMMON },
	{ "stfsux", D_OP(31)|D_XO(695), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA,    I_COMMON },
	{ "stfsx",  D_OP(31)|D_XO(663), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_COMMON },
	{ "sth",    D_OP(44),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_COMMON },
	{ "sthbrx", D_OP(31)|D_XO(918), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "sthu",   D_OP(45),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA,    I_COMMON },
	{ "sthux",  D_OP(31)|D_XO(439), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA,    I_COMMON },
	{ "sthx",   D_OP(31)|D_XO(407), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_COMMON },
	{ "stm",    D_OP(47),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWER },
	{ "stmw",   D_OP(47),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWERPC },
	{ "stsi",   D_OP(31)|D_XO(725), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   0,              I_POWER },
	{ "stswi",  D_OP(31)|D_XO(725), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   0,              I_POWERPC },
	{ "stswx",  D_OP(31)|D_XO(661), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "stsx",   D_OP(31)|D_XO(661), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWER },
	{ "stu",    D_OP(37),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA,    I_POWER },
	{ "stux",   D_OP(31)|D_XO(183), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA,    I_POWER },
	{ "stw",    D_OP(36),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0,              I_POWERPC },
	{ "stwbrx", D_OP(31)|D_XO(662), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "stwcx.", D_OP(31)|D_XO(150)|1,   M_RT|M_RA|M_RB,         F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "stwu",   D_OP(37),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA,    I_POWERPC },
	{ "stwux",  D_OP(31)|D_XO(183), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA,    I_POWERPC },
	{ "stwx",   D_OP(31)|D_XO(151), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWERPC },
	{ "stx",    D_OP(31)|D_XO(151), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0,              I_POWER },
	{ "subf",   D_OP(31)|D_XO(40),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "subfc",  D_OP(31)|D_XO(8),   M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "subfe",  D_OP(31)|D_XO(136), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC,    I_POWERPC },
	{ "subfic", D_OP(8),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0,              I_POWERPC },
	{ "subfme", D_OP(31)|D_XO(232), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWERPC },
	{ "subfze", D_OP(31)|D_XO(200), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC,    I_POWERPC },
	{ "svc",    D_OP(17)|2,         M_BD|M_AA|M_LK,             F_NONE,         FL_AA|FL_LK,    I_POWER }, // TODO: operands
	{ "sync",   D_OP(31)|D_XO(598), 0,                          F_NONE,         0,              I_POWERPC },
	{ "t",      D_OP(31)|D_XO(4),   M_TO|M_RA|M_RB,             F_TW,           0,              I_POWER },
	{ "ti",     D_OP(3),            M_TO|M_RA|M_SIMM,           F_TWI,          0,              I_POWER },
	{ "tlbi",   D_OP(31)|D_XO(306), M_RB,                       F_RB,           0,              I_POWER },
	{ "tlbia",  D_OP(31)|D_XO(370), 0,                          F_NONE,         0,              I_POWERPC },
	{ "tlbie",  D_OP(31)|D_XO(306), M_RB,                       F_RB,           0,              I_POWERPC },
	{ "tlbsync",D_OP(31)|D_XO(566), 0,                          F_NONE,         0,              I_POWERPC },
	{ "tw",     D_OP(31)|D_XO(4),   M_TO|M_RA|M_RB,             F_TW,           0,              I_POWERPC },
	{ "twi",    D_OP(3),            M_TO|M_RA|M_SIMM,           F_TWI,          0,              I_POWERPC },
	{ "wrtee",  D_OP(31)|D_XO(131), M_RT,                       F_RT,           0,              I_POWERPC },
	{ "wrteei", D_OP(31)|D_XO(163), 0,                          0,              0,              I_POWERPC },
	{ "xor",    D_OP(31)|D_XO(316), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC,          I_COMMON },
	{ "xori",   D_OP(26),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "xoril",  D_OP(26),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },
	{ "xoris",  D_OP(27),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWERPC },
	{ "xoriu",  D_OP(27),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0,              I_POWER },

	/*
	 * PowerPC 603e/EC603e-specific instructions
	 */

	{ "tlbld",  D_OP(31)|D_XO(978), M_RB,                       F_RB,           0,              I_POWERPC },
	{ "tlbli",  D_OP(31)|D_XO(1010),M_RB,                       F_RB,           0,              I_POWERPC },

	// POWER2 instructions
	{ "lfq",    D_OP(56),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_POWER },
	{ "lfqu",   D_OP(57),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_POWER },
	{ "lfqux",  D_OP(31)|D_XO(823), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_POWER },
	{ "lfqx",   D_OP(31)|D_XO(791), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_POWER },
	{ "stfq",   D_OP(60),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_POWER },
	{ "stfqu",  D_OP(61),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0,              I_POWER },
	{ "stfqux", D_OP(31)|D_XO(951), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_POWER },
	{ "stfqx",  D_OP(31)|D_XO(919), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0,              I_POWER },
};

/*
 * CR Bits
 *
 * Use an index of BI&3 into this table to obtain the CR field bit name.
 */

const char *const powerpc_disassembler::crbit[4] = { "lt", "gt", "eq", "so" };
const char *const powerpc_disassembler::crnbit[4] = { "ge", "le", "ne", "nso" };


/*
 * SPR():
 *
 * Decode the SPR (or TBR) field and return the register name. If
 * no name is associated with the field value, return the value itself.
 */

std::string powerpc_disassembler::SPR(int spr_field)
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
		case SPR_XER:       return "xer";
		case SPR_LR:        return "lr";
		case SPR_CTR:       return "ctr";

		/* VEA SPR register indexes */
		case SPRVEA_TBL_R:  return "tbl";
		case SPRVEA_TBU_R:  return "tbu";

		/* OEA SPR register indexes */
		case SPROEA_DSISR:  return "dsisr";
		case SPROEA_DAR:    return "dar";
		case SPROEA_DEC:    return "dec";
		case SPROEA_SDR1:   return "sdr1";
		case SPROEA_SRR0:   return "srr0";
		case SPROEA_SRR1:   return "srr1";
		case SPROEA_SPRG0:  return "sprg0";
		case SPROEA_SPRG1:  return "sprg1";
		case SPROEA_SPRG2:  return "sprg2";
		case SPROEA_SPRG3:  return "sprg3";
		case SPROEA_ASR:    return "asr";
		case SPROEA_EAR:    return "ear";
		case SPROEA_PVR:    return "pvr";
		case SPROEA_IBAT0U: return "ibat0u";
		case SPROEA_IBAT0L: return "ibat0l";
		case SPROEA_IBAT1U: return "ibat1u";
		case SPROEA_IBAT1L: return "ibat1l";
		case SPROEA_IBAT2U: return "ibat2u";
		case SPROEA_IBAT2L: return "ibat2l";
		case SPROEA_IBAT3U: return "ibat3u";
		case SPROEA_IBAT3L: return "ibat3l";
		case SPROEA_DBAT0U: return "dbat0u";
		case SPROEA_DBAT0L: return "dbat0l";
		case SPROEA_DBAT1U: return "dbat1u";
		case SPROEA_DBAT1L: return "dbat1l";
		case SPROEA_DBAT2U: return "dbat2u";
		case SPROEA_DBAT2L: return "dbat2l";
		case SPROEA_DBAT3U: return "dbat3u";
		case SPROEA_DBAT3L: return "dbat3l";
		case SPROEA_DABR:   return "dabr/iac2";  // unsupported on 603e/EC603e

		/* PowerPC 603E SPR register indexes */
		case SPR603_HID0:   return "hid0/dbsr";
		case SPR603_HID1:   return "hid1";
		case SPR603_DMISS:  return "dmiss";
		case SPR603_DCMP:   return "dcmp";
		case SPR603_HASH1:  return "hash1";
		case SPR603_HASH2:  return "hash2/icdbdr";
		case SPR603_IMISS:  return "imiss";
		case SPR603_ICMP:   return "icmp/dear";
		case SPR603_RPA:    return "rpa/evpr";
		case SPR603_IABR:   return "iabr/dbcr";

		/* PowerPC 4XX SPR register indexes */
		case SPR4XX_SGR:    return "sgr";
		case SPR4XX_DCWR:   return "dcwr";
		case SPR4XX_PID:    return "pid";
		case SPR4XX_TBHU:   return "tbhu";
		case SPR4XX_TBLU:   return "tblu";
//      case SPR4XX_ICDBDR: return "icdbdr";  // same as SPR603E_HASH2
//      case SPR4XX_DEAR:   return "dear";  // same as SPR603E_ICMP
//      case SPR4XX_EVPR:   return "evpr";  // same as SPR603E_RPA
		case SPR4XX_CDBCR:  return "cdbcr";
		case SPR4XX_TSR:    return "tsr";
		case SPR4XX_TCR:    return "tcr";
		case SPR4XX_PIT:    return "pit";
		case SPR4XX_TBHI:   return "tbhi";
		case SPR4XX_TBLO:   return "tblo";
		case SPR4XX_SRR2:   return "srr2";
		case SPR4XX_SRR3:   return "srr3";
//      case SPR4XX_DBSR:   return "dbsr";  // same as SPR603E_HID0
//      case SPR4XX_DBCR:   return "dbcr";  // same as SPR603E_IABR
		case SPR4XX_IAC1:   return "iac1";
//      case SPR4XX_IAC2:   return "iac2";  // same as SPROEA_DABR
		case SPR4XX_DAC1:   return "dac1";
		case SPR4XX_DAC2:   return "dac2";
		case SPR4XX_DCCR:   return "dccr";
		case SPR4XX_ICCR:   return "iccr";
		case SPR4XX_PBL1:   return "pbl1";
		case SPR4XX_PBU1:   return "pbu1";
		case SPR4XX_PBL2:   return "pbl2";
		case SPR4XX_PBU2:   return "pbu2";

		// POWER SPR indexes
		case 0: return "mq";
		case 4: return "rtcu";
		case 5: return "rtcl";
		case 6: return "dec";

		default:            return util::string_format("%d", spr);
	}
}

std::string powerpc_disassembler::DCR(int dcr_field)
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
		case 144:   return "bear";
		case 145:   return "besr";
		case 128:   return "br0";
		case 129:   return "br1";
		case 130:   return "br2";
		case 131:   return "br3";
		case 132:   return "br4";
		case 133:   return "br5";
		case 134:   return "br6";
		case 135:   return "br7";
		case 112:   return "brh0";
		case 113:   return "brh1";
		case 114:   return "brh2";
		case 115:   return "brh3";
		case 116:   return "brh4";
		case 117:   return "brh5";
		case 118:   return "brh6";
		case 119:   return "brh7";
		case 196:   return "dmacc0";
		case 204:   return "dmacc1";
		case 212:   return "dmacc2";
		case 220:   return "dmacc3";
		case 192:   return "dmacr0";
		case 200:   return "dmacr1";
		case 208:   return "dmacr2";
		case 216:   return "dmacr3";
		case 193:   return "dmact0";
		case 201:   return "dmact1";
		case 209:   return "dmact2";
		case 217:   return "dmact3";
		case 194:   return "dmada0";
		case 202:   return "dmada1";
		case 210:   return "dmada2";
		case 218:   return "dmada3";
		case 195:   return "dmasa0";
		case 203:   return "dmasa1";
		case 211:   return "dmasa2";
		case 219:   return "dmasa3";
		case 224:   return "dmasr";
		case 66:    return "exier";
		case 64:    return "exisr";
		case 160:   return "iocr";

		default:    return util::string_format("%d", dcr);
	}
}


/*
 * DecodeSigned16():
 *
 * Predecodes the SIMM field for us. If do_unsigned, it is printed as an
 * unsigned 16-bit integer.
 */

std::string powerpc_disassembler::DecodeSigned16(uint32_t op, int do_unsigned)
{
	int16_t s;

	s = G_SIMM(op);
	if (do_unsigned)    // sign extend to unsigned 32-bits
		return util::string_format("0x%04X", (uint32_t) s);
	else                // print as signed 16 bits
	{
		if (s < 0)
		{
			s *= -1;
			return util::string_format("-0x%04X", s);
		}
		else
			return util::string_format("0x%04X",s);
	}
}

/*
 * Mask():
 *
 * Generate a mask from bit MB through ME (PPC-style backwards bit numbering.)
 */

uint32_t powerpc_disassembler::Mask(unsigned const mb, unsigned const me)
{
	return (mb > me) ?
		(0xffffffffU >> mb) | (0xffffffffU << (31 - me)) :
		(0xffffffffU >> mb) & (0xffffffffU << (31 - me));
}

/*
 * Simplified():
 *
 * Handles all simplified instruction forms. Returns true if one was decoded,
 * otherwise false to indicate disassembly should carry on as normal.
 */

bool powerpc_disassembler::Simplified(uint32_t op, uint32_t vpc, std::string &signed16, std::string &mnem, std::string &oprs, offs_t &flags)
{
	uint32_t  value, disp;

	value = G_SIMM(op); // value is fully sign-extended SIMM field
	if (value & 0x8000)
		value |= 0xffff0000;

	if (op == (D_OP(24)|D_RT(0)|D_RA(0)|D_UIMM(0)))
		mnem += "nop";        // ori r0,r0,0 -> nop
	else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(444)))
	{
		if (G_RT(op) == G_RB(op))
		{
			mnem += "mr";     // orx rA,rT,rT -> mrx rA,rT
			if (op & M_RC)  mnem += ".";
			oprs = util::string_format("r%d,r%d", G_RA(op), G_RT(op));
		}
		else
			return false;
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(124)))
	{
		if (G_RT(op) == G_RB(op))
		{
			mnem += "not";    // nor rA,rT,rT -> not rA,rT
			if (op & M_RC)  mnem += ".";
			oprs = util::string_format("r%d,r%d", G_RA(op), G_RT(op));
		}
		else
			return false;
	}
	else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(14))
	{
		if (G_RA(op) == 0)
		{
			mnem += "li";     // addi rT,0,value -> li rT,value
			oprs = util::string_format("r%d,0x%08X", G_RT(op), value);
		}
		else
			return false;
	}
	else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(15))
	{
		if (G_RA(op) == 0)
		{
			mnem += "li"; // addis rT,0,value -> li rT,(value<<16)
			oprs = util::string_format("r%d,0x%08X", G_RT(op), value << 16);
		}
		else
		{
			mnem += "addi";   // addis rT,rA,SIMM -> addi rT,rA,SIMM<<16
			oprs = util::string_format("r%d,r%d,0x%08X", G_RT(op), G_RA(op), value << 16);
		}
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(29))
	{
		mnem += "andi.";  // andis. rA,rT,UIMM -> andi. rA,rT,UIMM<<16
		oprs = util::string_format("r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(25))
	{
		mnem += "ori";    // oris rA,rT,UIMM -> ori rA,rT,UIMM<<16
		oprs = util::string_format("r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(27))
	{
		mnem += "xori";   // xoris rA,rT,UIMM -> xori rA,rT,UIMM<<16
		oprs = util::string_format("r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
	}
	else if ((op & ~(M_RT|M_RA|M_SH|M_MB|M_ME|M_RC)) == D_OP(21) && G_SH(op) == 0)
	{
		mnem += "and";    // rl[w]inm[.] rA,rT,0,MB,ME -> and[.] rA,rT,MASK
		if (op & M_RC) mnem += ".";
		oprs = util::string_format("r%d,r%d,0x%08X", G_RA(op), G_RT(op), Mask(G_MB(op), G_ME(op)));
	}
	else if ((op & ~(M_BO|M_BI|M_BD|M_AA|M_LK)) == D_OP(16))
	{
		disp = G_BD(op) * 4;
		if (disp & 0x00008000)
			disp |= 0xffff0000;

		switch (G_BO(op))
		{
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
				mnem += "b";
				mnem += crnbit[G_BI(op) & 3];
				flags |= STEP_COND;
				break;
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				mnem += "b";
				mnem += crbit[G_BI(op) & 3];
				flags |= STEP_COND;
				break;
			case 0x10:  case 0x11:  case 0x18:  case 0x19:
				mnem += "bdnz";
				flags |= STEP_COND;
				break;
			case 0x12:  case 0x13:  case 0x1a:  case 0x1b:
				mnem += "bdz";
				flags |= STEP_COND;
				break;
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				mnem += "b";
				break;
			default:
				return false;
		}

		if (op & M_LK)
		{
			mnem += "l";
			flags |= STEP_OVER;
		}
		if (op & M_AA)  mnem += "a";

		if (!(G_BO(op) & 0x10) && G_BI(op) / 4 != 0)
			oprs = util::string_format("cr%d,0x%08X", G_BI(op) / 4, disp + ((op & M_AA) ? 0 : vpc));
		else
			oprs = util::string_format("0x%08X", disp + ((op & M_AA) ? 0 : vpc));
	}
	else if ((op & ~(M_BO|M_BI|M_LK)) == (D_OP(19)|D_XO(528)) || (op & ~(M_BO|M_BI|M_LK)) == (D_OP(19)|D_XO(16)))
	{
		switch (G_BO(op))
		{
			case 0x04:  case 0x05:  case 0x06:  case 0x07:
				mnem += "b";
				mnem += crnbit[G_BI(op) & 3];
				flags |= STEP_COND;
				break;
			case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
				mnem += "b";
				mnem += crbit[G_BI(op) & 3];
				flags |= STEP_COND;
				break;
			case 0x10:  case 0x11:  case 0x18:  case 0x19:
				mnem += "bdnz";
				flags |= STEP_COND;
				break;
			case 0x12:  case 0x13:  case 0x1a:  case 0x1b:
				mnem += "bdz";
				flags |= STEP_COND;
				break;
			case 0x14:  case 0x15:  case 0x16:  case 0x17:
			case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
				mnem += "b";
				break;
			default:
				return false;
		}

		if (G_XO(op) == 528)
			mnem += "ctr";
		else
		{
			mnem += "lr";
			if (!(op & M_LK))
				flags |= STEP_OUT;
		}
		if (op & M_LK)
		{
			mnem += "l";
			flags |= STEP_OVER;
		}
		if (op & M_AA)  mnem += "a";

		if (!(G_BO(op) & 0x10) && G_BI(op) / 4 != 0)
			oprs = util::string_format("cr%d", G_BI(op) / 4);
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(40)))
	{
		mnem += "sub";
		if (op & M_OE) mnem += "o";
		if (op & M_RC) mnem += ".";
		oprs = util::string_format("r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
	}
	else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(8)))
	{
		mnem += "subc";
		if (op & M_OE) mnem += "o";
		if (op & M_RC) mnem += ".";
		oprs = util::string_format("r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
	}
	else
		return false;   // no match

	return true;
}

offs_t powerpc_disassembler::dasm_one(std::ostream &stream, uint32_t pc, uint32_t op)
{
	std::string signed16, mnem, oprs;
	uint32_t disp;
	int i,j;
	offs_t flags = SUPPORTED;

	/*
	 * Decode signed 16-bit fields (SIMM and d) to spare us the work later
	 */

	signed16 = DecodeSigned16(op, 0);

	/*
	 * Try simplified forms first, then real instructions
	 */

	if( Simplified(op, pc, signed16, mnem, oprs, flags) ) {
		util::stream_format(stream, "%s", mnem);
		for( j = mnem.size(); j < 10; j++ ) {
			util::stream_format(stream, " ");
		}
		util::stream_format(stream, "%s", oprs);
		return 4 | flags;
	}

	/*
	 * Search for the instruction in the list and print it if there's a match
	 */

	for (i = 0; i < sizeof(itab) / sizeof(IDESCR); i++)
	{
		if ((itab[i].implementation & m_implementation) && (op & ~itab[i].mask) == itab[i].match)  // check for match
		{
			/*
			 * Base mnemonic followed be O, ., L, A
			 */

			mnem += itab[i].mnem;
			if (itab[i].flags & FL_OE)  if (op & M_OE) mnem += "o";
			if (itab[i].flags & FL_RC)  if (op & M_RC) mnem += ".";
			if (itab[i].flags & FL_LK)  if (op & M_LK) mnem += "l";
			if (itab[i].flags & FL_AA)  if (op & M_AA) mnem += "a";

			/*
			 * Print operands
			 */

			switch (itab[i].format)
			{
			case F_RT_RA_RB:
				oprs = util::string_format("r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_RT_RA_0_SIMM:
				if (G_RA(op))
					oprs = util::string_format("r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
				else
					oprs = util::string_format("r%d,0,%s", G_RT(op), signed16);
				break;

			case F_ADDIS:
				if (G_RA(op))
					oprs = util::string_format("r%d,r%d,0x%04X", G_RT(op), G_RA(op), G_SIMM(op));
				else
					oprs = util::string_format("r%d,0,0x%04X", G_RT(op), G_SIMM(op));
				break;

			case F_RT_RA_SIMM:
				oprs = util::string_format("r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
				break;

			case F_RT_RA:
				oprs = util::string_format("r%d,r%d", G_RT(op), G_RA(op));
				break;

			case F_RA_RT_RB:
				oprs = util::string_format("r%d,r%d,r%d", G_RA(op), G_RT(op), G_RB(op));
				break;

			case F_RA_RT_UIMM:
				oprs = util::string_format("r%d,r%d,0x%04X", G_RA(op), G_RT(op), G_UIMM(op));
				break;

			case F_LI:
				disp = G_LI(op) * 4;
				if (disp & 0x02000000)  // sign extend
					disp |= 0xfc000000;
				oprs = util::string_format("0x%08X", disp + ((op & M_AA) ? 0 : pc));
				break;

			case F_BCx:
				disp = G_BD(op) * 4;
				if (disp & 0x00008000)
					disp |= 0xffff0000;

				if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
					oprs = util::string_format("0x%02X,%d,0x%08X", G_BO(op), G_BI(op), disp + ((op & M_AA) ? 0 : pc));
				else                    // BI gives us the condition bit
					oprs = util::string_format("0x%02X,cr%d[%s],0x%08X", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3], disp + ((op & M_AA) ? 0 : pc));
				if ((G_BO(op) & 0x14) != 0x14)
					flags |= STEP_COND;
				break;

			case F_BO_BI:
				if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
					oprs = util::string_format("0x%02X,%d", G_BO(op), G_BI(op));
				else
					oprs = util::string_format("0x%02X,cr%d[%s]", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3]);
				if ((G_BO(op) & 0x14) != 0x14)
					flags |= STEP_COND;
				break;

			case F_CMP:
				if (G_L(op))
					mnem += "d";
				if (G_CRFD(op) == 0)
					oprs = util::string_format("r%d,r%d", G_RA(op), G_RB(op));
				else
					oprs = util::string_format("cr%d,r%d,r%d", G_CRFD(op), G_RA(op), G_RB(op));
				break;

			case F_CMP_SIMM:
				if (G_L(op))
					mnem += "d";
				if (G_CRFD(op) == 0)
					oprs = util::string_format("r%d,%s", G_RA(op), signed16);
				else
					oprs = util::string_format("cr%d,r%d,%s", G_CRFD(op), G_RA(op), signed16);
				break;

			case F_CMP_UIMM:
				if (G_L(op))
					mnem += "d";
				if (G_CRFD(op) == 0)
					oprs = util::string_format("r%d,0x%04X", G_RA(op), G_UIMM(op));
				else
					oprs = util::string_format("cr%d,r%d,0x%04X", G_CRFD(op), G_RA(op), G_UIMM(op));
				break;

			case F_RA_RT:
				oprs = util::string_format("r%d,r%d", G_RA(op), G_RT(op));
				break;

			case F_CRBD_CRBA_CRBB:
				oprs = util::string_format("cr%d[%s],cr%d[%s],cr%d[%s]", G_CRBD(op) / 4, crbit[G_CRBD(op) & 3], G_CRBA(op) / 4, crbit[G_CRBA(op) & 3], G_CRBB(op) / 4, crbit[G_CRBB(op) & 3]);
				break;

			case F_RA_0_RB:
				if (G_RA(op))
					oprs = util::string_format("r%d,r%d", G_RA(op), G_RB(op));
				else
					oprs = util::string_format("0,r%d", G_RB(op));
				break;

			case F_RT_RA_0_RB:
				if (G_RA(op))
					oprs = util::string_format("r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				else
					oprs = util::string_format("r%d,0,r%d", G_RT(op), G_RB(op));
				break;

			case F_FRT_FRB:
				oprs = util::string_format("f%d,f%d", G_RT(op), G_RB(op));
				break;

			case F_FRT_FRA_FRB:
				oprs = util::string_format("f%d,f%d,f%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_FCMP:
				oprs = util::string_format("cr%d,f%d,f%d", G_CRFD(op), G_RA(op), G_RB(op));
				break;

			case F_FRT_FRA_FRC_FRB:
				oprs = util::string_format("f%d,f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op), G_RB(op));
				break;

			case F_FRT_FRA_FRC:
				oprs = util::string_format("f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op));
				break;

			case F_RT_D_RA_0:
				if (G_RA(op))
					oprs = util::string_format("r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				else
					oprs = util::string_format("r%d,0x%08X", G_RT(op), (uint32_t) ((int16_t) G_D(op)));
				break;

			case F_RT_D_RA:
				oprs = util::string_format("r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				break;

			case F_FRT_D_RA_0:
				if (G_RA(op))
					oprs = util::string_format("f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				else
					oprs = util::string_format("f%d,0x%08X", G_RT(op), (uint32_t) ((int16_t) G_D(op)));
				break;

			case F_FRT_D_RA:
				oprs = util::string_format("f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
				break;

			case F_FRT_RA_RB:
				oprs = util::string_format("f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				break;

			case F_FRT_RA_0_RB:
				if (G_RA(op))
					oprs = util::string_format("f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
				else
					oprs = util::string_format("f%d,0,r%d", G_RT(op), G_RB(op));
				break;

			case F_RT_RA_0_NB:
				if (G_RA(op))
					oprs = util::string_format("r%d,r%d,%d", G_RT(op), G_RA(op), G_NB(op) ? G_NB(op) : 32);
				else
					oprs = util::string_format("r%d,0,%d", G_RT(op), G_NB(op) ? G_NB(op) : 32);
				break;

			case F_CRFD_CRFS:
				oprs = util::string_format("cr%d,cr%d", G_CRFD(op), G_CRFS(op));
				break;

			case F_MCRXR:
				oprs = util::string_format("cr%d", G_CRFD(op));
				break;

			case F_RT:
				oprs = util::string_format("r%d", G_RT(op));
				break;

			case F_MFFSx:
				oprs = util::string_format("f%d", G_RT(op));
				break;

			case F_FCRBD:
				oprs = util::string_format("fpscr[%d]", G_CRBD(op));
				break;

			case F_RT_SPR:
				oprs = util::string_format("r%d,", G_RT(op)) + SPR(G_SPR(op));
				break;

			case F_RT_DCR:
				oprs = util::string_format("r%d,", G_RT(op)) + DCR(G_DCR(op));
				break;

			case F_MFSR:
				oprs = util::string_format("r%d,sr%d", G_RT(op), G_SR(op));
				break;

			case F_MTCRF:
				oprs = util::string_format("0x%02X,r%d", G_CRM(op), G_RT(op));
				break;

			case F_MTFSFx:
				oprs = util::string_format("0x%02X,f%d", G_FM(op), G_RB(op));
				break;

			case F_MTFSFIx:
				oprs = util::string_format("cr%d,0x%X", G_CRFD(op), G_IMM(op));
				break;

			case F_MTSPR:
				oprs = SPR(G_SPR(op)) + util::string_format(",r%d", G_RT(op));
				break;

			case F_MTDCR:
				oprs = DCR(G_DCR(op)) + util::string_format(",r%d", G_RT(op));
				break;

			case F_MTSR:
				oprs = util::string_format("sr%d,r%d", G_SR(op), G_RT(op));
				break;

			case F_RT_RB:
				oprs = util::string_format("r%d,r%d", G_RT(op), G_RB(op));
				break;

			case F_RA_RT_SH_MB_ME:
				oprs = util::string_format("r%d,r%d,%d,0x%08X", G_RA(op), G_RT(op), G_SH(op), Mask(G_MB(op), G_ME(op)));
				break;

			case F_RA_RT_RB_MB_ME:
				oprs = util::string_format("r%d,r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_RB(op), Mask(G_MB(op), G_ME(op)));
				break;

			case F_SRAWIx:
				oprs = util::string_format("r%d,r%d,%d", G_RA(op), G_RT(op), G_SH(op));
				break;

			case F_RB:
				oprs = util::string_format("r%d", G_RB(op));
				break;

			case F_TW:
				oprs = util::string_format("%d,r%d,r%d", G_TO(op), G_RA(op), G_RB(op));
				break;

			case F_TWI:
				oprs = util::string_format("%d,r%d,%s", G_TO(op), G_RA(op), signed16);
				break;

			case F_NONE:
			default:
				break;
			}

			if ((itab[i].flags & FL_LK) && (op & M_LK))
				flags |= STEP_OVER;
			else if (itab[i].flags & FL_SO)
				flags |= STEP_OUT;

			util::stream_format(stream, "%s", mnem);
			for( j = mnem.size(); j < 10; j++ ) {
				util::stream_format(stream, " ");
			}
			util::stream_format(stream, "%s", oprs);
			return 4 | flags;
		}
	}

	util::stream_format(stream, "?");
	return 4 | flags;
}

offs_t powerpc_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return dasm_one(stream, pc, opcodes.r32(pc));
}

u32 powerpc_disassembler::opcode_alignment() const
{
	return 4;
}
