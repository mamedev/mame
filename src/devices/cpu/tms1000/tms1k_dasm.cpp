// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family disassembler

*/

#include "emu.h"
#include "tms1k_dasm.h"

const char *const tms1000_base_disassembler::s_mnemonic[] =
{
	"?",
	"A10AAC", "A6AAC", "A8AAC", "AC1AC", "ACACC", "ACNAA", "ALEC", "ALEM", "AMAAC", "BRANCH", "CALL", "CCLA",
	"CLA", "CLO", "COMC", "COMX", "COMX8", "CPAIZ", "CTMDYN", "DAN", "DMAN", "DMEA", "DNAA",
	"DYN", "IA", "IMAC", "IYC", "KNEZ", "LDP", "LDX", "LDX", "LDX", "MNEA", "MNEZ",
	"NDMEA", "OFF", "RBIT", "REAC", "RETN", "RSTR", "SAL", "SAMAN", "SBIT",
	"SBL", "SEAC", "SETR", "TAM", "TAMACS", "TAMDYN", "TAMIY", "TAMIYC", "TAMZA",
	"TAY", "TBIT", "TCMIY", "TCY", "TDO", "TKA", "TKM", "TMA",
	"TMY", "TYA", "XDA", "XMA", "YMCY", "YNEA", "YNEC"
};

const u32 tms1000_base_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STEP_OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, STEP_OUT, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
};

const u8 tms1000_base_disassembler::s_addressing[] =
{
	zB0,
	zB0, zB0, zB0, zI4, zI4, zI4, zI4, zB0, zB0, zB7, zB7, zB0,
	zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0, zB0,
	zB0, zB0, zB0, zB0, zB0, zI4, zI2, zI3, zI4, zB0, zB0,
	zB0, zB0, zI2, zB0, zB0, zB0, zB0, zB0, zI2,
	zB0, zB0, zB0, zB0, zI4, zB0, zB0, zB0, zB0,
	zB0, zI2, zI4, zI4, zB0, zB0, zB0, zB0,
	zB0, zB0, zB0, zB0, zI4, zB0, zI4
};



// opcode luts

