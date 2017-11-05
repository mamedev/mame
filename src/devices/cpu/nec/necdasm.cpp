// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
   NEC V-series Disassembler

   Originally Written for i386 by Ville Linde
   Converted to NEC-V by Aaron Giles
*/

#include "emu.h"
#include "nec_common.h"

static const uint8_t *Iconfig;

enum
{
	PARAM_REG8 = 1,     /* 8-bit register */
	PARAM_REG16,        /* 16-bit register */
	PARAM_REG2_8,       /* 8-bit register */
	PARAM_REG2_16,      /* 16-bit register */
	PARAM_RM8,          /* 8-bit memory or register */
	PARAM_RM16,         /* 16-bit memory or register */
	PARAM_RMPTR8,       /* 8-bit memory or register */
	PARAM_RMPTR16,      /* 16-bit memory or register */
	PARAM_I3,           /* 3-bit immediate */
	PARAM_I4,           /* 4-bit immediate */
	PARAM_I8,           /* 8-bit signed immediate */
	PARAM_I16,          /* 16-bit signed immediate */
	PARAM_UI8,          /* 8-bit unsigned immediate */
	PARAM_IMM,          /* 16-bit immediate */
	PARAM_ADDR,         /* 16:16 address */
	PARAM_REL8,         /* 8-bit PC-relative displacement */
	PARAM_REL16,        /* 16-bit PC-relative displacement */
	PARAM_MEM_OFFS,     /* 16-bit mem offset */
	PARAM_SREG,         /* segment register */
	PARAM_SFREG,        /* V25/V35 special function register */
	PARAM_1,            /* used by shift/rotate instructions */
	PARAM_AL,
	PARAM_CL,
	PARAM_DL,
	PARAM_BL,
	PARAM_AH,
	PARAM_CH,
	PARAM_DH,
	PARAM_BH,
	PARAM_AW,
	PARAM_CW,
	PARAM_DW,
	PARAM_BW,
	PARAM_SP,
	PARAM_BP,
	PARAM_IX,
	PARAM_IY
};

enum
{
	MODRM = 1,
	GROUP,
	FPU,
	TWO_BYTE,
	PREFIX,
	SEG_PS,
	SEG_DS0,
	SEG_DS1,
	SEG_SS
};

struct NEC_I386_OPCODE {
	char mnemonic[32];
	uint32_t flags;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	offs_t dasm_flags;
};

struct NEC_GROUP_OP {
	char mnemonic[32];
	const NEC_I386_OPCODE *opcode;
};

static const uint8_t *opcode_ptr;
static const uint8_t *opcode_ptr_base;

