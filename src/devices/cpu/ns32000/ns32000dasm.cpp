// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*****************************************************************************
*
*   ns32000dasm.cpp
*
*   NS32000 CPU Disassembly
*
*****************************************************************************/

#include "emu.h"
#include "ns32000dasm.h"

// instruction field extraction
#define Format0cond(x)   ((x >> 4) & 0x0f)

#define Format1op(x)     ((x >> 4) & 0x0f)

#define Format2i(x)      ((x >> 0) & 0x03)
#define Format2op(x)     ((x >> 4) & 0x07)
#define Format2short(x)  ((x >> 7) & 0x0f)
#define Format2gen(x)    ((x >> 11) & 0x1f)

#define Format3i(x)      ((x >> 0) & 0x03)
#define Format3op(x)     ((x >> 7) & 0x0f)
#define Format3gen(x)    ((x >> 11) & 0x1f)

#define Format4i(x)      ((x >> 0) & 0x03)
#define Format4op(x)     ((x >> 2) & 0x0f)
#define Format4gen2(x)   ((x >> 6) & 0x1f)
#define Format4gen1(x)   ((x >> 11) & 0x1f)

#define Format5i(x)      ((x >> 8) & 0x03)
#define Format5op(x)     ((x >> 10) & 0x07)
#define Format5short(x)  ((x >> 15) & 0x0f)

#define Format6i(x)      ((x >> 8) & 0x03)
#define Format6op(x)     ((x >> 10) & 0x0f)
#define Format6gen2(x)   ((x >> 14) & 0x1f)
#define Format6gen1(x)   ((x >> 19) & 0x1f)

#define Format7i(x)      ((x >> 8) & 0x03)
#define Format7op(x)     ((x >> 10) & 0x0f)
#define Format7gen2(x)   ((x >> 14) & 0x1f)
#define Format7gen1(x)   ((x >> 19) & 0x1f)

#define Format8i(x)      ((x >> 8) & 0x03)
#define Format8op(x)     (((x >> 8) & 0x04) | ((x >> 6) & 0x03))
#define Format8reg(x)    ((x >> 11) & 0x07)
#define Format8gen2(x)   ((x >> 14) & 0x1f)
#define Format8gen1(x)   ((x >> 19) & 0x1f)

#define Format9i(x)      ((x >> 8) & 0x03)
#define Format9f(x)      ((x >> 10) & 0x01)
#define Format9op(x)     ((x >> 11) & 0x07)
#define Format9gen2(x)   ((x >> 14) & 0x1f)
#define Format9gen1(x)   ((x >> 19) & 0x1f)

#define Format11f(x)     ((x >> 8) & 0x01)
#define Format11op(x)    ((x >> 10) & 0x0f)
#define Format11gen2(x)  ((x >> 14) & 0x1f)
#define Format11gen1(x)  ((x >> 19) & 0x1f)

#define Format14i(x)     ((x >> 8) & 0x01)
#define Format14op(x)    ((x >> 10) & 0x0f)
#define Format14short(x) ((x >> 15) & 0x1f)
#define Format14gen1(x)  ((x >> 19) & 0x1f)