const u8 tms1000_disassembler::tms1000_mnemonic[256] =
{
/* 0x00 */
	zCOMX,   zA8AAC,  zYNEA,   zTAM,    zTAMZA,  zA10AAC, zA6AAC,  zDAN,    zTKA,    zKNEZ,   zTDO,    zCLO,    zRSTR,   zSETR,   zIA,     zRETN,   // 0
	zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    // 1
	zTAMIY,  zTMA,    zTMY,    zTYA,    zTAY,    zAMAAC,  zMNEZ,   zSAMAN,  zIMAC,   zALEM,   zDMAN,   zIYC,    zDYN,    zCPAIZ,  zXMA,    zCLA,    // 2
	zSBIT,   zSBIT,   zSBIT,   zSBIT,   zRBIT,   zRBIT,   zRBIT,   zRBIT,   zTBIT,   zTBIT,   zTBIT,   zTBIT,   zLDX2,   zLDX2,   zLDX2,   zLDX2,   // 3
	zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    // 4
	zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   // 5
	zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  // 6
	zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   // 7
/* 0x80 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 8
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 9
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // A
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // B
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // C
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // D
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // E
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL    // F
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
};


const u8 tms1100_disassembler::tms1100_mnemonic[256] =
{
/* 0x00 */
	zMNEA,   zALEM,   zYNEA,   zXMA,    zDYN,    zIYC,    zAMAAC,  zDMAN,   zTKA,    zCOMX,   zTDO,    zCOMC,   zRSTR,   zSETR,   zKNEZ,   zRETN,   // 0
	zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    // 1
	zTAY,    zTMA,    zTMY,    zTYA,    zTAMDYN, zTAMIYC, zTAMZA,  zTAM,    zLDX3,   zLDX3,   zLDX3,   zLDX3,   zLDX3,   zLDX3,   zLDX3,   zLDX3,   // 2
	zSBIT,   zSBIT,   zSBIT,   zSBIT,   zRBIT,   zRBIT,   zRBIT,   zRBIT,   zTBIT,   zTBIT,   zTBIT,   zTBIT,   zSAMAN,  zCPAIZ,  zIMAC,   zMNEZ,   // 3
	zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    // 4
	zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   // 5
	zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  // 6
	zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zAC1AC,  zCLA,    // 7
/* 0x80 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 8
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 9
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // A
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // B
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // C
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // D
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // E
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL    // F
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
};


const u8 tms0980_disassembler::tms0980_mnemonic[512] =
{
/* 0x000 */
	zCOMX,   zALEM,   zYNEA,   zXMA,    zDYN,    zIYC,    zCLA,    zDMAN,   zTKA,    zMNEA,   zTKM,    0,       0,       zSETR,   zKNEZ,   0,       // 0
	zDMEA,   zDNAA,   zCCLA,   zNDMEA,  0,       zAMAAC,  0,       0,       zCTMDYN, zXDA,    0,       0,       0,       0,       0,       0,       // 1
	zTBIT,   zTBIT,   zTBIT,   zTBIT,   0,       0,       0,       0,       zTAY,    zTMA,    zTMY,    zTYA,    zTAMDYN, zTAMIYC, zTAMZA,  zTAM,    // 2
	zSAMAN,  zCPAIZ,  zIMAC,   zMNEZ,   0,       0,       0,       0,       zTCY,    zYNEC,   zTCMIY,  zACACC,  zACNAA,  zTAMACS, zALEC,   zYMCY,   // 3
	zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    // 4
	zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   // 5
	zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  // 6
	zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  // 7
/* 0x080 */
	zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    // 8
	zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   // 9
	zSBIT,   zSBIT,   zSBIT,   zSBIT,   zRBIT,   zRBIT,   zRBIT,   zRBIT,   0,       0,       0,       0,       0,       0,       0,       0,       // A
	zTDO,    zSAL,    zCOMX8,  zSBL,    zREAC,   zSEAC,   zOFF,    0,       0,       0,       0,       0,       0,       0,       0,       zRETN,   // B
	zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  // C
	zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, // D
	zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   // E
	zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   // F
/* 0x100 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 0
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 1
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 2
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 3
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 4
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 5
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 6
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 7
/* 0x180 */
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // 8
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // 9
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // A
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // B
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // C
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // D
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // E
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL    // F
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
};


const u8 tp0320_disassembler::tp0320_mnemonic[512] =
{
/* 0x000 */
	0,       zALEM,   zYNEA,   zXMA,    zDYN,    zIYC,    zCLA,    zDMAN,   zTKA,    zMNEA,   zTKM,    0,       0,       zSETR,   zKNEZ,   0,       // 0
	zDMEA,   zDNAA,   zCCLA,   zNDMEA,  0,       zAMAAC,  0,       0,       zCTMDYN, zXDA,    0,       0,       0,       0,       0,       0,       // 1
	zTBIT,   zTBIT,   zTBIT,   zTBIT,   0,       0,       0,       0,       zTAY,    zTMA,    zTMY,    zTYA,    zTAMDYN, zTAMIYC, zTAMZA,  zTAM,    // 2
	zSAMAN,  zCPAIZ,  zIMAC,   zMNEZ,   0,       0,       zRSTR,   zYMCY,   zTCY,    zYNEC,   zTCMIY,  zACACC,  zACNAA,  zTAMACS, zALEC,   0,       // 3
	zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    zTCY,    // 4
	zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   zYNEC,   // 5
	zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  zTCMIY,  // 6
	zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  zACACC,  // 7
/* 0x080 */
	zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    zLDP,    // 8
	zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   zLDX4,   // 9
	zSBIT,   zSBIT,   zSBIT,   zSBIT,   zRBIT,   zRBIT,   zRBIT,   zRBIT,   0,       0,       0,       0,       0,       0,       0,       0,       // A
	zTDO,    zSAL,    zCOMX8,  zSBL,    zREAC,   zSEAC,   zOFF,    0,       0,       0,       0,       0,       0,       0,       0,       zRETN,   // B
	zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  zACNAA,  // C
	zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, zTAMACS, // D
	zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   zALEC,   // E
	zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   zYMCY,   // F
/* 0x100 */
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 0
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 1
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 2
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 3
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 4
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 5
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 6
	zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, zBRANCH, // 7
/* 0x180 */
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // 8
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // 9
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // A
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // B
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // C
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // D
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   // E
	zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL,   zCALL    // F
/*  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F  */
};



