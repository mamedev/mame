// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont

#include "emu.h"
#include "v60d.h"

// Register names
const char *const v60_disassembler::v60_reg_names[69] = {
	"R0", "R1", "R2", "R3",
	"R4", "R5", "R6", "R7",
	"R8", "R9", "R10", "R11",
	"R12", "R13", "R14", "R15",
	"R16", "R17", "R18", "R19",
	"R20", "R21", "R22", "R23",
	"R24", "R25", "R26", "R27",
	"R28", "AP", "FP", "SP",
	"PC", "PSW","Unk","Unk",
	"ISP", "L0SP", "L1SP", "L2SP",
	"L3SP", "SBR","TR","SYCW",
	"TKCW", "PIR", "Reserved","Reserved",
	"Reserved","Reserved","Reserved","PSW2",
	"ATBR0", "ATLR0", "ATBR1", "ATLR1",
	"ATBR2", "ATLR2", "ATBR3", "ATLR3",
	"TRMODE", "ADTR0", "ADTR1","ADTMR0",
	"ADTMR1","Reserved","Reserved","Reserved"
};

void v60_disassembler::out_AM_Register(int reg, std::ostream &stream)
{
	stream << v60_reg_names[reg];
}

void v60_disassembler::out_AM_RegisterIndirect(int reg, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		stream << '@';
	util::stream_format(stream, "[%s]", v60_reg_names[reg]);
}

void v60_disassembler::out_AM_RegisterIndirectIndexed(int rn, int rx, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@[%s]", v60_reg_names[rx], v60_reg_names[rn]);
	else
		util::stream_format(stream, "[%s](%s)", v60_reg_names[rn], v60_reg_names[rx]);

}

void v60_disassembler::out_AM_Autoincrement(int reg, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		stream << '@';
	util::stream_format(stream, "[%s+]", v60_reg_names[reg]);
}

void v60_disassembler::out_AM_Autodecrement(int reg, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		stream << '@';
	util::stream_format(stream, "[-%s]", v60_reg_names[reg]);
}

void v60_disassembler::out_AM_Displacement(int reg, int disp, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%s%X%s[%s]",
			disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,
			opsize & 0x80 ? "@" : "",
			v60_reg_names[reg]);
}

void v60_disassembler::out_AM_DisplacementIndexed(int rn, int rx, int disp, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@%s%X[%s]", v60_reg_names[rx], disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn]);
	else
		util::stream_format(stream, "%s%X[%s](%s)", disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn], v60_reg_names[rx]);
}

void v60_disassembler::out_AM_PCDisplacement(offs_t pc, int disp, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%X%s[PC]", pc+disp, opsize & 0x80 ? "@" : "");
}

void v60_disassembler::out_AM_PCDisplacementIndexed(offs_t pc, int disp, int rx, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@%X[PC]", v60_reg_names[rx], pc+disp);
	else
		util::stream_format(stream, "%X[PC](%s)", pc+disp, v60_reg_names[rx]);
}

void v60_disassembler::out_AM_DisplacementIndirect(int reg, int disp, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%s[%s%X[%s]]",
			opsize & 0x80 ? "@" : "",
			disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,
			v60_reg_names[reg]);
}

void v60_disassembler::out_AM_DisplacementIndirectIndexed(int rn, int rx, int disp, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@[%s%X[%s]]", v60_reg_names[rx], disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn]);
	else
		util::stream_format(stream, "[%s%X[%s]](%s)", disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn], v60_reg_names[rx]);
}

void v60_disassembler::out_AM_PCDisplacementIndirect(offs_t pc, int disp, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%s[%X[PC]]", opsize & 0x80 ? "@" : "", pc+disp);
}

void v60_disassembler::out_AM_PCDisplacementIndirectIndexed(offs_t pc, int disp, int rx, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@[%X[PC]]", v60_reg_names[rx], pc+disp);
	else
		util::stream_format(stream, "[%X[PC]](%s)", pc+disp, v60_reg_names[rx]);
}

void v60_disassembler::out_AM_DoubleDisplacement(int reg, int disp2, int disp1, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%s%X%s[%s%X[%s]]",
			disp1 >= 0 ? "" : "-", disp1 >= 0 ? disp1 : -disp1,
			opsize & 0x80 ? "@" : "",
			disp2 >= 0 ? "" : "-", disp2 >= 0 ? disp2 : -disp2,
			v60_reg_names[reg]);
}

void v60_disassembler::out_AM_PCDoubleDisplacement(offs_t pc, int disp2, int disp1, int opsize, std::ostream &stream)
{
	util::stream_format(stream, "%s%X%s[%X[PC]]",
			disp1 >= 0 ? "" : "-", disp1 >= 0 ? disp1 : -disp1,
			opsize & 0x80 ? "@" : "",
			disp2 + pc);
}

void v60_disassembler::out_AM_DirectAddress(unsigned addr, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		stream << '@';
	util::stream_format(stream, "%X", addr);
}

void v60_disassembler::out_AM_DirectAddressIndexed(unsigned addr, int rx, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@%X", v60_reg_names[rx], addr);
	else
		util::stream_format(stream, "%X(%s)", addr, v60_reg_names[rx]);
}

void v60_disassembler::out_AM_DirectAddressDeferred(unsigned addr, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		stream << '@';
	util::stream_format(stream, "[%X]", addr);
}

void v60_disassembler::out_AM_DirectAddressDeferredIndexed(unsigned addr, int rx, int opsize, std::ostream &stream)
{
	if(opsize & 0x80)
		util::stream_format(stream, "%s@[%X]", v60_reg_names[rx], addr);
	else
		util::stream_format(stream, "[%X](%s)", addr, v60_reg_names[rx]);
}

void v60_disassembler::out_AM_Immediate(unsigned value, int opsize, std::ostream &stream)
{
	if(opsize == 0)
		value &= 0xff;
	else if(opsize == 1)
		value &= 0xffff;

	util::stream_format(stream, "#%X", value);
}

