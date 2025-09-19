// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Felipe Sanches
/*******************************************************************

Toshiba TLCS-900/H disassembly

*******************************************************************/

#ifndef MAME_CPU_TLCS900_DASM900_H
#define MAME_CPU_TLCS900_DASM900_H

#pragma once

class tlcs900_disassembler : public util::disasm_interface
{
public:
	tlcs900_disassembler() = default;
	virtual ~tlcs900_disassembler() = default;

	tlcs900_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count)
		: m_symbols(symbols), m_symbol_count(symbol_count)
	{
	}
	template<size_t N> tlcs900_disassembler(std::pair<u16, char const *> const (&symbols)[N]) : tlcs900_disassembler(symbols, N) {}

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		M_ADC, M_ADD, M_AND, M_ANDCF, M_BIT, M_BS1B,
		M_BS1F, M_CALL, M_CALR, M_CCF, M_CHG, M_CP,
		M_CPD, M_CPDW, M_CPDR, M_CPDRW, M_CPI, M_CPIR,
		M_CPIRW, M_CPIW, M_CPL, M_DAA, M_DB, M_DEC,
		M_DECF, M_DECW, M_DIV, M_DIVS, M_DJNZ, M_EI,
		M_EX, M_EXTS, M_EXTZ, M_HALT, M_INC, M_INCF,
		M_INCW, M_JP, M_JR, M_JRL, M_LD, M_LDA,
		M_LDC, M_LDCF, M_LDD, M_LDDR, M_LDDRW, M_LDDW,
		M_LDF, M_LDI, M_LDIR, M_LDIRW, M_LDIW, M_LDW,
		M_LDX, M_LINK, M_MAX, M_MDEC1, M_MDEC2, M_MDEC4,
		M_MINC1, M_MINC2, M_MINC4, M_MIRR, M_MUL, M_MULA,
		M_MULS, M_NEG, M_NOP, M_NORMAL, M_OR, M_ORCF,
		M_PAA, M_POP, M_POPW, M_PUSH, M_PUSHW, M_RCF,
		M_RES, M_RET, M_RETD, M_RETI, M_RL, M_RLC,
		M_RLCW, M_RLD, M_RLW, M_RR, M_RRC, M_RRCW,
		M_RRD, M_RRW, M_SBC, M_SCC, M_SCF, M_SET,
		M_SLA, M_SLAW, M_SLL, M_SLLW, M_SRA, M_SRAW,
		M_SRL, M_SRLW, M_STCF, M_SUB, M_SWI, M_TSET,
		M_UNLK, M_XOR, M_XORCF, M_ZCF,
		M_80, M_88, M_90, M_98, M_A0, M_A8, M_B0, M_B8,
		M_C0, oC8, M_D0, oD8, M_E0, M_E8, M_F0
	};

	enum e_operand
	{
		O_NONE,
		O_A,        /* current register set register A */
		O_C8,       /* current register set byte */
		O_C16,      /* current register set word */
		O_C32,      /* current register set long word */
		O_MC16,     /* current register set mul/div register word */
		O_CC,       /* condition */
		O_CR8,      /* byte control register */
		O_CR16,     /* word control register */
		O_CR32,     /* long word control register */
		O_D8,       /* byte displacement */
		O_D16,      /* word displacement */
		O_F,            /* F register */
		O_I3,       /* immediate 3 bit (part of last byte) */
		O_I8,       /* immediate byte */
		O_I16,      /* immediate word */
		O_I24,      /* immediate 3 byte address */
		O_I32,      /* immediate long word */
		O_M,            /* memory location (defined by extension) */
		O_M8,       /* (8) */
		O_M16,      /* (i16) */
		O_R,            /* register */
		O_SR        /* status register */
	};

	struct tlcs900inst
	{
		e_mnemonics mnemonic;
		e_operand   operand1;
		e_operand   operand2;
	};

	static const char *const s_mnemonic[];
	static const tlcs900inst mnemonic_80[256];
	static const tlcs900inst mnemonic_88[256];
	static const tlcs900inst mnemonic_90[256];
	static const tlcs900inst mnemonic_98[256];
	static const tlcs900inst mnemonic_a0[256];
	static const tlcs900inst mnemonic_b0[256];
	static const tlcs900inst mnemonic_b8[256];
	static const tlcs900inst mnemonic_c0[256];
	static const tlcs900inst mnemonic_c8[256];
	static const tlcs900inst mnemonic_d0[256];
	static const tlcs900inst mnemonic_d8[256];
	static const tlcs900inst mnemonic_e0[256];
	static const tlcs900inst mnemonic_e8[256];
	static const tlcs900inst mnemonic_f0[256];
	static const tlcs900inst mnemonic[256];

	static const char *const s_reg8[8];
	static const char *const s_reg16[8];
	static const char *const s_reg32[8];
	static const char *const s_mulreg16[8];
	static const char *const s_allreg8[256];
	static const char *const s_allreg16[256];
	static const char *const s_allreg32[256];
	static const char *const s_cond[16];

	std::pair<u16, char const *> const *m_symbols;
	std::size_t m_symbol_count;

	template <typename T> std::string address(T offset, int size) const;
};


