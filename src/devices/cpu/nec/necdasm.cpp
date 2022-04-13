// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
   NEC V-series Disassembler

   Originally Written for i386 by Ville Linde
   Converted to NEC-V by Aaron Giles
*/

#include "emu.h"
#include "necdasm.h"

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::necv_opcode_table1[256] =
{
	// 0x00
	{"add",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"add",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"add",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"add",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"add",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"add",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"push    ds1",     0,              0,                  0,                  0               },
	{"pop     ds1",     0,              0,                  0,                  0               },
	{"or",              MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"or",              MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"or",              MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"or",              MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"or",              0,              PARAM_AL,           PARAM_UI8,          0               },
	{"or",              0,              PARAM_AW,           PARAM_IMM,          0               },
	{"push    ps",      0,              0,                  0,                  0               },
	{"two_byte",        TWO_BYTE,       0,                  0,                  0               },
	// 0x10
	{"addc",            MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"addc",            MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"addc",            MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"addc",            MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"addc",            0,              PARAM_AL,           PARAM_UI8,          0               },
	{"addc",            0,              PARAM_AW,           PARAM_IMM,          0               },
	{"push    ss",      0,              0,                  0,                  0               },
	{"pop     ss",      0,              0,                  0,                  0               },
	{"subc",            MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"subc",            MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"subc",            MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"subc",            MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"subc",            0,              PARAM_AL,           PARAM_UI8,          0               },
	{"subc",            0,              PARAM_AW,           PARAM_IMM,          0               },
	{"push    ds0",     0,              0,                  0,                  0               },
	{"pop     ds0",     0,              0,                  0,                  0               },
	// 0x20
	{"and",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"and",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"and",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"and",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"and",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"and",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"ds1:",            SEG_DS1,        0,                  0,                  0               },
	{"adj4a",           0,              0,                  0,                  0               },
	{"sub",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"sub",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"sub",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"sub",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"sub",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"sub",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"ps:",             SEG_PS,         0,                  0,                  0               },
	{"adj4s",           0,              0,                  0,                  0               },
	// 0x30
	{"xor",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"xor",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"xor",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"xor",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"xor",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"xor",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"ss:",             SEG_SS,         0,                  0,                  0               },
	{"adjba",           0,              0,                  0,                  0               },
	{"cmp",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"cmp",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"cmp",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"cmp",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"cmp",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"cmp",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"ds0:",            SEG_DS0,        0,                  0,                  0               },
	{"adjbs",           0,              0,                  0,                  0               },
	// 0x40
	{"inc",             0,              PARAM_AW,           0,                  0               },
	{"inc",             0,              PARAM_CW,           0,                  0               },
	{"inc",             0,              PARAM_DW,           0,                  0               },
	{"inc",             0,              PARAM_BW,           0,                  0               },
	{"inc",             0,              PARAM_SP,           0,                  0               },
	{"inc",             0,              PARAM_BP,           0,                  0               },
	{"inc",             0,              PARAM_IX,           0,                  0               },
	{"inc",             0,              PARAM_IY,           0,                  0               },
	{"dec",             0,              PARAM_AW,           0,                  0               },
	{"dec",             0,              PARAM_CW,           0,                  0               },
	{"dec",             0,              PARAM_DW,           0,                  0               },
	{"dec",             0,              PARAM_BW,           0,                  0               },
	{"dec",             0,              PARAM_SP,           0,                  0               },
	{"dec",             0,              PARAM_BP,           0,                  0               },
	{"dec",             0,              PARAM_IX,           0,                  0               },
	{"dec",             0,              PARAM_IY,           0,                  0               },
	// 0x50
	{"push",            0,              PARAM_AW,           0,                  0               },
	{"push",            0,              PARAM_CW,           0,                  0               },
	{"push",            0,              PARAM_DW,           0,                  0               },
	{"push",            0,              PARAM_BW,           0,                  0               },
	{"push",            0,              PARAM_SP,           0,                  0               },
	{"push",            0,              PARAM_BP,           0,                  0               },
	{"push",            0,              PARAM_IX,           0,                  0               },
	{"push",            0,              PARAM_IY,           0,                  0               },
	{"pop",             0,              PARAM_AW,           0,                  0               },
	{"pop",             0,              PARAM_CW,           0,                  0               },
	{"pop",             0,              PARAM_DW,           0,                  0               },
	{"pop",             0,              PARAM_BW,           0,                  0               },
	{"pop",             0,              PARAM_SP,           0,                  0               },
	{"pop",             0,              PARAM_BP,           0,                  0               },
	{"pop",             0,              PARAM_IX,           0,                  0               },
	{"pop",             0,              PARAM_IY,           0,                  0               },
	// 0x60
	{"push    r",       0,              0,                  0,                  0               },
	{"pop     r",       0,              0,                  0,                  0               },
	{"chkind",          MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"brkn",            0,              PARAM_UI8,          0,                  0,              STEP_OVER},    /* V25S/V35S only */
	{"repnc",           PREFIX,         0,                  0,                  0               },
	{"repc",            PREFIX,         0,                  0,                  0               },
	{"fpo2    0",       0,              0,                  0,                  0               },  /* for a coprocessor that was never made */
	{"fpo2    1",       0,              0,                  0,                  0               },  /* for a coprocessor that was never made */
	{"push",            0,              PARAM_IMM,          0,                  0               },
	{"mul",             MODRM,          PARAM_REG16,        PARAM_RM16,         PARAM_IMM       },
	{"push",            0,              PARAM_I8,           0,                  0               },
	{"mul",             MODRM,          PARAM_REG16,        PARAM_RM16,         PARAM_I8        },
	{"inmb",            0,              0,                  0,                  0               },
	{"inmw",            0,              0,                  0,                  0               },
	{"outmb",           0,              0,                  0,                  0               },
	{"outmw",           0,              0,                  0,                  0               },
	// 0x70
	{"bv",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bnv",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bc",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bnc",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"be",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bne",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bnh",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bh",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bn",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bp",              0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bpe",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bpo",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"blt",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bge",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"ble",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bgt",             0,              PARAM_REL8,         0,                  0,              STEP_COND},
	// 0x80
	{"immb",            GROUP,          0,                  0,                  0               },
	{"immw",            GROUP,          0,                  0,                  0               },
	{"immb",            GROUP,          0,                  0,                  0               },
	{"immws",           GROUP,          0,                  0,                  0               },
	{"test",            MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"test",            MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"xch",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"xch",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"mov",             MODRM,          PARAM_RM16,         PARAM_REG16,        0               },
	{"mov",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"mov",             MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov",             MODRM,          PARAM_RM16,         PARAM_SREG,         0               },
	{"ldea",            MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov",             MODRM,          PARAM_SREG,         PARAM_RM16,         0               },
	{"pop",             MODRM,          PARAM_RM16,         0,                  0               },
	// 0x90
	{"nop",             0,              0,                  0,                  0               },
	{"xch",             0,              PARAM_AW,           PARAM_CW,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_DW,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_BW,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_SP,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_BP,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_IX,           0               },
	{"xch",             0,              PARAM_AW,           PARAM_IY,           0               },
	{"cvtbw",           0,              0,                  0,                  0               },
	{"cvtwl",           0,              0,                  0,                  0               },
	{"call",            0,              PARAM_ADDR,         0,                  0,              STEP_OVER},
	{"poll",            0,              0,                  0,                  0               },
	{"push    psw",     0,              0,                  0,                  0               },
	{"pop     psw",     0,              0,                  0,                  0               },
	{"mov     psw,ah",  0,              0,                  0,                  0               },
	{"mov     ah,psw",  0,              0,                  0,                  0               },
	// 0xa0
	{"mov",             0,              PARAM_AL,           PARAM_MEM_OFFS,     0               },
	{"mov",             0,              PARAM_AW,           PARAM_MEM_OFFS,     0               },
	{"mov",             0,              PARAM_MEM_OFFS,     PARAM_AL,           0               },
	{"mov",             0,              PARAM_MEM_OFFS,     PARAM_AW,           0               },
	{"movbkb",          0,              0,                  0,                  0               },
	{"movbkw",          0,              0,                  0,                  0               },
	{"cmpbkb",          0,              0,                  0,                  0               },
	{"cmpbkw",          0,              0,                  0,                  0               },
	{"test",            0,              PARAM_AL,           PARAM_UI8,          0               },
	{"test",            0,              PARAM_AW,           PARAM_IMM,          0               },
	{"stmb",            0,              0,                  0,                  0               },
	{"stmw",            0,              0,                  0,                  0               },
	{"ldmb",            0,              0,                  0,                  0               },
	{"ldmw",            0,              0,                  0,                  0               },
	{"cmpmb",           0,              0,                  0,                  0               },
	{"cmpmw",           0,              0,                  0,                  0               },
	// 0xb0
	{"mov",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_CL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_DL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_BL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_AH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_CH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_DH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_BH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_AW,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_CW,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_DW,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_BW,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_SP,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_BP,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_IX,           PARAM_IMM,          0               },
	{"mov",             0,              PARAM_IY,           PARAM_IMM,          0               },
	// 0xc0
	{"shiftbi",         GROUP,          0,                  0,                  0               },
	{"shiftwi",         GROUP,          0,                  0,                  0               },
	{"ret",             0,              PARAM_I16,          0,                  0,              STEP_OUT},
	{"ret",             0,              0,                  0,                  0,              STEP_OUT},
	{"mov     ds1,",    MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov     ds0,",    MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov",             MODRM,          PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"mov",             MODRM,          PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"prepare",         0,              PARAM_I16,          PARAM_UI8,          0               },
	{"dispose",         0,              0,                  0,                  0               },
	{"retf",            0,              PARAM_I16,          0,                  0,              STEP_OUT},
	{"retf",            0,              0,                  0,                  0,              STEP_OUT},
	{"brk     3",       0,              0,                  0,                  0,              STEP_OVER},
	{"brk",             0,              PARAM_UI8,          0,                  0,              STEP_OVER},
	{"brkv",            0,              0,                  0,                  0,              STEP_OVER | STEP_COND},
	{"reti",            0,              0,                  0,                  0,              STEP_OUT},
	// 0xd0
	{"shiftb",          GROUP,          0,                  0,                  0               },
	{"shiftw",          GROUP,          0,                  0,                  0               },
	{"shiftbv",         GROUP,          0,                  0,                  0               },
	{"shiftwv",         GROUP,          0,                  0,                  0               },
	{"cvtbd",           0,              PARAM_I8,           0,                  0               },
	{"cvtdb",           0,              PARAM_I8,           0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"trans",           0,              0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	// 0xe0
	{"dbnzne",          0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"dbnze",           0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"dbnz",            0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"bcwz",            0,              PARAM_REL8,         0,                  0,              STEP_COND},
	{"in",              0,              PARAM_AL,           PARAM_UI8,          0               },
	{"in",              0,              PARAM_AW,           PARAM_UI8,          0               },
	{"out",             0,              PARAM_UI8,          PARAM_AL,           0               },
	{"out",             0,              PARAM_UI8,          PARAM_AW,           0               },
	{"call",            0,              PARAM_REL16,        0,                  0,              STEP_OVER},
	{"br",              0,              PARAM_REL16,        0,                  0               },
	{"br",              0,              PARAM_ADDR,         0,                  0               },
	{"br",              0,              PARAM_REL8,         0,                  0               },
	{"in",              0,              PARAM_AL,           PARAM_DW,           0               },
	{"in",              0,              PARAM_AW,           PARAM_DW,           0               },
	{"out",             0,              PARAM_DW,           PARAM_AL,           0               },
	{"out",             0,              PARAM_DW,           PARAM_AW,           0               },
	// 0xf0
	{"buslock",         PREFIX,         0,                  0,                  0               },
	{"brks",            0,              PARAM_UI8,          0,                  0,              STEP_OVER},    /* V25S/V35S only */
	{"repne",           PREFIX,         0,                  0,                  0               },
	{"rep",             PREFIX,         0,                  0,                  0               },
	{"halt",            0,              0,                  0,                  0               },
	{"not1    cy",      0,              0,                  0,                  0               },
	{"group1b",         GROUP,          0,                  0,                  0               },
	{"group1w",         GROUP,          0,                  0,                  0               },
	{"clr1    cy",      0,              0,                  0,                  0               },
	{"set1    cy",      0,              0,                  0,                  0               },
	{"di",              0,              0,                  0,                  0               },
	{"ei",              0,              0,                  0,                  0               },
	{"clr1    dir",     0,              0,                  0,                  0               },
	{"set1    dir",     0,              0,                  0,                  0               },
	{"group2b",         GROUP,          0,                  0,                  0               },
	{"group2w",         GROUP,          0,                  0,                  0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::necv_opcode_table2[256] =
{
	// 0x00
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x10
	{"test1",           MODRM,          PARAM_RMPTR8,       PARAM_CL,           0               },
	{"test1",           MODRM,          PARAM_RMPTR16,      PARAM_CL,           0               },
	{"clr1",            MODRM,          PARAM_RMPTR8,       PARAM_CL,           0               },
	{"clr1",            MODRM,          PARAM_RMPTR16,      PARAM_CL,           0               },
	{"set1",            MODRM,          PARAM_RMPTR8,       PARAM_CL,           0               },
	{"set1",            MODRM,          PARAM_RMPTR16,      PARAM_CL,           0               },
	{"not1",            MODRM,          PARAM_RMPTR8,       PARAM_CL,           0               },
	{"not1",            MODRM,          PARAM_RMPTR16,      PARAM_CL,           0               },
	{"test1",           MODRM,          PARAM_RMPTR8,       PARAM_I3,           0               },
	{"test1",           MODRM,          PARAM_RMPTR16,      PARAM_I4,           0               },
	{"clr1",            MODRM,          PARAM_RMPTR8,       PARAM_I3,           0               },
	{"clr1",            MODRM,          PARAM_RMPTR16,      PARAM_I4,           0               },
	{"set1",            MODRM,          PARAM_RMPTR8,       PARAM_I3,           0               },
	{"set1",            MODRM,          PARAM_RMPTR16,      PARAM_I4,           0               },
	{"not1",            MODRM,          PARAM_RMPTR8,       PARAM_I3,           0               },
	{"not1",            MODRM,          PARAM_RMPTR16,      PARAM_I4,           0               },
	// 0x20
	{"add4s",           0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"sub4s",           0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"movspa",          0,              0,                  0,                  0               },  /* V25/V35 only */
	{"cmp4s",           0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"rol4",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"ror4",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"brkcs",           MODRM,          PARAM_REG2_16,      0,                  0,              STEP_OVER},    /* V25/V35 only */
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x30
	{"???",             0,              0,                  0,                  0               },
	{"ins",             MODRM,          PARAM_REG2_8,       PARAM_REG8,         0               },
	{"???",             0,              0,                  0,                  0               },
	{"ext",             MODRM,          PARAM_REG2_8,       PARAM_REG8,         0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"ins",             MODRM,          PARAM_REG2_8,       PARAM_I4,           0               },
	{"???",             0,              0,                  0,                  0               },
	{"ext",             MODRM,          PARAM_REG2_8,       PARAM_I4,           0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x40
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x50
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x60
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x70
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x80
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x90
	{"???",             0,              0,                  0,                  0               },
	{"retrbi",          0,              0,                  0,                  0               },  /* V25/V35 only */
	{"fint",            0,              0,                  0,                  0               },  /* V25/V35 only */
	{"???",             0,              0,                  0,                  0               },
	{"tsksw",           MODRM,          PARAM_REG2_16,      0,                  0               },  /* V25/V35 only */
	{"movspb",          MODRM,          PARAM_REG2_16,      0,                  0               },  /* V25/V35 only */
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"btclr",           0,              PARAM_SFREG,        PARAM_I3,           PARAM_REL8,     STEP_COND},  /* V25/V35 only */
	{"???",             0,              0,                  0,                  0               },
	{"stop",            0,              0,                  0,                  0               },  /* V25/V35 only */
	{"???",             0,              0,                  0,                  0               },
	// 0xa0
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0xb0
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0xc0
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0xd0
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0xe0
	{"brkxa",           0,              PARAM_UI8,          0,                  0               },  /* V33,53 only */
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0xf0
	{"retxa",           0,              PARAM_UI8,          0,                  0               },  /* V33,53 only */
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"brkem",           0,              PARAM_UI8,          0,                  0               }   /* V20,30,40,50 only */
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::immb_table[8] =
{
	{"add",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"or",              0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"addc",            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"subc",            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"and",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"sub",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"xor",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"cmp",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::immw_table[8] =
{
	{"add",             0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"or",              0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"addc",            0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"subc",            0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"and",             0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"sub",             0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"xor",             0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"cmp",             0,              PARAM_RMPTR16,      PARAM_IMM,          0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::immws_table[8] =
{
	{"add",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"or",              0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"addc",            0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"subc",            0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"and",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"sub",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"xor",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"cmp",             0,              PARAM_RMPTR16,      PARAM_I8,           0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftbi_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"rolc",            0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"rorc",            0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"???",             0,              PARAM_RMPTR8,       PARAM_I8,           0               },
	{"shra",            0,              PARAM_RMPTR8,       PARAM_I8,           0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftwi_table[8] =
{
	{"rol",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"ror",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"rolc",            0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"rorc",            0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"shl",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"shr",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"???",             0,              PARAM_RMPTR16,      PARAM_I8,           0               },
	{"shra",            0,              PARAM_RMPTR16,      PARAM_I8,           0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftb_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"rolc",            0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"rorc",            0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"???",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"shra",            0,              PARAM_RMPTR8,       PARAM_1,            0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftw_table[8] =
{
	{"rol",             0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"ror",             0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"rolc",            0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"rorc",            0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"shl",             0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"shr",             0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"???",             0,              PARAM_RMPTR16,      PARAM_1,            0               },
	{"shra",            0,              PARAM_RMPTR16,      PARAM_1,            0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftbv_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"rolc",            0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"rorc",            0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"???",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"shra",            0,              PARAM_RMPTR8,       PARAM_CL,           0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::shiftwv_table[8] =
{
	{"rol",             0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"ror",             0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"rolc",            0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"rorc",            0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"shl",             0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"shr",             0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"???",             0,              PARAM_RMPTR16,      PARAM_CL,           0               },
	{"shra",            0,              PARAM_RMPTR16,      PARAM_CL,           0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::group1b_table[8] =
{
	{"test",            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"not",             0,              PARAM_RMPTR8,       0,                  0               },
	{"neg",             0,              PARAM_RMPTR8,       0,                  0               },
	{"mulu",            0,              PARAM_RMPTR8,       0,                  0               },
	{"mul",             0,              PARAM_RMPTR8,       0,                  0               },
	{"divu",            0,              PARAM_RMPTR8,       0,                  0               },
	{"div",             0,              PARAM_RMPTR8,       0,                  0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::group1w_table[8] =
{
	{"test",            0,              PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"not",             0,              PARAM_RMPTR16,      0,                  0               },
	{"neg",             0,              PARAM_RMPTR16,      0,                  0               },
	{"mulu",            0,              PARAM_RMPTR16,      0,                  0               },
	{"mul",             0,              PARAM_RMPTR16,      0,                  0               },
	{"divu",            0,              PARAM_RMPTR16,      0,                  0               },
	{"div",             0,              PARAM_RMPTR16,      0,                  0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::group2b_table[8] =
{
	{"inc",             0,              PARAM_RMPTR8,       0,                  0               },
	{"dec",             0,              PARAM_RMPTR8,       0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const nec_disassembler::NEC_I386_OPCODE nec_disassembler::group2w_table[8] =
{
	{"inc",             0,              PARAM_RMPTR16,      0,                  0               },
	{"dec",             0,              PARAM_RMPTR16,      0,                  0               },
	{"call",            0,              PARAM_RMPTR16,      0,                  0,              STEP_OVER},
	{"call    far ptr ",0,              PARAM_RM16,         0,                  0,              STEP_OVER},
	{"br",              0,              PARAM_RMPTR16,      0,                  0               },
	{"br      far ptr ",0,              PARAM_RM16,         0,                  0               },
	{"push",            0,              PARAM_RMPTR16,      0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const nec_disassembler::NEC_GROUP_OP nec_disassembler::group_op_table[] =
{
	{ "immb",               immb_table              },
	{ "immw",               immw_table              },
	{ "immws",              immws_table             },
	{ "shiftbi",            shiftbi_table           },
	{ "shiftwi",            shiftwi_table           },
	{ "shiftb",             shiftb_table            },
	{ "shiftw",             shiftw_table            },
	{ "shiftbv",            shiftbv_table           },
	{ "shiftwv",            shiftwv_table           },
	{ "group1b",            group1b_table           },
	{ "group1w",            group1w_table           },
	{ "group2b",            group2b_table           },
	{ "group2w",            group2w_table           }
};



const char *const nec_disassembler::nec_reg[8] = { "aw",  "cw",  "dw",  "bw",  "sp",  "bp",  "ix",  "iy" };
const char *const nec_disassembler::nec_reg8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
const char *const nec_disassembler::nec_sreg[8] = { "ds1", "ps", "ss", "ds0", "???", "???", "???", "???" };
const char *const nec_disassembler::nec_sfreg[256] =
{
	/* 0x00 */
	"p0",   "pm0",  "pmc0", "???",  "???",  "???",  "???",  "???",
	"p1",   "pm1",  "pmc1", "???",  "???",  "???",  "???",  "???",
	/* 0x10 */
	"p2",   "pm2",  "pmc2", "???",  "???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	/* 0x20 */
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	/* 0x30 */
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	"pt",   "???",  "???",  "pmt",  "???",  "???",  "???",  "???",
	/* 0x40 */
	"intm", "???",  "???",  "???",  "ems0", "ems1", "ems2", "???",
	"???",  "???",  "???",  "???",  "exic0","exic1","exic2","???",
	/* 0x50 */
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	/* 0x60 */
	"rxb0", "???",  "txb0", "???",  "???",  "srms0","stms0","???",
	"scm0", "scc0", "brg0", "scs0", "seic0","sric0","stic0","???",
	/* 0x70 */
	"rxb1", "???",  "txb1", "???",  "???",  "srms1","stms1","???",
	"scm1", "scc1", "brg1", "scs1", "seic1","sric1","stic1","???",
	/* 0x80 */
	"tm0",  "???",  "md0",  "???",  "???",  "???",  "???",  "???",
	"tm1",  "???",  "md1",  "???",  "???",  "???",  "???",  "???",
	/* 0x90 */
	"tmc0", "tmc1", "???",  "???",  "tmms0","tmms1","tmms2","???",
	"???",  "???",  "???",  "???",  "tmic0","tmic1","tmic2","???",
	/* 0xa0 */
	"dmac0","dmam0","dmac1","dmam1","???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "dic0", "dic1", "???",  "???",
	/* 0xb0 */
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	/* 0xc0 */
	"sar0l","sar0m","sar0h","???",  "dar0l","dar0m","dar0h","???",
	"tc0l", "tc0h", "???",  "???",  "???",  "???",  "???",  "???",
	/* 0xd0 */
	"sar1l","sar1m","sar1h","???",  "dar1l","dar1m","dar1h","???",
	"tc1l", "tc1h", "???",  "???",  "???",  "???",  "???",  "???",
	/* 0xe0 */
	"stbc", "rfm",  "???",  "???",  "???",  "???",  "???",  "???",
	"wtc",  "???",  "flag", "prc",  "tbic", "???",  "???",  "irqs",
	/* 0xf0 */
	"???",  "???",  "???",  "???",  "???",  "???",  "???",  "???",
	"???",  "???",  "???",  "???",  "ispr", "???",  "???",  "idb"
};

#define MODRM_REG1  ((m_modrm >> 3) & 0x7)
#define MODRM_REG2  (m_modrm & 0x7)

#define MAX_LENGTH  8

uint8_t nec_disassembler::FETCH(offs_t pc_base, offs_t &pc, const data_buffer &opcodes)
{
	if ((pc - pc_base) + 1 > MAX_LENGTH)
		return 0xff;
	u8 r = opcodes.r8(pc);
	pc++;
	return r;
}

uint16_t nec_disassembler::FETCH16(offs_t pc_base, offs_t &pc, const data_buffer &opcodes)
{
	if ((pc - pc_base) + 1 > MAX_LENGTH)
		return 0xff;
	u16 r = opcodes.r16(pc);
	pc += 2;
	return r;
}

std::string nec_disassembler::hexstring(uint32_t value, int digits)
{
	std::string buffer;
	if (digits)
		buffer = string_format("%0*Xh", digits, value);
	else
		buffer = string_format("%Xh", value);
	return buffer[0] > '9' ? '0' + buffer : buffer;
}

std::string nec_disassembler::shexstring(uint32_t value, int digits, bool always)
{
	if (value >= 0x80000000)
		return '-' + hexstring(-value, digits);
	else if (always)
		return '+' + hexstring(value, digits);
	else
		return hexstring(value, digits);
}

void nec_disassembler::handle_modrm(offs_t pc_base, offs_t &pc, const data_buffer &params)
{
	m_modrm_string = "";

	m_modrm = FETCH(pc_base, pc, params);
	u8 mod = (m_modrm >> 6) & 0x3;
	u8 rm = (m_modrm & 0x7);

	if( m_modrm >= 0xc0 )
		return;

	switch(m_segment)
	{
		case SEG_PS: m_modrm_string += "ps:"; break;
		case SEG_DS0: m_modrm_string += "ds0:"; break;
		case SEG_DS1: m_modrm_string += "ds1:"; break;
		case SEG_SS: m_modrm_string += "ss:"; break;
	}

	m_modrm_string += '[';
	switch( rm )
	{
		case 0: m_modrm_string += "bw+ix"; break;
		case 1: m_modrm_string += "bw+iy"; break;
		case 2: m_modrm_string += "bp+ix"; break;
		case 3: m_modrm_string += "bp+iy"; break;
		case 4: m_modrm_string += "ix"; break;
		case 5: m_modrm_string += "iy"; break;
		case 6: m_modrm_string += mod == 0 ? hexstring(u16(FETCH16(pc_base, pc, params)), 0) : "bp"; break;
		case 7: m_modrm_string += "bw"; break;
	}
	if( mod == 1 )
		m_modrm_string += shexstring(s8(FETCH(pc_base, pc, params)), 0, true);
	else if( mod == 2 )
		m_modrm_string += shexstring(s16(FETCH16(pc_base, pc, params)), 0, true);
	m_modrm_string += ']';
}

void nec_disassembler::handle_param(std::ostream &stream, uint32_t param, offs_t pc_base, offs_t &pc, const data_buffer &params)
{
	uint8_t i8;
	uint16_t i16;
	uint16_t ptr;
	uint32_t addr;
	int8_t d8;
	int16_t d16;

	switch(param)
	{
		case PARAM_REG8:
			util::stream_format( stream, "%s", nec_reg8[MODRM_REG1] );
			break;

		case PARAM_REG16:
			util::stream_format( stream, "%s", nec_reg[MODRM_REG1] );
			break;

		case PARAM_REG2_8:
			util::stream_format( stream, "%s", nec_reg8[MODRM_REG2] );
			break;

		case PARAM_REG2_16:
			util::stream_format( stream, "%s", nec_reg[MODRM_REG2] );
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( m_modrm >= 0xc0 ) {
				util::stream_format( stream, "%s", nec_reg8[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR8)
					util::stream_format( stream, "byte ptr " );
				util::stream_format( stream, "%s", m_modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( m_modrm >= 0xc0 ) {
				util::stream_format( stream, "%s", nec_reg[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR16)
					util::stream_format( stream, "word ptr " );
				util::stream_format( stream, "%s", m_modrm_string );
			}
			break;

		case PARAM_I3:
			i8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%d", i8 & 0x07 );
			break;

		case PARAM_I4:
			i8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%d", i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%s", shexstring((int8_t)i8, 0, false) );
			break;

		case PARAM_I16:
			i16 = FETCH16(pc_base, pc, params);
			util::stream_format( stream, "%s", shexstring((int16_t)i16, 0, false) );
			break;

		case PARAM_UI8:
			i8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%s", shexstring((uint8_t)i8, 0, false) );
			break;

		case PARAM_IMM:
			i16 = FETCH16(pc_base, pc, params);
			util::stream_format( stream, "%s", hexstring(i16, 0) );
			break;

		case PARAM_ADDR:
			addr = FETCH16(pc_base, pc, params);
			ptr = FETCH16(pc_base, pc, params);
			util::stream_format( stream, "%s:", hexstring(ptr, 4) );
			util::stream_format( stream, "%s", hexstring(addr, 0) );
			break;

		case PARAM_REL16:
			/* make sure to keep the relative offset within the segment */
			d16 = FETCH16(pc_base, pc, params);
			util::stream_format( stream, "%s", hexstring((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF), 0) );
			break;

		case PARAM_REL8:
			d8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%s", hexstring(pc + d8, 0) );
			break;

		case PARAM_MEM_OFFS:
			switch(m_segment)
			{
				case SEG_PS: util::stream_format( stream, "ps:" ); break;
				case SEG_DS0: util::stream_format( stream, "ds0:" ); break;
				case SEG_DS1: util::stream_format( stream, "ds1:" ); break;
				case SEG_SS: util::stream_format( stream, "ss:" ); break;
			}

			i16 = FETCH16(pc_base, pc, params);
			util::stream_format( stream, "[%s]", hexstring(i16, 0) );
			break;

		case PARAM_SREG:
			util::stream_format( stream, "%s", nec_sreg[MODRM_REG1] );
			break;

		case PARAM_SFREG:
			i8 = FETCH(pc_base, pc, params);
			util::stream_format( stream, "%s", nec_sfreg[i8] );
			break;

		case PARAM_1:
			util::stream_format( stream, "1" );
			break;

		case PARAM_AL: util::stream_format( stream, "al" ); break;
		case PARAM_CL: util::stream_format( stream, "cl" ); break;
		case PARAM_DL: util::stream_format( stream, "dl" ); break;
		case PARAM_BL: util::stream_format( stream, "bl" ); break;
		case PARAM_AH: util::stream_format( stream, "ah" ); break;
		case PARAM_CH: util::stream_format( stream, "ch" ); break;
		case PARAM_DH: util::stream_format( stream, "dh" ); break;
		case PARAM_BH: util::stream_format( stream, "bh" ); break;

		case PARAM_AW: util::stream_format( stream, "aw" ); break;
		case PARAM_CW: util::stream_format( stream, "cw" ); break;
		case PARAM_DW: util::stream_format( stream, "dw" ); break;
		case PARAM_BW: util::stream_format( stream, "bw" ); break;
		case PARAM_SP: util::stream_format( stream, "sp" ); break;
		case PARAM_BP: util::stream_format( stream, "bp" ); break;
		case PARAM_IX: util::stream_format( stream, "ix" ); break;
		case PARAM_IY: util::stream_format( stream, "iy" ); break;
	}
}

void nec_disassembler::handle_fpu(std::ostream &stream, uint8_t op1, uint8_t op2, offs_t pc_base, offs_t &pc, const data_buffer &params)
{
	switch (op1 & 0x7)
	{
		case 0:     // Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fadd    dword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "fmul    dword ptr %s", m_modrm_string); break;
					case 2: util::stream_format(stream, "fcom    dword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fcomp   dword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fsub    dword ptr %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fsubr   dword ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fdiv    dword ptr %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fdivr   dword ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fadd    st(0),st(%d)", op2 & 0x7); break;
					case 1: util::stream_format(stream, "fcom    st(0),st(%d)", op2 & 0x7); break;
					case 2: util::stream_format(stream, "fsub    st(0),st(%d)", op2 & 0x7); break;
					case 3: util::stream_format(stream, "fdiv    st(0),st(%d)", op2 & 0x7); break;
					case 4: util::stream_format(stream, "fmul    st(0),st(%d)", op2 & 0x7); break;
					case 5: util::stream_format(stream, "fcomp   st(0),st(%d)", op2 & 0x7); break;
					case 6: util::stream_format(stream, "fsubr   st(0),st(%d)", op2 & 0x7); break;
					case 7: util::stream_format(stream, "fdivr   st(0),st(%d)", op2 & 0x7); break;
				}
			}
			break;
		}

		case 1:     // Group D9
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fld     dword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fst     dword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fstp    dword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fldenv  word ptr %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fldcw   word ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fstenv  word ptr %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fstcw   word ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "fld     st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						util::stream_format(stream, "fxch    st(0),st(%d)", op2 & 0x7); break;

					case 0x10: util::stream_format(stream, "fnop"); break;
					case 0x20: util::stream_format(stream, "fchs"); break;
					case 0x21: util::stream_format(stream, "fabs"); break;
					case 0x24: util::stream_format(stream, "ftst"); break;
					case 0x25: util::stream_format(stream, "fxam"); break;
					case 0x28: util::stream_format(stream, "fld1"); break;
					case 0x29: util::stream_format(stream, "fldl2t"); break;
					case 0x2a: util::stream_format(stream, "fldl2e"); break;
					case 0x2b: util::stream_format(stream, "fldpi"); break;
					case 0x2c: util::stream_format(stream, "fldlg2"); break;
					case 0x2d: util::stream_format(stream, "fldln2"); break;
					case 0x2e: util::stream_format(stream, "fldz"); break;
					case 0x30: util::stream_format(stream, "f2xm1"); break;
					case 0x31: util::stream_format(stream, "fyl2x"); break;
					case 0x32: util::stream_format(stream, "fptan"); break;
					case 0x33: util::stream_format(stream, "fpatan"); break;
					case 0x34: util::stream_format(stream, "fxtract"); break;
					case 0x35: util::stream_format(stream, "fprem1"); break;
					case 0x36: util::stream_format(stream, "fdecstp"); break;
					case 0x37: util::stream_format(stream, "fincstp"); break;
					case 0x38: util::stream_format(stream, "fprem"); break;
					case 0x39: util::stream_format(stream, "fyl2xp1"); break;
					case 0x3a: util::stream_format(stream, "fsqrt"); break;
					case 0x3b: util::stream_format(stream, "fsincos"); break;
					case 0x3c: util::stream_format(stream, "frndint"); break;
					case 0x3d: util::stream_format(stream, "fscale"); break;
					case 0x3e: util::stream_format(stream, "fsin"); break;
					case 0x3f: util::stream_format(stream, "fcos"); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 2:     // Group DA
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fiadd   dword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "fimul   dword ptr %s", m_modrm_string); break;
					case 2: util::stream_format(stream, "ficom   dword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "ficomp  dword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fisub   dword ptr %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fisubr  dword ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fidiv   dword ptr %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fidivr  dword ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "fcmovb  st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						util::stream_format(stream, "fcmove  st(0),st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						util::stream_format(stream, "fcmovbe st(0),st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						util::stream_format(stream, "fcmovu  st(0),st(%d)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;

				}
			}
			break;
		}

		case 3:     // Group DB
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    dword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fist    dword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fistp   dword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "??? (FPU)"); break;
					case 5: util::stream_format(stream, "fld     tword ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "??? (FPU)"); break;
					case 7: util::stream_format(stream, "fstp    tword ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "fcmovnb st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						util::stream_format(stream, "fcmovne st(0),st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						util::stream_format(stream, "fcmovnbe st(0),st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						util::stream_format(stream, "fcmovnu st(0),st(%d)", op2 & 0x7); break;

					case 0x22: util::stream_format(stream, "fclex"); break;
					case 0x23: util::stream_format(stream, "finit"); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						util::stream_format(stream, "fucomi  st(0),st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						util::stream_format(stream, "fcomi   st(0),st(%d)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 4:     // Group DC
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fadd    qword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "fmul    qword ptr %s", m_modrm_string); break;
					case 2: util::stream_format(stream, "fcom    qword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fcomp   qword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fsub    qword ptr %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fsubr   qword ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fdiv    qword ptr %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fdivr   qword ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "fadd    st(%d),st(0)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						util::stream_format(stream, "fmul    st(%d),st(0)", op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						util::stream_format(stream, "fsubr   st(%d),st(0)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						util::stream_format(stream, "fsub    st(%d),st(0)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						util::stream_format(stream, "fdivr   st(%d),st(0)", op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						util::stream_format(stream, "fdiv    st(%d),st(0)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 5:     // Group DD
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fld     qword ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fst     qword ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fstp    qword ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "frstor  %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "??? (FPU)"); break;
					case 6: util::stream_format(stream, "fsave   %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fstsw   word ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "ffree   st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						util::stream_format(stream, "fst     st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						util::stream_format(stream, "fstp    st(%d)", op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						util::stream_format(stream, "fucom   st(%d), st(0)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						util::stream_format(stream, "fucomp  st(%d)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 6:     // Group DE
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fiadd   word ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "fimul   word ptr %s", m_modrm_string); break;
					case 2: util::stream_format(stream, "ficom   word ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "ficomp  word ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fisub   word ptr %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fisubr  word ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fidiv   word ptr %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fidivr  word ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						util::stream_format(stream, "faddp   st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						util::stream_format(stream, "fmulp   st(%d)", op2 & 0x7); break;

					case 0x19: util::stream_format(stream, "fcompp"); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						util::stream_format(stream, "fsubrp  st(%d)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						util::stream_format(stream, "fsubp   st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						util::stream_format(stream, "fdivrp  st(%d), st(0)", op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						util::stream_format(stream, "fdivp   st(%d)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 7:     // Group DF
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm( pc_base, pc, params );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    word ptr %s", m_modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fist    word ptr %s", m_modrm_string); break;
					case 3: util::stream_format(stream, "fistp   word ptr %s", m_modrm_string); break;
					case 4: util::stream_format(stream, "fbld    %s", m_modrm_string); break;
					case 5: util::stream_format(stream, "fild    qword ptr %s", m_modrm_string); break;
					case 6: util::stream_format(stream, "fbstp   %s", m_modrm_string); break;
					case 7: util::stream_format(stream, "fistp   qword ptr %s", m_modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x20: util::stream_format(stream, "fstsw   aw"); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						util::stream_format(stream, "fucomip st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						util::stream_format(stream, "fcomip  st(%d),st(0)", op2 & 0x7); break;

					default: util::stream_format(stream, "??? (FPU)"); break;
				}
			}
			break;
		}
	}
}

void nec_disassembler::decode_opcode(std::ostream &stream, const NEC_I386_OPCODE *op, uint8_t op1, offs_t pc_base, offs_t &pc, const data_buffer &opcodes, const data_buffer &params)
{
	int i;
	uint8_t op2;

	switch( op->flags )
	{
		case TWO_BYTE:
			op2 = FETCH(pc_base, pc, params);
			decode_opcode( stream, &necv_opcode_table2[op2], op1, pc_base, pc, opcodes, params );
			return;

		case SEG_PS:
		case SEG_DS0:
		case SEG_DS1:
		case SEG_SS:
			m_segment = op->flags;
			op2 = FETCH(pc_base, pc, opcodes);
			if (m_decryption_table) op2 = m_decryption_table[op2];
			decode_opcode( stream, &necv_opcode_table1[op2], op1, pc_base, pc, opcodes, params );
			return;

		case PREFIX:
			util::stream_format( stream, "%-8s", op->mnemonic );
			op2 = FETCH(pc_base, pc, opcodes);
			if (m_decryption_table) op2 = m_decryption_table[op2];
			decode_opcode( stream, &necv_opcode_table1[op2], op1, pc_base, pc, opcodes, params );
			return;

		case GROUP:
			handle_modrm( pc_base, pc, params );
			for( i=0; i < std::size(group_op_table); i++ ) {
				if( strcmp(op->mnemonic, group_op_table[i].mnemonic) == 0 )
				{
					decode_opcode( stream, &group_op_table[i].opcode[MODRM_REG1], op1, pc_base, pc, opcodes, params );
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = FETCH(pc_base, pc, params);
			handle_fpu( stream, op1, op2, pc_base, pc, params);
			return;

		case MODRM:
			handle_modrm( pc_base, pc, params );
			break;
	}

	util::stream_format( stream, "%-8s", op->mnemonic );
	m_dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		handle_param(stream, op->param1, pc_base, pc, params);
	}

	if( op->param2 != 0 ) {
		util::stream_format( stream, "," );
		handle_param(stream, op->param2, pc_base, pc, params);
	}

	if( op->param3 != 0 ) {
		util::stream_format( stream, "," );
		handle_param(stream, op->param3, pc_base, pc, params);
	}
	return;

handle_unknown:
	util::stream_format(stream, "???");

}
offs_t nec_disassembler::dis80(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t flags = 0;
	uint8_t op;
	unsigned prevpc = pc;
	switch (op = opcodes.r8(pc++))
	{
		case 0x00: util::stream_format(stream, "nop"); break;
		case 0x01: util::stream_format(stream, "lxi  cw,$%04x", params.r16(pc)); pc+=2; break;
		case 0x02: util::stream_format(stream, "stax cw"); break;
		case 0x03: util::stream_format(stream, "inx  cw"); break;
		case 0x04: util::stream_format(stream, "inr  ch"); break;
		case 0x05: util::stream_format(stream, "dcr  ch"); break;
		case 0x06: util::stream_format(stream, "mvi  ch,$%02x", params.r8(pc)); pc++; break;
		case 0x07: util::stream_format(stream, "rlc"); break;
		case 0x08: util::stream_format(stream, "nop"); break;
		case 0x09: util::stream_format(stream, "dad  cw"); break;
		case 0x0a: util::stream_format(stream, "ldax cw"); break;
		case 0x0b: util::stream_format(stream, "dcx  cw"); break;
		case 0x0c: util::stream_format(stream, "inr  cl"); break;
		case 0x0d: util::stream_format(stream, "dcr  cl"); break;
		case 0x0e: util::stream_format(stream, "mvi  cl,$%02x", params.r8(pc)); pc++; break;
		case 0x0f: util::stream_format(stream, "rrc"); break;
		case 0x10: util::stream_format(stream, "nop"); break;
		case 0x11: util::stream_format(stream, "lxi  dw,$%04x", params.r16(pc)); pc+=2; break;
		case 0x12: util::stream_format(stream, "stax dw"); break;
		case 0x13: util::stream_format(stream, "inx  dw"); break;
		case 0x14: util::stream_format(stream, "inr  dh"); break;
		case 0x15: util::stream_format(stream, "dcr  dh"); break;
		case 0x16: util::stream_format(stream, "mvi  dh,$%02x", params.r8(pc)); pc++; break;
		case 0x17: util::stream_format(stream, "ral"); break;
		case 0x18: util::stream_format(stream, "nop"); break;
		case 0x19: util::stream_format(stream, "dad  dw"); break;
		case 0x1a: util::stream_format(stream, "ldax dw"); break;
		case 0x1b: util::stream_format(stream, "dcx  dw"); break;
		case 0x1c: util::stream_format(stream, "inr  dl"); break;
		case 0x1d: util::stream_format(stream, "dcr  dl"); break;
		case 0x1e: util::stream_format(stream, "mvi  dl,$%02x", params.r8(pc)); pc++; break;
		case 0x1f: util::stream_format(stream, "rar"); break;
		case 0x20: util::stream_format(stream, "nop"); break;
		case 0x21: util::stream_format(stream, "lxi  bw,$%04x", params.r16(pc)); pc+=2; break;
		case 0x22: util::stream_format(stream, "shld $%04x", params.r16(pc)); pc+=2; break;
		case 0x23: util::stream_format(stream, "inx  bw"); break;
		case 0x24: util::stream_format(stream, "inr  bh"); break;
		case 0x25: util::stream_format(stream, "dcr  bh"); break;
		case 0x26: util::stream_format(stream, "mvi  bh,$%02x", params.r8(pc)); pc++; break;
		case 0x27: util::stream_format(stream, "daa"); break;
		case 0x28: util::stream_format(stream, "nop"); break;
		case 0x29: util::stream_format(stream, "dad  bw"); break;
		case 0x2a: util::stream_format(stream, "lhld $%04x", params.r16(pc)); pc+=2; break;
		case 0x2b: util::stream_format(stream, "dcx  bw"); break;
		case 0x2c: util::stream_format(stream, "inr  bl"); break;
		case 0x2d: util::stream_format(stream, "dcr  bl"); break;
		case 0x2e: util::stream_format(stream, "mvi  bl,$%02x", params.r8(pc)); pc++; break;
		case 0x2f: util::stream_format(stream, "cma"); break;
		case 0x30: util::stream_format(stream, "nop"); break;
		case 0x31: util::stream_format(stream, "lxi  bp,$%04x", params.r16(pc)); pc+=2; break;
		case 0x32: util::stream_format(stream, "stax $%04x", params.r16(pc)); pc+=2; break;
		case 0x33: util::stream_format(stream, "inx  bp"); break;
		case 0x34: util::stream_format(stream, "inr  m"); break;
		case 0x35: util::stream_format(stream, "dcr  m"); break;
		case 0x36: util::stream_format(stream, "mvi  m,$%02x", params.r8(pc)); pc++; break;
		case 0x37: util::stream_format(stream, "stc"); break;
		case 0x38: util::stream_format(stream, "nop"); break;
		case 0x39: util::stream_format(stream, "dad  bp"); break;
		case 0x3a: util::stream_format(stream, "ldax $%04x", params.r16(pc)); pc+=2; break;
		case 0x3b: util::stream_format(stream, "dcx  bp"); break;
		case 0x3c: util::stream_format(stream, "inr  al"); break;
		case 0x3d: util::stream_format(stream, "dcr  al"); break;
		case 0x3e: util::stream_format(stream, "mvi  al,$%02x", params.r8(pc)); pc++; break;
		case 0x3f: util::stream_format(stream, "cmc"); break;
		case 0x40: util::stream_format(stream, "mov  ch,ch"); break;
		case 0x41: util::stream_format(stream, "mov  ch,cl"); break;
		case 0x42: util::stream_format(stream, "mov  ch,dh"); break;
		case 0x43: util::stream_format(stream, "mov  ch,dl"); break;
		case 0x44: util::stream_format(stream, "mov  ch,bh"); break;
		case 0x45: util::stream_format(stream, "mov  ch,bl"); break;
		case 0x46: util::stream_format(stream, "mov  ch,m"); break;
		case 0x47: util::stream_format(stream, "mov  ch,al"); break;
		case 0x48: util::stream_format(stream, "mov  cl,ch"); break;
		case 0x49: util::stream_format(stream, "mov  cl,cl"); break;
		case 0x4a: util::stream_format(stream, "mov  cl,dh"); break;
		case 0x4b: util::stream_format(stream, "mov  cl,dl"); break;
		case 0x4c: util::stream_format(stream, "mov  cl,bh"); break;
		case 0x4d: util::stream_format(stream, "mov  cl,bl"); break;
		case 0x4e: util::stream_format(stream, "mov  cl,m"); break;
		case 0x4f: util::stream_format(stream, "mov  cl,al"); break;
		case 0x50: util::stream_format(stream, "mov  dh,ch"); break;
		case 0x51: util::stream_format(stream, "mov  dh,cl"); break;
		case 0x52: util::stream_format(stream, "mov  dh,dh"); break;
		case 0x53: util::stream_format(stream, "mov  dh,dl"); break;
		case 0x54: util::stream_format(stream, "mov  dh,bh"); break;
		case 0x55: util::stream_format(stream, "mov  dh,bl"); break;
		case 0x56: util::stream_format(stream, "mov  dh,m"); break;
		case 0x57: util::stream_format(stream, "mov  dh,al"); break;
		case 0x58: util::stream_format(stream, "mov  dl,ch"); break;
		case 0x59: util::stream_format(stream, "mov  dl,cl"); break;
		case 0x5a: util::stream_format(stream, "mov  dl,dh"); break;
		case 0x5b: util::stream_format(stream, "mov  dl,dl"); break;
		case 0x5c: util::stream_format(stream, "mov  dl,bh"); break;
		case 0x5d: util::stream_format(stream, "mov  dl,bl"); break;
		case 0x5e: util::stream_format(stream, "mov  dl,m"); break;
		case 0x5f: util::stream_format(stream, "mov  dl,al"); break;
		case 0x60: util::stream_format(stream, "mov  bh,ch"); break;
		case 0x61: util::stream_format(stream, "mov  bh,cl"); break;
		case 0x62: util::stream_format(stream, "mov  bh,dh"); break;
		case 0x63: util::stream_format(stream, "mov  bh,dl"); break;
		case 0x64: util::stream_format(stream, "mov  bh,bh"); break;
		case 0x65: util::stream_format(stream, "mov  bh,bl"); break;
		case 0x66: util::stream_format(stream, "mov  bh,m"); break;
		case 0x67: util::stream_format(stream, "mov  bh,al"); break;
		case 0x68: util::stream_format(stream, "mov  bl,ch"); break;
		case 0x69: util::stream_format(stream, "mov  bl,cl"); break;
		case 0x6a: util::stream_format(stream, "mov  bl,dh"); break;
		case 0x6b: util::stream_format(stream, "mov  bl,dl"); break;
		case 0x6c: util::stream_format(stream, "mov  bl,bh"); break;
		case 0x6d: util::stream_format(stream, "mov  bl,bl"); break;
		case 0x6e: util::stream_format(stream, "mov  bl,m"); break;
		case 0x6f: util::stream_format(stream, "mov  bl,al"); break;
		case 0x70: util::stream_format(stream, "mov  m,ch"); break;
		case 0x71: util::stream_format(stream, "mov  m,cl"); break;
		case 0x72: util::stream_format(stream, "mov  m,dh"); break;
		case 0x73: util::stream_format(stream, "mov  m,dl"); break;
		case 0x74: util::stream_format(stream, "mov  m,bh"); break;
		case 0x75: util::stream_format(stream, "mov  m,bl"); break;
		case 0x76: util::stream_format(stream, "hlt"); break;
		case 0x77: util::stream_format(stream, "mov  m,al"); break;
		case 0x78: util::stream_format(stream, "mov  al,ch"); break;
		case 0x79: util::stream_format(stream, "mov  al,cl"); break;
		case 0x7a: util::stream_format(stream, "mov  al,dh"); break;
		case 0x7b: util::stream_format(stream, "mov  al,dl"); break;
		case 0x7c: util::stream_format(stream, "mov  al,bh"); break;
		case 0x7d: util::stream_format(stream, "mov  al,bl"); break;
		case 0x7e: util::stream_format(stream, "mov  al,m"); break;
		case 0x7f: util::stream_format(stream, "mov  al,al"); break;
		case 0x80: util::stream_format(stream, "add  ch"); break;
		case 0x81: util::stream_format(stream, "add  cl"); break;
		case 0x82: util::stream_format(stream, "add  dh"); break;
		case 0x83: util::stream_format(stream, "add  dl"); break;
		case 0x84: util::stream_format(stream, "add  bh"); break;
		case 0x85: util::stream_format(stream, "add  bl"); break;
		case 0x86: util::stream_format(stream, "add  m"); break;
		case 0x87: util::stream_format(stream, "add  al"); break;
		case 0x88: util::stream_format(stream, "adc  ch"); break;
		case 0x89: util::stream_format(stream, "adc  cl"); break;
		case 0x8a: util::stream_format(stream, "adc  dh"); break;
		case 0x8b: util::stream_format(stream, "adc  dl"); break;
		case 0x8c: util::stream_format(stream, "adc  bh"); break;
		case 0x8d: util::stream_format(stream, "adc  bl"); break;
		case 0x8e: util::stream_format(stream, "adc  m"); break;
		case 0x8f: util::stream_format(stream, "adc  al"); break;
		case 0x90: util::stream_format(stream, "sub  ch"); break;
		case 0x91: util::stream_format(stream, "sub  cl"); break;
		case 0x92: util::stream_format(stream, "sub  dh"); break;
		case 0x93: util::stream_format(stream, "sub  dl"); break;
		case 0x94: util::stream_format(stream, "sub  bh"); break;
		case 0x95: util::stream_format(stream, "sub  bl"); break;
		case 0x96: util::stream_format(stream, "sub  m"); break;
		case 0x97: util::stream_format(stream, "sub  al"); break;
		case 0x98: util::stream_format(stream, "sbb  ch"); break;
		case 0x99: util::stream_format(stream, "sbb  cl"); break;
		case 0x9a: util::stream_format(stream, "sbb  dh"); break;
		case 0x9b: util::stream_format(stream, "sbb  dl"); break;
		case 0x9c: util::stream_format(stream, "sbb  bh"); break;
		case 0x9d: util::stream_format(stream, "sbb  bl"); break;
		case 0x9e: util::stream_format(stream, "sbb  m"); break;
		case 0x9f: util::stream_format(stream, "sbb  al"); break;
		case 0xa0: util::stream_format(stream, "ana  ch"); break;
		case 0xa1: util::stream_format(stream, "ana  cl"); break;
		case 0xa2: util::stream_format(stream, "ana  dh"); break;
		case 0xa3: util::stream_format(stream, "ana  dl"); break;
		case 0xa4: util::stream_format(stream, "ana  bh"); break;
		case 0xa5: util::stream_format(stream, "ana  bl"); break;
		case 0xa6: util::stream_format(stream, "ana  m"); break;
		case 0xa7: util::stream_format(stream, "ana  al"); break;
		case 0xa8: util::stream_format(stream, "xra  ch"); break;
		case 0xa9: util::stream_format(stream, "xra  cl"); break;
		case 0xaa: util::stream_format(stream, "xra  dh"); break;
		case 0xab: util::stream_format(stream, "xra  dl"); break;
		case 0xac: util::stream_format(stream, "xra  bh"); break;
		case 0xad: util::stream_format(stream, "xra  bl"); break;
		case 0xae: util::stream_format(stream, "xra  m"); break;
		case 0xaf: util::stream_format(stream, "xra  al"); break;
		case 0xb0: util::stream_format(stream, "ora  ch"); break;
		case 0xb1: util::stream_format(stream, "ora  cl"); break;
		case 0xb2: util::stream_format(stream, "ora  dh"); break;
		case 0xb3: util::stream_format(stream, "ora  dl"); break;
		case 0xb4: util::stream_format(stream, "ora  bh"); break;
		case 0xb5: util::stream_format(stream, "ora  bl"); break;
		case 0xb6: util::stream_format(stream, "ora  m"); break;
		case 0xb7: util::stream_format(stream, "ora  al"); break;
		case 0xb8: util::stream_format(stream, "cmp  ch"); break;
		case 0xb9: util::stream_format(stream, "cmp  cl"); break;
		case 0xba: util::stream_format(stream, "cmp  dh"); break;
		case 0xbb: util::stream_format(stream, "cmp  dl"); break;
		case 0xbc: util::stream_format(stream, "cmp  bh"); break;
		case 0xbd: util::stream_format(stream, "cmp  bl"); break;
		case 0xbe: util::stream_format(stream, "cmp  m"); break;
		case 0xbf: util::stream_format(stream, "cmp  al"); break;
		case 0xc0: util::stream_format(stream, "rnz"); flags = STEP_OUT | STEP_COND; break;
		case 0xc1: util::stream_format(stream, "pop  cw"); break;
		case 0xc2: util::stream_format(stream, "jnz  $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xc3: util::stream_format(stream, "jmp  $%04x", params.r16(pc)); pc+=2; break;
		case 0xc4: util::stream_format(stream, "cnz  $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xc5: util::stream_format(stream, "push cw"); break;
		case 0xc6: util::stream_format(stream, "adi  $%02x", params.r8(pc)); pc++; break;
		case 0xc7: util::stream_format(stream, "rst  0"); flags = STEP_OVER; break;
		case 0xc8: util::stream_format(stream, "rz"); flags = STEP_OUT | STEP_COND; break;
		case 0xc9: util::stream_format(stream, "ret"); flags = STEP_OUT; break;
		case 0xca: util::stream_format(stream, "jz   $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xcb: util::stream_format(stream, "jmp  $%04x", params.r16(pc)); pc+=2; break;
		case 0xcc: util::stream_format(stream, "cz   $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xcd: util::stream_format(stream, "call $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xce: util::stream_format(stream, "aci  $%02x", params.r8(pc)); pc++; break;
		case 0xcf: util::stream_format(stream, "rst  1"); flags = STEP_OVER; break;
		case 0xd0: util::stream_format(stream, "rnc"); flags = STEP_OUT | STEP_COND; break;
		case 0xd1: util::stream_format(stream, "pop  dw"); break;
		case 0xd2: util::stream_format(stream, "jnc  $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xd3: util::stream_format(stream, "out  $%02x", params.r8(pc)); pc++; break;
		case 0xd4: util::stream_format(stream, "cnc  $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xd5: util::stream_format(stream, "push dw"); break;
		case 0xd6: util::stream_format(stream, "sui  $%02x", params.r8(pc)); pc++; break;
		case 0xd7: util::stream_format(stream, "rst  2"); flags = STEP_OVER; break;
		case 0xd8: util::stream_format(stream, "rc"); flags = STEP_OUT | STEP_COND; break;
		case 0xd9: util::stream_format(stream, "shlx d (*)"); break;
		case 0xda: util::stream_format(stream, "jc   $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xdb: util::stream_format(stream, "in   $%02x", params.r8(pc)); pc++; break;
		case 0xdc: util::stream_format(stream, "cc   $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xdd: util::stream_format(stream, "call $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xde: util::stream_format(stream, "sbi  $%02x", params.r8(pc)); pc++; break;
		case 0xdf: util::stream_format(stream, "rst  3"); flags = STEP_OVER; break;
		case 0xe0: util::stream_format(stream, "rpo"); flags = STEP_OUT | STEP_COND; break;
		case 0xe1: util::stream_format(stream, "pop  bw"); break;
		case 0xe2: util::stream_format(stream, "jpo  $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xe3: util::stream_format(stream, "xthl"); break;
		case 0xe4: util::stream_format(stream, "cpo  $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER | STEP_COND; break;
		case 0xe5: util::stream_format(stream, "push bw"); break;
		case 0xe6: util::stream_format(stream, "ani  $%02x", params.r8(pc)); pc++; break;
		case 0xe7: util::stream_format(stream, "rst  4"); flags = STEP_OVER; break;
		case 0xe8: util::stream_format(stream, "rpe"); flags = STEP_OUT | STEP_COND; break;
		case 0xe9: util::stream_format(stream, "pchl"); break;
		case 0xea: util::stream_format(stream, "jpe  $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xeb: util::stream_format(stream, "xchg"); break;
		case 0xec: util::stream_format(stream, "cpe  $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER | STEP_COND; break;
		case 0xed:
			switch (params.r8(pc))
			{
				case 0xed:
					util::stream_format(stream, "calln $%02x", params.r8(++pc)); pc++; flags = STEP_OVER; break;
				case 0xfd:
					util::stream_format(stream, "retem $%02x", params.r8(pc)); pc++; flags = STEP_OUT; break;
				default:
					util::stream_format(stream, "call $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
			}
			break;
		case 0xee: util::stream_format(stream, "xri  $%02x", params.r8(pc)); pc++; break;
		case 0xef: util::stream_format(stream, "rst  5"); flags = STEP_OVER; break;
		case 0xf0: util::stream_format(stream, "rp"); flags = STEP_OUT | STEP_COND; break;
		case 0xf1: util::stream_format(stream, "pop  psw"); break;
		case 0xf2: util::stream_format(stream, "jp   $%04x", params.r16(pc)); pc+=2; flags = STEP_COND; break;
		case 0xf3: util::stream_format(stream, "di"); break;
		case 0xf4: util::stream_format(stream, "cp   $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER | STEP_COND; break;
		case 0xf5: util::stream_format(stream, "push psw"); break;
		case 0xf6: util::stream_format(stream, "ori  $%02x", params.r8(pc)); pc++; break;
		case 0xf7: util::stream_format(stream, "rst  6"); flags = STEP_OVER; break;
		case 0xf8: util::stream_format(stream, "rm"); flags = STEP_OUT | STEP_COND; break;
		case 0xf9: util::stream_format(stream, "sphl"); break;
		case 0xfa: util::stream_format(stream, "jm   $%04x", params.r16(pc)); pc+=2; break;
		case 0xfb: util::stream_format(stream, "ei"); break;
		case 0xfc: util::stream_format(stream, "cm   $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER | STEP_COND; break;
		case 0xfd: util::stream_format(stream, "call $%04x", params.r16(pc)); pc+=2; flags = STEP_OVER; break;
		case 0xfe: util::stream_format(stream, "cpi  $%02x", params.r8(pc)); pc++; break;
		case 0xff: util::stream_format(stream, "rst  7"); flags = STEP_OVER; break;
	}
	return (pc - prevpc) | flags | SUPPORTED;
}

offs_t nec_disassembler::disassemble(std::ostream &stream, offs_t eip, const data_buffer &opcodes, const data_buffer &params)
{
	if(!(m_config->get_mode()))
		return dis80(stream, eip, opcodes, params);

	uint8_t op;

	offs_t pc = eip;
	m_dasm_flags = 0;
	m_segment = 0;

	op = FETCH(eip, pc, opcodes);

	if (m_decryption_table)
		op = m_decryption_table[op];

	decode_opcode( stream, &necv_opcode_table1[op], op, eip, pc, opcodes, params );
	return (pc-eip) | m_dasm_flags | SUPPORTED;
}

nec_disassembler::nec_disassembler(config *conf, const u8 *decryption_table) : m_config(conf), m_decryption_table(decryption_table)
{
}

u32 nec_disassembler::opcode_alignment() const
{
	return 1;
}

