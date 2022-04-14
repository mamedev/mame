// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP "hybrid" processor disassembler
// ********************************************************************************

#include "emu.h"
#include "hphybrid_dasm.h"
#include "hphybrid_defs.h"

void hp_hybrid_disassembler::addr_2_str(std::ostream &stream, uint16_t addr , bool indirect)
{
	if (m_flags & HP_HYBRID_DASM_HAS_15BITS) {
		addr &= 0x7fff;
	}

	util::stream_format(stream, "$%04x" , addr);

	// Common registers
	switch (addr) {
	case HP_REG_A_ADDR:
		stream << "(A)";
		break;

	case HP_REG_B_ADDR:
		stream << "(B)";
		break;

	case HP_REG_P_ADDR:
		stream << "(P)";
		break;

	case HP_REG_R_ADDR:
		stream << "(R)";
		break;

	case HP_REG_R4_ADDR:
		stream << "(R4)";
		break;

	case HP_REG_R5_ADDR:
		stream << "(R5)";
		break;

	case HP_REG_R6_ADDR:
		stream << "(R6)";
		break;

	case HP_REG_R7_ADDR:
		stream << "(R7)";
		break;

	case HP_REG_IV_ADDR:
		stream << "(IV)";
		break;

	case HP_REG_PA_ADDR:
		stream << "(PA)";
		break;

	case HP_REG_DMAPA_ADDR:
		stream << "(DMAPA)";
		break;

	case HP_REG_DMAMA_ADDR:
		stream << "(DMAMA)";
		break;

	case HP_REG_DMAC_ADDR:
		stream << "(DMAC)";
		break;

	case HP_REG_C_ADDR:
		stream << "(C)";
		break;

	case HP_REG_D_ADDR:
		stream << "(D)";
		break;

	default:
		// EMC registers
		if (m_flags & HP_HYBRID_DASM_HAS_EMC) {
			if (m_flags & HP_HYBRID_DASM_HAS_15BITS) {
				switch (addr) {
				case HP_REG_AR1_ADDR & 0x7fff:
					stream << "(Ar1)";
					break;

				case (HP_REG_AR1_ADDR & 0x7fff) + 1:
					stream << "(Ar1_2)";
					break;

				case (HP_REG_AR1_ADDR & 0x7fff) + 2:
					stream << "(Ar1_3)";
					break;

				case (HP_REG_AR1_ADDR & 0x7fff) + 3:
					stream << "(Ar1_4)";
					break;
				}
			} else {
				switch (addr) {
				case HP_REG_AR1_ADDR:
					stream << "(Ar1)";
					break;

				case HP_REG_AR1_ADDR + 1:
					stream << "(Ar1_2)";
					break;

				case HP_REG_AR1_ADDR + 2:
					stream << "(Ar1_3)";
					break;

				case HP_REG_AR1_ADDR + 3:
					stream << "(Ar1_4)";
					break;
				}
			}
			switch (addr) {
			case HP_REG_AR2_ADDR:
				stream << "(Ar2)";
				break;

			case HP_REG_AR2_ADDR + 1:
				stream << "(Ar2_2)";
				break;

			case HP_REG_AR2_ADDR + 2:
				stream << "(Ar2_3)";
				break;

			case HP_REG_AR2_ADDR + 3:
				stream << "(Ar2_4)";
				break;

			case HP_REG_SE_ADDR:
				stream << "(SE)";
				break;

			case HP_REG_R25_ADDR:
				stream << "(R25)";
				break;

			case HP_REG_R26_ADDR:
				stream << "(R26)";
				break;

			case HP_REG_R27_ADDR:
				stream << "(R27)";
				break;

			default:
				break;
			}
		}
		// AEC registers
		if (m_flags & HP_HYBRID_DASM_HAS_AEC) {
			switch (addr) {
			case HP_REG_R32_ADDR:
				stream << "(R32)";
				break;

			case HP_REG_R33_ADDR:
				stream << "(R33)";
				break;

			case HP_REG_R34_ADDR:
				stream << "(R34)";
				break;

			case HP_REG_R35_ADDR:
				stream << "(R35)";
				break;

			case HP_REG_R36_ADDR:
				stream << "(R36)";
				break;

			case HP_REG_R37_ADDR:
				stream << "(R37)";
				break;

			default:
				break;
			}
		}
		break;
	}

	if (indirect) {
		stream << ",I";
	}
}

void hp_hybrid_disassembler::param_none(std::ostream &stream, offs_t pc , uint16_t opcode)
{
}

void hp_hybrid_disassembler::param_loc(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	uint16_t base;
	uint16_t off;

	if (opcode & 0x0400) {
		if (m_flags & HP_HYBRID_DASM_ABS_MODE) {
			// Current page, absolute mode
			base = (pc & 0xfc00) | 0x200;
		} else {
			// Current page, relative mode
			base = pc;
		}
	} else {
		// Base page
		base = 0;
	}

	off = opcode & 0x3ff;
	if (off & 0x200) {
		off -= 0x400;
	}

	addr_2_str(stream, base + off , (opcode & 0x8000) != 0);
}