class tmp94c241_disassembler : public tlcs900_disassembler
{
public:
	tmp94c241_disassembler() : tlcs900_disassembler(m_sfr_names) { };

private:
	std::pair<u16, char const *> const m_sfr_names[185] = {
		/* TLCS-900/H2 type 8 bit I/O: */
		{ 0x00, "P0" }, { 0x02, "P0CR" }, { 0x03, "P0FC" },
		{ 0x04, "P1" }, { 0x06, "P1CR" }, { 0x07, "P1FC" },
		{ 0x08, "P2" }, { 0x0a, "P2CR" }, { 0x0b, "P2FC" },
		{ 0x0c, "P3" }, { 0x0e, "P3CR" }, { 0x0f, "P3FC" },
		{ 0x10, "P4" }, { 0x12, "P4CR" }, { 0x13, "P4FC" },
		{ 0x14, "P5" }, { 0x16, "P5CR" }, { 0x17, "P5FC" },
		{ 0x18, "P6" }, { 0x1a, "P6CR" }, { 0x1b, "P6FC" },
		{ 0x1c, "P7" }, { 0x1e, "P7CR" }, { 0x1f, "P7FC" },
		{ 0x20, "P8" }, { 0x22, "P8CR" }, { 0x23, "P8FC" },
		{ 0x28, "PA" }, { 0x2b, "PAFC" },
		{ 0x2c, "PB" }, { 0x2f, "PBFC" },
		{ 0x30, "PC" }, { 0x32, "PCCR" }, { 0x33, "PCFC" },
		{ 0x34, "PD" }, { 0x36, "PDCR" }, { 0x37, "PDFC" },
		{ 0x38, "PE" }, { 0x3a, "PECR" }, { 0x3b, "PEFC" },
		{ 0x3c, "PF" }, { 0x3e, "PFCR" }, { 0x3f, "PFFC" },
		{ 0x40, "PG" },
		{ 0x44, "PH" }, { 0x46, "PHCR" }, { 0x47, "PHFC" },
		{ 0x68, "PZ" }, { 0x6a, "PZCR" },

		/* TLCS-90 type I/O: */
		{ 0x80, "T8RUN" }, { 0x81, "TRDC" }, { 0x82, "T02FFCR" },
		{ 0x84, "T01MOD" }, { 0x85, "T23MOD" },
		{ 0x88, "TREG0" }, { 0x89, "TREG1" }, { 0x8a, "TREG2" }, { 0x8b, "TREG3" },
		{ 0x90, "TREG4L" }, { 0x91, "TREG4H" }, { 0x92, "TREG5L" }, { 0x93, "TREG5H" },
		{ 0x94, "CAP4L" }, { 0x95, "CAP4H" }, { 0x96, "CAP5L" }, { 0x97, "CAP5H" },
		{ 0x98, "T4MOD" }, { 0x99, "T4FFCR" }, { 0x9e, "T16RUN" }, { 0x9f, "T16CR" },
		{ 0xa0, "TREG6L" }, { 0xa1, "TREG6H" }, { 0xa2, "TREG7L" }, { 0xa3, "TREG7H" },
		{ 0xa4, "CAP6L" }, { 0xa5, "CAP6H" }, { 0xa6, "CAP7L" }, { 0xa7, "CAP7H" },
		{ 0xa8, "T6MOD" }, { 0xa9, "T6FFCR" },
		{ 0xb0, "TREG8L" }, { 0xb1, "TREG8H" }, { 0xb2, "TREG9L" }, { 0xb3, "TREG9H" },
		{ 0xb4, "CAP8L" }, { 0xb5, "CAP8H" }, { 0xb6, "CAP9L" }, { 0xb7, "CAP9H" },
		{ 0xb8, "T8MOD" }, { 0xb9, "T8FFCR" },
		{ 0xc0, "TREGAL" }, { 0xc1, "TREGAH" }, { 0xc2, "TREGBL" }, { 0xc3, "TREGBH" },
		{ 0xc4, "CAPAL" }, { 0xc5, "CAPAH" }, { 0xc6, "CAPBL" }, { 0xc7, "CAPBH" },
		{ 0xc8, "TAMOD" }, { 0xc9, "TAFFCR" },
		{ 0xd0, "SC0BUF" }, { 0xd1, "SC0CR" }, { 0xd2, "SC0MOD" }, { 0xd3, "BR0CR" },
		{ 0xd4, "SC1BUF" }, { 0xd5, "SC1CR" }, { 0xd6, "SC1MOD" }, { 0xd7, "BR1CR" },

		/* TLCS-900/H2 type 8 bit I/O: */
		{ 0xe0, "INTE45" }, { 0xe1, "INTE67" }, { 0xe2, "INTE89" }, { 0xe3, "INTEAB" },
		{ 0xe4, "INTET01" }, { 0xe5, "INTET23" }, { 0xe6, "INTET45" }, { 0xe7, "INTET67" },
		{ 0xe8, "INTET89" }, { 0xe9, "INTETAB" }, { 0xea, "INTES0" }, { 0xeb, "INTES1" },
		{ 0xec, "INTETC01" }, { 0xed, "INTETC23" },
		{ 0xee, "INTETC45" }, { 0xef, "INTETC67" },
		{ 0xf0, "INTE0AD" }, { 0xf6, "IIMC" }, { 0xf7, "INTNMWDT" }, { 0xf8, "INTCLR" },
		{ 0x100, "DMA0V" }, { 0x101, "DMA1V" }, { 0x102, "DMA2V" }, { 0x103, "DMA3V" },
		{ 0x104, "DMA4V" }, { 0x105, "DMA5V" }, { 0x106, "DMA6V" }, { 0x107, "DMA7V" },
		{ 0x108, "DMAB" }, { 0x109, "DMAR" }, { 0x10a, "CLKMOD" },

		/* TLCS-90 type I/O: */
		{ 0x110, "WDMOD" }, { 0x111, "WDCR" },
		{ 0x120, "ADREG04L" }, { 0x121, "ADREG04H" },
		{ 0x122, "ADREG15L" }, { 0x123, "ADREG15H" },
		{ 0x124, "ADREG26L" }, { 0x125, "ADREG26H" },
		{ 0x126, "ADREG37L" }, { 0x127, "ADREG37H" },
		{ 0x128, "ADMOD1" }, { 0x129, "ADMOD2" },
		{ 0x130, "DAREG0" }, { 0x131, "DAREG1" },
		{ 0x132, "DADRV" },

		/* TLCS-900/H2 type 8 bit I/O: */
		{ 0x140, "B0CSL" }, { 0x141, "B0CSH" }, { 0x142, "MAMR0" }, { 0x143, "MSAR0" },
		{ 0x144, "B1CSL" }, { 0x145, "B1CSH" }, { 0x146, "MAMR1" }, { 0x147, "MSAR1" },
		{ 0x148, "B2CSL" }, { 0x149, "B2CSH" }, { 0x14a, "MAMR2" }, { 0x14b, "MSAR2" },
		{ 0x14c, "B3CSL" }, { 0x14d, "B3CSH" }, { 0x14e, "MAMR3" }, { 0x14f, "MSAR3" },
		{ 0x150, "B4CSL" }, { 0x151, "B4CSH" }, { 0x152, "MAMR4" }, { 0x153, "MSAR4" },
		{ 0x154, "B5CSL" }, { 0x155, "B5CSH" }, { 0x156, "MAMR5" }, { 0x157, "MSAR5" },
		{ 0x160, "DRAM0CRL" }, { 0x161, "DRAM0CRH" },
		{ 0x162, "DRAM1CRL" }, { 0x163, "DRAM1CRH" },
		{ 0x164, "DRAM0REF" }, { 0x165, "DRAM1REF" },
		{ 0x166, "PMEMCR" },
	};
};


