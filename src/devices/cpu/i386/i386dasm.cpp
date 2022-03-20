// license:BSD-3-Clause
// copyright-holders:Ville Linde, Peter Ferrie
/*
   i386 Disassembler

   Written by Ville Linde
*/

#include "emu.h"
#include "i386dasm.h"

const i386_disassembler::I386_OPCODE i386_disassembler::i386_opcode_table1[256] =
{
	// 0x00
	{"add",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"add",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"add",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"add",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"add",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"add",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"push    es",      0,              0,                  0,                  0               },
	{"pop     es",      0,              0,                  0,                  0               },
	{"or",              MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"or",              MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"or",              MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"or",              MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"or",              0,              PARAM_AL,           PARAM_UI8,          0               },
	{"or",              0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"push    cs",      0,              0,                  0,                  0               },
	{"two_byte",        TWO_BYTE,       0,                  0,                  0               },
	// 0x10
	{"adc",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"adc",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"adc",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"adc",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"adc",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"adc",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"push    ss",      0,              0,                  0,                  0               },
	{"pop     ss",      0,              0,                  0,                  0               },
	{"sbb",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"sbb",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"sbb",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"sbb",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"sbb",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"sbb",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"push    ds",      0,              0,                  0,                  0               },
	{"pop     ds",      0,              0,                  0,                  0               },
	// 0x20
	{"and",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"and",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"and",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"and",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"and",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"and",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"seg_es",          SEG_ES,         0,                  0,                  0               },
	{"daa",             0,              0,                  0,                  0               },
	{"sub",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"sub",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"sub",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"sub",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"sub",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"sub",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"seg_cs",          SEG_CS,         0,                  0,                  0               },
	{"das",             0,              0,                  0,                  0               },
	// 0x30
	{"xor",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"xor",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"xor",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"xor",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"xor",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"xor",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"seg_ss",          SEG_SS,         0,                  0,                  0               },
	{"aaa",             0,              0,                  0,                  0               },
	{"cmp",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"cmp",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"cmp",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"cmp",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmp",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"cmp",             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"seg_ds",          SEG_DS,         0,                  0,                  0               },
	{"aas",             0,              0,                  0,                  0               },
	// 0x40
	{"inc",             ISREX,          PARAM_EAX,          0,                  0               },
	{"inc",             ISREX,          PARAM_ECX,          0,                  0               },
	{"inc",             ISREX,          PARAM_EDX,          0,                  0               },
	{"inc",             ISREX,          PARAM_EBX,          0,                  0               },
	{"inc",             ISREX,          PARAM_ESP,          0,                  0               },
	{"inc",             ISREX,          PARAM_EBP,          0,                  0               },
	{"inc",             ISREX,          PARAM_ESI,          0,                  0               },
	{"inc",             ISREX,          PARAM_EDI,          0,                  0               },
	{"dec",             ISREX,          PARAM_EAX,          0,                  0               },
	{"dec",             ISREX,          PARAM_ECX,          0,                  0               },
	{"dec",             ISREX,          PARAM_EDX,          0,                  0               },
	{"dec",             ISREX,          PARAM_EBX,          0,                  0               },
	{"dec",             ISREX,          PARAM_ESP,          0,                  0               },
	{"dec",             ISREX,          PARAM_EBP,          0,                  0               },
	{"dec",             ISREX,          PARAM_ESI,          0,                  0               },
	{"dec",             ISREX,          PARAM_EDI,          0,                  0               },
	// 0x50
	{"push",            ALWAYS64,       PARAM_EAX,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_ECX,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_EDX,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_EBX,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_ESP,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_EBP,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_ESI,          0,                  0               },
	{"push",            ALWAYS64,       PARAM_EDI,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_EAX,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_ECX,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_EDX,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_EBX,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_ESP,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_EBP,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_ESI,          0,                  0               },
	{"pop",             ALWAYS64,       PARAM_EDI,          0,                  0               },
	// 0x60
	{"pusha\0pushad\0<invalid>",VAR_NAME,0,                 0,                  0               },
	{"popa\0popad\0<invalid>",  VAR_NAME,0,                 0,                  0               },
	{"bound",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"arpl",            MODRM | SPECIAL64_ENT(0),PARAM_RM,  PARAM_REG16,        0               },
	{"seg_fs",          SEG_FS,         0,                  0,                  0               },
	{"seg_gs",          SEG_GS,         0,                  0,                  0               },
	{"op_size",         OP_SIZE,        0,                  0,                  0               },
	{"addr_size",       ADDR_SIZE,      0,                  0,                  0               },
	{"push",            0,              PARAM_IMM,          0,                  0               },
	{"imul",            MODRM,          PARAM_REG,          PARAM_RM,           PARAM_IMM       },
	{"push",            0,              PARAM_I8,           0,                  0               },
	{"imul",            MODRM,          PARAM_REG,          PARAM_RM,           PARAM_I8        },
	{"insb",            0,              0,                  0,                  0               },
	{"insw\0insd\0insd",VAR_NAME,       0,                  0,                  0               },
	{"outsb",           0,              PARAM_PREIMP,       0,                  0               },
	{"outsw\0outsd\0outsd",VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	// 0x70
	{"jo",              0,              PARAM_REL8,         0,                  0               },
	{"jno",             0,              PARAM_REL8,         0,                  0               },
	{"jb",              0,              PARAM_REL8,         0,                  0               },
	{"jae",             0,              PARAM_REL8,         0,                  0               },
	{"je",              0,              PARAM_REL8,         0,                  0               },
	{"jne",             0,              PARAM_REL8,         0,                  0               },
	{"jbe",             0,              PARAM_REL8,         0,                  0               },
	{"ja",              0,              PARAM_REL8,         0,                  0               },
	{"js",              0,              PARAM_REL8,         0,                  0               },
	{"jns",             0,              PARAM_REL8,         0,                  0               },
	{"jp",              0,              PARAM_REL8,         0,                  0               },
	{"jnp",             0,              PARAM_REL8,         0,                  0               },
	{"jl",              0,              PARAM_REL8,         0,                  0               },
	{"jge",             0,              PARAM_REL8,         0,                  0               },
	{"jle",             0,              PARAM_REL8,         0,                  0               },
	{"jg",              0,              PARAM_REL8,         0,                  0               },
	// 0x80
	{"group80",         GROUP,          0,                  0,                  0               },
	{"group81",         GROUP,          0,                  0,                  0               },
	{"group80",         GROUP,          0,                  0,                  0               },
	{"group83",         GROUP,          0,                  0,                  0               },
	{"test",            MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"test",            MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"xchg",            MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"xchg",            MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"mov",             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{"mov",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"mov",             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{"mov",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"mov",             MODRM,          PARAM_RM,           PARAM_SREG,         0               },
	{"lea",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"mov",             MODRM,          PARAM_SREG,         PARAM_RM,           0               },
	{"pop",             MODRM,          PARAM_RM,           0,                  0               },
	// 0x90
	{"nop\0???\0???\0pause",    VAR_NAME4,          0,                  0,                  0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_ECX,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_EDX,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_EBX,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_ESP,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_EBP,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_ESI,          0               },
	{"xchg",            0,              PARAM_EAX,          PARAM_EDI,          0               },
	{"cbw\0cwde\0cdqe", VAR_NAME,       0,                  0,                  0               },
	{"cwd\0cdq\0cqo",   VAR_NAME,       0,                  0,                  0               },
	{"call",            ALWAYS64,       PARAM_ADDR,         0,                  0,              STEP_OVER},
	{"wait",            0,              0,                  0,                  0               },
	{"pushf\0pushfd\0pushfq",VAR_NAME,  0,                  0,                  0               },
	{"popf\0popfd\0popfq",VAR_NAME,     0,                  0,                  0               },
	{"sahf",            0,              0,                  0,                  0               },
	{"lahf",            0,              0,                  0,                  0               },
	// 0xa0
	{"mov",             0,              PARAM_AL,           PARAM_MEM_OFFS,     0               },
	{"mov",             0,              PARAM_EAX,          PARAM_MEM_OFFS,     0               },
	{"mov",             0,              PARAM_MEM_OFFS,     PARAM_AL,           0               },
	{"mov",             0,              PARAM_MEM_OFFS,     PARAM_EAX,          0               },
	{"movsb",           0,              PARAM_PREIMP,       0,                  0               },
	{"movsw\0movsd\0movsq",VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{"cmpsb",           0,              PARAM_PREIMP,       0,                  0               },
	{"cmpsw\0cmpsd\0cmpsq",VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{"test",            0,              PARAM_AL,           PARAM_UI8,          0               },
	{"test",            0,              PARAM_EAX,          PARAM_IMM,          0               },
	{"stosb",           0,              0,                  0,                  0               },
	{"stosw\0stosd\0stosq",VAR_NAME,    0,                  0,                  0               },
	{"lodsb",           0,              PARAM_PREIMP,       0,                  0               },
	{"lodsw\0lodsd\0lodsq",VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{"scasb",           0,              0,                  0,                  0               },
	{"scasw\0scasd\0scasq",VAR_NAME,    0,                  0,                  0               },
	// 0xb0
	{"mov",             0,              PARAM_AL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_CL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_DL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_BL,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_AH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_CH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_DH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_BH,           PARAM_UI8,          0               },
	{"mov",             0,              PARAM_EAX,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_ECX,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_EDX,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_EBX,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_ESP,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_EBP,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_ESI,          PARAM_IMM64,        0               },
	{"mov",             0,              PARAM_EDI,          PARAM_IMM64,        0               },
	// 0xc0
	{"groupC0",         GROUP,          0,                  0,                  0               },
	{"groupC1",         GROUP,          0,                  0,                  0               },
	{"ret",             0,              PARAM_UI16,         0,                  0,              STEP_OUT},
	{"ret",             0,              0,                  0,                  0,              STEP_OUT},
	{"les",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"lds",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"mov",             MODRM,          PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"mov",             MODRM,          PARAM_RMPTR,        PARAM_IMM,          0               },
	{"enter",           0,              PARAM_UI16,         PARAM_UI8,          0               },
	{"leave",           0,              0,                  0,                  0               },
	{"retf",            0,              PARAM_UI16,         0,                  0,              STEP_OUT},
	{"retf",            0,              0,                  0,                  0,              STEP_OUT},
	{"int 3",           0,              0,                  0,                  0,              STEP_OVER},
	{"int",             0,              PARAM_UI8,          0,                  0,              STEP_OVER},
	{"into",            0,              0,                  0,                  0               },
	{"iret",            0,              0,                  0,                  0,              STEP_OUT},
	// 0xd0
	{"groupD0",         GROUP,          0,                  0,                  0               },
	{"groupD1",         GROUP,          0,                  0,                  0               },
	{"groupD2",         GROUP,          0,                  0,                  0               },
	{"groupD3",         GROUP,          0,                  0,                  0               },
	{"aam",             0,              PARAM_UI8,          0,                  0               },
	{"aad",             0,              PARAM_UI8,          0,                  0               },
	{"salc",            0,              0,                  0,                  0               }, //AMD docs name it
	{"xlat",            0,              0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	{"escape",          FPU,            0,                  0,                  0               },
	// 0xe0
	{"loopne",          0,              PARAM_REL8,         0,                  0,              STEP_OVER},
	{"loopz",           0,              PARAM_REL8,         0,                  0,              STEP_OVER},
	{"loop",            0,              PARAM_REL8,         0,                  0,              STEP_OVER},
	{"jcxz\0jecxz\0jrcxz",VAR_NAME,     PARAM_REL8,         0,                  0               },
	{"in",              0,              PARAM_AL,           PARAM_UI8,          0               },
	{"in",              0,              PARAM_EAX,          PARAM_UI8,          0               },
	{"out",             0,              PARAM_UI8,          PARAM_AL,           0               },
	{"out",             0,              PARAM_UI8,          PARAM_EAX,          0               },
	{"call",            0,              PARAM_REL,          0,                  0,              STEP_OVER},
	{"jmp",             0,              PARAM_REL,          0,                  0               },
	{"jmp",             0,              PARAM_ADDR,         0,                  0               },
	{"jmp",             0,              PARAM_REL8,         0,                  0               },
	{"in",              0,              PARAM_AL,           PARAM_DX,           0               },
	{"in",              0,              PARAM_EAX,          PARAM_DX,           0               },
	{"out",             0,              PARAM_DX,           PARAM_AL,           0               },
	{"out",             0,              PARAM_DX,           PARAM_EAX,          0               },
	// 0xf0
	{"lock",            0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"repne",           PREFIX,         0,                  0,                  0,              STEP_OVER},
	{"rep",             PREFIX,         0,                  0,                  0,              STEP_OVER},
	{"hlt",             0,              0,                  0,                  0               },
	{"cmc",             0,              0,                  0,                  0               },
	{"groupF6",         GROUP,          0,                  0,                  0               },
	{"groupF7",         GROUP,          0,                  0,                  0               },
	{"clc",             0,              0,                  0,                  0               },
	{"stc",             0,              0,                  0,                  0               },
	{"cli",             0,              0,                  0,                  0               },
	{"sti",             0,              0,                  0,                  0               },
	{"cld",             0,              0,                  0,                  0               },
	{"std",             0,              0,                  0,                  0               },
	{"groupFE",         GROUP,          0,                  0,                  0               },
	{"groupFF",         GROUP,          0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::x64_opcode_alt[] =
{
	{"movsxd",          MODRM | ALWAYS64,PARAM_REG,         PARAM_RMPTR32,      0               },
};

const i386_disassembler::I386_OPCODE i386_disassembler::i386_opcode_table2[256] =
{
	// 0x00
	{"group0F00",       GROUP,          0,                  0,                  0               },
	{"group0F01",       GROUP,          0,                  0,                  0               },
	{"lar",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"lsl",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"???",             0,              0,                  0,                  0               },
	{"syscall",         0,              0,                  0,                  0               },
	{"clts",            0,              0,                  0,                  0               },
	{"sysret",          0,              0,                  0,                  0               },
	{"invd",            0,              0,                  0,                  0               },
	{"wbinvd",          0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"ud2",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"group0F0D",           GROUP,              0,                  0,                  0               }, //AMD only
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x10
	{"movups\0"
		"movupd\0"
		"movsd\0"
		"movss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"movups\0"
		"movupd\0"
		"movsd\0"
		"movss",            MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{"group0F12",      GROUP|GROUP_MOD, 0,                  0,                  0                   },
	{"movlps\0"
		"movlpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{"unpcklps\0"
		"unpcklpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"unpckhps\0"
		"unpckhpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{ "group0F16",     GROUP|GROUP_MOD, 0,                  0,                  0                   },
	{"movhps\0"
		"movhpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMMM,          PARAM_XMM,         0               },
	{"group0F18",       GROUP,          0,                  0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	{"nop_hint",        0,              PARAM_RMPTR8,               0,                  0               },
	// 0x20
	{"mov",             MODRM,          PARAM_REG2_32,      PARAM_CREG,         0               },
	{"mov",             MODRM,          PARAM_REG2_32,      PARAM_DREG,         0               },
	{"mov",             MODRM,          PARAM_CREG,         PARAM_REG2_32,      0               },
	{"mov",             MODRM,          PARAM_DREG,         PARAM_REG2_32,      0               },
	{"mov",             MODRM,          PARAM_REG2_32,      PARAM_TREG,         0               },
	{"???",             0,              0,                  0,                  0               },
	{"mov",             MODRM,          PARAM_TREG,         PARAM_REG2_32,      0               },
	{"???",             0,              0,                  0,                  0               },
	{"movaps\0"
		"movapd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"movaps\0"
		"movapd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{"cvtpi2ps\0"
		"cvtpi2pd\0"
		"cvtsi2sd\0"
		"cvtsi2ss",     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_RMXMM,        0               },
	{"movntps\0"
		"movntpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{"cvttps2pi\0"
		"cvttpd2pi\0"
		"cvttsd2si\0"
		"cvttss2si",        MODRM|VAR_NAME4,PARAM_REGORXMM,     PARAM_XMMM,         0               },
	{"cvtps2pi\0"
		"cvtpd2pi\0"
		"cvtsd2si\0"
		"cvtss2si",     MODRM|VAR_NAME4,PARAM_REGORXMM,     PARAM_XMMM,         0               },
	{"ucomiss\0"
		"ucomisd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"comiss\0"
		"comisd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x30
	{"wrmsr",           0,              0,                  0,                  0               },
	{"rdtsc",           0,              0,                  0,                  0               },
	{"rdmsr",           0,              0,                  0,                  0               },
	{"rdpmc",           0,              0,                  0,                  0               },
	{"sysenter",        0,              0,                  0,                  0               },
	{"sysexit",         0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"three_byte",          THREE_BYTE,         0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"three_byte",          THREE_BYTE,         0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	// 0x40
	{"cmovo",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovno",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovb",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovae",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmove",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovne",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovbe",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmova",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovs",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovns",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovpe",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovpo",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovl",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovge",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovle",          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"cmovg",           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	// 0x50
	{"movmskps\0"
		"movmskpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_REG3264,      PARAM_XMMM,         0               },
	{"sqrtps\0"
		"sqrtpd\0"
		"sqrtsd\0"
		"sqrtss",           MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"rsqrtps\0"
		"???\0"
		"???\0"
		"rsqrtss",          MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"rcpps\0"
		"???\0"
		"???\0"
		"rcpss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"andps\0"
		"andpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"andnps\0"
		"andnpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"orps\0"
		"orpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"xorps\0"
		"xorpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"addps\0"
		"addpd\0"
		"addsd\0"
		"addss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"mulps\0"
		"mulpd\0"
		"mulsd\0"
		"mulss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"cvtps2pd\0"
		"cvtpd2ps\0"
		"cvtsd2ss\0"
		"cvtss2sd",     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"cvtdq2ps\0"
		"cvtps2dq\0"
		"???\0"
		"cvttps2dq",        MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"subps\0"
		"subpd\0"
		"subsd\0"
		"subss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"minps\0"
		"minpd\0"
		"minsd\0"
		"minss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"divps\0"
		"divpd\0"
		"divsd\0"
		"divss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"maxps\0"
		"maxpd\0"
		"maxsd\0"
		"maxss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x60
	{"punpcklbw",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"punpcklwd",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"punpckldq",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"packsswb",        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pcmpgtb",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pcmpgtw",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pcmpgtd",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"packuswb",        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"punpckhbw",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"punpckhwd",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"punpckhdq",       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"packssdw",        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"???\0"
		"punpcklqdq\0"
		"???\0"
		"???\0",        MODRM|VAR_NAME4,            PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"punpckhqdq\0"
		"???\0"
		"???\0",        MODRM|VAR_NAME4,            PARAM_XMM,          PARAM_XMMM,         0               },
	{"movd",            MODRM,          PARAM_MMX,          PARAM_RM,           0               },
	{"movq\0"
		"movdqa\0"
		"???\0"
		"movdqu",           MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	// 0x70
	{"pshufw\0"
		"pshufd\0"
		"pshuflw\0"
		"pshufhw",          MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         PARAM_UI8       },
	{"group0F71",       GROUP,          0,                  0,                  0               },
	{"group0F72",       GROUP,          0,                  0,                  0               },
	{"group0F73",       GROUP,          0,                  0,                  0               },
	{"pcmpeqb",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pcmpeqw",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pcmpeqd",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"emms",            0,              0,                  0,                  0               },
	{"vmread",          MODRM,              PARAM_RM,               PARAM_REG,              0               },
	{"vmwrite",         MODRM,              PARAM_RM,               PARAM_REG,              0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???\0"
		"haddpd\0"
		"haddps\0"
		"???",              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{"???\0"
		"hsubpd\0"
		"hsubps\0"
		"???",              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{"movd\0"
		"movd\0"
		"???\0"
		"movq",         MODRM|VAR_NAME4,PARAM_RM,           PARAM_MMX,          0               },
	{"movq\0"
		"movdqa\0"
		"???\0"
		"movdqu",           MODRM|VAR_NAME4,PARAM_MMXM,         PARAM_MMX,          0               },
	// 0x80
	{"jo",              0,              PARAM_REL,          0,                  0               },
	{"jno",             0,              PARAM_REL,          0,                  0               },
	{"jb",              0,              PARAM_REL,          0,                  0               },
	{"jae",             0,              PARAM_REL,          0,                  0               },
	{"je",              0,              PARAM_REL,          0,                  0               },
	{"jne",             0,              PARAM_REL,          0,                  0               },
	{"jbe",             0,              PARAM_REL,          0,                  0               },
	{"ja",              0,              PARAM_REL,          0,                  0               },
	{"js",              0,              PARAM_REL,          0,                  0               },
	{"jns",             0,              PARAM_REL,          0,                  0               },
	{"jp",              0,              PARAM_REL,          0,                  0               },
	{"jnp",             0,              PARAM_REL,          0,                  0               },
	{"jl",              0,              PARAM_REL,          0,                  0               },
	{"jge",             0,              PARAM_REL,          0,                  0               },
	{"jle",             0,              PARAM_REL,          0,                  0               },
	{"jg",              0,              PARAM_REL,          0,                  0               },
	// 0x90
	{"seto",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setno",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setb",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setae",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"sete",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setne",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setbe",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"seta",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"sets",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setns",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setp",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setnp",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setl",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setge",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setle",           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{"setg",            MODRM,          PARAM_RMPTR8,       0,                  0               },
	// 0xa0
	{"push    fs",      0,              0,                  0,                  0               },
	{"pop     fs",      0,              0,                  0,                  0               },
	{"cpuid",           0,              0,                  0,                  0               },
	{"bt",              MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"shld",            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_UI8       },
	{"shld",            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_CL        },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"push    gs",      0,              0,                  0,                  0               },
	{"pop     gs",      0,              0,                  0,                  0               },
	{"rsm",             0,              0,                  0,                  0               },
	{"bts",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"shrd",            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_UI8       },
	{"shrd",            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_CL        },
	{"group0FAE",       GROUP,          0,                  0,                  0               },
	{"imul",            MODRM,          PARAM_REG,          PARAM_RM,           0               },
	// 0xb0
	{"cmpxchg",         MODRM,          PARAM_RM8,          PARAM_REG,          0               },
	{"cmpxchg",         MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"lss",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"btr",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"lfs",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"lgs",             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{"movzx",           MODRM,          PARAM_REG,          PARAM_RMPTR8,       0               },
	{"movzx",           MODRM,          PARAM_REG,          PARAM_RMPTR16,      0               },
	{"???\0"
		"???\0"
		"???\0"
		"popcnt",           MODRM|VAR_NAME4,        PARAM_REG,              PARAM_RM16,             0               },
	{"ud2",             0,              0,                  0,                  0               },
	{"group0FBA",       GROUP,          0,                  0,                  0               },
	{"btc",             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"bsf\0"
		"???\0"
		"???\0"
		"tzcnt",            MODRM|VAR_NAME4,    PARAM_REG,          PARAM_RM,           0               },
	{"bsr\0"
		"???\0"
		"???\0"
		"lzcnt",            MODRM|VAR_NAME4,    PARAM_REG,          PARAM_RM,           0,              STEP_OVER},
	{"movsx",           MODRM,          PARAM_REG,          PARAM_RMPTR8,       0               },
	{"movsx",           MODRM,          PARAM_REG,          PARAM_RMPTR16,      0               },
	// 0xc0
	{"xadd",            MODRM,          PARAM_RM8,          PARAM_REG,          0               },
	{"xadd",            MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"cmpps\0"
		"cmppd\0"
		"cmpsd\0"
		"cmpss",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"movnti",          MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{"pinsrw",          MODRM,          PARAM_MMX,          PARAM_RM,           PARAM_UI8       },
	{"pextrw",          MODRM,          PARAM_MMX,          PARAM_RM,           PARAM_UI8       },
	{"shufps\0"
		"shufpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         PARAM_UI8       },
	{"group0FC7",           GROUP,          0,          0,                  0               },
	{"bswap",           0,              PARAM_EAX,          0,                  0               },
	{"bswap",           0,              PARAM_ECX,          0,                  0               },
	{"bswap",           0,              PARAM_EDX,          0,                  0               },
	{"bswap",           0,              PARAM_EBX,          0,                  0               },
	{"bswap",           0,              PARAM_ESP,          0,                  0               },
	{"bswap",           0,              PARAM_EBP,          0,                  0               },
	{"bswap",           0,              PARAM_ESI,          0,                  0               },
	{"bswap",           0,              PARAM_EDI,          0,                  0               },
	// 0xd0
	{"???\0"
		"addsubpd\0"
		"addsubps\0"
		"???\0",            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"psrlw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psrld",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psrlq",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddq",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmullw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"???\0"
		"movq\0"
		"movdq2q\0"
		"movq2dq",          MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmovmskb",        MODRM,          PARAM_REG3264,      PARAM_MMXM,         0               },
	{"psubusb",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubusw",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pminub",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pand",            MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddusb",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddusw",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmaxub",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pandn",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	// 0xe0
	{"pavgb",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psraw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psrad",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pavgw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmulhuw",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmulhw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"???\0"
		"cvttpd2dq\0"
		"cvtpd2dq\0"
		"cvtdq2pd",     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"movntq\0"
		"movntdq\0"
		"???\0"
		"???\0",            MODRM|VAR_NAME4,    PARAM_M64,          PARAM_MMX,          0               },
	{"psubsb",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubsw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pminsw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"por",             MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddsb",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddsw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmaxsw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pxor",            MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	// 0xf0
	{"???\0"
		"???\0"
		"lddqu\0"
		"???",              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{"psllw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pslld",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psllq",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmuludq",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"pmaddwd",         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psadbw",          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"maskmovq\0"
		"maskmovdqu\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubb",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubd",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"psubq",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddb",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddw",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"paddd",           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::i386_opcode_table0F38[256] =
{
	// 0x00
	{"pshufb\0"
		"pshufb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phaddw\0"
		"phaddw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phaddd\0"
		"phadd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phaddsw\0"
		"phaddsw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"pmaddubsw\0"
		"pmaddubsw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phsubw\0"
		"phsubw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phsubd\0"
		"phsubd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"phsubsw\0"
		"phsubsw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"psignb\0"
		"psignb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"psignw\0"
		"psignw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"psignd\0"
		"psignd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"pmulhrsw\0"
		"pmulhrsw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x10
	{"???\0"
		"pblendvb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"blendvps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{"???\0"
		"blendvpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"ptest\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"pabsb\0"
		"pabsb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"pabsw\0"
		"pabsw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"pabsd\0"
		"pabsd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	// 0x20
	{"???\0"
		"pmovsxbw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???\0"
		"pmovsxbd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{"???\0"
		"pmovsxbq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM16,            0               },
	{"???\0"
		"pmovsxwd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???\0"
		"pmovsxwq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{"???\0"
		"pmovsxdq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"pmuldq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pcmpeqq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"movntdqa\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"packusdw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x30
	{"???\0"
		"pmovzxbw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???\0"
		"pmovzxbd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{"???\0"
		"pmovzxbq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM16,            0               },
	{"???\0"
		"pmovzxwd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???\0"
		"pmovzxwq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{"???\0"
		"pmovzxdq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"pcmpgtq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pminsb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pminsd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pminuw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pminud\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pmaxsb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pmaxsd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pmaxuw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"pmaxud\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x40
	{"???\0"
		"pmulld\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"phminposuw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x50
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x60
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x70
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x80
	{"???\0"
		"invept\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{"???\0"
		"invvpid\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{"???\0"
		"invpcid\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x90
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xa0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xb0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xc0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xd0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"aesimc\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"aesenc\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"aesenclast\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"aesdec\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{"???\0"
		"aesdeclast\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	// 0xe0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xf0
	{"movbe\0"
		"???\0"
		"crc32\0"
		"???",              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_RMPTR,            0               }, // not quite correct
	{"movbe\0"
		"???\0"
		"crc32\0"
		"???",              MODRM|VAR_NAME4,    PARAM_RMPTR,            PARAM_REG32,            0               }, // not quite correct
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
};

const i386_disassembler::I386_OPCODE i386_disassembler::i386_opcode_table0F3A[256] =
{
	// 0x00
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"roundps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"roundpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"roundss\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"roundsd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"blendps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"blendpd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"pblendw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"palignr\0"
		"palignr\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	// 0x10
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"pextrb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_RM8,          PARAM_XMM,          PARAM_UI8           },
	{"???\0"
		"pextrw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_RM16,         PARAM_XMM,          PARAM_UI8           },
	{"???\0"
		"pextrd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_RM8,          PARAM_XMM,          PARAM_UI8           },
	{"???\0"
		"extractps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_RM32,         PARAM_XMM,          PARAM_UI8           },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x20
	{"???\0"
		"pinsrb\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM8,          PARAM_UI8           },
	{"???\0"
		"insertps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM8,          PARAM_UI8           },
	{"???\0"
		"pinsrd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM32,         PARAM_UI8           },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x30
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x40
	{"???\0"
		"dpps\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"dppd\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"mpsadbw\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"pclmulqdq\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x50
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x60
	{"???\0"
		"pcmestrm\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"pcmestri\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"pcmistrm\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???\0"
		"pcmistri\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x70
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x80
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0x90
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xa0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xb0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xc0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xd0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???\0"
		"aeskeygenassist\0"
		"???\0"
		"???",              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	// 0xe0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	// 0xf0
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
	{"???",             0,              0,          0,              0               },
};

const i386_disassembler::I386_OPCODE i386_disassembler::group80_table[8] =
{
	{"add",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"or",              0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"adc",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"sbb",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"and",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"sub",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"xor",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"cmp",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group81_table[8] =
{
	{"add",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"or",              0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"adc",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"sbb",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"and",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"sub",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"xor",             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"cmp",             0,              PARAM_RMPTR,        PARAM_IMM,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group83_table[8] =
{
	{"add",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"or",              0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"adc",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"sbb",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"and",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"sub",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"xor",             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{"cmp",             0,              PARAM_RMPTR,        PARAM_I8,           0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupC0_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"rcl",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"rcr",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"sal",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"sar",             0,              PARAM_RMPTR8,       PARAM_UI8,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupC1_table[8] =
{
	{"rol",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"ror",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"rcl",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"rcr",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"shl",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"shr",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"sal",             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{"sar",             0,              PARAM_RMPTR,        PARAM_UI8,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupD0_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"rcl",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"rcr",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"sal",             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{"sar",             0,              PARAM_RMPTR8,       PARAM_1,            0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupD1_table[8] =
{
	{"rol",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"ror",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"rcl",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"rcr",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"shl",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"shr",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"sal",             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{"sar",             0,              PARAM_RMPTR,        PARAM_1,            0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupD2_table[8] =
{
	{"rol",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"ror",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"rcl",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"rcr",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"shl",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"shr",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"sal",             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{"sar",             0,              PARAM_RMPTR8,       PARAM_CL,           0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupD3_table[8] =
{
	{"rol",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"ror",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"rcl",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"rcr",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"shl",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"shr",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"sal",             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{"sar",             0,              PARAM_RMPTR,        PARAM_CL,           0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupF6_table[8] =
{
	{"test",            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"test",            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{"not",             0,              PARAM_RMPTR8,       0,                  0               },
	{"neg",             0,              PARAM_RMPTR8,       0,                  0               },
	{"mul",             0,              PARAM_RMPTR8,       0,                  0               },
	{"imul",            0,              PARAM_RMPTR8,       0,                  0               },
	{"div",             0,              PARAM_RMPTR8,       0,                  0               },
	{"idiv",            0,              PARAM_RMPTR8,       0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupF7_table[8] =
{
	{"test",            0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"test",            0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{"not",             0,              PARAM_RMPTR,        0,                  0               },
	{"neg",             0,              PARAM_RMPTR,        0,                  0               },
	{"mul",             0,              PARAM_RMPTR,        0,                  0               },
	{"imul",            0,              PARAM_RMPTR,        0,                  0               },
	{"div",             0,              PARAM_RMPTR,        0,                  0               },
	{"idiv",            0,              PARAM_RMPTR,        0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::groupFE_table[8] =
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

const i386_disassembler::I386_OPCODE i386_disassembler::groupFF_table[8] =
{
	{"inc",             0,              PARAM_RMPTR,        0,                  0               },
	{"dec",             0,              PARAM_RMPTR,        0,                  0               },
	{"call",            ALWAYS64,       PARAM_RMPTR,        0,                  0,              STEP_OVER},
	{"call    far ptr ",0,              PARAM_RM,           0,                  0,              STEP_OVER},
	{"jmp",             ALWAYS64,       PARAM_RMPTR,        0,                  0               },
	{"jmp     far ptr ",0,              PARAM_RM,           0,                  0               },
	{"push",            0,              PARAM_RMPTR,        0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F00_table[8] =
{
	{"sldt",            0,              PARAM_RM,           0,                  0               },
	{"str",             0,              PARAM_RM,           0,                  0               },
	{"lldt",            0,              PARAM_RM,           0,                  0               },
	{"ltr",             0,              PARAM_RM,           0,                  0               },
	{"verr",            0,              PARAM_RM,           0,                  0               },
	{"verw",            0,              PARAM_RM,           0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F01_table[8] =
{
	{"sgdt",            0,              PARAM_RM,           0,                  0               },
	{"sidt",            0,              PARAM_RM,           0,                  0               },
	{"lgdt",            0,              PARAM_RM,           0,                  0               },
	{"lidt",            0,              PARAM_RM,           0,                  0               },
	{"smsw",            0,              PARAM_RM,           0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"lmsw",            0,              PARAM_RM,           0,                  0               },
	{"invlpg",          0,              PARAM_RM,           0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F0D_table[8] =
{
	{"prefetch",        0,              PARAM_RM8,          0,                  0               },
	{"prefetchw",       0,              PARAM_RM8,          0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F12_table[4] =
{
	{ "movlps\0"
		"movlpd\0"
		"movddup\0"
		"movsldup",     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{ "movlps\0"
		"movlpd\0"
		"movddup\0"
		"movsldup",     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{ "movlps\0"
		"movlpd\0"
		"movddup\0"
		"movsldup",     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{ "movhlps\0"
		"???\0"
		"movddup\0"
		"movsldup",     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F16_table[4] =
{
	{ "movhps\0"
		"movhpd\0"
		"???\0"
		"movshdup",     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{ "movhps\0"
		"movhpd\0"
		"???\0"
		"movshdup",     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{ "movhps\0"
		"movhpd\0"
		"???\0"
		"movshdup",     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{ "movlhps\0"
		"movhpd\0"
		"???\0"
		"movshdup",     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F18_table[8] =
{
	{"prefetchnta",     0,              PARAM_RM8,          0,                  0               },
	{"prefetch0",       0,              PARAM_RM8,          0,                  0               },
	{"prefetch1",       0,              PARAM_RM8,          0,                  0               },
	{"prefetch2",       0,              PARAM_RM8,          0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F71_table[8] =
{
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"psrlw",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"psraw",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"psllw",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F72_table[8] =
{
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"psrld",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"psrad",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"pslld",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0F73_table[8] =
{
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"psrlq",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"psrldq",          0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"psllq",           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{"pslldq",          0,              PARAM_MMX2,         PARAM_UI8,          0               },
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0FAE_table[8] =
{
	{"fxsave",          0,              PARAM_RM,           0,                  0               },
	{"fxrstor",         0,              PARAM_RM,           0,                  0               },
	{"ldmxcsr",         0,              PARAM_RM,           0,                  0               },
	{"stmxscr",         0,              PARAM_RM,           0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"lfence",          0,              0,                  0,                  0               },
	{"mfence",          0,              0,                  0,                  0               },
	{"sfence",          0,              0,                  0,                  0               }
};


const i386_disassembler::I386_OPCODE i386_disassembler::group0FBA_table[8] =
{
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"bt",              0,              PARAM_RM,           PARAM_UI8,          0               },
	{"bts",             0,              PARAM_RM,           PARAM_UI8,          0               },
	{"btr",             0,              PARAM_RM,           PARAM_UI8,          0               },
	{"btc",             0,              PARAM_RM,           PARAM_UI8,          0               }
};

const i386_disassembler::I386_OPCODE i386_disassembler::group0FC7_table[8] =
{
	{"???",             0,              0,                  0,                  0               },
	{"cmpxchg8b",           MODRM,              PARAM_M64PTR,               0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"???",             0,              0,                  0,                  0               },
	{"vmptrld\0"
		"vmclear\0"
		"???\0"
		"vmxon",            MODRM|VAR_NAME4,        PARAM_M64PTR,               0,                  0               },
	{"vmptrtst",            MODRM,              PARAM_M64PTR,               0,                  0               }
};

const i386_disassembler::GROUP_OP i386_disassembler::group_op_table[] =
{
	{ "group80",            group80_table           },
	{ "group81",            group81_table           },
	{ "group83",            group83_table           },
	{ "groupC0",            groupC0_table           },
	{ "groupC1",            groupC1_table           },
	{ "groupD0",            groupD0_table           },
	{ "groupD1",            groupD1_table           },
	{ "groupD2",            groupD2_table           },
	{ "groupD3",            groupD3_table           },
	{ "groupF6",            groupF6_table           },
	{ "groupF7",            groupF7_table           },
	{ "groupFE",            groupFE_table           },
	{ "groupFF",            groupFF_table           },
	{ "group0F00",          group0F00_table         },
	{ "group0F01",          group0F01_table         },
	{ "group0F0D",          group0F0D_table         },
	{ "group0F12",          group0F12_table         },
	{ "group0F16",          group0F16_table         },
	{ "group0F18",          group0F18_table         },
	{ "group0F71",          group0F71_table         },
	{ "group0F72",          group0F72_table         },
	{ "group0F73",          group0F73_table         },
	{ "group0FAE",          group0FAE_table         },
	{ "group0FBA",          group0FBA_table         },
	{ "group0FC7",          group0FC7_table         }
};



const char *const i386_disassembler::i386_reg[3][16] =
{
	{"ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",  "r8w", "r9w", "r10w","r11w","r12w","r13w","r14w","r15w"},
	{"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d","r11d","r12d","r13d","r14d","r15d"},
	{"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"}
};

const char *const i386_disassembler::i386_reg8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *const i386_disassembler::i386_reg8rex[16] = {"al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l"};
const char *const i386_disassembler::i386_sreg[8] = {"es", "cs", "ss", "ds", "fs", "gs", "???", "???"};

inline uint8_t i386_disassembler::FETCH(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	if ((pc - base_pc) + 1 > max_length)
		return 0xff;
	uint8_t d = opcodes.r8(pc);
	pc++;
	return d;
}

inline uint16_t i386_disassembler::FETCH16(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	if ((pc - base_pc) + 2 > max_length)
		return 0xffff;
	uint16_t d = opcodes.r16(pc);
	pc += 2;
	return d;
}

inline uint32_t i386_disassembler::FETCH32(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	if ((pc - base_pc) + 4 > max_length)
		return 0xffffffff;
	uint32_t d = opcodes.r32(pc);
	pc += 4;
	return d;
}

inline uint8_t i386_disassembler::FETCHD(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	return FETCH(base_pc, pc, opcodes);
}

inline uint16_t i386_disassembler::FETCHD16(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	return FETCH16(base_pc, pc, opcodes);
}

inline uint32_t i386_disassembler::FETCHD32(offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	return FETCH32(base_pc, pc, opcodes);
}

char *i386_disassembler::hexstring(uint32_t value, int digits)
{
	static char buffer[20];
	buffer[0] = '0';
	if (digits)
		sprintf(&buffer[1], "%0*Xh", digits, value);
	else
		sprintf(&buffer[1], "%Xh", value);
	return (buffer[1] >= '0' && buffer[1] <= '9') ? &buffer[1] : &buffer[0];
}

char *i386_disassembler::hexstring64(uint32_t lo, uint32_t hi)
{
	static char buffer[40];
	buffer[0] = '0';
	if (hi != 0)
		sprintf(&buffer[1], "%X%08Xh", hi, lo);
	else
		sprintf(&buffer[1], "%Xh", lo);
	return (buffer[1] >= '0' && buffer[1] <= '9') ? &buffer[1] : &buffer[0];
}

std::string i386_disassembler::hexstringpc(uint64_t pc)
{
	if (m_config->get_mode() == 64)
		return hexstring64(uint32_t(pc), uint32_t(pc >> 32));
	else
		return hexstring(uint32_t(pc), 0);
}

std::string i386_disassembler::shexstring(uint32_t value, int digits, bool always)
{
	if (value >= 0x80000000)
		return util::string_format("-%s", hexstring(-value, digits));
	else if (always)
		return util::string_format("+%s", hexstring(value, digits));
	else
		return hexstring(value, digits);
}

void i386_disassembler::handle_sib_byte(std::ostream &stream, uint8_t mod, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	uint32_t i32;
	uint8_t scale, i, base;
	uint8_t sib = FETCHD(base_pc, pc, opcodes);

	scale = (sib >> 6) & 0x3;
	i = ((sib >> 3) & 0x7) | sibex;
	base = (sib & 0x7) | rmex;

	if (base == 5 && mod == 0) {
		i32 = FETCHD32(base_pc, pc, opcodes);
		util::stream_format(stream, "%s", hexstring(i32, 0));
	} else if (base != 5 || mod != 3)
		util::stream_format(stream, "%s", i386_reg[address_size][base]);

	if ( i != 4 ) {
		util::stream_format(stream, "+%s", i386_reg[address_size][i]);
		if (scale)
			util::stream_format(stream, "*%d", 1 << scale);
	}
}

void i386_disassembler::handle_modrm(std::ostream &stream, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	int8_t disp8;
	int16_t disp16;
	int32_t disp32;
	uint8_t mod, rm;

	modrm = FETCHD(base_pc, pc, opcodes);
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7) | rmex;

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_CS: util::stream_format(stream, "cs:"); break;
		case SEG_DS: util::stream_format(stream, "ds:"); break;
		case SEG_ES: util::stream_format(stream, "es:"); break;
		case SEG_FS: util::stream_format(stream, "fs:"); break;
		case SEG_GS: util::stream_format(stream, "gs:"); break;
		case SEG_SS: util::stream_format(stream, "ss:"); break;
	}

	util::stream_format(stream, "[" );
	if( address_size == 2 ) {
		if ((rm & 7) == 4)
			handle_sib_byte(stream, mod, base_pc, pc, opcodes);
		else if ((rm & 7) == 5 && mod == 0) {
			disp32 = FETCHD32(base_pc, pc, opcodes);
			util::stream_format(stream, "rip%s", shexstring(disp32, 0, true));
		} else
			util::stream_format(stream, "%s", i386_reg[2][rm]);
		if( mod == 1 ) {
			disp8 = FETCHD(base_pc, pc, opcodes);
			if (disp8 != 0)
				util::stream_format(stream, "%s", shexstring((int32_t)disp8, 0, true) );
		} else if( mod == 2 ) {
			disp32 = FETCHD32(base_pc, pc, opcodes);
			if (disp32 != 0)
				util::stream_format(stream, "%s", shexstring(disp32, 0, true) );
		}
	} else if (address_size == 1) {
		if ((rm & 7) == 4)
			handle_sib_byte(stream, mod, base_pc, pc, opcodes);
		else if ((rm & 7) == 5 && mod == 0) {
			disp32 = FETCHD32(base_pc, pc, opcodes);
			if (m_config->get_mode() == 64)
				util::stream_format(stream, "eip%s", shexstring(disp32, 0, true) );
			else
				util::stream_format(stream, "%s", hexstring(disp32, 0) );
		} else
			util::stream_format(stream, "%s", i386_reg[1][rm]);
		if( mod == 1 ) {
			disp8 = FETCHD(base_pc, pc, opcodes);
			if (disp8 != 0)
				util::stream_format(stream, "%s", shexstring((int32_t)disp8, 0, true) );
		} else if( mod == 2 ) {
			disp32 = FETCHD32(base_pc, pc, opcodes);
			if (disp32 != 0)
				util::stream_format(stream, "%s", shexstring(disp32, 0, true) );
		}
	} else {
		switch( rm )
		{
			case 0: util::stream_format(stream, "bx+si" ); break;
			case 1: util::stream_format(stream, "bx+di" ); break;
			case 2: util::stream_format(stream, "bp+si" ); break;
			case 3: util::stream_format(stream, "bp+di" ); break;
			case 4: util::stream_format(stream, "si" ); break;
			case 5: util::stream_format(stream, "di" ); break;
			case 6:
				if( mod == 0 ) {
					disp16 = FETCHD16(base_pc, pc, opcodes);
					util::stream_format(stream, "%s", hexstring((unsigned) (uint16_t) disp16, 0) );
				} else {
					util::stream_format(stream, "bp" );
				}
				break;
			case 7: util::stream_format(stream, "bx" ); break;
		}
		if( mod == 1 ) {
			disp8 = FETCHD(base_pc, pc, opcodes);
			if (disp8 != 0)
				util::stream_format(stream, "%s", shexstring((int32_t)disp8, 0, true) );
		} else if( mod == 2 ) {
			disp16 = FETCHD16(base_pc, pc, opcodes);
			if (disp16 != 0)
				util::stream_format(stream, "%s", shexstring((int32_t)disp16, 0, true) );
		}
	}
	util::stream_format(stream, "]" );
}

void i386_disassembler::handle_modrm(std::string &buffer, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	std::stringstream stream;
	handle_modrm(stream, base_pc, pc, opcodes);
	buffer = stream.str();
}

void i386_disassembler::handle_param(std::ostream &stream, uint32_t param, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	uint8_t i8;
	uint16_t i16;
	uint32_t i32;
	uint16_t ptr;
	uint32_t addr;
	int8_t d8;
	int16_t d16;
	int32_t d32;

	switch(param)
	{
		case PARAM_REG:
			util::stream_format(stream, "%s", i386_reg[operand_size][MODRM_REG1() | regex] );
			break;

		case PARAM_REG8:
			util::stream_format(stream, "%s", (rex ? i386_reg8rex : i386_reg8)[MODRM_REG1() | regex] );
			break;

		case PARAM_REG16:
			util::stream_format(stream, "%s", i386_reg[0][MODRM_REG1() | regex] );
			break;

		case PARAM_REG32:
			util::stream_format(stream, "%s", i386_reg[1][MODRM_REG1() | regex] );
			break;

		case PARAM_REG3264:
			util::stream_format(stream, "%s", i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG1() | regex] );
			break;

		case PARAM_MMX:
			if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
				util::stream_format(stream, "xmm%d", MODRM_REG1() | regex );
			else
				util::stream_format(stream, "mm%d", MODRM_REG1() | regex );
			break;

		case PARAM_MMX2:
			if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
				util::stream_format(stream, "xmm%d", MODRM_REG2() | regex );
			else
				util::stream_format(stream, "mm%d", MODRM_REG2() | regex );
			break;

		case PARAM_XMM:
			util::stream_format(stream, "xmm%d", MODRM_REG1() | regex );
			break;

		case PARAM_REGORXMM:
			if (pre0f != 0xf2 && pre0f != 0xf3)
				util::stream_format(stream, "xmm%d", MODRM_REG1() | regex );
			else
				util::stream_format(stream, "%s", i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG1() | regex] );
			break;

		case PARAM_REG2_32:
			util::stream_format(stream, "%s", i386_reg[1][MODRM_REG2() | rmex] );
			break;

		case PARAM_RM:
		case PARAM_RMPTR:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "%s", i386_reg[operand_size][MODRM_REG2() | rmex] );
			} else {
				if (param == PARAM_RMPTR)
				{
					if( operand_size == 2 )
						util::stream_format(stream, "qword ptr " );
					else if (operand_size == 1)
						util::stream_format(stream, "dword ptr " );
					else
						util::stream_format(stream, "word ptr " );
				}
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "%s", (rex ? i386_reg8rex : i386_reg8)[MODRM_REG2() | rmex] );
			} else {
				if (param == PARAM_RMPTR8)
					util::stream_format(stream, "byte ptr " );
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "%s", i386_reg[0][MODRM_REG2() | rmex] );
			} else {
				if (param == PARAM_RMPTR16)
					util::stream_format(stream, "word ptr " );
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_RM32:
		case PARAM_RMPTR32:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "%s", i386_reg[1][MODRM_REG2() | rmex] );
			} else {
				if (param == PARAM_RMPTR32)
					util::stream_format(stream, "dword ptr " );
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_RMXMM:
			if( modrm >= 0xc0 ) {
				if (pre0f != 0xf2 && pre0f != 0xf3)
					util::stream_format(stream, "xmm%d", MODRM_REG2() | rmex );
				else
					util::stream_format(stream, "%s", i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG2() | rmex] );
			} else {
				if (param == PARAM_RMPTR32)
					util::stream_format(stream, "dword ptr " );
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_M64:
		case PARAM_M64PTR:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "???" );
			} else {
				if (param == PARAM_M64PTR)
					util::stream_format(stream, "qword ptr " );
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_MMXM:
			if( modrm >= 0xc0 ) {
				if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
					util::stream_format(stream, "xmm%d", MODRM_REG2() | rmex );
				else
					util::stream_format(stream, "mm%d", MODRM_REG2() | rmex );
			} else {
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_XMMM:
			if( modrm >= 0xc0 ) {
				util::stream_format(stream, "xmm%d", MODRM_REG2() | rmex );
			} else {
				util::stream_format(stream, "%s", modrm_string );
			}
			break;

		case PARAM_I4:
			i8 = FETCHD(base_pc, pc, opcodes);
			util::stream_format(stream, "%d", i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = FETCHD(base_pc, pc, opcodes);
			util::stream_format(stream, "%s", shexstring((int8_t)i8, 0, false) );
			break;

		case PARAM_I16:
			i16 = FETCHD16(base_pc, pc, opcodes);
			util::stream_format(stream, "%s", shexstring((int16_t)i16, 0, false) );
			break;

		case PARAM_UI8:
			i8 = FETCHD(base_pc, pc, opcodes);
			util::stream_format(stream, "%s", shexstring((uint8_t)i8, 0, false) );
			break;

		case PARAM_UI16:
			i16 = FETCHD16(base_pc, pc, opcodes);
			util::stream_format(stream, "%s", shexstring((uint16_t)i16, 0, false) );
			break;

		case PARAM_IMM64:
			if (operand_size == 2) {
				uint32_t lo32 = FETCHD32(base_pc, pc, opcodes);
				i32 = FETCHD32(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstring64(lo32, i32) );
			} else if( operand_size ) {
				i32 = FETCHD32(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstring(i32, 0) );
			} else {
				i16 = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstring(i16, 0) );
			}
			break;

		case PARAM_IMM:
			if( operand_size ) {
				i32 = FETCHD32(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstring(i32, 0) );
			} else {
				i16 = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstring(i16, 0) );
			}
			break;

		case PARAM_ADDR:
			if( operand_size ) {
				addr = FETCHD32(base_pc, pc, opcodes);
				ptr = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "%s:", hexstring(ptr, 4) );
				util::stream_format(stream, "%s", hexstring(addr, 0) );
			} else {
				addr = FETCHD16(base_pc, pc, opcodes);
				ptr = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "%s:", hexstring(ptr, 4) );
				util::stream_format(stream, "%s", hexstring(addr, 0) );
			}
			break;

		case PARAM_REL:
			if( operand_size ) {
				d32 = FETCHD32(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstringpc(pc + d32) );
			} else {
				/* make sure to keep the relative offset within the segment */
				d16 = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "%s", hexstringpc((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF)) );
			}
			break;

		case PARAM_REL8:
			d8 = FETCHD(base_pc, pc, opcodes);
			util::stream_format(stream, "%s", hexstringpc(pc + d8) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_CS: util::stream_format(stream, "cs:" ); break;
				case SEG_DS: util::stream_format(stream, "ds:" ); break;
				case SEG_ES: util::stream_format(stream, "es:" ); break;
				case SEG_FS: util::stream_format(stream, "fs:" ); break;
				case SEG_GS: util::stream_format(stream, "gs:" ); break;
				case SEG_SS: util::stream_format(stream, "ss:" ); break;
			}

			if( address_size ) {
				i32 = FETCHD32(base_pc, pc, opcodes);
				util::stream_format(stream, "[%s]", hexstring(i32, 0) );
			} else {
				i16 = FETCHD16(base_pc, pc, opcodes);
				util::stream_format(stream, "[%s]", hexstring(i16, 0) );
			}
			break;

		case PARAM_PREIMP:
			switch(segment)
			{
				case SEG_CS: util::stream_format(stream, "cs:" ); break;
				case SEG_DS: util::stream_format(stream, "ds:" ); break;
				case SEG_ES: util::stream_format(stream, "es:" ); break;
				case SEG_FS: util::stream_format(stream, "fs:" ); break;
				case SEG_GS: util::stream_format(stream, "gs:" ); break;
				case SEG_SS: util::stream_format(stream, "ss:" ); break;
			}
			break;

		case PARAM_SREG:
			util::stream_format(stream, "%s", i386_sreg[MODRM_REG1()] );
			break;

		case PARAM_CREG:
			util::stream_format(stream, "cr%d", MODRM_REG1() | regex );
			break;

		case PARAM_TREG:
			util::stream_format(stream, "tr%d", MODRM_REG1() | regex );
			break;

		case PARAM_DREG:
			util::stream_format(stream, "dr%d", MODRM_REG1() | regex );
			break;

		case PARAM_1:
			util::stream_format(stream, "1" );
			break;

		case PARAM_DX:
			util::stream_format(stream, "dx" );
			break;

		case PARAM_XMM0:
			util::stream_format(stream, "xmm0" );
			break;

		case PARAM_AL: util::stream_format(stream, "al" ); break;
		case PARAM_CL: util::stream_format(stream, "cl" ); break;
		case PARAM_DL: util::stream_format(stream, "dl" ); break;
		case PARAM_BL: util::stream_format(stream, "bl" ); break;
		case PARAM_AH: util::stream_format(stream, "ah" ); break;
		case PARAM_CH: util::stream_format(stream, "ch" ); break;
		case PARAM_DH: util::stream_format(stream, "dh" ); break;
		case PARAM_BH: util::stream_format(stream, "bh" ); break;

		case PARAM_EAX: util::stream_format(stream, "%s", i386_reg[operand_size][0 | rmex] ); break;
		case PARAM_ECX: util::stream_format(stream, "%s", i386_reg[operand_size][1 | rmex] ); break;
		case PARAM_EDX: util::stream_format(stream, "%s", i386_reg[operand_size][2 | rmex] ); break;
		case PARAM_EBX: util::stream_format(stream, "%s", i386_reg[operand_size][3 | rmex] ); break;
		case PARAM_ESP: util::stream_format(stream, "%s", i386_reg[operand_size][4 | rmex] ); break;
		case PARAM_EBP: util::stream_format(stream, "%s", i386_reg[operand_size][5 | rmex] ); break;
		case PARAM_ESI: util::stream_format(stream, "%s", i386_reg[operand_size][6 | rmex] ); break;
		case PARAM_EDI: util::stream_format(stream, "%s", i386_reg[operand_size][7 | rmex] ); break;
	}
}

void i386_disassembler::handle_fpu(std::ostream &stream, uint8_t op1, uint8_t op2, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	switch (op1 & 0x7)
	{
		case 0:     // Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				handle_modrm(modrm_string, base_pc, pc, opcodes);
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
					case 1: util::stream_format(stream, "fmul    st(0),st(%d)", op2 & 0x7); break;
					case 2: util::stream_format(stream, "fcom    st(0),st(%d)", op2 & 0x7); break;
					case 3: util::stream_format(stream, "fcomp   st(0),st(%d)", op2 & 0x7); break;
					case 4: util::stream_format(stream, "fsub    st(0),st(%d)", op2 & 0x7); break;
					case 5: util::stream_format(stream, "fsubr   st(0),st(%d)", op2 & 0x7); break;
					case 6: util::stream_format(stream, "fdiv    st(0),st(%d)", op2 & 0x7); break;
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
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
					case 0x29:
						util::stream_format(stream, "fucompp"); break;

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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    dword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fisttp  dword ptr %s", modrm_string); break;
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fld     qword ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fisttp  qword ptr %s", modrm_string); break;
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
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
				handle_modrm(modrm_string, base_pc, pc, opcodes);
				switch ((op2 >> 3) & 0x7)
				{
					case 0: util::stream_format(stream, "fild    word ptr %s", modrm_string); break;
					case 1: util::stream_format(stream, "fisttp  word ptr %s", modrm_string); break;
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
					case 0x20: util::stream_format(stream, "fstsw   ax"); break;

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

void i386_disassembler::decode_opcode(std::ostream &stream, const I386_OPCODE *op, uint8_t op1, offs_t base_pc, offs_t &pc, const data_buffer &opcodes)
{
	int i;
	uint8_t op2;

	if ((op->flags & SPECIAL64) && (address_size == 2))
		op = &x64_opcode_alt[op->flags >> 24];

	switch( op->flags & FLAGS_MASK )
	{
		case ISREX:
			if (m_config->get_mode() == 64)
			{
				rex = op1;
				operand_size = (op1 & 8) ? 2 : 1;
				regex = (op1 << 1) & 8;
				sibex = (op1 << 2) & 8;
				rmex = (op1 << 3) & 8;
				op2 = FETCH(base_pc, pc, opcodes);
				decode_opcode(stream, &i386_opcode_table1[op2], op1, base_pc, pc, opcodes);
				return;
			}
			break;

		case OP_SIZE:
			rex = regex = sibex = rmex = 0;
			if (operand_size < 2 && operand_prefix == 0)
			{
				operand_size ^= 1;
				operand_prefix = 1;
			}
			op2 = FETCH(base_pc, pc, opcodes);
			decode_opcode(stream, &i386_opcode_table1[op2], op2, base_pc, pc, opcodes);
			return;

		case ADDR_SIZE:
			rex = regex = sibex = rmex = 0;
			if(address_prefix == 0)
			{
				if (m_config->get_mode() != 64)
					address_size ^= 1;
				else
					address_size ^= 3;
				address_prefix = 1;
			}
			op2 = FETCH(base_pc, pc, opcodes);
			decode_opcode(stream, &i386_opcode_table1[op2], op2, base_pc, pc, opcodes);
			return;

		case TWO_BYTE:
			if (pc - 2 >= base_pc)
				pre0f = opcodes.r8(pc-2);
			op2 = FETCHD(base_pc, pc, opcodes);
			decode_opcode(stream, &i386_opcode_table2[op2], op1, base_pc, pc, opcodes);
			return;

		case THREE_BYTE:
			op2 = FETCHD(base_pc, pc, opcodes);
			if (opcodes.r8(pc-2) == 0x38)
				decode_opcode(stream, &i386_opcode_table0F38[op2], op1, base_pc, pc, opcodes);
			else
				decode_opcode(stream, &i386_opcode_table0F3A[op2], op1, base_pc, pc, opcodes);
			return;

		case SEG_CS:
		case SEG_DS:
		case SEG_ES:
		case SEG_FS:
		case SEG_GS:
		case SEG_SS:
			rex = regex = sibex = rmex = 0;
			segment = op->flags;
			op2 = FETCH(base_pc, pc, opcodes);
			decode_opcode(stream, &i386_opcode_table1[op2], op2, base_pc, pc, opcodes);
			return;

		case PREFIX:
			op2 = FETCH(base_pc, pc, opcodes);
			if ((op2 != 0x0f) && (op2 != 0x90))
				util::stream_format(stream, "%-7s ", op->mnemonic );
			if ((op2 == 0x90) && !pre0f)
				pre0f = op1;
			decode_opcode(stream, &i386_opcode_table1[op2], op2, base_pc, pc, opcodes);
			dasm_flags |= op->dasm_flags;
			return;

		case GROUP:
			handle_modrm(modrm_string, base_pc, pc, opcodes);
			for( i=0; i < std::size(group_op_table); i++ ) {
				if( strcmp(op->mnemonic, group_op_table[i].mnemonic) == 0 ) {
					if (op->flags & GROUP_MOD)
						decode_opcode(stream, &group_op_table[i].opcode[MODRM_MOD()], op1, base_pc, pc, opcodes);
					else
						decode_opcode(stream, &group_op_table[i].opcode[MODRM_REG1()], op1, base_pc, pc, opcodes);
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = FETCHD(base_pc, pc, opcodes);
			handle_fpu(stream, op1, op2, base_pc, pc, opcodes);
			return;

		case MODRM:
			handle_modrm(modrm_string, base_pc, pc, opcodes);
			break;
	}

	if ((op->flags & ALWAYS64) && m_config->get_mode() == 64)
		operand_size = 2;

	if ((op->flags & VAR_NAME) && operand_size > 0)
	{
		const char *mnemonic = op->mnemonic + strlen(op->mnemonic) + 1;
		if (operand_size == 2)
			mnemonic += strlen(mnemonic) + 1;
		util::stream_format(stream, "%-7s ", mnemonic );
	}
	else if (op->flags & VAR_NAME4)
	{
		const char *mnemonic = op->mnemonic;
		int which = (pre0f == 0xf3) ? 3 : (pre0f == 0xf2) ? 2 : (pre0f == 0x66) ? 1 : 0;
		while (which--)
			mnemonic += strlen(mnemonic) + 1;
		util::stream_format(stream, "%-7s ", mnemonic );
	}
	else
		util::stream_format(stream, "%-7s ", op->mnemonic );
	dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		handle_param(stream, op->param1, base_pc, pc, opcodes);
	}

	if( op->param2 != 0 ) {
		util::stream_format(stream, "," );
		handle_param(stream, op->param2, base_pc, pc, opcodes);
	}

	if( op->param3 != 0 ) {
		util::stream_format(stream, "," );
		handle_param(stream, op->param3, base_pc, pc, opcodes);
	}
	return;

handle_unknown:
	util::stream_format(stream, "???");
}

offs_t i386_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t base_pc = pc;
	uint8_t op;

	switch(m_config->get_mode())
	{
		case 1: /* 8086/8088/80186/80188 */
			address_size = 0;
			operand_size = 0;
			max_length = 8; /* maximum without redundant prefixes - not enforced by chip */
			break;
		case 2: /* 80286 */
			address_size = 0;
			operand_size = 0;
			max_length = 10;
			break;
		case 16: /* 80386+ 16-bit code segment */
			address_size = 0;
			operand_size = 0;
			max_length = 15;
			break;
		case 32: /* 80386+ 32-bit code segment */
			address_size = 1;
			operand_size = 1;
			max_length = 15;
			break;
		case 64: /* x86_64 */
			address_size = 2;
			operand_size = 1;
			max_length = 15;
			break;
	}
	dasm_flags = 0;
	segment = 0;
	pre0f = 0;
	rex = regex = sibex = rmex = 0;
	address_prefix = 0;
	operand_prefix = 0;

	op = FETCH(base_pc, pc, opcodes);

	decode_opcode( stream, &i386_opcode_table1[op], op, base_pc, pc, opcodes);
	return (pc-base_pc) | dasm_flags | SUPPORTED;
}

i386_disassembler::i386_disassembler(config *conf) : m_config(conf)
{
}

u32 i386_disassembler::opcode_alignment() const
{
	return 1;
}