void hp_hybrid_disassembler::param_addr32(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	addr_2_str(stream, opcode & 0x1f , (opcode & 0x8000) != 0);
}

void hp_hybrid_disassembler::param_skip(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	uint16_t off = opcode & 0x3f;
	if (off & 0x20) {
		off -= 0x40;
	}
	addr_2_str(stream , pc + off , false);
}

void hp_hybrid_disassembler::param_skip_sc(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	param_skip(stream, pc, opcode);

	if (opcode & 0x80) {
		if (opcode & 0x40) {
			stream << ",S";
		} else {
			stream << ",C";
		}
	}
}

void hp_hybrid_disassembler::param_ret(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	int off = opcode & 0x3f;

	if (off & 0x20) {
		off -= 0x40;
	}

	util::stream_format(stream , "%d" , off);
	if (opcode & 0x40) {
		stream << ",P";
	}
}

void hp_hybrid_disassembler::param_n16(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	util::stream_format(stream , "%u" , (opcode & 0xf) + 1);
}

void hp_hybrid_disassembler::param_reg_id(std::ostream &stream, offs_t pc , uint16_t opcode)
{
	addr_2_str(stream, opcode & 7, false);

	if (opcode & 0x80) {
		stream << ",D";
	} else {
		stream << ",I";
	}
}

const hp_hybrid_disassembler::dis_entry_t hp_hybrid_disassembler::dis_table_common[] = {
	// *** BPC Instructions ***
	{0xffff , 0x0000 , "NOP" , &hp_hybrid_disassembler::param_none , 0 },
	{0x7800 , 0x0000 , "LDA" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x0800 , "LDB" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x1000 , "CPA" , &hp_hybrid_disassembler::param_loc , STEP_COND },
	{0x7800 , 0x1800 , "CPB" , &hp_hybrid_disassembler::param_loc , STEP_COND },
	{0x7800 , 0x2000 , "ADA" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x2800 , "ADB" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x3000 , "STA" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x3800 , "STB" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x4000 , "JSM" , &hp_hybrid_disassembler::param_loc , STEP_OVER },
	{0x7800 , 0x4800 , "ISZ" , &hp_hybrid_disassembler::param_loc , STEP_COND },
	{0x7800 , 0x5000 , "AND" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x5800 , "DSZ" , &hp_hybrid_disassembler::param_loc , STEP_COND },
	{0x7800 , 0x6000 , "IOR" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7800 , 0x6800 , "JMP" , &hp_hybrid_disassembler::param_loc , 0 },
	{0x7fe0 , 0x7000 , "EXE" , &hp_hybrid_disassembler::param_addr32 , 0 },
	{0xffc0 , 0x7400 , "RZA" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7C00 , "RZB" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7440 , "RIA" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7C40 , "RIB" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x74c0 , "SDS" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7500 , "SZA" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7D00 , "SZB" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7540 , "SIA" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7D40 , "SIB" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7480 , "SFS" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7580 , "SFC" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x75c0 , "SDC" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7c80 , "SSS" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7d80 , "SSC" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7cc0 , "SHS" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xffc0 , 0x7dc0 , "SHC" , &hp_hybrid_disassembler::param_skip , STEP_COND },
	{0xff00 , 0x7600 , "SLA" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0x7e00 , "SLB" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0x7700 , "RLA" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0x7f00 , "RLB" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xf400 , "SAP" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xfc00 , "SBP" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xf500 , "SAM" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xfd00 , "SBM" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xf600 , "SOC" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xf700 , "SOS" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xfe00 , "SEC" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xff00 , 0xff00 , "SES" , &hp_hybrid_disassembler::param_skip_sc , STEP_COND },
	{0xffff , 0xf020 , "TCA" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0xf820 , "TCB" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0xf060 , "CMA" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0xf860 , "CMB" , &hp_hybrid_disassembler::param_none , 0 },
	{0xff80 , 0xf080 , "RET" , &hp_hybrid_disassembler::param_ret , STEP_OUT },
	{0xfff0 , 0xf100 , "AAR" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xfff0 , 0xf900 , "ABR" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xffff , 0xf14f , "CLA" , &hp_hybrid_disassembler::param_none , 0 },
	{0xfff0 , 0xf140 , "SAR" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xffff , 0xf94f , "CLB" , &hp_hybrid_disassembler::param_none , 0 },
	{0xfff0 , 0xf940 , "SBR" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xfff0 , 0xf180 , "SAL" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xfff0 , 0xf980 , "SBL" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xfff0 , 0xf1c0 , "RAR" , &hp_hybrid_disassembler::param_n16 , 0 },
	{0xfff0 , 0xf9c0 , "RBR" , &hp_hybrid_disassembler::param_n16 , 0 },
	// *** IOC Instructions ***
	{0xffff , 0x7110 , "EIR" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7118 , "DIR" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7120 , "DMA" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7128 , "PCM" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7138 , "DDR" , &hp_hybrid_disassembler::param_none , 0 },
	{0xff78 , 0x7160 , "PWC" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7168 , "PWD" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7960 , "PBC" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7968 , "PBD" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7170 , "WWC" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7178 , "WWD" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7970 , "WBC" , &hp_hybrid_disassembler::param_reg_id , 0 },
	{0xff78 , 0x7978 , "WBD" , &hp_hybrid_disassembler::param_reg_id , 0 },
	// *** END ***
	{0 , 0 , nullptr , nullptr , 0 }
};
const hp_hybrid_disassembler::dis_entry_t hp_hybrid_disassembler::dis_table_ioc16[] = {
	// *** IOC-16 instructions ***
	{0xffff , 0x7100 , "SDO" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7108 , "SDI" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7140 , "DBL" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7148 , "CBL" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7150 , "DBU" , &hp_hybrid_disassembler::param_none , 0 },
	{0xffff , 0x7158 , "CBU" , &hp_hybrid_disassembler::param_none , 0 },
	// *** END ***
	{0 , 0 , nullptr , nullptr , 0 }
};
const hp_hybrid_disassembler::dis_entry_t hp_hybrid_disassembler::dis_table_emc[] = {
	// *** EMC Instructions ***
	{0xffff , 0x7200 , "MWA" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7220 , "CMY" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7260 , "CMX" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7280 , "FXA" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xfff0 , 0x7300 , "XFR" , &hp_5061_3001_disassembler::param_n16 , 0 },
	{0xffff , 0x7340 , "NRM" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xfff0 , 0x7380 , "CLR" , &hp_5061_3001_disassembler::param_n16 , 0 },
	{0xffff , 0x73c0 , "CDC" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7a00 , "FMP" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7a21 , "FDV" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7b00 , "MRX" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7b21 , "DRS" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7b40 , "MRY" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7b61 , "MLY" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7b8f , "MPY" , &hp_5061_3001_disassembler::param_none , 0 },
	// *** END ***
	{0 , 0 , nullptr , nullptr , 0 }
};
const hp_hybrid_disassembler::dis_entry_t hp_hybrid_disassembler::dis_table_aec[] = {
	// *** Undocumented AEC instructions of 5061-3001 ***
	{0xffff , 0x7026 , "CIM" , &hp_5061_3001_disassembler::param_none , 0 },
	{0xffff , 0x7027 , "SIM" , &hp_5061_3001_disassembler::param_none , 0 },
	// *** END ***
	{0 , 0 , nullptr , nullptr , 0 }
};

