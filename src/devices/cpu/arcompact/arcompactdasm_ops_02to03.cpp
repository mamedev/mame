// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"

#include "arcompactdasm.h"


int arcompact_disassembler::handle_dasm32_LD_r_o(std::ostream& stream, offs_t pc, uint32_t op, const data_buffer& opcodes)
{
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 0bbb ssss ssss SBBB DaaZ ZXAA AAAA
	int size = 4;

	uint8_t areg = dasm_common32_get_areg(op);
	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
	int D = (op & 0x00000800) >> 11;// op &= ~0x00000800;
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	uint8_t breg = dasm_common32_get_breg(op);

	uint32_t sdat = s | (S << 8);
	if (sdat & 0x100)
		sdat |= 0xffffff00;

	uint32_t limm = 0;
	if (breg == DASM_REG_LIMM)
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
	}

	if ((s == 0x04) && (S == 0) && (Z == 0) && (X == 0) & (a == 2) && (D == 0) && (breg == 28))
	{
		util::stream_format(stream, "POP %s", regnames[areg]);
	}
	else
	{
		util::stream_format(stream, "LD%s%s%s%s %s <- [", datasize[Z], dataextend[X], addressmode[a], cachebit[D], regnames[areg]);
		if (breg == DASM_REG_LIMM) util::stream_format(stream, "0x%08x, ", limm);
		else util::stream_format(stream, "%s, ", regnames[breg]);
		util::stream_format(stream, "0x%03x]", sdat);
	}
	return size;
}

int arcompact_disassembler::handle_dasm32_ST_r_o(std::ostream& stream, offs_t pc, uint32_t op, const data_buffer& opcodes)
{
	int size = 4;
	uint32_t limm = 0;
	int got_limm = 0;
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 1bbb ssss ssss SBBB CCCC CCDa aZZR
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;

	uint8_t breg = dasm_common32_get_breg(op);
	uint32_t sdat = s | (S << 8);
	if (sdat & 0x100)
		sdat |= 0xffffff00;

	int Z = (op & 0x00000006) >> 1; op &= ~0x00000006;
	int a = (op & 0x00000018) >> 3; op &= ~0x00000018;
	int D = (op & 0x00000020) >> 5; op &= ~0x00000020;
	uint8_t creg = dasm_common32_get_creg(op);

	if (breg == DASM_REG_LIMM)
	{
		limm = dasm_get_limm_32bit_opcode(pc, opcodes);
		size = 8;
		got_limm = 1;
	}

	if ((s == 0xfc) && (S == 1) && (Z == 0) && (a == 1) && (D == 0) && (breg == 28))
	{
		util::stream_format(stream, "PUSH %s", regnames[creg]);
	}
	else
	{
		util::stream_format(stream, "ST%s%s%s [", datasize[Z], addressmode[a], cachebit[D]);
		if (breg == DASM_REG_LIMM) util::stream_format(stream, "0x%08x, ", limm);
		else util::stream_format(stream, "%s, ", regnames[breg]);
		util::stream_format(stream, "0x%03x", sdat);
		util::stream_format(stream, "] <- ");

		if (creg == DASM_REG_LIMM)
		{
			if (!got_limm)
			{
				limm = dasm_get_limm_32bit_opcode(pc, opcodes);
				size = 8;
			}
			util::stream_format(stream, "0x%08x", limm);

		}
		else
		{
			util::stream_format(stream, "%s", regnames[creg]);
		}
	}
	return size;
}