u32 v60_disassembler::decode_AM(unsigned ipc, offs_t pc, int m, int opsize, const data_buffer &opcodes, std::ostream &stream)
{
	unsigned char mod = opcodes.r8(pc);
	if(m) {
		switch(mod>>5) {
		case 0: // Double displacement (8 bit)
			out_AM_DoubleDisplacement(mod&0x1F, opcodes.r8(pc+1), opcodes.r8(pc+2), opsize, stream);
			return 3;

		case 1: // Double displacement (16 bit)
			out_AM_DoubleDisplacement(mod&0x1F, opcodes.r16(pc+1), opcodes.r16(pc+3), opsize, stream);
			return 5;

		case 2: // Double displacement (32 bit)
			out_AM_DoubleDisplacement(mod&0x1F, opcodes.r32(pc+1), opcodes.r32(pc+5), opsize, stream);
			return 9;

		case 3: // Register
			out_AM_Register(mod&0x1F, stream);
			return 1;

		case 4: // Autoincrement
			out_AM_Autoincrement(mod&0x1F, opsize, stream);
			return 1;

		case 5: // Autodecrement
			out_AM_Autodecrement(mod&0x1F, opsize, stream);
			return 1;

		case 6:
			switch (opcodes.r8(pc+1)>>5)
				{
				case 0: // Displacement indexed (8 bit)
					out_AM_DisplacementIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r8(pc+2), opsize, stream);
					return 3;

				case 1: // Displacement indexed (16 bit)
					out_AM_DisplacementIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r16(pc+2), opsize, stream);
					return 4;

				case 2: // Displacement indexed (32 bit)
					out_AM_DisplacementIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r32(pc+2), opsize, stream);
					return 6;

				case 3: // Register indirect indexed
					out_AM_RegisterIndirectIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opsize, stream);
					return 2;

				case 4: // Displacement indirect indexed (8 bit)
					out_AM_DisplacementIndirectIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r8(pc+2), opsize, stream);
					return 3;

				case 5: // Displacement indirect indexed (16 bit)
					out_AM_DisplacementIndirectIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r16(pc+2), opsize, stream);
					return 4;

				case 6: // Displacement indirect indexed (32 bit)
					out_AM_DisplacementIndirectIndexed(opcodes.r8(pc+1)&0x1F, mod&0x1F, opcodes.r32(pc+2), opsize, stream);
					return 6;

				case 7:
					switch (opcodes.r8(pc+1)&0x1F)
						{
						case 16: // PC Displacement Indexed (8 bit)
							out_AM_PCDisplacementIndexed(ipc, opcodes.r8(pc+2), mod&0x1F, opsize, stream);
							return 3;

						case 17: // PC Displacement Indexed (16 bit)
							out_AM_PCDisplacementIndexed(ipc, opcodes.r16(pc+2), mod&0x1F, opsize, stream);
							return 4;

						case 18: // PC Displacement Indexed (32 bit)
							out_AM_PCDisplacementIndexed(ipc, opcodes.r32(pc+2), mod&0x1F, opsize, stream);
							return 6;

						case 19: // Direct Address Indexed
							out_AM_DirectAddressIndexed(opcodes.r32(pc+2), mod&0x1F, opsize, stream);
							return 6;

						case 24: // PC Displacement Indirect Indexed(8 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, opcodes.r8(pc+2), mod&0x1F, opsize, stream);
							return 3;

						case 25: // PC Displacement Indirect Indexed (16 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, opcodes.r16(pc+2), mod&0x1F, opsize, stream);
							return 4;

						case 26: // PC Displacement Indirect Indexed (32 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, opcodes.r32(pc+2), mod&0x1F, opsize, stream);
							return 6;

						case 27: // Direct Address Deferred Indexed
							out_AM_DirectAddressDeferredIndexed(opcodes.r32(pc+2), mod&0x1F, opsize, stream);
							return 6;

						default:
							stream << "!ERRAM3";
							return 1;
						}

				default:
					stream << "!ERRAM2";
					return 1;
				}

		default:
			stream << "!ERRAM1";
			return 1;
		}
	} else {
		switch(mod>>5) {
		case 0: // Displacement (8 bit)
			out_AM_Displacement(mod&0x1F, opcodes.r8(pc+1), opsize, stream);
			return 2;

		case 1: // Displacement (16 bit)
			out_AM_Displacement(mod&0x1F, opcodes.r16(pc+1), opsize, stream);
			return 3;

		case 2: // Displacement (32 bit)
			out_AM_Displacement(mod&0x1F, opcodes.r32(pc+1), opsize, stream);
			return 5;

		case 3: // Register indirect
			out_AM_RegisterIndirect(mod&0x1F, opsize, stream);
			return 1;

		case 4: // Displacement indirect (8 bit)
			out_AM_DisplacementIndirect(mod&0x1F, opcodes.r8(pc+1), opsize, stream);
			return 2;

		case 5: // Displacement indirect (16 bit)
			out_AM_DisplacementIndirect(mod&0x1F, opcodes.r16(pc+1), opsize, stream);
			return 3;

		case 6: // Displacement indirect (32 bit)
			out_AM_DisplacementIndirect(mod&0x1F, opcodes.r32(pc+1), opsize, stream);
			return 5;

		case 7:
			switch(mod&0x1F) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				out_AM_Immediate(mod&0x1F, opsize, stream);
				return 1;

			case 16: // PC Displacement (8 bit)
				out_AM_PCDisplacement(ipc, opcodes.r8(pc+1), opsize, stream);
				return 2;

			case 17: // PC Displacement (16 bit)
				out_AM_PCDisplacement(ipc, opcodes.r16(pc+1), opsize, stream);
				return 3;

			case 18: // PC Displacement (32 bit)
				out_AM_PCDisplacement(ipc, opcodes.r32(pc+1), opsize, stream);
				return 5;

			case 19: // Direct Address
				out_AM_DirectAddress(opcodes.r32(pc+1), opsize, stream);
				return 5;


			case 20:
				switch(opsize&0x7F) {
				case 0: // Immediate (8 bit)
					out_AM_Immediate(opcodes.r8(pc+1), opsize, stream);
					return 2;

				case 1: // Immediate (16 bit)
					out_AM_Immediate(opcodes.r16(pc+1), opsize, stream);
					return 3;

				case 2: // Immediate (32 bit)
					out_AM_Immediate(opcodes.r32(pc+1), opsize, stream);
					return 5;

				default:
					stream << "!ERRAM6";
					return 1;
				}

			case 24: // PC Displacement Indirect (8 bit)
				out_AM_PCDisplacementIndirect(ipc, opcodes.r8(pc+1), opsize, stream);
				return 2;

			case 25: // PC Displacement Indirect (16 bit)
				out_AM_PCDisplacementIndirect(ipc, opcodes.r16(pc+1), opsize, stream);
				return 3;

			case 26: // PC Displacement Indirect (32 bit)
				out_AM_PCDisplacementIndirect(ipc, opcodes.r32(pc+1), opsize, stream);
				return 5;

			case 27: // Direct Address Deferred
				out_AM_DirectAddressDeferred(opcodes.r32(pc+1), opsize, stream);
				return 5;

			case 28: // PC Double Displacement (8 bit)
				out_AM_PCDoubleDisplacement(ipc, opcodes.r8(pc+1), opcodes.r8(pc+2), opsize, stream);
				return 3;

			case 29: // PC Double Displacement (16 bit)
				out_AM_PCDoubleDisplacement(ipc, opcodes.r16(pc+1), opcodes.r16(pc+3), opsize, stream);
				return 5;

			case 30: // PC Double Displacement (32 bit)
				out_AM_PCDoubleDisplacement(ipc, opcodes.r32(pc+1), opcodes.r32(pc+5), opsize, stream);
				return 9;

			default:
				stream << "!ERRAM5";
				return 1;
			}

		default:
			stream << "!ERRAM4";
			return 1;
		}
	}
}


u32 v60_disassembler::decode_F1(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	unsigned char code = opcodes.r8(pc);
	util::stream_format(stream, "%-8s", opnm);
	if(code & 0x20) {
		int ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream) + 2;
		stream << ", ";
		out_AM_Register(code & 0x1f, stream);
		return ret;
	} else {
		out_AM_Register(code & 0x1f, stream);
		stream << ", ";
		return decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream) + 2;
	}
}

u32 v60_disassembler::decode_F2(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	int ret;
	unsigned char code = opcodes.r8(pc);
	util::stream_format(stream, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream);
	stream << ", ";
	ret += decode_AM(ipc, pc+1+ret, code & 0x20, opsize2, opcodes, stream);
	return ret+2;
}

u32 v60_disassembler::decode_F1F2(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	if(opcodes.r8(pc) & 0x80)
		return decode_F2(opnm, opsize1, opsize2, ipc, pc, opcodes, stream);
	else
		return decode_F1(opnm, opsize1, opsize2, ipc, pc, opcodes, stream);
}

u32 v60_disassembler::decode_F3(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "%-8s", opnm);
	return decode_AM(ipc, pc, opcodes.r8(pc-1) & 1, opsize1, opcodes, stream) + 1;
}

u32 v60_disassembler::decode_F4a(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "%-8s%X", opnm, ipc + static_cast<int8_t>(opcodes.r8(pc)));
	return 2;
}

u32 v60_disassembler::decode_F4b(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "%-8s%X", opnm, ipc + static_cast<int16_t>(opcodes.r16(pc)));
	return 3;
}

u32 v60_disassembler::decode_F5(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	stream << opnm;
	return 1;
}

u32 v60_disassembler::decode_F6(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "%-8s%s, %X[PC]", opnm, v60_reg_names[opcodes.r8(pc) & 0x1f], ipc+opcodes.r16(pc+1));
	return 4;
}