class tmp95c061_disassembler : public tlcs900_disassembler
{
public:
	tmp95c061_disassembler() : tlcs900_disassembler(m_sfr_names) { };

private:
	std::pair<u16, char const *> const m_sfr_names[106] = {
		{ 0x01, "P1" }, { 0x04, "P1CR" }, { 0x06, "P2" }, { 0x09, "P2FC" },
		{ 0x0d, "P5" }, { 0x10, "P5CR" }, { 0x11, "P5FC" }, { 0x12, "P6" },
		{ 0x13, "P7" }, { 0x15, "P6FC" }, { 0x16, "P7CR" }, { 0x17, "P7FC" },
		{ 0x18, "P8" }, { 0x19, "P9" }, { 0x1a, "P8CR" }, { 0x1b, "P8FC" },
		{ 0x1e, "PA" }, { 0x1f, "PB" }, { 0x20, "TRUN" }, { 0x22, "TREG0" },
		{ 0x23, "TREG1" }, { 0x24, "T01MOD" }, { 0x25, "TFFCR" }, { 0x26, "TREG2" },
		{ 0x27, "TREG3" }, { 0x28, "T23MOD" }, { 0x29, "TRDC" }, { 0x2c, "PACR" },
		{ 0x2d, "PAFC" }, { 0x2e, "PBCR" }, { 0x2f, "PBFC" }, { 0x30, "TREG4L" },
		{ 0x31, "TREG4H" }, { 0x32, "TREG5L" }, { 0x33, "TREG5H" }, { 0x34, "CAP1L" },
		{ 0x35, "CAP1H" }, { 0x36, "CAP2L" }, { 0x37, "CAP2H" }, { 0x38, "T4MOD" },
		{ 0x39, "T4FFCR" }, { 0x3a, "T45CR" }, { 0x3c, "MSAR0" }, { 0x3d, "MAMR0" },
		{ 0x3e, "MSAR1" }, { 0x3f, "MAMR1" }, { 0x40, "TREG6L" }, { 0x41, "TREG6H" },
		{ 0x42, "TREG7L" }, { 0x43, "TREG7H" }, { 0x44, "CAP3L" }, { 0x45, "CAP3H" },
		{ 0x46, "CAP4L" }, { 0x47, "CAP4H" }, { 0x48, "T5MOD" }, { 0x49, "T5FFCR" },
		{ 0x4c, "PG0REG" }, { 0x4d, "PG1REG" }, { 0x4e, "PG01CR" }, { 0x50, "SC0BUF" },
		{ 0x51, "SC0CR" }, { 0x52, "SC0MOD" }, { 0x53, "BR0CR" }, { 0x54, "SC1BUF" },
		{ 0x55, "SC1CR" }, { 0x56, "SC1MOD" }, { 0x57, "BR1CR" }, { 0x58, "ODE" },
		{ 0x5a, "DREFCR" }, { 0x5b, "DMEMCR" }, { 0x5c, "MSAR2" }, { 0x5d, "MAMR2" },
		{ 0x5e, "MSAR3" }, { 0x5f, "MAMR3" }, { 0x60, "ADREG0L" }, { 0x61, "ADREG0H" },
		{ 0x62, "ADREG1L" }, { 0x63, "ADREG1H" }, { 0x64, "ADREG2L" }, { 0x65, "ADREG2H" },
		{ 0x66, "ADREG3L" }, { 0x67, "ADREG3H" }, { 0x68, "B0CS" }, { 0x69, "B1CS" },
		{ 0x6a, "B2CS" }, { 0x6b, "B3CS" }, { 0x6c, "BEXCS" }, { 0x6d, "ADMOD" },
		{ 0x6e, "WDMOD" }, { 0x6f, "WDCR" }, { 0x70, "INTE0AD" }, { 0x71, "INTE45" },
		{ 0x72, "INTE67" }, { 0x73, "INTET10" }, { 0x74, "INTET32" }, { 0x75, "INTET54" },
		{ 0x76, "INTET76" }, { 0x77, "INTES0" }, { 0x78, "INTES1" }, { 0x79, "INTETC01" },
		{ 0x7a, "INTETC23" }, { 0x7b, "IIMC" }, { 0x7c, "DMA0V" }, { 0x7d, "DMA1V" },
		{ 0x7e, "DMA2V" }, { 0x7f, "DMA3V" }
	};
};