// disasm

const u8 tms1000_base_disassembler::i2_value[4] =
{
	0, 2, 1, 3
};

const u8 tms1000_base_disassembler::i3_value[8] =
{
	0, 4, 2, 6, 1, 5, 3, 7
};

const u8 tms1000_base_disassembler::i4_value[16] =
{
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

offs_t tms1000_base_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 op = m_opcode_9bits ? opcodes.r16(pc) & 0x1ff : opcodes.r8(pc);

	// convert to mnemonic/param
	u16 instr = m_lut_mnemonic[op];
	util::stream_format(stream, "%-8s ", s_mnemonic[instr]);

	switch( s_addressing[instr] )
	{
		case zI2:
			util::stream_format(stream, "%d", i2_value[op & 0x03]);
			break;
		case zI3:
			util::stream_format(stream, "%d", i3_value[op & 0x07]);
			break;
		case zI4:
			util::stream_format(stream, "%d", i4_value[op & 0x0f]);
			break;
		case zB7:
			util::stream_format(stream, "$%02X", op & (m_opcode_9bits ? 0x7f : 0x3f));
			break;
		default:
			break;
	}

	return 1 | s_flags[instr] | SUPPORTED;
}

tms1000_disassembler::tms1000_disassembler() : tms1000_base_disassembler(tms1000_mnemonic, false, 6)
{
}

tms1100_disassembler::tms1100_disassembler() : tms1000_base_disassembler(tms1100_mnemonic, false, 6)
{
}

tms0980_disassembler::tms0980_disassembler() : tms1000_base_disassembler(tms0980_mnemonic, true, 7)
{
}

tp0320_disassembler::tp0320_disassembler() : tms1000_base_disassembler(tp0320_mnemonic, true, 7)
{
}

tms1000_base_disassembler::tms1000_base_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits) : m_lut_mnemonic(lut_mnemonic), m_opcode_9bits(opcode_9bits), m_pc_bits(pc_bits)
{
}

offs_t tms1000_base_disassembler::pc_linear_to_real(offs_t pc) const
{
	switch(m_pc_bits) {
	case 6: {
		static const u8 l2r6[64] = {
			0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0x1e, 0x3c, 0x39, 0x33,
			0x27, 0x0e, 0x1d, 0x3a, 0x35, 0x2b, 0x16, 0x2c, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0b, 0x17, 0x2e,
			0x1c, 0x38, 0x31, 0x23, 0x06, 0x0d, 0x1b, 0x36, 0x2d, 0x1a, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11,
			0x22, 0x04, 0x09, 0x13, 0x26, 0x0c, 0x19, 0x32, 0x25, 0x0a, 0x15, 0x2a, 0x14, 0x28, 0x10, 0x20,
		};
		return (pc & ~0x3f) | l2r6[pc & 0x3f];
	}
	case 7: {
		static const u8 l2r7[128] = {
			0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0x7e, 0x7d, 0x7b, 0x77, 0x6f, 0x5f, 0x3e, 0x7c,
			0x79, 0x73, 0x67, 0x4f, 0x1e, 0x3d, 0x7a, 0x75, 0x6b, 0x57, 0x2e, 0x5c, 0x38, 0x70, 0x61, 0x43,
			0x06, 0x0d, 0x1b, 0x37, 0x6e, 0x5d, 0x3a, 0x74, 0x69, 0x53, 0x26, 0x4c, 0x18, 0x31, 0x62, 0x45,
			0x0a, 0x15, 0x2b, 0x56, 0x2c, 0x58, 0x30, 0x60, 0x41, 0x02, 0x05, 0x0b, 0x17, 0x2f, 0x5e, 0x3c,
			0x78, 0x71, 0x63, 0x47, 0x0e, 0x1d, 0x3b, 0x76, 0x6d, 0x5b, 0x36, 0x6c, 0x59, 0x32, 0x64, 0x49,
			0x12, 0x25, 0x4a, 0x14, 0x29, 0x52, 0x24, 0x48, 0x10, 0x21, 0x42, 0x04, 0x09, 0x13, 0x27, 0x4e,
			0x1c, 0x39, 0x72, 0x65, 0x4b, 0x16, 0x2d, 0x5a, 0x34, 0x68, 0x51, 0x22, 0x44, 0x08, 0x11, 0x23,
			0x46, 0x0c, 0x19, 0x33, 0x66, 0x4d, 0x1a, 0x35, 0x6a, 0x55, 0x2a, 0x54, 0x28, 0x50, 0x20, 0x40,
		};
		return (pc & ~0x7f) | l2r7[pc & 0x7f];
	}
	}
	return 0;
}