static const NEC_I386_OPCODE necv_opcode_table1[256] =
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
	{"brkn",            0,              PARAM_UI8,          0,                  0,              DASMFLAG_STEP_OVER},    /* V25S/V35S only */
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
	{"bv",              0,              PARAM_REL8,         0,                  0               },
	{"bnv",             0,              PARAM_REL8,         0,                  0               },
	{"bc",              0,              PARAM_REL8,         0,                  0               },
	{"bnc",             0,              PARAM_REL8,         0,                  0               },
	{"be",              0,              PARAM_REL8,         0,                  0               },
	{"bne",             0,              PARAM_REL8,         0,                  0               },
	{"bnh",             0,              PARAM_REL8,         0,                  0               },
	{"bh",              0,              PARAM_REL8,         0,                  0               },
	{"bn",              0,              PARAM_REL8,         0,                  0               },
	{"bp",              0,              PARAM_REL8,         0,                  0               },
	{"bpe",             0,              PARAM_REL8,         0,                  0               },
	{"bpo",             0,              PARAM_REL8,         0,                  0               },
	{"blt",             0,              PARAM_REL8,         0,                  0               },
	{"bge",             0,              PARAM_REL8,         0,                  0               },
	{"ble",             0,              PARAM_REL8,         0,                  0               },
	{"bgt",             0,              PARAM_REL8,         0,                  0               },
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
	{"call",            0,              PARAM_ADDR,         0,                  0,              DASMFLAG_STEP_OVER},
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
	{"ret",             0,              PARAM_I16,          0,                  0,              DASMFLAG_STEP_OUT},
	{"ret",             0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
	{"mov     ds1,",    MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov     ds0,",    MODRM,          PARAM_REG16,        PARAM_RM16,         0               },
	{"mov",             MODRM,          PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"mov",             MODRM,          PARAM_RMPTR16,      PARAM_IMM,          0               },
	{"prepare",         0,              PARAM_I16,          PARAM_UI8,          0               },
	{"dispose",         0,              0,                  0,                  0               },
	{"retf",            0,              PARAM_I16,          0,                  0,              DASMFLAG_STEP_OUT},
	{"retf",            0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
	{"brk     3",       0,              0,                  0,                  0,              DASMFLAG_STEP_OVER},
	{"brk",             0,              PARAM_UI8,          0,                  0,              DASMFLAG_STEP_OVER},
	{"brkv",            0,              0,                  0,                  0               },
	{"reti",            0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
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
	{"dbnzne",          0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{"dbnze",           0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{"dbnz",            0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{"bcwz",            0,              PARAM_REL8,         0,                  0               },
	{"in",              0,              PARAM_AL,           PARAM_UI8,          0               },
	{"in",              0,              PARAM_AW,           PARAM_UI8,          0               },
	{"out",             0,              PARAM_UI8,          PARAM_AL,           0               },
	{"out",             0,              PARAM_UI8,          PARAM_AW,           0               },
	{"call",            0,              PARAM_REL16,        0,                  0,              DASMFLAG_STEP_OVER},
	{"br",              0,              PARAM_REL16,        0,                  0               },
	{"br",              0,              PARAM_ADDR,         0,                  0               },
	{"br",              0,              PARAM_REL8,         0,                  0               },
	{"in",              0,              PARAM_AL,           PARAM_DW,           0               },
	{"in",              0,              PARAM_AW,           PARAM_DW,           0               },
	{"out",             0,              PARAM_DW,           PARAM_AL,           0               },
	{"out",             0,              PARAM_DW,           PARAM_AW,           0               },
	// 0xf0
	{"buslock",         PREFIX,         0,                  0,                  0               },
	{"brks",            0,              PARAM_UI8,          0,                  0,              DASMFLAG_STEP_OVER},    /* V25S/V35S only */
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

static const NEC_I386_OPCODE necv_opcode_table2[256] =
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
	{"brkcs",           MODRM,          PARAM_REG2_16,      0,                  0,              DASMFLAG_STEP_OVER},    /* V25/V35 only */
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
	{"btclr",           0,              PARAM_SFREG,        PARAM_I3,           PARAM_REL8      },  /* V25/V35 only */
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

static const NEC_I386_OPCODE immb_table[8] =
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

static const NEC_I386_OPCODE immw_table[8] =
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

static const NEC_I386_OPCODE immws_table[8] =
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

static const NEC_I386_OPCODE shiftbi_table[8] =
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

static const NEC_I386_OPCODE shiftwi_table[8] =
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

static const NEC_I386_OPCODE shiftb_table[8] =
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

static const NEC_I386_OPCODE shiftw_table[8] =
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

static const NEC_I386_OPCODE shiftbv_table[8] =
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

static const NEC_I386_OPCODE shiftwv_table[8] =
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

static const NEC_I386_OPCODE group1b_table[8] =
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

static const NEC_I386_OPCODE group1w_table[8] =
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

static const NEC_I386_OPCODE group2b_table[8] =
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

static const NEC_I386_OPCODE group2w_table[8] =
{
	{"inc",             0,              PARAM_RMPTR16,      0,                  0               },
	{"dec",             0,              PARAM_RMPTR16,      0,                  0               },
	{"call",            0,              PARAM_RMPTR16,      0,                  0,              DASMFLAG_STEP_OVER},
	{"call    far ptr ",0,              PARAM_RM16,         0,                  0,              DASMFLAG_STEP_OVER},
	{"br",              0,              PARAM_RMPTR16,      0,                  0               },
	{"br      far ptr ",0,              PARAM_RM16,         0,                  0               },
	{"push",            0,              PARAM_RMPTR16,      0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

static const NEC_GROUP_OP group_op_table[] =
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



static const char *const nec_reg[8] = { "aw",  "cw",  "dw",  "bw",  "sp",  "bp",  "ix",  "iy" };
static const char *const nec_reg8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static const char *const nec_sreg[8] = { "ds1", "ps", "ss", "ds0", "???", "???", "???", "???" };
static const char *const nec_sfreg[256] =
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

static uint32_t pc;
static uint8_t modrm;
static uint32_t segment;
static offs_t dasm_flags;
static char modrm_string[256];

#define MODRM_REG1  ((modrm >> 3) & 0x7)
#define MODRM_REG2  (modrm & 0x7)

#define MAX_LENGTH  8

static inline uint8_t FETCH(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

#if 0
static inline uint16_t FETCH16(void)
{
	uint16_t d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}
#endif

static inline uint8_t FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

static inline uint16_t FETCHD16(void)
{
	uint16_t d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

static char *hexstring(uint32_t value, int digits)
{
	static char buffer[20];
	buffer[0] = '0';
	if (digits)
		sprintf(&buffer[1], "%0*Xh", digits, value);
	else
		sprintf(&buffer[1], "%Xh", value);
	return (buffer[1] >= '0' && buffer[1] <= '9') ? &buffer[1] : &buffer[0];
}

static char *shexstring(uint32_t value, int digits, bool always)
{
	static char buffer[20];
	if (value >= 0x80000000)
		sprintf(buffer, "-%s", hexstring(-value, digits));
	else if (always)
		sprintf(buffer, "+%s", hexstring(value, digits));
	else
		return hexstring(value, digits);
	return buffer;
}

static void handle_modrm(char* s)
{
	int8_t disp8;
	int16_t disp16;
	uint8_t mod, rm;

	modrm = FETCHD();
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7);

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_PS: s += sprintf( s, "ps:" ); break;
		case SEG_DS0: s += sprintf( s, "ds0:" ); break;
		case SEG_DS1: s += sprintf( s, "ds1:" ); break;
		case SEG_SS: s += sprintf( s, "ss:" ); break;
	}

	s += sprintf( s, "[" );
	switch( rm )
	{
		case 0: s += sprintf( s, "bw+ix" ); break;
		case 1: s += sprintf( s, "bw+iy" ); break;
		case 2: s += sprintf( s, "bp+ix" ); break;
		case 3: s += sprintf( s, "bp+iy" ); break;
		case 4: s += sprintf( s, "ix" ); break;
		case 5: s += sprintf( s, "iy" ); break;
		case 6:
			if( mod == 0 ) {
				disp16 = FETCHD16();
				s += sprintf( s, "%s", hexstring((unsigned) (uint16_t) disp16, 0) );
			} else {
				s += sprintf( s, "bp" );
			}
			break;
		case 7: s += sprintf( s, "bw" ); break;
	}
	if( mod == 1 ) {
		disp8 = FETCHD();
		s += sprintf( s, "%s", shexstring((int32_t)disp8, 0, true) );
	} else if( mod == 2 ) {
		disp16 = FETCHD16();
		s += sprintf( s, "%s", shexstring((int32_t)disp16, 0, true) );
	}
	s += sprintf( s, "]" );}

static void handle_param(std::ostream &stream, uint32_t param)
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
			if( modrm >= 0xc0 ) {
				util::stream_format( stream, "%s", nec_reg8[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR8)
					util::stream_format( stream, "byte ptr " );
				util::stream_format( stream, "%s", modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				util::stream_format( stream, "%s", nec_reg[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR16)
					util::stream_format( stream, "word ptr " );
				util::stream_format( stream, "%s", modrm_string );
			}
			break;

		case PARAM_I3:
			i8 = FETCHD();
			util::stream_format( stream, "%d", i8 & 0x07 );
			break;

		case PARAM_I4:
			i8 = FETCHD();
			util::stream_format( stream, "%d", i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = FETCHD();
			util::stream_format( stream, "%s", shexstring((int8_t)i8, 0, false) );
			break;

		case PARAM_I16:
			i16 = FETCHD16();
			util::stream_format( stream, "%s", shexstring((int16_t)i16, 0, false) );
			break;

		case PARAM_UI8:
			i8 = FETCHD();
			util::stream_format( stream, "%s", shexstring((uint8_t)i8, 0, false) );
			break;

		case PARAM_IMM:
			i16 = FETCHD16();
			util::stream_format( stream, "%s", hexstring(i16, 0) );
			break;

		case PARAM_ADDR:
			addr = FETCHD16();
			ptr = FETCHD16();
			util::stream_format( stream, "%s:", hexstring(ptr, 4) );
			util::stream_format( stream, "%s", hexstring(addr, 0) );
			break;

		case PARAM_REL16:
			/* make sure to keep the relative offset within the segment */
			d16 = FETCHD16();
			util::stream_format( stream, "%s", hexstring((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF), 0) );
			break;

		case PARAM_REL8:
			d8 = FETCHD();
			util::stream_format( stream, "%s", hexstring(pc + d8, 0) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_PS: util::stream_format( stream, "ps:" ); break;
				case SEG_DS0: util::stream_format( stream, "ds0:" ); break;
				case SEG_DS1: util::stream_format( stream, "ds1:" ); break;
				case SEG_SS: util::stream_format( stream, "ss:" ); break;
			}

			i16 = FETCHD16();
			util::stream_format( stream, "[%s]", hexstring(i16, 0) );
			break;

		case PARAM_SREG:
			util::stream_format( stream, "%s", nec_sreg[MODRM_REG1] );
			break;

		case PARAM_SFREG:
			i8 = FETCHD();
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

static void handle_fpu(std::ostream &stream, uint8_t op1, uint8_t op2)
{
	switch (op1 & 0x7)
	{
		case 0:     // Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fadd    dword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fmul    dword ptr %s", modrm_string); break;
					case 2: util::stream_format(stream, "fcom    dword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fcomp   dword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fsub    dword ptr %s", modrm_string); break;
					case 5: util::stream_format(stream, "fsubr   dword ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fdiv    dword ptr %s", modrm_string); break;
					case 7: util::stream_format(stream, "fdivr   dword ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fld     dword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fst     dword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fstp    dword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fldenv  word ptr %s", modrm_string); break;
					case 5: util::stream_format(stream, "fldcw   word ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fstenv  word ptr %s", modrm_string); break;
					case 7: util::stream_format(stream, "fstcw   word ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fiadd   dword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fimul   dword ptr %s", modrm_string); break;
					case 2: util::stream_format(stream, "ficom   dword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "ficomp  dword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fisub   dword ptr %s", modrm_string); break;
					case 5: util::stream_format(stream, "fisubr  dword ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fidiv   dword ptr %s", modrm_string); break;
					case 7: util::stream_format(stream, "fidivr  dword ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    dword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fist    dword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fistp   dword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "??? (FPU)"); break;
					case 5: util::stream_format(stream, "fld     tword ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "??? (FPU)"); break;
					case 7: util::stream_format(stream, "fstp    tword ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fadd    qword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fmul    qword ptr %s", modrm_string); break;
					case 2: util::stream_format(stream, "fcom    qword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fcomp   qword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fsub    qword ptr %s", modrm_string); break;
					case 5: util::stream_format(stream, "fsubr   qword ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fdiv    qword ptr %s", modrm_string); break;
					case 7: util::stream_format(stream, "fdivr   qword ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fld     qword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fst     qword ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fstp    qword ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "frstor  %s", modrm_string); break;
					case 5: util::stream_format(stream, "??? (FPU)"); break;
					case 6: util::stream_format(stream, "fsave   %s", modrm_string); break;
					case 7: util::stream_format(stream, "fstsw   word ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fiadd   word ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fimul   word ptr %s", modrm_string); break;
					case 2: util::stream_format(stream, "ficom   word ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "ficomp  word ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fisub   word ptr %s", modrm_string); break;
					case 5: util::stream_format(stream, "fisubr  word ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fidiv   word ptr %s", modrm_string); break;
					case 7: util::stream_format(stream, "fidivr  word ptr %s", modrm_string); break;
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
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    word ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "??? (FPU)"); break;
					case 2: util::stream_format(stream, "fist    word ptr %s", modrm_string); break;
					case 3: util::stream_format(stream, "fistp   word ptr %s", modrm_string); break;
					case 4: util::stream_format(stream, "fbld    %s", modrm_string); break;
					case 5: util::stream_format(stream, "fild    qword ptr %s", modrm_string); break;
					case 6: util::stream_format(stream, "fbstp   %s", modrm_string); break;
					case 7: util::stream_format(stream, "fistp   qword ptr %s", modrm_string); break;
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

static void decode_opcode(std::ostream &stream, const NEC_I386_OPCODE *op, uint8_t op1 )
{
	int i;
	uint8_t op2;

	switch( op->flags )
	{
		case TWO_BYTE:
			op2 = FETCHD();
			decode_opcode( stream, &necv_opcode_table2[op2], op1 );
			return;

		case SEG_PS:
		case SEG_DS0:
		case SEG_DS1:
		case SEG_SS:
			segment = op->flags;
			op2 = FETCH();
			if (Iconfig) op2 = Iconfig[op2];
			decode_opcode( stream, &necv_opcode_table1[op2], op1 );
			return;

		case PREFIX:
			util::stream_format( stream, "%-8s", op->mnemonic );
			op2 = FETCH();
			if (Iconfig) op2 = Iconfig[op2];
			decode_opcode( stream, &necv_opcode_table1[op2], op1 );
			return;

		case GROUP:
			handle_modrm( modrm_string );
			for( i=0; i < ARRAY_LENGTH(group_op_table); i++ ) {
				if( strcmp(op->mnemonic, group_op_table[i].mnemonic) == 0 )
				{
					decode_opcode( stream, &group_op_table[i].opcode[MODRM_REG1], op1 );
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = FETCHD();
			handle_fpu( stream, op1, op2);
			return;

		case MODRM:
			handle_modrm( modrm_string );
			break;
	}

	util::stream_format( stream, "%-8s", op->mnemonic );
	dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		handle_param(stream, op->param1);
	}

	if( op->param2 != 0 ) {
		util::stream_format( stream, "," );
		handle_param(stream, op->param2);
	}

	if( op->param3 != 0 ) {
		util::stream_format( stream, "," );
		handle_param(stream, op->param3);
	}
	return;

handle_unknown:
	util::stream_format(stream, "???");
}

int necv_dasm_one(std::ostream &stream, uint32_t eip, const uint8_t *oprom, const uint8_t *decryption_table)
{
	uint8_t op;
	Iconfig = decryption_table;

	opcode_ptr = opcode_ptr_base = oprom;
	pc = eip;
	dasm_flags = 0;
	segment = 0;

	op = FETCH();

	if (Iconfig) op = Iconfig[op];

	decode_opcode( stream, &necv_opcode_table1[op], op );
	return (pc-eip) | dasm_flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE( nec )
{
	return necv_dasm_one(stream, pc, oprom, nullptr);
}