u32 v60_disassembler::decode_F7a(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	int ret;
	unsigned char code = opcodes.r8(pc);
	unsigned char code2;

	util::stream_format(stream, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream);
	stream << ", ";

	code2 = opcodes.r8(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, stream);
	else
		out_AM_Immediate(code2, 1, stream);
	stream << ", ";

	ret += decode_AM(ipc, pc+2+ret, code & 0x20, opsize2, opcodes, stream);
	stream << ", ";

	code2 = opcodes.r8(pc+2+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, stream);
	else
		out_AM_Immediate(code2, 1, stream);

	return ret+4;
}

u32 v60_disassembler::decode_F7b(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	int ret;
	unsigned char code = opcodes.r8(pc);
	unsigned char code2;

	util::stream_format(stream, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream);
	stream << ", ";

	code2 = opcodes.r8(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, stream);
	else
		out_AM_Immediate(code2, 1, stream);
	stream << ", ";

	ret += decode_AM(ipc, pc+2+ret, code & 0x20, opsize2, opcodes, stream);

	return ret+3;
}

u32 v60_disassembler::decode_F7c(const char *opnm, int opsize1, int opsize2, unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	int ret;
	unsigned char code = opcodes.r8(pc);
	unsigned char code2;

	util::stream_format(stream, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, opcodes, stream);
	stream << ", ";

	ret += decode_AM(ipc, pc+1+ret, code & 0x20, opsize2, opcodes, stream);
	stream << ", ";

	code2 = opcodes.r8(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, stream);
	else
		out_AM_Immediate(code2, 1, stream);

	return ret+3;
}

u32 v60_disassembler::dopUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$%02X", opcodes.r8(pc));
	return 1;
}

u32 v60_disassembler::dop58UNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$58");
	return 1;
}

u32 v60_disassembler::dop59UNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$59");
	return 1;
}

u32 v60_disassembler::dop5AUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5A");
	return 1;
}

u32 v60_disassembler::dop5BUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5B");
	return 1;
}

u32 v60_disassembler::dop5CUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5C");
	return 1;
}

u32 v60_disassembler::dop5DUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5D");
	return 1;
}

u32 v60_disassembler::dop5EUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5E");
	return 1;
}

u32 v60_disassembler::dop5FUNHANDLED(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	util::stream_format(stream, "$5F");
	return 1;
}

#define DEFINE_EASY_OPCODE(name, opnm, ftype, opsize1, opsize2) \
	u32 v60_disassembler::dop ## name(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) \
	{ \
		return decode_ ## ftype(opnm, opsize1, opsize2, ipc, pc, opcodes, stream); \
	}

#define DEFINE_EASY_OPCODE_EX(name, opnm, ftype, opsize1, opsize2, flags) \
	u32 v60_disassembler::dop ## name(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) \
	{ \
		return decode_ ## ftype(opnm, opsize1, opsize2, ipc, pc, opcodes, stream) | (flags); \
	}