offs_t hp_hybrid_disassembler::disassemble_table(uint16_t opcode , offs_t pc , const dis_entry_t *table , std::ostream &stream)
{
	const dis_entry_t *p;

	for (p = table; p->m_op_mask; p++) {
		if ((opcode & p->m_op_mask) == p->m_opcode) {
			stream << p->m_mnemonic << " ";
			(this->*(p->m_param_fn))(stream , pc , opcode);
			return 1 | p->m_dasm_flags | SUPPORTED;
		}
	}

	return 0;
}

offs_t hp_hybrid_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t opcode = opcodes.r16(pc);
	offs_t res;

	res = disassemble_table(opcode, pc, dis_table_common, stream);

	if (res == 0 && (m_flags & HP_HYBRID_DASM_HAS_15BITS) == 0) {
		res = disassemble_table(opcode, pc, dis_table_ioc16, stream);
	}
	if (res == 0 && (m_flags & HP_HYBRID_DASM_HAS_EMC) != 0) {
		res = disassemble_table(opcode, pc, dis_table_emc, stream);
	}
	if (res == 0 && (m_flags & HP_HYBRID_DASM_HAS_AEC) != 0) {
		res = disassemble_table(opcode, pc, dis_table_aec, stream);
	}

	if (res == 0) {
		// Unknown opcode
		stream << "???";
		res = 1 | SUPPORTED;
	}

	return res;
}

hp_hybrid_disassembler::hp_hybrid_disassembler(unsigned flags)
	: m_flags(flags)
{
}

hp_5061_3011_disassembler::hp_5061_3011_disassembler(bool relative_mode)
	: hp_hybrid_disassembler(relative_mode ? 0 : HP_HYBRID_DASM_ABS_MODE)
{
}

hp_5061_3001_disassembler::hp_5061_3001_disassembler(bool relative_mode)
	: hp_hybrid_disassembler(HP_HYBRID_DASM_HAS_EMC |
							 HP_HYBRID_DASM_HAS_AEC |
							 (relative_mode ? 0 : HP_HYBRID_DASM_ABS_MODE))
{
}

hp_09825_67907_disassembler::hp_09825_67907_disassembler(bool relative_mode)
	: hp_hybrid_disassembler(HP_HYBRID_DASM_HAS_15BITS |
							 HP_HYBRID_DASM_HAS_EMC |
							 (relative_mode ? 0 : HP_HYBRID_DASM_ABS_MODE))
{
}

u32 hp_hybrid_disassembler::opcode_alignment() const
{
	return 1;
}