// instructions
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format0_op[1] =
{
	{ "Bcond",         DISP,             0,                0,                0,     0 }
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format1_op[16] =
{
	{ "BSR",           DISP,             0,                0,                0,     STEP_OVER },
	{ "RET",           DISP,             0,                0,                0,     STEP_OUT },
	{ "CXP",           DISP,             0,                0,                0,     STEP_OVER },
	{ "RXP",           DISP,             0,                0,                0,     STEP_OUT },
	{ "RETT",          0,                0,                0,                0,     STEP_OUT },
	{ "RETI",          DISP,             0,                0,                0,     STEP_OUT },
	{ "SAVE",          IMM,              0,                0,                0,     0 },
	{ "RESTORE",       IMM,              0,                0,                0,     0 },
	{ "ENTER",         IMM,              DISP,             0,                0,     0 },
	{ "EXIT",          IMM,              0,                0,                0,     0 },
	{ "NOP",           0,                0,                0,                0,     0 },
	{ "WAIT",          0,                0,                0,                0,     0 },
	{ "DIA",           0,                0,                0,                0,     0 },
	{ "FLAG",          0,                0,                0,                0,     0 },
	{ "SVC",           0,                0,                0,                0,     0 },
	{ "BPT",           0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format2_op[8] =
{
	{ "ADDQi",         QUICK,            GEN | RMW   | I,  0,                0,     0 },
	{ "CMPQi",         QUICK,            GEN | READ  | I,  0,                0,     0 },
	{ "SPRi",          SHORT,            GEN | WRITE | I,  0,                0,     0 },
	{ "Scondi",        GEN | WRITE | I,  0,                0,                0,     0 },
	{ "ACBi",          QUICK,            GEN | RMW   | I,  DISP,             0,     0 },
	{ "MOVQi",         QUICK,            GEN | WRITE | I,  0,                0,     0 },
	{ "LPRi",          SHORT,            GEN | READ  | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format3_op[16] =
{
	{ "CXPD",          GEN | ADDR,       0,                0,                0,     STEP_OVER },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "BICPSRi",       GEN | READ  | I,  0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "JUMP",          GEN | ADDR,       0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "BISPSRB",       GEN | READ | B,   0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "ADJSPi",        GEN | READ  | I,  0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "JSR",           GEN | ADDR,       0,                0,                0,     STEP_OVER },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "CASEi",         GEN | READ  | I,  0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format4_op[16] =
{
	{ "ADDi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "CMPi",          GEN | READ  | I,  GEN | READ  | I,  0,                0,     0 },
	{ "BICi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "ADDCi",         GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "MOVi",          GEN | READ  | I,  GEN | WRITE | I,  0,                0,     0 },
	{ "ORi",           GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "SUBi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "ADDR",          GEN | ADDR,       GEN | WRITE | D,  0,                0,     0 },
	{ "ANDi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "SUBCi",         GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "TBITi",         GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "XORi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format5_op[16] =
{
	{ "MOVSi",         OPTIONS,          0,                0,                0,     0 },
	{ "CMPSi",         OPTIONS,          0,                0,                0,     0 },
	{ "SETCFG",        SHORT,            0,                0,                0,     0 },
	{ "SKPSi",         OPTIONS,          0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format6_op[16] =
{
	{ "ROTi",          GEN | READ  | B,  GEN | RMW   | I,  0,                0,     0 },
	{ "ASHi",          GEN | READ  | B,  GEN | RMW   | I,  0,                0,     0 },
	{ "CBITi",         GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "CBITIi",        GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "LSHi",          GEN | READ  | B,  GEN | RMW   | I,  0,                0,     0 },
	{ "SBITi",         GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "SBITIi",        GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "NEGi",          GEN | READ  | I,  GEN | WRITE | I,  0,                0,     0 },
	{ "NOTi",          GEN | READ  | I,  GEN | WRITE | I,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "SUBPi",         GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "ABSi",          GEN | READ  | I,  GEN | READ  | I,  0,                0,     0 },
	{ "COM",           GEN | READ  | I,  GEN | WRITE | I,  0,                0,     0 },
	{ "IBITi",         GEN | READ  | I,  GEN | REGADDR,    0,                0,     0 },
	{ "ADDPi",         GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format7_op[16] =
{
	{ "MOVMi",         GEN | ADDR,       GEN | ADDR,       DISP,             0,     0 },
	{ "CMPMi",         GEN | ADDR,       GEN | ADDR,       DISP,             0,     0 },
	{ "INSSi",         GEN | READ  | I,  GEN | REGADDR,    IMM,              0,     0 },
	{ "EXTSi",         GEN | REGADDR,    GEN | WRITE | I,  IMM,              0,     0 },
	{ "MOVXBW",        GEN | READ  | B,  GEN | WRITE | W,  0,                0,     0 },
	{ "MOVZBW",        GEN | READ  | B,  GEN | WRITE | W,  0,                0,     0 },
	{ "MOVZiD",        GEN | READ  | I,  GEN | WRITE | D,  0,                0,     0 },
	{ "MOVXiD",        GEN | READ  | I,  GEN | WRITE | D,  0,                0,     0 },
	{ "MULi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "MEIi",          GEN | READ  | I,  GEN | RMW   | I2, 0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "DEIi",          GEN | READ  | I,  GEN | RMW   | I2, 0,                0,     0 },
	{ "QUOi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "REMi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "MODi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
	{ "DIVi",          GEN | READ  | I,  GEN | RMW   | I,  0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format8_op[] =
{
	{ "EXTi",          REG,              GEN | REGADDR,    GEN | WRITE | I,  DISP,  0 },
	{ "CVTP",          REG,              GEN | ADDR,       GEN | WRITE | D,  0,     0 },
	{ "INSi",          REG,              GEN | READ  | I,  GEN | REGADDR,    DISP,  0 },
	{ "CHECKi",        REG,              GEN | ADDR,       GEN | READ  | I,  0,     0 },
	{ "INDEXi",        REG,              GEN | READ  | I,  GEN | READ  | I,  0,     0 },
	{ "FFSi",          GEN | READ  | I,  GEN | RMW   | B,  0,                0,     0 },
	{ "MOVSUi",        GEN | ADDR,       GEN | ADDR,       0,                0,     0 },
	{ "MOVUSi",        GEN | ADDR,       GEN | ADDR,       0,                0,     0 }
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format9_op[] =
{
	{ "MOVif",         GEN | READ  | I,  GEN | WRITE | F,  0,                0,     0 },
	{ "LFSR",          GEN | READ  | D,  0,                0,                0,     0 },
	{ "MOVLF",         GEN | READ  | L,  GEN | WRITE | F,  0,                0,     0 },
	{ "MOVFL",         GEN | READ  | F,  GEN | WRITE | L,  0,                0,     0 },
	{ "ROUNDfi",       GEN | READ  | F,  GEN | WRITE | I,  0,                0,     0 },
	{ "TRUNCfi",       GEN | READ  | F,  GEN | WRITE | I,  0,                0,     0 },
	{ "SFSR",          GEN | WRITE | D,  0,                0,                0,     0 },
	{ "FLOORfi",       GEN | READ  | F,  GEN | WRITE | I,  0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format11_op[] =
{
	{ "ADDf",          GEN | READ  | F,  GEN | RMW   | F,  0,                0,     0 },
	{ "MOVf",          GEN | READ  | F,  GEN | WRITE | F,  0,                0,     0 },
	{ "CMPf",          GEN | READ  | F,  GEN | READ  | F,  0,                0,     0 }, // POLYf
	{ "Trap (SLAVE)",  0,                0,                0,                0,     0 }, // DOTf
	{ "SUBf",          GEN | READ  | F,  GEN | RMW   | F,  0,                0,     0 }, // SCALBf
	{ "NEGf",          GEN | READ  | F,  GEN | WRITE | F,  0,                0,     0 }, // LOGBf
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "DIVf",          GEN | READ  | F,  GEN | RMW   | F,  0,                0,     0 },
	{ "Trap (SLAVE)",  0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "MULf",          GEN | READ  | F,  GEN | RMW   | F,  0,                0,     0 },
	{ "ABSf",          GEN | READ  | F,  GEN | READ  | F,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};
const ns32000_disassembler::NS32000_OPCODE ns32000_disassembler::format14_op[] =
{
	{ "RDVAL",         GEN | ADDR,       0,                0,                0,     0 },
	{ "WRVAL",         GEN | ADDR,       0,                0,                0,     0 },
	{ "LMR",           SHORT,            GEN | READ  | D,  0,                0,     0 },
	{ "SMR",           SHORT,            GEN | WRITE | D,  0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 }, // CINV
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
	{ "Trap (UND)",    0,                0,                0,                0,     0 },
};


const char *const ns32000_disassembler::Format0[] =
{
	"Bcond"
};
const char *const ns32000_disassembler::Format1[] =
{
	"BSR", "RET", "CXP", "RXP", "RETT", "RETI", "SAVE", "RESTORE", "ENTER", "EXIT", "NOP", "WAIT", "DIA", "FLAG", "SVC", "BPT"
};
const char *const ns32000_disassembler::Format2[] =
{
	"ADDQi", "CMPQi", "SPRi", "Scondi", "ACBi", "MOVQi", "LPRi"
};
const char *const ns32000_disassembler::Format3[] =
{
	"CXPD", "Trap (UND)", "BICPSR", "Trap (UND)", "JUMP", "Trap (UND)", "BISPSR", "Trap (UND)", "Trap (UND)", "Trap (UND)", "ADJSPi", "Trap (UND)", "JSR", "Trap (UND)", "CASEi", "Trap (UND)"
};
const char *const ns32000_disassembler::Format4[] =
{
	"ADDi", "CMPi", "BICi", "Trap (UND)", "ADDCi", "MOVi", "ORi", "Trap (UND)", "SUBi", "ADDR", "ANDi", "Trap (UND)", "SUBCi", "TBITi", "XORi", "Trap (UND)"
};
const char *const ns32000_disassembler::Format5[] =
{
	"MOVSi", "CMPSi", "SETCFG", "SKPSi", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)"
};
const char *const ns32000_disassembler::Format6[] =
{
	"ROTi", "ASHi", "CBITi", "CBITIi", "Trap (UND)", "LSHi", "SBITi", "SBITIi", "NEGi", "NOTi", "Trap (UND)", "SUBPi", "ABSi", "COM", "IBITi", "ADDPi"
};
const char *const ns32000_disassembler::Format7[] =
{
	"MOVMi", "CMPMi", "INSSi", "EXTSi", "MOVXBW", "MOVZBW", "MOVZiD", "MOVXiD", "MULi", "MEIi", "Trap (UND)", "DEIi", "QUOi", "REMi", "MODi", "DIVi"
};
const char *const ns32000_disassembler::Format8[] =
{
	"EXTi", "CVTP", "INSi", "CHECKi", "INDEXi", "FFSi", "MOVSUi", "MOVUSi"
};
const char *const ns32000_disassembler::Format9[] =
{
	"MOVif", "LFSR", "MOVLF", "MOVFL", "ROUNDfi", "TRUNCfi", "SFSR", "FLOORfi"
};
const char *const ns32000_disassembler::Format11[] =
{
	"ADDf", "MOVf", "CMPf", "Trap (SLAVE)", "SUBf", "NEGf", "Trap (UND)", "Trap (UND)", "DIVf", "Trap (SLAVE)", "Trap (UND)", "Trap (UND)", "MULf", "ABSf", "Trap (UND)", "Trap (UND)"
};
const char *const ns32000_disassembler::Format14[] =
{
	"RDVAL", "WRVAL", "LMR", "SMR", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)", "Trap (UND)"
};

// types
const char *const ns32000_disassembler::iType[] =
{
	"B", "W", " ", "D"
};
const char *const ns32000_disassembler::fType[] =
{
	"L", "F"
};
const char *const ns32000_disassembler::cType[] =
{
	"Q", "D"
};

// index byte sizes
const char *const ns32000_disassembler::indexSize[] =
{
	"B", "W", "D", "Q"
};

// short codes
const char *const ns32000_disassembler::cond[] =
{
	"EQ", "NE", "CS", "CC", "HI", "LS", "GT", "LE", "FS", "FC", "LO", "HS", "LT", "GE", "R", "N"
};
const char *const ns32000_disassembler::areg[] =
{
	"US", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "FP", "SP", "SB", "(reserved)", "(reserved)", "PSR", "INTBASE", "MOD"
};
const char *const ns32000_disassembler::mreg[] =
{
	"BPR0", "BPR1", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "(reserved)", "MSR", "BCNT", "PTB0", "PTB1", "(reserved)", "EIA"
};

// register names
const char *const ns32000_disassembler::R[] =
{
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"
};
const char *const ns32000_disassembler::M[] =
{
	"FP", "SP", "SB"
};
const char *const ns32000_disassembler::PR[] =
{
	"UPSR", "DCR", "BPC", "DSR", "CAR", "", "", "", "FP", "SP", "SB", "USP", "CFG", "PSR", "INTBASE", "MOD"
};

int8_t ns32000_disassembler::short2int(uint8_t val)
{
	return (val & 0x08) ? val | 0xf0 : val;
}

std::string ns32000_disassembler::mnemonic_index(std::string form, std::string itype, std::string ftype)
{
	if (itype.size() && form.find('i') != std::string::npos)
		form.replace(form.find('i'), 1, itype);
	if (ftype.size() && form.find('f') != std::string::npos)
		form.replace(form.find('f'), 1, ftype);
	return form;
}

uint8_t ns32000_disassembler::opcode_format(uint8_t byte)
{
	switch (byte & 0x0f)
	{
	case 0x0a: return 0;
	case 0x02: return 1;
	case 0x0c:
	case 0x0d:
	case 0x0f:
		if ((byte & 0x70) != 0x70)
			return 2;
		else
			return 3;
	}

	if ((byte & 0x03) != 0x02) return 4;

	switch (byte)
	{
	case 0x0e: return 5;
	case 0x4e: return 6;
	case 0xce: return 7;
	case 0x2e:
	case 0x6e:
	case 0xae:
	case 0xee: return 8;
	case 0x3e: return 9;
	case 0x7e: return 10;
	case 0xbe: return 11;
	case 0xfe: return 12;
	case 0x9e: return 13;
	case 0x1e: return 14;
	}

	return 99; /* unknown format */
}

inline std::string ns32000_disassembler::get_option_list(uint8_t cfg)
{
	std::string option_list;

	option_list.append("[");
	if (BIT(cfg, 0)) option_list.append("I,");
	if (BIT(cfg, 1)) option_list.append("F,");
	if (BIT(cfg, 2)) option_list.append("M,");
	if (BIT(cfg, 3)) option_list.append("C,");
	if (option_list.back() == ',') option_list.pop_back();
	option_list.append("]");

	return option_list;
}

inline std::string ns32000_disassembler::get_options(uint8_t opts)
{
	std::string options;

	options.append(" ");
	if ((opts & 0x02) == 0x02) options.append("B,");
	if ((opts & 0x04) == 0x04) options.append("W,");
	if ((opts & 0x0c) == 0x0c) options.append("U,");
	if (options.back() == ',') options.pop_back();

	return options;
}

inline int32_t ns32000_disassembler::get_disp(offs_t &pc, const data_buffer &opcodes)
{
	/* displacement can be upto 3 bytes */
	uint32_t disp = bitswap<32>(opcodes.r32(pc), 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24);

	switch ((disp >> 29) & 0x07)
	{
	case 0: case 1: /* 7 bit positive */
		disp = (disp >> 24);
		pc += 1;
		break;

	case 2: case 3: /* 7 bit negative */
		disp = (disp >> 24) | 0xffffff80;
		pc += 1;
		break;

	case 4: /* 14 bit positive */
		disp = (disp >> 16) & 0x3fff;
		pc += 2;
		break;

	case 5: /* 14 bit negative */
		disp = (disp >> 16) | 0xffffc000;
		pc += 2;
		break;

	case 6: /* 30 bit positive */
		disp = disp & 0x3fffffff;
		pc += 4;
		break;

	case 7: /* 30 bit negative */
		pc += 4;
		break;
	}

	return disp;
}

inline std::string ns32000_disassembler::get_reg_list(offs_t &pc, const data_buffer &opcodes, bool reverse)
{
	std::string reg_list;

	uint8_t byte = opcodes.r8(pc++);

	reg_list.append("[");
	for (int i = 0; i < 8; i++)
	{
		if (BIT(byte, i)) reg_list.append(R[reverse ? (~i & 7) : (i & 7)]).append(",");
	}
	if (reg_list.back() == ',') reg_list.pop_back();
	reg_list.append("]");

	return reg_list;
}

void ns32000_disassembler::stream_gen(std::ostream &stream, u8 gen_addr, u8 op_len, offs_t &pc, const data_buffer &opcodes)
{
	uint8_t index_byte;
	int32_t disp1, disp2;

	switch (gen_addr)
	{
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		/* Register */
		util::stream_format(stream, "%s", R[gen_addr & 0x07]);
		break;
	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		/* Register Relative */
		disp1 = get_disp(pc, opcodes);
		util::stream_format(stream, "%+d(%s)", disp1, R[gen_addr & 0x07]);
		break;
	case 0x10: case 0x11: case 0x12:
		/* Memory Relative */
		disp1 = get_disp(pc, opcodes);
		disp2 = get_disp(pc, opcodes);
		util::stream_format(stream, "#X%02X(#X%02X(%s))", disp2, disp1, M[gen_addr & 0x03]);
		break;
	case 0x13:
		/* Reserved */
		util::stream_format(stream, "(reserved)");
		break;
	case 0x14:
		/* Immediate */
		disp1 = bitswap<32>(opcodes.r32(pc), 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24);
		util::stream_format(stream, "#X%02X", (op_len == 0 ? (disp1 >> 24) : (op_len == 1 ? (disp1 >> 16) : disp1)));
		pc += op_len + 1;
		break;
	case 0x15:
		/* Absolute */
		disp1 = get_disp(pc, opcodes);
		util::stream_format(stream, "@#X%02X", disp1);
		break;
	case 0x16:
		/* External */
		disp1 = get_disp(pc, opcodes);
		disp2 = get_disp(pc, opcodes);
		util::stream_format(stream, "EXT(%d)+%d", disp1, disp2);
		break;
	case 0x17:
		/* Top Of Stack */
		util::stream_format(stream, "TOS");
		break;
	case 0x18: case 0x19: case 0x1a:
		/* Memory Space */
		disp1 = get_disp(pc, opcodes);
		util::stream_format(stream, "#X%02X(%s)", disp1, M[gen_addr & 0x03]);
		break;
	case 0x1b:
		/* Memory Space */
		disp1 = get_disp(pc, opcodes);
		util::stream_format(stream, "#X%06X", m_base_pc + disp1);
		break;
	case 0x1c:
		/* Scaled Index */
		index_byte = opcodes.r8(pc++);
		stream_gen(stream, index_byte >> 3, op_len, pc, opcodes);
		util::stream_format(stream, "[%s:%c]", R[index_byte & 0x07], indexSize[gen_addr & 0x03]);
		break;
	}
}

offs_t ns32000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = SUPPORTED;

	uint32_t opcode;
	std::string mnemonic;
	uint8_t temp8;

	/* opcode can be upto 3 bytes */
	opcode = opcodes.r32(pc);
	m_base_pc = pc;

	switch (opcode_format(opcode))
	{
	case 0: /* Format 0 */
		pc += 1;
		util::stream_format(stream, "%-8s #X%06X", std::string("B").append(cond[Format0cond(opcode)]), m_base_pc + get_disp(pc, opcodes));
		break;

	case 0x01: /* Format 1 */
		pc += 1;
		switch (Format1op(opcode))
		{
		case 0x00:
			util::stream_format(stream, "%-8s #X%06X", Format1[Format1op(opcode)], m_base_pc + get_disp(pc, opcodes));
			break;
		case 0x01:
			util::stream_format(stream, "%-8s #X%02X", Format1[Format1op(opcode)], get_disp(pc, opcodes) & 0xffffff);
			break;
		case 0x02:
			util::stream_format(stream, "%-8s EXT(%d)", Format1[Format1op(opcode)], get_disp(pc, opcodes));
			break;
		case 0x03: case 0x04:
			util::stream_format(stream, "%-8s #X%02X", Format1[Format1op(opcode)], get_disp(pc, opcodes) & 0xffffff);
			break;
		case 0x05:
			util::stream_format(stream, "%-8s", Format1[Format1op(opcode)]);
			break;
		case 0x06:
			util::stream_format(stream, "%-8s %s", Format1[Format1op(opcode)], get_reg_list(pc, opcodes, false));
			break;
		case 0x07: case 0x09:
			util::stream_format(stream, "%-8s %s", Format1[Format1op(opcode)], get_reg_list(pc, opcodes, true));
			break;
		case 0x08:
			util::stream_format(stream, "%-8s %s, %d", Format1[Format1op(opcode)], get_reg_list(pc, opcodes, false), get_disp(pc, opcodes));
			break;
		case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "%-8s", Format1[Format1op(opcode)]);
			break;
		}
		break;

	case 0x02: /* Format 2 */
		pc += 2;
		mnemonic = mnemonic_index(Format2[Format2op(opcode)], iType[Format2i(opcode)], "");
		switch (Format2op(opcode))
		{
		case 0x00: case 0x01: case 0x05:
			util::stream_format(stream, "%-8s %+d, ", mnemonic, short2int(Format2short(opcode)));
			stream_gen(stream, Format2gen(opcode), Format2i(opcode), pc, opcodes);
			break;
		case 0x02: case 0x06:
			util::stream_format(stream, "%-8s %s, ", mnemonic, areg[Format2short(opcode)]);
			stream_gen(stream, Format2gen(opcode), Format2i(opcode), pc, opcodes);
			break;
		case 0x03:
			util::stream_format(stream, "%-8s ", std::string("S").append(cond[Format2short(opcode)]).append(iType[Format2i(opcode)]));
			stream_gen(stream, Format2gen(opcode), Format2i(opcode), pc, opcodes);
			break;
		case 0x04:
			util::stream_format(stream, "%-8s %+d, ", mnemonic, short2int(Format2short(opcode)));
			stream_gen(stream, Format2gen(opcode), Format2i(opcode), pc, opcodes);
			util::stream_format(stream, ", #X%06x", m_base_pc + get_disp(pc, opcodes));
			break;
		}
		break;

	case 0x03: /* Format 3 */
		pc += 2;
		mnemonic = mnemonic_index(Format3[Format3op(opcode)], iType[Format3i(opcode)], "");
		switch (Format3op(opcode))
		{
		case 0x00: case 0x02: case 0x04: case 0x06: case 0x0a: case 0x0c: case 0x0e:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format3gen(opcode), Format3i(opcode), pc, opcodes);
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	case 0x04: /* Format 4 */
		pc += 2;
		mnemonic = mnemonic_index(Format4[Format4op(opcode)], iType[Format4i(opcode)], "");
		util::stream_format(stream, "%-8s ", mnemonic);
		stream_gen(stream, Format4gen1(opcode), Format4i(opcode), pc, opcodes);
		util::stream_format(stream, ", ");
		stream_gen(stream, Format4gen2(opcode), Format4i(opcode), pc, opcodes);
		break;

	case 0x05: /* Format 5 */
		pc += 3;
		mnemonic = mnemonic_index(Format5[Format5op(opcode)], iType[Format5i(opcode)], "");
		switch ((opcode >> 10) & 0x0f)
		{
		case 0x00: case 0x01: case 0x03:
			if (Format5short(opcode) & 0x01)
				util::stream_format(stream, "%-8s %s", std::string(Format5[Format5op(opcode)]).append("T"), get_options(Format5short(opcode)));
			else
				util::stream_format(stream, "%-8s %s", mnemonic, get_options(Format5short(opcode)));
			break;
		case 0x02:
			util::stream_format(stream, "%-8s %s", Format5[Format5op(opcode)], get_option_list(Format5short(opcode)));
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	case 0x06: /* Format 6 */
		pc += 3;
		mnemonic = mnemonic_index(Format6[Format6op(opcode)], iType[Format6i(opcode)], "");
		switch ((opcode >> 10) & 0x0f)
		{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format6gen1(opcode), Format6i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format6gen2(opcode), Format6i(opcode), pc, opcodes);
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	case 0x07: /* Format 7 */
		pc += 3;
		mnemonic = mnemonic_index(Format7[Format7op(opcode)], iType[Format7i(opcode)], "");
		switch (Format7op(opcode))
		{
		case 0x00: case 0x01:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format7gen1(opcode), Format7i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format7gen2(opcode), Format7i(opcode), pc, opcodes);
			util::stream_format(stream, ", %d", get_disp(pc, opcodes));
			break;
		case 0x02: case 0x03:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format7gen1(opcode), Format7i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format7gen2(opcode), Format7i(opcode), pc, opcodes);
			temp8 = opcodes.r8(pc++);
			util::stream_format(stream, ", %d, %d", temp8 >> 5, temp8 + 1);
			break;
		case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format7gen1(opcode), Format7i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format7gen2(opcode), Format7i(opcode), pc, opcodes);
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	case 0x08: /* Format 8 */
		pc += 3;
		mnemonic = mnemonic_index(Format8[Format8op(opcode)], iType[Format8i(opcode)], "");
		switch (Format8op(opcode))
		{
		case 0x00: case 0x02:
			util::stream_format(stream, "%-8s %s, ", mnemonic, R[Format8reg(opcode)]);
			stream_gen(stream, Format8gen2(opcode), Format8i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format8gen1(opcode), Format8i(opcode), pc, opcodes);
			util::stream_format(stream, ", %d", get_disp(pc, opcodes));
			break;
		case 0x01: case 0x03: case 0x04:
			util::stream_format(stream, "%-8s %s, ", mnemonic, R[Format8reg(opcode)]);
			stream_gen(stream, Format8gen2(opcode), Format8i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format8gen1(opcode), Format8i(opcode), pc, opcodes);
			break;
		case 0x05:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format8gen2(opcode), Format8i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format8gen1(opcode), Format8i(opcode), pc, opcodes);
			break;
		case 0x06:
			if (Format8reg(opcode) == 0x01)
				util::stream_format(stream, "%-8s ", std::string(Format8[Format8op(opcode)]).append(iType[Format8i(opcode)]));
			else
				util::stream_format(stream, "%-8s ", std::string(Format8[Format8op(opcode) + 1]).append(iType[Format8i(opcode)]));
			stream_gen(stream, Format8gen2(opcode), Format8i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format8gen1(opcode), Format8i(opcode), pc, opcodes);
			break;
		}
		break;

	case 0x09: /* Format 9 */
		pc += 3;
		mnemonic = mnemonic_index(Format9[Format9op(opcode)], iType[Format9i(opcode)], fType[Format9f(opcode)]);
		switch (Format9op(opcode))
		{
		case 0x00: case 0x02: case 0x04: case 0x05: case 0x07:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format9gen1(opcode), Format9i(opcode), pc, opcodes);
			util::stream_format(stream, ", ");
			stream_gen(stream, Format9gen2(opcode), Format9i(opcode), pc, opcodes);
			break;
		case 0x01: case 0x06:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format9gen1(opcode), Format9i(opcode), pc, opcodes);
			break;
		}
		break;

	case 0x0b: /* Format 11 */
		pc += 3;
		mnemonic = mnemonic_index(Format11[Format11op(opcode)], "", fType[Format11f(opcode)]);
		switch (Format11op(opcode))
		{
		case 0x00: case 0x01: case 0x02: case 0x04: case 0x05 : case 0x08: case 0x0c : case 0x0d:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format11gen1(opcode), Format11f(opcode), pc, opcodes);
			util::stream_format(stream, ",");
			stream_gen(stream, Format11gen2(opcode), Format11f(opcode), pc, opcodes);
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	case 0x0a: /* Format 10 */
	case 0x0c: /* Format 12 */
	case 0x0d: /* Format 13 */
		pc += 3;
		util::stream_format(stream, "Trap (UND)");
		break;

	case 0x0e: /* Format 14 */
		pc += 3;
		mnemonic = mnemonic_index(Format14[Format14op(opcode)], iType[Format14i(opcode)], "");
		switch (Format14op(opcode))
		{
		case 0x00: case 0x01:
			util::stream_format(stream, "%-8s ", mnemonic);
			stream_gen(stream, Format14gen1(opcode), Format14i(opcode), pc, opcodes);
			break;
		case 0x02: case 0x03:
			util::stream_format(stream, "%-8s %s, ", mnemonic, mreg[Format14short(opcode)]);
			stream_gen(stream, Format14gen1(opcode), Format14i(opcode), pc, opcodes);
			break;
		default: /* Trap */
			util::stream_format(stream, "%-8s ", mnemonic);
			break;
		}
		break;

	default:
		pc += 1;
		util::stream_format(stream, "unknown instruction format");
		break;
	}

	return (pc - m_base_pc) | flags;
}