#define DEFINE_TRIPLE_OPCODE(name, string, ftype) \
	DEFINE_EASY_OPCODE(name##B,string ".b", ftype, 0, 0) \
	DEFINE_EASY_OPCODE(name##H,string ".h", ftype, 1, 1) \
	DEFINE_EASY_OPCODE(name##W,string ".w", ftype, 2, 2)

#define DEFINE_DOUBLE_OPCODE(name, string, ftype) \
	DEFINE_EASY_OPCODE(name##B,string ".b", ftype, 0, 0) \
	DEFINE_EASY_OPCODE(name##H,string ".h", ftype, 1, 1)

#define DEFINE_FPU_OPCODE(name,string,ftype) \
	DEFINE_EASY_OPCODE(name##S,string ".s", ftype, 2, 2) \
	DEFINE_EASY_OPCODE(name##L,string ".l", ftype, 2, 2)


DEFINE_FPU_OPCODE(ABSF, "absf", F2)
DEFINE_TRIPLE_OPCODE(ADD, "add", F1F2)
DEFINE_TRIPLE_OPCODE(ADDC, "addc", F1F2)
DEFINE_EASY_OPCODE(ADDDC, "adddc", F7c, 0, 0)
DEFINE_FPU_OPCODE(ADDF, "addf", F2)
DEFINE_TRIPLE_OPCODE(AND, "and", F1F2)
DEFINE_EASY_OPCODE(ANDBSU, "andbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ANDBSD, "andbsd", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ANDNBSU, "andnbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ANDNBSD, "andnbsd", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(BGT8, "bgt", F4a, 0, 0)
DEFINE_EASY_OPCODE(BGT16, "bgt", F4b, 0, 0)
DEFINE_EASY_OPCODE(BGE8, "bge", F4a, 0, 0)
DEFINE_EASY_OPCODE(BGE16, "bge", F4b, 0, 0)
DEFINE_EASY_OPCODE(BLT8, "blt", F4a, 0, 0)
DEFINE_EASY_OPCODE(BLT16, "blt", F4b, 0, 0)
DEFINE_EASY_OPCODE(BLE8, "ble", F4a, 0, 0)
DEFINE_EASY_OPCODE(BLE16, "ble", F4b, 0, 0)
DEFINE_EASY_OPCODE(BH8, "bh", F4a, 0, 0)
DEFINE_EASY_OPCODE(BH16, "bh", F4b, 0, 0)
DEFINE_EASY_OPCODE(BNL8, "bnl", F4a, 0, 0)
DEFINE_EASY_OPCODE(BNL16, "bnl", F4b, 0, 0)
DEFINE_EASY_OPCODE(BL8, "bl", F4a, 0, 0)
DEFINE_EASY_OPCODE(BL16, "bl", F4b, 0, 0)
DEFINE_EASY_OPCODE(BNH8, "bnh", F4a, 0, 0)
DEFINE_EASY_OPCODE(BNH16, "bnh", F4b, 0, 0)
DEFINE_EASY_OPCODE(BE8, "be", F4a, 0, 0)
DEFINE_EASY_OPCODE(BE16, "be", F4b, 0, 0)
DEFINE_EASY_OPCODE(BNE8, "bne", F4a, 0, 0)
DEFINE_EASY_OPCODE(BNE16, "bne", F4b, 0, 0)
DEFINE_EASY_OPCODE(BV8, "bv", F4a, 0, 0)
DEFINE_EASY_OPCODE(BV16, "bv", F4b, 0, 0)
DEFINE_EASY_OPCODE(BNV8, "bnv", F4a, 0, 0)
DEFINE_EASY_OPCODE(BNV16, "bnv", F4b, 0, 0)
DEFINE_EASY_OPCODE(BN8, "bn", F4a, 0, 0)
DEFINE_EASY_OPCODE(BN16, "bn", F4b, 0, 0)
DEFINE_EASY_OPCODE(BP8, "bp", F4a, 0, 0)
DEFINE_EASY_OPCODE(BP16, "bp", F4b, 0, 0)
DEFINE_EASY_OPCODE(BR8, "br", F4a, 0, 0)
DEFINE_EASY_OPCODE(BR16, "br", F4b, 0, 0)
DEFINE_EASY_OPCODE(BRK, "brk", F5, 0, 0)
DEFINE_EASY_OPCODE(BRKV, "brkv", F5, 0, 0)
DEFINE_EASY_OPCODE_EX(BSR, "bsr", F4b, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(CALL, "call", F1F2, 0, 2, STEP_OVER)
DEFINE_EASY_OPCODE(CAXI, "caxi", F1, 2, 2)
DEFINE_EASY_OPCODE(CHKAR, "chkar", F1F2, 0, 0) // ?
DEFINE_EASY_OPCODE(CHKAW, "chkaw", F1F2, 0, 0) // ?
DEFINE_EASY_OPCODE(CHKAE, "chkae", F1F2, 0, 0)
DEFINE_EASY_OPCODE(CHLVL, "chlvl", F1F2, 0, 0)
DEFINE_EASY_OPCODE(CLR1, "clr1", F1F2, 2, 2)
DEFINE_EASY_OPCODE(CLRTLB, "clrtlb", F3, 0, 0) // ?
DEFINE_EASY_OPCODE(CLRTLBA, "clrtlba", F5, 0, 0)
DEFINE_TRIPLE_OPCODE(CMP, "cmp", F1F2)
DEFINE_EASY_OPCODE(CMPBFS, "cmpbfs", F7b, 0x82, 2)
DEFINE_EASY_OPCODE(CMPBFZ, "cmpbfz", F7b, 0x82, 2)
DEFINE_EASY_OPCODE(CMPBFL, "cmpbfl", F7b, 0x82, 2)
DEFINE_DOUBLE_OPCODE(CMPC, "cmpc", F7a)
DEFINE_DOUBLE_OPCODE(CMPCF, "cmpcf", F7a)
DEFINE_DOUBLE_OPCODE(CMPCS, "cmpcs", F7a)
DEFINE_FPU_OPCODE(CMPF, "cmpf", F2)
DEFINE_EASY_OPCODE(CVTSL, "cvt.sl", F2, 0, 1)
DEFINE_EASY_OPCODE(CVTLS, "cvt.ls", F2, 1, 0)
DEFINE_EASY_OPCODE(CVTWS, "cvt.ws", F2, 2, 0)
DEFINE_EASY_OPCODE(CVTWL, "cvt.wl", F2, 2, 1)
DEFINE_EASY_OPCODE(CVTSW, "cvt.sw", F2, 0, 2)
DEFINE_EASY_OPCODE(CVTLW, "cvt.lw", F2, 1, 2)
DEFINE_EASY_OPCODE(CVTDPZ, "cvtd.pz", F7c, 0, 1)
DEFINE_EASY_OPCODE(CVTDZP, "cvtd.zp", F7c, 1, 0)
DEFINE_EASY_OPCODE_EX(DBGT, "dbgt", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBGE, "dbge", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBLT, "dbgt", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBLE, "dbge", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBH, "dbh", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNL, "dbnl", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBL, "dbl", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNH, "dbnh", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBE, "dbe", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNE, "dbne", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBV, "dbe", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNV, "dbne", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBN, "dbn", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBP, "dbp", F6, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBR, "dbr", F6, 0, 0, STEP_OVER)
DEFINE_TRIPLE_OPCODE(DEC, "dec", F3)
DEFINE_EASY_OPCODE(DISPOSE, "dispose", F5, 0, 0)
DEFINE_TRIPLE_OPCODE(DIV, "div", F1F2)
DEFINE_FPU_OPCODE(DIVF, "divf", F2)
DEFINE_TRIPLE_OPCODE(DIVU, "divu", F1F2)
DEFINE_EASY_OPCODE(DIVX, "divx", F1F2, 2, 3)
DEFINE_EASY_OPCODE(DIVUX, "divux", F1F2, 2, 3)
DEFINE_EASY_OPCODE(EXTBFS, "extbfs", F7b, 0x82, 2)
DEFINE_EASY_OPCODE(EXTBFZ, "extbfz", F7b, 0x82, 2)
DEFINE_EASY_OPCODE(EXTBFL, "extbfl", F7b, 0x82, 2)
DEFINE_EASY_OPCODE(GETATE, "getate", F1F2, 0, 3) // ?
DEFINE_EASY_OPCODE(GETPSW, "getpsw", F3, 2, 0)
DEFINE_EASY_OPCODE(GETPTE, "getpte", F1F2, 0, 2) // ?
DEFINE_EASY_OPCODE(GETRA, "getra", F1F2, 0, 2) // ?
DEFINE_EASY_OPCODE(HALT, "halt", F5, 0, 0)
DEFINE_TRIPLE_OPCODE(IN, "in", F1F2)
DEFINE_TRIPLE_OPCODE(INC, "inc", F3)
DEFINE_EASY_OPCODE(INSBFL, "insbfl", F7c, 2, 0x82)
DEFINE_EASY_OPCODE(INSBFR, "insbfr", F7c, 2, 0x82)
DEFINE_EASY_OPCODE(JMP, "jmp", F3, 0, 0)
DEFINE_EASY_OPCODE_EX(JSR, "jsr", F3, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE(LDPR, "ldpr", F1F2, 2, 2)
DEFINE_EASY_OPCODE(LDTASK, "ldtask", F1F2, 2, 2)
DEFINE_TRIPLE_OPCODE(MOV, "mov", F1F2)
DEFINE_EASY_OPCODE(MOVBSU, "movbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(MOVBSD, "movbsd", F7b, 0x80, 0x80)
DEFINE_DOUBLE_OPCODE(MOVCU, "movcu", F7a)
DEFINE_DOUBLE_OPCODE(MOVCD, "movcd", F7a)
DEFINE_DOUBLE_OPCODE(MOVCFU, "movcfu", F7a)
DEFINE_DOUBLE_OPCODE(MOVCFD, "movcfd", F7a)
DEFINE_DOUBLE_OPCODE(MOVCS, "movcs", F7a)
DEFINE_EASY_OPCODE(MOVD, "mov.d", F1F2, 3, 3)
DEFINE_EASY_OPCODE(MOVEAB, "movea.b", F1F2, 0, 2)
DEFINE_EASY_OPCODE(MOVEAH, "movea.h", F1F2, 1, 2)
DEFINE_EASY_OPCODE(MOVEAW, "movea.w", F1F2, 2, 2)
DEFINE_FPU_OPCODE(MOVF, "movf", F2)
DEFINE_EASY_OPCODE(MOVSBH, "movs.bh", F1F2, 0, 1)
DEFINE_EASY_OPCODE(MOVSBW, "movs.bw", F1F2, 0, 2)
DEFINE_EASY_OPCODE(MOVSHW, "movs.hw", F1F2, 1, 2)
DEFINE_EASY_OPCODE(MOVTHB, "movt.hb", F1F2, 1, 0)
DEFINE_EASY_OPCODE(MOVTWB, "movt.wb", F1F2, 2, 0)
DEFINE_EASY_OPCODE(MOVTWH, "movt.wh", F1F2, 2, 1)
DEFINE_EASY_OPCODE(MOVZBH, "movz.bh", F1F2, 0, 1)
DEFINE_EASY_OPCODE(MOVZBW, "movz.bw", F1F2, 0, 2)
DEFINE_EASY_OPCODE(MOVZHW, "movz.hw", F1F2, 1, 2)
DEFINE_TRIPLE_OPCODE(MUL, "mul", F1F2)
DEFINE_FPU_OPCODE(MULF, "mulf", F2)
DEFINE_TRIPLE_OPCODE(MULU, "mulu", F1F2)
DEFINE_EASY_OPCODE(MULX, "mulx", F1F2, 2, 3)
DEFINE_EASY_OPCODE(MULUX, "mulux", F1F2, 2, 3)
DEFINE_TRIPLE_OPCODE(NEG, "neg", F1F2)
DEFINE_FPU_OPCODE(NEGF, "negf", F2)
DEFINE_EASY_OPCODE(NOP, "nop", F5, 0, 0)
DEFINE_TRIPLE_OPCODE(NOT, "not", F1F2)
DEFINE_EASY_OPCODE(NOT1, "not1", F1F2, 2, 2)
DEFINE_EASY_OPCODE(NOTBSU, "notbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(NOTBSD, "notbsd", F7b, 0x80, 0x80)
DEFINE_TRIPLE_OPCODE(OR, "or", F1F2)
DEFINE_EASY_OPCODE(ORBSU, "orbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ORBSD, "orbsd", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ORNBSU, "ornbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(ORNBSD, "ornbsd", F7b, 0x80, 0x80)
DEFINE_TRIPLE_OPCODE(OUT, "out", F1F2)
DEFINE_EASY_OPCODE(POP, "pop", F3, 2, 0)
DEFINE_EASY_OPCODE(POPM, "popm", F3, 2, 0)
DEFINE_EASY_OPCODE(PREPARE, "prepare", F3, 2, 0)
DEFINE_EASY_OPCODE(PUSH, "push", F3, 2, 0)
DEFINE_EASY_OPCODE(PUSHM, "pushm", F3, 2, 0)
DEFINE_TRIPLE_OPCODE(REM, "rem", F1F2)
DEFINE_TRIPLE_OPCODE(REMU, "remu", F1F2)
DEFINE_EASY_OPCODE_EX(RET, "ret", F3, 2, 0, STEP_OUT)
DEFINE_EASY_OPCODE_EX(RETIU, "retiu", F3, 1, 0, STEP_OUT)
DEFINE_EASY_OPCODE_EX(RETIS, "retis", F3, 1, 0, STEP_OUT)
DEFINE_EASY_OPCODE(ROTB, "rot.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(ROTH, "rot.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(ROTW, "rot.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE(ROTCB, "rotc.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(ROTCH, "rotc.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(ROTCW, "rotc.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE_EX(RSR, "rsr", F5, 0, 0, STEP_OUT)
DEFINE_EASY_OPCODE(RVBIT, "rvbit", F1F2, 0, 0)
DEFINE_EASY_OPCODE(RVBYT, "rvbyt", F1F2, 2, 2)
DEFINE_EASY_OPCODE(SCH0BSU, "sch0bsu", F7b, 0x80, 2)
DEFINE_EASY_OPCODE(SCH0BSD, "sch0bsd", F7b, 0x80, 2)
DEFINE_EASY_OPCODE(SCH1BSU, "sch1bsu", F7b, 0x80, 2)
DEFINE_EASY_OPCODE(SCH1BSD, "sch1bsd", F7b, 0x80, 2)
DEFINE_EASY_OPCODE(SCHCUB, "schcu.b", F7b, 0, 0)
DEFINE_EASY_OPCODE(SCHCDB, "schcd.b", F7b, 0, 0)
DEFINE_EASY_OPCODE(SCHCUH, "schcu.h", F7b, 1, 1)
DEFINE_EASY_OPCODE(SCHCDH, "schcd.h", F7b, 1, 1)
DEFINE_EASY_OPCODE(SCLFS, "sclf.s", F2, 1, 2)
DEFINE_EASY_OPCODE(SCLFL, "sclf.l", F2, 1, 2)
DEFINE_EASY_OPCODE(SET1, "set1", F1F2, 2, 2)
DEFINE_EASY_OPCODE(SETF, "setf", F1F2, 0, 0)
DEFINE_EASY_OPCODE(SHAB, "sha.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(SHAH, "sha.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(SHAW, "sha.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE(SHLB, "shl.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(SHLH, "shl.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(SHLW, "shl.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE(SKPCUB, "skpcu.b", F7b, 0, 0)
DEFINE_EASY_OPCODE(SKPCDB, "skpcd.b", F7b, 0, 0)
DEFINE_EASY_OPCODE(SKPCUH, "skpcu.h", F7b, 1, 1)
DEFINE_EASY_OPCODE(SKPCDH, "skpcd.h", F7b, 1, 1)
DEFINE_EASY_OPCODE(STPR, "stpr", F1F2, 2, 2)
DEFINE_EASY_OPCODE(STTASK, "sttask", F3, 2, 0)
DEFINE_TRIPLE_OPCODE(SUB, "sub", F1F2)
DEFINE_TRIPLE_OPCODE(SUBC, "subc", F1F2)
DEFINE_EASY_OPCODE(SUBDC, "subdc", F7c, 0, 0)
DEFINE_EASY_OPCODE(SUBRDC, "subrdc", F7c, 0, 0)
DEFINE_FPU_OPCODE(SUBF, "subf", F2)
DEFINE_EASY_OPCODE(TASI, "tasi", F3, 0, 0)
DEFINE_EASY_OPCODE(TB, "tb", F6, 0, 0)
DEFINE_TRIPLE_OPCODE(TEST, "test", F3)
DEFINE_EASY_OPCODE(TEST1, "test1", F1F2, 2, 2)
DEFINE_EASY_OPCODE_EX(TRAP, "trap", F3, 0, 0, STEP_OVER)
DEFINE_EASY_OPCODE(TRAPFL, "trapfl", F5, 0, 0)
DEFINE_EASY_OPCODE(UPDATE, "update", F1F2, 0, 3) // ?
DEFINE_EASY_OPCODE(UPDPSWH, "updpsw.h", F1F2, 2, 2)
DEFINE_EASY_OPCODE(UPDPSWW, "updpsw.w", F1F2, 2, 2)
DEFINE_EASY_OPCODE(UPDPTE, "updpte", F1F2, 0, 2) // ?
DEFINE_TRIPLE_OPCODE(XCH, "xch", F1)
DEFINE_TRIPLE_OPCODE(XOR, "xor", F1F2)
DEFINE_EASY_OPCODE(XORBSU, "xorbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(XORBSD, "xorbsd", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(XORNBSU, "xornbsu", F7b, 0x80, 0x80)
DEFINE_EASY_OPCODE(XORNBSD, "xornbsd", F7b, 0x80, 0x80)

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_58[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCMPCB,
	/* 0x01 */ &v60_disassembler::dopCMPCFB,
	/* 0x02 */ &v60_disassembler::dopCMPCSB,
	/* 0x03 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x04 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x05 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x06 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x07 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x08 */ &v60_disassembler::dopMOVCUB,
	/* 0x09 */ &v60_disassembler::dopMOVCDB,
	/* 0x0A */ &v60_disassembler::dopMOVCFUB,
	/* 0x0B */ &v60_disassembler::dopMOVCFDB,
	/* 0x0C */ &v60_disassembler::dopMOVCSB,
	/* 0x0D */ &v60_disassembler::dop58UNHANDLED,
	/* 0x0E */ &v60_disassembler::dop58UNHANDLED,
	/* 0x0F */ &v60_disassembler::dop58UNHANDLED,
	/* 0x10 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x11 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x12 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x13 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x14 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x15 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x16 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x17 */ &v60_disassembler::dop58UNHANDLED,
	/* 0x18 */ &v60_disassembler::dopSCHCUB,
	/* 0x19 */ &v60_disassembler::dopSCHCDB,
	/* 0x1A */ &v60_disassembler::dopSKPCUB,
	/* 0x1B */ &v60_disassembler::dopSKPCDB,
	/* 0x1C */ &v60_disassembler::dop58UNHANDLED,
	/* 0x1D */ &v60_disassembler::dop58UNHANDLED,
	/* 0x1E */ &v60_disassembler::dop58UNHANDLED,
	/* 0x1F */ &v60_disassembler::dop58UNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_59[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopADDDC,
	/* 0x01 */ &v60_disassembler::dopSUBDC,
	/* 0x02 */ &v60_disassembler::dopSUBRDC,
	/* 0x03 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x04 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x05 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x06 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x07 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x08 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x09 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0A */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0B */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0C */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0D */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0E */ &v60_disassembler::dop59UNHANDLED,
	/* 0x0F */ &v60_disassembler::dop59UNHANDLED,
	/* 0x10 */ &v60_disassembler::dopCVTDPZ,
	/* 0x11 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x12 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x13 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x14 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x15 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x16 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x17 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x18 */ &v60_disassembler::dopCVTDZP,
	/* 0x19 */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1A */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1B */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1C */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1D */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1E */ &v60_disassembler::dop59UNHANDLED,
	/* 0x1F */ &v60_disassembler::dop59UNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5A[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCMPCH,
	/* 0x01 */ &v60_disassembler::dopCMPCFH,
	/* 0x02 */ &v60_disassembler::dopCMPCSH,
	/* 0x03 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x04 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopMOVCUH,
	/* 0x09 */ &v60_disassembler::dopMOVCDH,
	/* 0x0A */ &v60_disassembler::dopMOVCFUH,
	/* 0x0B */ &v60_disassembler::dopMOVCFDH,
	/* 0x0C */ &v60_disassembler::dopMOVCSH,
	/* 0x0D */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x10 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x11 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x12 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x13 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x14 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x15 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x16 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x17 */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x18 */ &v60_disassembler::dopSCHCUH,
	/* 0x19 */ &v60_disassembler::dopSCHCDH,
	/* 0x1A */ &v60_disassembler::dopSKPCUH,
	/* 0x1B */ &v60_disassembler::dopSKPCDH,
	/* 0x1C */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5AUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5AUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5B[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopSCH0BSU,
	/* 0x01 */ &v60_disassembler::dopSCH0BSD,
	/* 0x02 */ &v60_disassembler::dopSCH1BSU,
	/* 0x03 */ &v60_disassembler::dopSCH1BSD,
	/* 0x04 */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopMOVBSU,
	/* 0x09 */ &v60_disassembler::dopMOVBSD,
	/* 0x0A */ &v60_disassembler::dopNOTBSU,
	/* 0x0B */ &v60_disassembler::dopNOTBSD,
	/* 0x0C */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x0D */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x10 */ &v60_disassembler::dopANDBSU,
	/* 0x11 */ &v60_disassembler::dopANDBSD,
	/* 0x12 */ &v60_disassembler::dopANDNBSU,
	/* 0x13 */ &v60_disassembler::dopANDNBSD,
	/* 0x14 */ &v60_disassembler::dopORBSU,
	/* 0x15 */ &v60_disassembler::dopORBSD,
	/* 0x16 */ &v60_disassembler::dopORNBSU,
	/* 0x17 */ &v60_disassembler::dopORNBSD,
	/* 0x18 */ &v60_disassembler::dopXORBSU,
	/* 0x19 */ &v60_disassembler::dopXORBSD,
	/* 0x1A */ &v60_disassembler::dopXORNBSU,
	/* 0x1B */ &v60_disassembler::dopXORNBSD,
	/* 0x1C */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5BUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5BUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5C[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCMPFS,
	/* 0x01 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x02 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x03 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x04 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopMOVFS,
	/* 0x09 */ &v60_disassembler::dopNEGFS,
	/* 0x0A */ &v60_disassembler::dopABSFS,
	/* 0x0B */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x0C */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x0D */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x10 */ &v60_disassembler::dopSCLFS,
	/* 0x11 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x12 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x13 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x14 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x15 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x16 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x17 */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x18 */ &v60_disassembler::dopADDFS,
	/* 0x19 */ &v60_disassembler::dopSUBFS,
	/* 0x1A */ &v60_disassembler::dopMULFS,
	/* 0x1B */ &v60_disassembler::dopDIVFS,
	/* 0x1C */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5CUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5CUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5D[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCMPBFS,
	/* 0x01 */ &v60_disassembler::dopCMPBFZ,
	/* 0x02 */ &v60_disassembler::dopCMPBFL,
	/* 0x03 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x04 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopEXTBFS,
	/* 0x09 */ &v60_disassembler::dopEXTBFZ,
	/* 0x0A */ &v60_disassembler::dopEXTBFL,
	/* 0x0B */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x0C */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x0D */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x10 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x11 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x12 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x13 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x14 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x15 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x16 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x17 */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x18 */ &v60_disassembler::dopINSBFR,
	/* 0x19 */ &v60_disassembler::dopINSBFL,
	/* 0x1A */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x1B */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x1C */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5DUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5DUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5E[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCMPFL,
	/* 0x01 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x02 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x03 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x04 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopMOVFL,
	/* 0x09 */ &v60_disassembler::dopNEGFL,
	/* 0x0A */ &v60_disassembler::dopABSFL,
	/* 0x0B */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x0C */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x0D */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x10 */ &v60_disassembler::dopSCLFL,
	/* 0x11 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x12 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x13 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x14 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x15 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x16 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x17 */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x18 */ &v60_disassembler::dopADDFL,
	/* 0x19 */ &v60_disassembler::dopSUBFL,
	/* 0x1A */ &v60_disassembler::dopMULFL,
	/* 0x1B */ &v60_disassembler::dopDIVFL,
	/* 0x1C */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5EUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5EUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_5F[32])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopCVTWS,
	/* 0x01 */ &v60_disassembler::dopCVTSW,
	/* 0x02 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x03 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x04 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x05 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x06 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x07 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopCVTLS,
	/* 0x09 */ &v60_disassembler::dopCVTLW,
	/* 0x0A */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x0B */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x0C */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x0D */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x0E */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x0F */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x10 */ &v60_disassembler::dopCVTSL,
	/* 0x11 */ &v60_disassembler::dopCVTWL,
	/* 0x12 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x13 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x14 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x15 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x16 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x17 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x18 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x19 */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1A */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1B */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1C */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1D */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1E */ &v60_disassembler::dop5FUNHANDLED,
	/* 0x1F */ &v60_disassembler::dop5FUNHANDLED
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_C6[8])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x0 */ &v60_disassembler::dopDBV,
	/* 0x1 */ &v60_disassembler::dopDBL,
	/* 0x2 */ &v60_disassembler::dopDBE,
	/* 0x3 */ &v60_disassembler::dopDBNH,
	/* 0x4 */ &v60_disassembler::dopDBN,
	/* 0x5 */ &v60_disassembler::dopDBR,
	/* 0x6 */ &v60_disassembler::dopDBLT,
	/* 0x7 */ &v60_disassembler::dopDBLE
};

u32 (v60_disassembler::*const v60_disassembler::dasm_optable_C7[8])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x0 */ &v60_disassembler::dopDBNV,
	/* 0x1 */ &v60_disassembler::dopDBNL,
	/* 0x2 */ &v60_disassembler::dopDBNE,
	/* 0x3 */ &v60_disassembler::dopDBH,
	/* 0x4 */ &v60_disassembler::dopDBP,
	/* 0x5 */ &v60_disassembler::dopTB,
	/* 0x6 */ &v60_disassembler::dopDBGE,
	/* 0x7 */ &v60_disassembler::dopDBGT
};

u32 v60_disassembler::dop58(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_58[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop59(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_59[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5A(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5A[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5B(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5B[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5C(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5C[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5D(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5D[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5E(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5E[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dop5F(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_5F[opcodes.r8(pc) & 0x1f])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dopC6(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_C6[opcodes.r8(pc) >> 5])(ipc, pc, opcodes, stream);
}

u32 v60_disassembler::dopC7(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream)
{
	return (this->*dasm_optable_C7[opcodes.r8(pc) >> 5])(ipc, pc, opcodes, stream);
}

u32 (v60_disassembler::*const v60_disassembler::dasm_optable[256])(unsigned ipc, offs_t pc, const data_buffer &opcodes, std::ostream &stream) =
{
	/* 0x00 */ &v60_disassembler::dopHALT,
	/* 0x01 */ &v60_disassembler::dopLDTASK,
	/* 0x02 */ &v60_disassembler::dopSTPR,
	/* 0x03 */ &v60_disassembler::dopGETRA,
	/* 0x04 */ &v60_disassembler::dopGETPTE,
	/* 0x05 */ &v60_disassembler::dopGETATE,
	/* 0x06 */ &v60_disassembler::dopUNHANDLED,
	/* 0x07 */ &v60_disassembler::dopUNHANDLED,
	/* 0x08 */ &v60_disassembler::dopRVBIT,
	/* 0x09 */ &v60_disassembler::dopMOVB,
	/* 0x0A */ &v60_disassembler::dopMOVSBH,
	/* 0x0B */ &v60_disassembler::dopMOVZBH,
	/* 0x0C */ &v60_disassembler::dopMOVSBW,
	/* 0x0D */ &v60_disassembler::dopMOVZBW,
	/* 0x0E */ &v60_disassembler::dopUNHANDLED,
	/* 0x0F */ &v60_disassembler::dopUNHANDLED,
	/* 0x10 */ &v60_disassembler::dopCLRTLBA,
	/* 0x11 */ &v60_disassembler::dopUNHANDLED,
	/* 0x12 */ &v60_disassembler::dopLDPR,
	/* 0x13 */ &v60_disassembler::dopUPDPSWW,
	/* 0x14 */ &v60_disassembler::dopUPDPTE,
	/* 0x15 */ &v60_disassembler::dopUPDATE,
	/* 0x16 */ &v60_disassembler::dopUNHANDLED,
	/* 0x17 */ &v60_disassembler::dopUNHANDLED,
	/* 0x18 */ &v60_disassembler::dopUNHANDLED,
	/* 0x19 */ &v60_disassembler::dopMOVTHB,
	/* 0x1A */ &v60_disassembler::dopUNHANDLED,
	/* 0x1B */ &v60_disassembler::dopMOVH,
	/* 0x1C */ &v60_disassembler::dopMOVSHW,
	/* 0x1D */ &v60_disassembler::dopMOVZHW,
	/* 0x1E */ &v60_disassembler::dopUNHANDLED,
	/* 0x1F */ &v60_disassembler::dopUNHANDLED,
	/* 0x20 */ &v60_disassembler::dopINB,
	/* 0x21 */ &v60_disassembler::dopOUTB,
	/* 0x22 */ &v60_disassembler::dopINH,
	/* 0x23 */ &v60_disassembler::dopOUTH,
	/* 0x24 */ &v60_disassembler::dopINW,
	/* 0x25 */ &v60_disassembler::dopOUTW,
	/* 0x26 */ &v60_disassembler::dopUNHANDLED,
	/* 0x27 */ &v60_disassembler::dopUNHANDLED,
	/* 0x28 */ &v60_disassembler::dopUNHANDLED,
	/* 0x29 */ &v60_disassembler::dopMOVTWB,
	/* 0x2A */ &v60_disassembler::dopUNHANDLED,
	/* 0x2B */ &v60_disassembler::dopMOVTWH,
	/* 0x2C */ &v60_disassembler::dopRVBYT,
	/* 0x2D */ &v60_disassembler::dopMOVW,
	/* 0x2E */ &v60_disassembler::dopUNHANDLED,
	/* 0x2F */ &v60_disassembler::dopUNHANDLED,
	/* 0x30 */ &v60_disassembler::dopUNHANDLED,
	/* 0x31 */ &v60_disassembler::dopUNHANDLED,
	/* 0x32 */ &v60_disassembler::dopUNHANDLED,
	/* 0x33 */ &v60_disassembler::dopUNHANDLED,
	/* 0x34 */ &v60_disassembler::dopUNHANDLED,
	/* 0x35 */ &v60_disassembler::dopUNHANDLED,
	/* 0x36 */ &v60_disassembler::dopUNHANDLED,
	/* 0x37 */ &v60_disassembler::dopUNHANDLED,
	/* 0x38 */ &v60_disassembler::dopNOTB,
	/* 0x39 */ &v60_disassembler::dopNEGB,
	/* 0x3A */ &v60_disassembler::dopNOTH,
	/* 0x3B */ &v60_disassembler::dopNEGH,
	/* 0x3C */ &v60_disassembler::dopNOTW,
	/* 0x3D */ &v60_disassembler::dopNEGW,
	/* 0x3E */ &v60_disassembler::dopUNHANDLED,
	/* 0x3F */ &v60_disassembler::dopMOVD,
	/* 0x40 */ &v60_disassembler::dopMOVEAB,
	/* 0x41 */ &v60_disassembler::dopXCHB,
	/* 0x42 */ &v60_disassembler::dopMOVEAH,
	/* 0x43 */ &v60_disassembler::dopXCHH,
	/* 0x44 */ &v60_disassembler::dopMOVEAW,
	/* 0x45 */ &v60_disassembler::dopXCHW,
	/* 0x46 */ &v60_disassembler::dopUNHANDLED,
	/* 0x47 */ &v60_disassembler::dopSETF,
	/* 0x48 */ &v60_disassembler::dopBSR,
	/* 0x49 */ &v60_disassembler::dopCALL,
	/* 0x4A */ &v60_disassembler::dopUPDPSWH,
	/* 0x4B */ &v60_disassembler::dopCHLVL,
	/* 0x4C */ &v60_disassembler::dopCAXI,
	/* 0x4D */ &v60_disassembler::dopCHKAR,
	/* 0x4E */ &v60_disassembler::dopCHKAW,
	/* 0x4F */ &v60_disassembler::dopCHKAE,
	/* 0x50 */ &v60_disassembler::dopREMB,
	/* 0x51 */ &v60_disassembler::dopREMUB,
	/* 0x52 */ &v60_disassembler::dopREMH,
	/* 0x53 */ &v60_disassembler::dopREMUH,
	/* 0x54 */ &v60_disassembler::dopREMW,
	/* 0x55 */ &v60_disassembler::dopREMUW,
	/* 0x56 */ &v60_disassembler::dopUNHANDLED,
	/* 0x57 */ &v60_disassembler::dopUNHANDLED,
	/* 0x58 */ &v60_disassembler::dop58,
	/* 0x59 */ &v60_disassembler::dop59,
	/* 0x5A */ &v60_disassembler::dop5A,
	/* 0x5B */ &v60_disassembler::dop5B,
	/* 0x5C */ &v60_disassembler::dop5C,
	/* 0x5D */ &v60_disassembler::dop5D,
	/* 0x5E */ &v60_disassembler::dop5E,
	/* 0x5F */ &v60_disassembler::dop5F,
	/* 0x60 */ &v60_disassembler::dopBV8,
	/* 0x61 */ &v60_disassembler::dopBNV8,
	/* 0x62 */ &v60_disassembler::dopBL8,
	/* 0x63 */ &v60_disassembler::dopBNL8,
	/* 0x64 */ &v60_disassembler::dopBE8,
	/* 0x65 */ &v60_disassembler::dopBNE8,
	/* 0x66 */ &v60_disassembler::dopBNH8,
	/* 0x67 */ &v60_disassembler::dopBH8,
	/* 0x68 */ &v60_disassembler::dopBN8,
	/* 0x69 */ &v60_disassembler::dopBP8,
	/* 0x6A */ &v60_disassembler::dopBR8,
	/* 0x6B */ &v60_disassembler::dopUNHANDLED,
	/* 0x6C */ &v60_disassembler::dopBLT8,
	/* 0x6D */ &v60_disassembler::dopBGE8,
	/* 0x6E */ &v60_disassembler::dopBLE8,
	/* 0x6F */ &v60_disassembler::dopBGT8,
	/* 0x70 */ &v60_disassembler::dopBV16,
	/* 0x71 */ &v60_disassembler::dopBNV16,
	/* 0x72 */ &v60_disassembler::dopBL16,
	/* 0x73 */ &v60_disassembler::dopBNL16,
	/* 0x74 */ &v60_disassembler::dopBE16,
	/* 0x75 */ &v60_disassembler::dopBNE16,
	/* 0x76 */ &v60_disassembler::dopBNH16,
	/* 0x77 */ &v60_disassembler::dopBH16,
	/* 0x78 */ &v60_disassembler::dopBN16,
	/* 0x79 */ &v60_disassembler::dopBP16,
	/* 0x7A */ &v60_disassembler::dopBR16,
	/* 0x7B */ &v60_disassembler::dopUNHANDLED,
	/* 0x7C */ &v60_disassembler::dopBLT16,
	/* 0x7D */ &v60_disassembler::dopBGE16,
	/* 0x7E */ &v60_disassembler::dopBLE16,
	/* 0x7F */ &v60_disassembler::dopBGT16,
	/* 0x80 */ &v60_disassembler::dopADDB,
	/* 0x81 */ &v60_disassembler::dopMULB,
	/* 0x82 */ &v60_disassembler::dopADDH,
	/* 0x83 */ &v60_disassembler::dopMULH,
	/* 0x84 */ &v60_disassembler::dopADDW,
	/* 0x85 */ &v60_disassembler::dopMULW,
	/* 0x86 */ &v60_disassembler::dopMULX,
	/* 0x87 */ &v60_disassembler::dopTEST1,
	/* 0x88 */ &v60_disassembler::dopORB,
	/* 0x89 */ &v60_disassembler::dopROTB,
	/* 0x8A */ &v60_disassembler::dopORH,
	/* 0x8B */ &v60_disassembler::dopROTH,
	/* 0x8C */ &v60_disassembler::dopORW,
	/* 0x8D */ &v60_disassembler::dopROTW,
	/* 0x8E */ &v60_disassembler::dopUNHANDLED,
	/* 0x8F */ &v60_disassembler::dopUNHANDLED,
	/* 0x90 */ &v60_disassembler::dopADDCB,
	/* 0x91 */ &v60_disassembler::dopMULUB,
	/* 0x92 */ &v60_disassembler::dopADDCH,
	/* 0x93 */ &v60_disassembler::dopMULUH,
	/* 0x94 */ &v60_disassembler::dopADDCW,
	/* 0x95 */ &v60_disassembler::dopMULUW,
	/* 0x96 */ &v60_disassembler::dopMULUX,
	/* 0x97 */ &v60_disassembler::dopSET1,
	/* 0x98 */ &v60_disassembler::dopSUBCB,
	/* 0x99 */ &v60_disassembler::dopROTCB,
	/* 0x9A */ &v60_disassembler::dopSUBCH,
	/* 0x9B */ &v60_disassembler::dopROTCH,
	/* 0x9C */ &v60_disassembler::dopSUBCW,
	/* 0x9D */ &v60_disassembler::dopROTCW,
	/* 0x9E */ &v60_disassembler::dopUNHANDLED,
	/* 0x9F */ &v60_disassembler::dopUNHANDLED,
	/* 0xA0 */ &v60_disassembler::dopANDB,
	/* 0xA1 */ &v60_disassembler::dopDIVB,
	/* 0xA2 */ &v60_disassembler::dopANDH,
	/* 0xA3 */ &v60_disassembler::dopDIVH,
	/* 0xA4 */ &v60_disassembler::dopANDW,
	/* 0xA5 */ &v60_disassembler::dopDIVW,
	/* 0xA6 */ &v60_disassembler::dopDIVX,
	/* 0xA7 */ &v60_disassembler::dopCLR1,
	/* 0xA8 */ &v60_disassembler::dopSUBB,
	/* 0xA9 */ &v60_disassembler::dopSHLB,
	/* 0xAA */ &v60_disassembler::dopSUBH,
	/* 0xAB */ &v60_disassembler::dopSHLH,
	/* 0xAC */ &v60_disassembler::dopSUBW,
	/* 0xAD */ &v60_disassembler::dopSHLW,
	/* 0xAE */ &v60_disassembler::dopUNHANDLED,
	/* 0xAF */ &v60_disassembler::dopUNHANDLED,
	/* 0xB0 */ &v60_disassembler::dopXORB,
	/* 0xB1 */ &v60_disassembler::dopDIVUB,
	/* 0xB2 */ &v60_disassembler::dopXORH,
	/* 0xB3 */ &v60_disassembler::dopDIVUH,
	/* 0xB4 */ &v60_disassembler::dopXORW,
	/* 0xB5 */ &v60_disassembler::dopDIVUW,
	/* 0xB6 */ &v60_disassembler::dopDIVUX,
	/* 0xB7 */ &v60_disassembler::dopNOT1,
	/* 0xB8 */ &v60_disassembler::dopCMPB,
	/* 0xB9 */ &v60_disassembler::dopSHAB,
	/* 0xBA */ &v60_disassembler::dopCMPH,
	/* 0xBB */ &v60_disassembler::dopSHAH,
	/* 0xBC */ &v60_disassembler::dopCMPW,
	/* 0xBD */ &v60_disassembler::dopSHAW,
	/* 0xBE */ &v60_disassembler::dopUNHANDLED,
	/* 0xBF */ &v60_disassembler::dopUNHANDLED,
	/* 0xC0 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC1 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC2 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC3 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC4 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC5 */ &v60_disassembler::dopUNHANDLED,
	/* 0xC6 */ &v60_disassembler::dopC6,
	/* 0xC7 */ &v60_disassembler::dopC7,
	/* 0xC8 */ &v60_disassembler::dopBRK,
	/* 0xC9 */ &v60_disassembler::dopBRKV,
	/* 0xCA */ &v60_disassembler::dopRSR,
	/* 0xCB */ &v60_disassembler::dopTRAPFL,
	/* 0xCC */ &v60_disassembler::dopDISPOSE,
	/* 0xCD */ &v60_disassembler::dopNOP,
	/* 0xCE */ &v60_disassembler::dopUNHANDLED,
	/* 0xCF */ &v60_disassembler::dopUNHANDLED,
	/* 0xD0 */ &v60_disassembler::dopDECB,
	/* 0xD1 */ &v60_disassembler::dopDECB,
	/* 0xD2 */ &v60_disassembler::dopDECH,
	/* 0xD3 */ &v60_disassembler::dopDECH,
	/* 0xD4 */ &v60_disassembler::dopDECW,
	/* 0xD5 */ &v60_disassembler::dopDECW,
	/* 0xD6 */ &v60_disassembler::dopJMP,
	/* 0xD7 */ &v60_disassembler::dopJMP,
	/* 0xD8 */ &v60_disassembler::dopINCB,
	/* 0xD9 */ &v60_disassembler::dopINCB,
	/* 0xDA */ &v60_disassembler::dopINCH,
	/* 0xDB */ &v60_disassembler::dopINCH,
	/* 0xDC */ &v60_disassembler::dopINCW,
	/* 0xDD */ &v60_disassembler::dopINCW,
	/* 0xDE */ &v60_disassembler::dopPREPARE,
	/* 0xDF */ &v60_disassembler::dopPREPARE,
	/* 0xE0 */ &v60_disassembler::dopTASI,
	/* 0xE1 */ &v60_disassembler::dopTASI,
	/* 0xE2 */ &v60_disassembler::dopRET,
	/* 0xE3 */ &v60_disassembler::dopRET,
	/* 0xE4 */ &v60_disassembler::dopPOPM,
	/* 0xE5 */ &v60_disassembler::dopPOPM,
	/* 0xE6 */ &v60_disassembler::dopPOP,
	/* 0xE7 */ &v60_disassembler::dopPOP,
	/* 0xE8 */ &v60_disassembler::dopJSR,
	/* 0xE9 */ &v60_disassembler::dopJSR,
	/* 0xEA */ &v60_disassembler::dopRETIU,
	/* 0xEB */ &v60_disassembler::dopRETIU,
	/* 0xEC */ &v60_disassembler::dopPUSHM,
	/* 0xED */ &v60_disassembler::dopPUSHM,
	/* 0xEE */ &v60_disassembler::dopPUSH,
	/* 0xEF */ &v60_disassembler::dopPUSH,
	/* 0xF0 */ &v60_disassembler::dopTESTB,
	/* 0xF1 */ &v60_disassembler::dopTESTB,
	/* 0xF2 */ &v60_disassembler::dopTESTH,
	/* 0xF3 */ &v60_disassembler::dopTESTH,
	/* 0xF4 */ &v60_disassembler::dopTESTW,
	/* 0xF5 */ &v60_disassembler::dopTESTW,
	/* 0xF6 */ &v60_disassembler::dopGETPSW,
	/* 0xF7 */ &v60_disassembler::dopGETPSW,
	/* 0xF8 */ &v60_disassembler::dopTRAP,
	/* 0xF9 */ &v60_disassembler::dopTRAP,
	/* 0xFA */ &v60_disassembler::dopRETIS,
	/* 0xFB */ &v60_disassembler::dopRETIS,
	/* 0xFC */ &v60_disassembler::dopSTTASK,
	/* 0xFD */ &v60_disassembler::dopSTTASK,
	/* 0xFE */ &v60_disassembler::dopCLRTLB,
	/* 0xFF */ &v60_disassembler::dopCLRTLB
};

offs_t v60_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return (this->*dasm_optable[opcodes.r8(pc)])(pc, pc+1, opcodes, stream) | SUPPORTED;
}

u32 v60_disassembler::opcode_alignment() const
{
	return 1;
}