class tmp95c063_disassembler : public tlcs900_disassembler
{
public:
	tmp95c063_disassembler() : tlcs900_disassembler(m_sfr_names) { };

private:
	std::pair<u16, char const *> const m_sfr_names[135] = {
		{ 0x01, "P1" }, { 0x04, "P1CR" }, { 0x06, "P2" },
		{ 0x09, "P2FC" }, { 0x0d, "P5" },
		{ 0x10, "P5CR" }, { 0x11, "P5FC" }, { 0x12, "P6" }, { 0x13, "P7" }, { 0x15, "P6FC" }, { 0x16, "P7CR" }, { 0x17, "P7FC" },
		{ 0x18, "P8" }, { 0x19, "P9" }, { 0x1a, "P8CR" }, { 0x1b, "P8FC" }, { 0x1c, "P9CR" }, { 0x1d, "P9FC" }, { 0x1e, "PA" }, { 0x1f, "PB" },
		{ 0x20, "T8RUN" }, { 0x21, "TRDC" }, { 0x22, "TREG0" }, { 0x23, "TREG1" }, { 0x24, "T01MOD" }, { 0x25, "T02FFCR" }, { 0x26, "TREG2" }, { 0x27, "TREG3" },
		{ 0x28, "T23MOD" }, { 0x29, "TREG4" }, { 0x2a, "TREG5" }, { 0x2b, "T45MOD" }, { 0x2c, "TA46FFCR" }, { 0x2d, "TREG6" }, { 0x2e, "TREG7" }, { 0x2f, "T67MOD" },
		{ 0x30, "TREG8L" }, { 0x31, "TREG8H" }, { 0x32, "TREG9L" }, { 0x33, "TREG9H" }, { 0x34, "CAP1L" }, { 0x35, "CAP1H" }, { 0x36, "CAP2L" }, { 0x37, "CAP2H" },
		{ 0x38, "T8MOD" }, { 0x39, "T8FFCR" }, { 0x3a, "T89CR" }, { 0x3b, "T16RUN" },
		{ 0x40, "TREGAL" }, { 0x41, "TREGAH" }, { 0x42, "TREGBL" }, { 0x43, "TREGBH" }, { 0x44, "CAP3L" }, { 0x45, "CAP3H" }, { 0x46, "CAP4L" }, { 0x47, "CAP4H" },
		{ 0x48, "T9MOD" }, { 0x49, "T9FFCR" }, { 0x4a, "DAREG0" }, { 0x4b, "DAREG1" }, { 0x4c, "PG0REG" }, { 0x4d, "PG1REG" }, { 0x4e, "PG01CR" }, { 0x4f, "DADRV" },
		{ 0x50, "SC0BUF" }, { 0x51, "SC0CR" }, { 0x52, "SC0MOD" }, { 0x53, "BR0CR" }, { 0x54, "SC1BUF" }, { 0x55, "SC1CR" }, { 0x56, "SC1MOD" }, { 0x57, "BR1CR" },
		{ 0x58, "ODE" }, { 0x5a, "DMA0V" }, { 0x5b, "DMA1V" }, { 0x5c, "DMA2V" }, { 0x5d, "DMA3V" }, { 0x5e, "ADMOD1" }, { 0x5f, "ADMOD2" },
		{ 0x60, "ADREG04L" }, { 0x61, "ADREG04H" }, { 0x62, "ADREG15L" }, { 0x63, "ADREG15H" }, { 0x64, "ADREG26L" }, { 0x65, "ADREG26H" }, { 0x66, "ADREG37L" }, { 0x67, "ADREG37H" },
		{ 0x6a, "SDMACR0" }, { 0x6b, "SDMACR1" }, { 0x6c, "SDMACR2" }, { 0x6d, "SDMACR3" }, { 0x6e, "WDMOD" }, { 0x6f, "WDCR" },
		{ 0x70, "INTE_0AD" }, { 0x71, "INTE12" }, { 0x72, "INTE34" }, { 0x73, "INTE56" }, { 0x74, "INT78" }, { 0x75, "INTET01" }, { 0x76, "INTET32" }, { 0x77, "INTET45" },
		{ 0x78, "INTET67" }, { 0x79, "INTET89" }, { 0x7a, "INTETAB" }, { 0x7b, "INTES0" }, { 0x7c, "INTES1" }, { 0x7d, "INTETC01" }, { 0x7e, "INTETC23" }, { 0x7f, "IIMC" },
		{ 0x80, "PACR" }, { 0x81, "PAFC" }, { 0x82, "PBCR" }, { 0x83, "PBFC" }, { 0x84, "PC" }, { 0x85, "PD"},
		{ 0x88, "PDCR" }, { 0x8a, "PE" }, { 0x8c, "PECR" }, { 0x8f, "BEXCS" },
		{ 0x90, "B0CS" }, { 0x91, "B1CS" }, { 0x92, "B2CS" }, { 0x93, "B3CS" }, { 0x94, "MSAR0" }, { 0x95, "MAMR0" }, { 0x96, "MSAR1" }, { 0x97, "MAMR1" },
		{ 0x98, "MSAR2" }, { 0x99, "MAMR2" }, { 0x9a, "MSAR3" }, { 0x9b, "MAMR3" }, { 0x9c, "DREFCR1" }, { 0x9d, "DMEMCR1" }, { 0x9e, "DREFCR3" }, { 0x9f, "DMEMCR3" }
	};
};


