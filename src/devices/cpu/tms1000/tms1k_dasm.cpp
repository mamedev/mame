// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family disassembler

*/

#include "emu.h"
#include "tms1k_dasm.h"

// constructor

tms1000_base_disassembler::tms1000_base_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits) :
	m_lut_mnemonic(lut_mnemonic),
	m_opcode_9bits(opcode_9bits),
	m_pc_bits(pc_bits)
{
	// init lfsr pc lut
	const u32 len = 1 << pc_bits;
	m_l2r = std::make_unique<u8[]>(len);
	m_r2l = std::make_unique<u8[]>(len);

	for (u32 i = 0, pc = 0; i < len; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;

		// see tms1k_base_device::next_pc()
		u32 mask = (1 << pc_bits) - 1;
		u32 high = 1 << (pc_bits - 1);
		u32 fb = (pc << 1 & high) == (pc & high);

		if (pc == (mask >> 1))
			fb = 1;
		else if (pc == mask)
			fb = 0;

		pc = (pc << 1 | fb) & mask;
	}
}

tms1000_disassembler::tms1000_disassembler() : tms1000_base_disassembler(tms1000_mnemonic, false, 6)
{ }

tms1100_disassembler::tms1100_disassembler() : tms1000_base_disassembler(tms1100_mnemonic, false, 6)
{ }

tms0980_disassembler::tms0980_disassembler() : tms1000_base_disassembler(tms0980_mnemonic, true, 7)
{ }

tp0320_disassembler::tp0320_disassembler() : tms1000_base_disassembler(tp0320_mnemonic, true, 7)
{ }


offs_t tms1000_base_disassembler::pc_linear_to_real(offs_t pc) const
{
	const u32 mask = (1 << m_pc_bits) - 1;
	return (pc & ~mask) | m_l2r[pc & mask];
}

offs_t tms1000_base_disassembler::pc_real_to_linear(offs_t pc) const
{
	const u32 mask = (1 << m_pc_bits) - 1;
	return (pc & ~mask) | m_r2l[pc & mask];
}


// common lookup tables

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
	util::stream_format(stream, "%-8s", s_mnemonic[instr]);

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