offs_t tms1000_base_disassembler::pc_real_to_linear(offs_t pc) const
{
	switch(m_pc_bits) {
	case 6: {
		static const u8 r2l6[64] = {
			0x00, 0x01, 0x1b, 0x02, 0x31, 0x1c, 0x24, 0x03, 0x2e, 0x32, 0x39, 0x1d, 0x35, 0x25, 0x11, 0x04,
			0x3e, 0x2f, 0x2c, 0x33, 0x3c, 0x3a, 0x16, 0x1e, 0x18, 0x36, 0x29, 0x26, 0x20, 0x12, 0x0c, 0x05,
			0x3f, 0x1a, 0x30, 0x23, 0x2d, 0x38, 0x34, 0x10, 0x3d, 0x2b, 0x3b, 0x15, 0x17, 0x28, 0x1f, 0x0b,
			0x19, 0x22, 0x37, 0x0f, 0x2a, 0x14, 0x27, 0x0a, 0x21, 0x0e, 0x13, 0x09, 0x0d, 0x08, 0x07, 0x06,
		};
		return (pc & ~0x3f) | r2l6[pc & 0x3f];
	}
	case 7: {
		static const u8 r2l7[128] = {
			0x00, 0x01, 0x39, 0x02, 0x5b, 0x3a, 0x20, 0x03, 0x6d, 0x5c, 0x30, 0x3b, 0x71, 0x21, 0x44, 0x04,
			0x58, 0x6e, 0x50, 0x5d, 0x53, 0x31, 0x65, 0x3c, 0x2c, 0x72, 0x76, 0x22, 0x60, 0x45, 0x14, 0x05,
			0x7e, 0x59, 0x6b, 0x6f, 0x56, 0x51, 0x2a, 0x5e, 0x7c, 0x54, 0x7a, 0x32, 0x34, 0x66, 0x1a, 0x3d,
			0x36, 0x2d, 0x4d, 0x73, 0x68, 0x77, 0x4a, 0x23, 0x1c, 0x61, 0x26, 0x46, 0x3f, 0x15, 0x0e, 0x06,
			0x7f, 0x38, 0x5a, 0x1f, 0x6c, 0x2f, 0x70, 0x43, 0x57, 0x4f, 0x52, 0x64, 0x2b, 0x75, 0x5f, 0x13,
			0x7d, 0x6a, 0x55, 0x29, 0x7b, 0x79, 0x33, 0x19, 0x35, 0x4c, 0x67, 0x49, 0x1b, 0x25, 0x3e, 0x0d,
			0x37, 0x1e, 0x2e, 0x42, 0x4e, 0x63, 0x74, 0x12, 0x69, 0x28, 0x78, 0x18, 0x4b, 0x48, 0x24, 0x0c,
			0x1d, 0x41, 0x62, 0x11, 0x27, 0x17, 0x47, 0x0b, 0x40, 0x10, 0x16, 0x0a, 0x0f, 0x09, 0x08, 0x07,
		};
		return (pc & ~0x7f) | r2l7[pc & 0x7f];
	}
	}
	return 0;
}

