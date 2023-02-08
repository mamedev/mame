// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler
\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"


inline uint32_t arcompact_disassembler::get_01_01_01_address_offset(uint32_t op)
{
	uint32_t address = (op & 0x00fe0000) >> 17;
	address |= ((op & 0x00008000) >> 15) << 7;
	address = util::sext(address, 8);
	return address;
}

int arcompact_disassembler::handle::dasm32_B_cc_D_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address = util::sext(address, 20);
	int n = (op & 0x00000020) >> 5;
	uint8_t condition = common32_get_condition(op);

	util::stream_format(stream, "B%s%s 0x%08x", conditions[condition], delaybit[n], (pc&0xfffffffc) + (address * 2));
	return size;
}

int arcompact_disassembler::handle::dasm32_B_D_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch Unconditionally Far
	// 0000 0sss ssss sss1 SSSS SSSS SSNR TTTT
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= (op & 0x0000000f) << 20;
	address = util::sext(address, 24);
	int n = (op & 0x00000020) >> 5;

	util::stream_format(stream, "B%s 0x%08x", delaybit[n], (pc&0xfffffffc) + (address * 2));

	return size;
}

int arcompact_disassembler::handle::dasm32_BL_cc_d_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;

	// Branch and Link Conditionally
	// 00001 sssssssss 00 SSSSSSSSSS N QQQQQ
	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address = util::sext(address, 20);

	int n = (op & 0x00000020) >> 5;

	uint8_t condition = common32_get_condition(op);

	util::stream_format(stream, "BL%s%s 0x%08x", conditions[condition], delaybit[n], (pc&0xfffffffc) + (address *2));
	return size;
}

int arcompact_disassembler::handle::dasm32_BL_d_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	uint32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        (op & 0x0000000f) << 20;
	address = util::sext(address, 24);

	int n = (op & 0x00000020) >> 5;

	util::stream_format(stream, "BL%s 0x%08x", delaybit[n], (pc&0xfffffffc) + (address *2));

	return size;
}



int arcompact_disassembler::handle01_01_00_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	int size = 4;

	// Branch on Compare / Bit Test - Register-Register
	// 00001 bbb sssssss 1 S BBB CCCCCC N 0 iiii
	uint32_t address = get_01_01_01_address_offset(op);

	uint8_t creg = common32_get_creg(op);
	uint8_t breg = common32_get_breg(op);
	int n = (op & 0x00000020) >> 5;

	if ((breg != DASM_REG_LIMM) && (creg != DASM_REG_LIMM))
	{
		util::stream_format( stream, "%s%s %s, %s to 0x%08x", optext, delaybit[n], regnames[breg], regnames[creg], (pc&0xfffffffc) + (address * 2) );
	}
	else
	{
		uint32_t limm;
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;

		if ((breg == DASM_REG_LIMM) && (creg != DASM_REG_LIMM))
		{
			util::stream_format( stream, "%s%s 0x%08x, %s to 0x%08x", optext, delaybit[n], limm, regnames[creg], (pc&0xfffffffc) + (address * 2) );
		}
		else if ((creg == DASM_REG_LIMM) && (breg != DASM_REG_LIMM))
		{
			util::stream_format( stream, "%s%s %s, 0x%08x to 0x%08x", optext, delaybit[n], regnames[breg], limm, (pc&0xfffffffc) + (address * 2) );
		}
		else
		{
			// b and c are LIMM? invalid??
			util::stream_format( stream, "%s%s 0x%08x, 0x%08x (illegal?) to 0x%08x", optext, delaybit[n], limm, limm, (pc&0xfffffffc) + (address * 2) );
		}
	}
	return size;
}


// register - register cases
int arcompact_disassembler::handle::dasm32_BREQ_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BREQ");
}

int arcompact_disassembler::handle::dasm32_BRNE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRNE");
}

int arcompact_disassembler::handle::dasm32_BRLT_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRLT");
}

int arcompact_disassembler::handle::dasm32_BRGE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRGE");
}

int arcompact_disassembler::handle::dasm32_BRLO_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRLO");
}

int arcompact_disassembler::handle::dasm32_BRHS_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BRHS");
}

int arcompact_disassembler::handle::dasm32_BBIT0_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BBIT0");
}

int arcompact_disassembler::handle::dasm32_BBIT1_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_00_helper( stream, pc, op, opcodes, "BBIT1");
}

int arcompact_disassembler::handle01_01_01_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext)
{
	int size = 4;

	// using 'b' as limm here makes no sense (comparing a long immediate against a short immediate) so I assume it isn't
	// valid?

	// Branch on Compare / Bit Test - Register-Immediate
	// 0000 1bbb ssss sss1 SBBB uuuu uuN1 iiii
	uint32_t address = get_01_01_01_address_offset(op);

	uint32_t u = common32_get_u6(op);
	uint8_t breg = common32_get_breg(op);
	int n = (op & 0x00000020) >> 5;

	util::stream_format(stream, "%s%s %s, 0x%02x 0x%08x", optext, delaybit[n], regnames[breg], u, (pc&0xfffffffc) + (address * 2));

	return size;
}

// register -immediate cases
int arcompact_disassembler::handle::dasm32_BREQ_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BREQ");
}

int arcompact_disassembler::handle::dasm32_BRNE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRNE");
}

int arcompact_disassembler::handle::dasm32_BRLT_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRLT");
}

int arcompact_disassembler::handle::dasm32_BRGE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRGE");
}

int arcompact_disassembler::handle::dasm32_BRLO_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRLO");
}

int arcompact_disassembler::handle::dasm32_BRHS_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BRHS");
}

int arcompact_disassembler::handle::dasm32_BBIT0_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BBIT0");
}

int arcompact_disassembler::handle::dasm32_BBIT1_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes)
{
	return handle01_01_01_helper(stream, pc, op, opcodes, "BBIT1");
}
