// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "tms1k_base.h"


enum e_mnemonics
{
	zILL = 0,
	zA10AAC, zA6AAC, zA8AAC, zAC1AC, zACACC, zACNAA, zALEC, zALEM, zAMAAC, zBRANCH, zCALL, zCCLA,
	zCLA, zCLO, zCOMC, zCOMX, zCOMX8, zCPAIZ, zCTMDYN, zDAN, zDMAN, zDMEA, zDNAA,
	zDYN, zIA, zIMAC, zIYC, zKNEZ, zLDP, zLDX2, zLDX3, zLDX4, zMNEA, zMNEZ,
	zNDMEA, zOFF, zRBIT, zREAC, zRETN, zRSTR, zSAL, zSAMAN, zSBIT,
	zSBL, zSEAC, zSETR, zTAM, zTAMACS, zTAMDYN, zTAMIY, zTAMIYC, zTAMZA,
	zTAY, zTBIT, zTCMIY, zTCY, zTDO, zTKA, zTKM, zTMA,
	zTMY, zTYA, zXDA, zXMA, zYMCY, zYNEA, zYNEC
};

static const char *const s_mnemonic[] =
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


#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, _OUT, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
};


enum e_addressing
{
	zB0 = 0, zI2, zI3, zI4, zB7
};

static const UINT8 s_addressing[] =
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

static const UINT8 tms1000_mnemonic[256] =
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


static const UINT8 tms1100_mnemonic[256] =
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


static const UINT8 tms0980_mnemonic[512] =
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


static const UINT8 tp0320_mnemonic[512] =
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

static const UINT8 i2_value[4] =
{
	0, 2, 1, 3
};

static const UINT8 i3_value[8] =
{
	0, 4, 2, 6, 1, 5, 3, 7
};

static const UINT8 i4_value[16] =
{
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

static offs_t tms1k_dasm(char *dst, const UINT8 *oprom, const UINT8 *lut_mnemonic, UINT16 opcode_mask)
{
	// get current opcode
	int pos = 0;
	UINT16 op = oprom[pos++];
	if (opcode_mask & 0x100)
		op = (op << 8 | oprom[pos++]) & 0x1ff;

	// convert to mnemonic/param
	UINT16 instr = lut_mnemonic[op];
	dst += sprintf(dst, "%-8s ", s_mnemonic[instr]);

	switch( s_addressing[instr] )
	{
		case zI2:
			dst += sprintf(dst, "%d", i2_value[op & 0x03]);
			break;
		case zI3:
			dst += sprintf(dst, "%d", i3_value[op & 0x07]);
			break;
		case zI4:
			dst += sprintf(dst, "%d", i4_value[op & 0x0f]);
			break;
		case zB7:
			if (opcode_mask & 0x100)
				dst += sprintf(dst, "$%02X", op << 1 & 0xfe);
			else
				dst += sprintf(dst, "$%02X", op & 0x3f);
			break;
		default:
			break;
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE(tms1000)
{
	return tms1k_dasm(buffer, oprom, tms1000_mnemonic, 0xff);
}

CPU_DISASSEMBLE(tms1100)
{
	return tms1k_dasm(buffer, oprom, tms1100_mnemonic, 0xff);
}

CPU_DISASSEMBLE(tms0980)
{
	return tms1k_dasm(buffer, oprom, tms0980_mnemonic, 0x1ff);
}

CPU_DISASSEMBLE(tp0320)
{
	return tms1k_dasm(buffer, oprom, tp0320_mnemonic, 0x1ff);
}