class tmp96c141_disassembler : public tlcs900_disassembler
{
public:
	tmp96c141_disassembler() : tlcs900_disassembler(m_sfr_names) { };

private:
	std::pair<u16, char const *> const m_sfr_names[98] = {
		{ 0x00, "P0" }, { 0x01, "P1" }, { 0x02, "P0CR" },
		{ 0x04, "P1CR" }, { 0x05, "P1FC" }, { 0x06, "P2" }, { 0x07, "P3" },
		{ 0x08, "P2CR" }, { 0x09, "P2FC" }, { 0x0a, "P3CR" }, { 0x0b, "P3FC" }, { 0x0c, "P4" }, { 0x0d, "P5" }, { 0x0e, "P4CR" },
		{ 0x10, "P4FC" }, { 0x12, "P6" }, { 0x13, "P7" }, { 0x14, "P6CR" }, { 0x15, "P7CR" }, { 0x16, "P6FC" }, { 0x17, "P7FC" },
		{ 0x18, "P8" }, { 0x19, "P9" }, { 0x1a, "P8CR" }, { 0x1b, "P9CR" }, { 0x1c, "P8FC" }, { 0x1d, "P9FC" },
		{ 0x20, "TRUN" }, { 0x22, "TREG0" }, { 0x23, "TREG1" }, { 0x24, "TMOD" }, { 0x25, "TFFCR" }, { 0x26, "TREG2" }, { 0x27, "TREG3" },
		{ 0x28, "P0MOD" }, { 0x29, "P1MOD" }, { 0x2a, "PFFCR" },
		{ 0x30, "TREG4L" }, { 0x31, "TREG4H" }, { 0x32, "TREG5L" }, { 0x33, "TREG5H" }, { 0x34, "CAP1L" }, { 0x35, "CAP1H" }, { 0x36, "CAP2L" }, { 0x37, "CAP2H" },
		{ 0x38, "T4MOD" }, { 0x39, "T4FFCR" }, { 0x3a, "T45CR" },
		{ 0x40, "TREG6L" }, { 0x41, "TREG6H" }, { 0x42, "TREG7L" }, { 0x43, "TREG7H" }, { 0x44, "CAP3L" }, { 0x45, "CAP3H" }, { 0x46, "CAP4L" }, { 0x47, "CAP4H" },
		{ 0x48, "T5MOD" }, { 0x49, "T5FFCR" }, { 0x4c, "PG0REG" }, { 0x4d, "PG1REG" }, { 0x4e, "PG01CR" },
		{ 0x50, "SC0BUF" }, { 0x51, "SC0CR" }, { 0x52, "SC0MOD" }, { 0x53, "BR0CR" }, { 0x54, "SC1BUF" }, { 0x55, "SC1CR" }, { 0x56, "SC1MOD" }, { 0x57, "BR1CR" },
		{ 0x58, "ODE" }, { 0x5c, "WDMOD" }, { 0x5d, "WDCR" }, { 0x5e, "ADMOD" },
		{ 0x60, "ADREG0L" }, { 0x61, "ADREG0H" }, { 0x62, "ADREG1L" }, { 0x63, "ADREG1H" }, { 0x64, "ADREG2L" }, { 0x65, "ADREG2H" }, { 0x66, "ADREG3L" }, { 0x67, "ADREG3H" },
		{ 0x68, "B0CS" }, { 0x69, "B1CS" }, { 0x6a, "B2CS" },
		{ 0x70, "INTE0AD" }, { 0x71, "INTE45" }, { 0x72, "INTE67" }, { 0x73, "INTET10" }, { 0x74, "INTEPW10" }, { 0x75, "INTET54" }, { 0x76, "INTET76" }, { 0x77, "INTES0" },
		{ 0x78, "INTES1" }, { 0x7b, "IIMC" }, { 0x7c, "DMA0V" }, { 0x7d, "DMA1V" }, { 0x7e, "DMA2V" }, { 0x7f, "DMA3V"}
	};
};

#endif
