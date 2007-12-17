
#include "debugger.h"
#include "v60.h"

static const UINT8 *rombase;
static offs_t pcbase;

#define readop(a)	rombase[(a) - pcbase]

static signed char read8(unsigned pc)
{
	return readop(pc);
}

static signed short read16(unsigned pc)
{
	return readop(pc) | (readop(pc+1) << 8);
}

static signed int read32(unsigned pc)
{
	return readop(pc) | (readop(pc+1) << 8)| (readop(pc+2) << 16)| (readop(pc+3) << 24);
}

static void out_AM_Register(int reg, char *out)
{
	strcat(out, v60_reg_names[reg]);
}

static void out_AM_RegisterIndirect(int reg, int opsize, char *out)
{
	if(opsize & 0x80)
		*out++ = '@';
	sprintf(out, "[%s]", v60_reg_names[reg]);
}

static void out_AM_RegisterIndirectIndexed(int rn, int rx, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@[%s]", v60_reg_names[rx], v60_reg_names[rn]);
	else
		sprintf(out, "[%s](%s)", v60_reg_names[rn], v60_reg_names[rx]);

}

static void out_AM_Autoincrement(int reg, int opsize, char *out)
{
	if(opsize & 0x80)
		*out++ = '@';
	sprintf(out, "[%s+]", v60_reg_names[reg]);
}

static void out_AM_Autodecrement(int reg, int opsize, char *out)
{
	if(opsize & 0x80)
		*out++ = '@';
	sprintf(out, "[-%s]", v60_reg_names[reg]);
}

static void out_AM_Displacement(int reg, int disp, int opsize, char *out)
{
	sprintf(out, "%s%X%s[%s]",
			disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,
			opsize & 0x80 ? "@" : "",
			v60_reg_names[reg]);
}

static void out_AM_DisplacementIndexed(int rn, int rx, int disp, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@%s%X[%s]", v60_reg_names[rx], disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn]);
	else
		sprintf(out, "%s%X[%s](%s)", disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn], v60_reg_names[rx]);
}

static void out_AM_PCDisplacement(unsigned pc, int disp, int opsize, char *out)
{
	sprintf(out, "%X%s[PC]", pc+disp, opsize & 0x80 ? "@" : "");
}

static void out_AM_PCDisplacementIndexed(unsigned pc, int disp, int rx, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@%X[PC]", v60_reg_names[rx], pc+disp);
	else
		sprintf(out, "%X[PC](%s)", pc+disp, v60_reg_names[rx]);
}

static void out_AM_DisplacementIndirect(int reg, int disp, int opsize, char *out)
{
	sprintf(out, "%s[%s%X[%s]]",
			opsize & 0x80 ? "@" : "",
			disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,
			v60_reg_names[reg]);
}

static void out_AM_DisplacementIndirectIndexed(int rn, int rx, int disp, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@[%s%X[%s]]", v60_reg_names[rx], disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn]);
	else
		sprintf(out, "[%s%X[%s]](%s)", disp >= 0 ? "" : "-", disp >= 0 ? disp : -disp,v60_reg_names[rn], v60_reg_names[rx]);
}

static void out_AM_PCDisplacementIndirect(unsigned pc, int disp, int opsize, char *out)
{
	sprintf(out, "%s[%X[PC]]", opsize & 0x80 ? "@" : "", pc+disp);
}

static void out_AM_PCDisplacementIndirectIndexed(unsigned pc, int disp, int rx, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@[%X[PC]]", v60_reg_names[rx], pc+disp);
	else
		sprintf(out, "[%X[PC]](%s)", pc+disp, v60_reg_names[rx]);
}

static void out_AM_DoubleDisplacement(int reg, int disp2, int disp1, int opsize, char *out)
{
	sprintf(out, "%s%X%s[%s%X[%s]]",
			disp1 >= 0 ? "" : "-", disp1 >= 0 ? disp1 : -disp1,
			opsize & 0x80 ? "@" : "",
			disp2 >= 0 ? "" : "-", disp2 >= 0 ? disp2 : -disp2,
			v60_reg_names[reg]);
}

static void out_AM_PCDoubleDisplacement(unsigned pc, int disp2, int disp1, int opsize, char *out)
{
	sprintf(out, "%s%X%s[%X[PC]]",
			disp1 >= 0 ? "" : "-", disp1 >= 0 ? disp1 : -disp1,
			opsize & 0x80 ? "@" : "",
			disp2 + pc);
}

static void out_AM_DirectAddress(unsigned addr, int opsize, char *out)
{
	if(opsize & 0x80)
		*out++ = '@';
	sprintf(out, "%X", addr);
}

static void out_AM_DirectAddressIndexed(unsigned addr, int rx, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@%X", v60_reg_names[rx], addr);
	else
		sprintf(out, "%X(%s)", addr, v60_reg_names[rx]);
}

static void out_AM_DirectAddressDeferred(unsigned addr, int opsize, char *out)
{
	if(opsize & 0x80)
		*out++ = '@';
	sprintf(out, "[%X]", addr);
}

static void out_AM_DirectAddressDeferredIndexed(unsigned addr, int rx, int opsize, char *out)
{
	if(opsize & 0x80)
		sprintf(out, "%s@[%X]", v60_reg_names[rx], addr);
	else
		sprintf(out, "[%X](%s)", addr, v60_reg_names[rx]);
}

static void out_AM_Immediate(unsigned value, int opsize, char *out)
{
	if(opsize == 0)
		value &= 0xff;
	else if(opsize == 1)
		value &= 0xffff;

	sprintf(out, "#%X", value);
}

static int decode_AM(unsigned ipc, unsigned pc, int m, int opsize, char *out)
{
	unsigned char mod = readop(pc);
	if(m) {
		switch(mod>>5) {
		case 0: // Double displacement (8 bit)
			out_AM_DoubleDisplacement(mod&0x1F, read8(pc+1), read8(pc+2), opsize, out);
			return 3;

		case 1: // Double displacement (16 bit)
			out_AM_DoubleDisplacement(mod&0x1F, read16(pc+1), read16(pc+3), opsize, out);
			return 5;

		case 2: // Double displacement (32 bit)
			out_AM_DoubleDisplacement(mod&0x1F, read32(pc+1), read32(pc+5), opsize, out);
			return 9;

		case 3: // Register
			out_AM_Register(mod&0x1F, out);
			return 1;

		case 4: // Autoincrement
			out_AM_Autoincrement(mod&0x1F, opsize, out);
			return 1;

		case 5: // Autodecrement
			out_AM_Autodecrement(mod&0x1F, opsize, out);
			return 1;

		case 6:
			switch (readop(pc+1)>>5)
				{
				case 0: // Displacement indexed (8 bit)
					out_AM_DisplacementIndexed(readop(pc+1)&0x1F, mod&0x1F, read8(pc+2), opsize, out);
					return 3;

				case 1: // Displacement indexed (16 bit)
					out_AM_DisplacementIndexed(readop(pc+1)&0x1F, mod&0x1F, read16(pc+2), opsize, out);
					return 4;

				case 2: // Displacement indexed (32 bit)
					out_AM_DisplacementIndexed(readop(pc+1)&0x1F, mod&0x1F, read32(pc+2), opsize, out);
					return 6;

				case 3:	// Register indirect indexed
					out_AM_RegisterIndirectIndexed(readop(pc+1)&0x1F, mod&0x1F, opsize, out);
					return 2;

				case 4: // Displacement indirect indexed (8 bit)
					out_AM_DisplacementIndirectIndexed(readop(pc+1)&0x1F, mod&0x1F, read8(pc+2), opsize, out);
					return 3;

				case 5: // Displacement indirect indexed (16 bit)
					out_AM_DisplacementIndirectIndexed(readop(pc+1)&0x1F, mod&0x1F, read16(pc+2), opsize, out);
					return 4;

				case 6: // Displacement indirect indexed (32 bit)
					out_AM_DisplacementIndirectIndexed(readop(pc+1)&0x1F, mod&0x1F, read32(pc+2), opsize, out);
					return 6;

				case 7:
					switch (readop(pc+1)&0x1F)
						{
						case 16: // PC Displacement Indexed (8 bit)
							out_AM_PCDisplacementIndexed(ipc, read8(pc+2), mod&0x1F, opsize, out);
							return 3;

						case 17: // PC Displacement Indexed (16 bit)
							out_AM_PCDisplacementIndexed(ipc, read16(pc+2), mod&0x1F, opsize, out);
							return 4;

						case 18: // PC Displacement Indexed (32 bit)
							out_AM_PCDisplacementIndexed(ipc, read32(pc+2), mod&0x1F, opsize, out);
							return 6;

						case 19: // Direct Address Indexed
							out_AM_DirectAddressIndexed(read32(pc+2), mod&0x1F, opsize, out);
							return 6;

						case 24: // PC Displacement Indirect Indexed(8 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, read8(pc+2), mod&0x1F, opsize, out);
							return 3;

						case 25: // PC Displacement Indirect Indexed (16 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, read16(pc+2), mod&0x1F, opsize, out);
							return 4;

						case 26: // PC Displacement Indirect Indexed (32 bit)
							out_AM_PCDisplacementIndirectIndexed(ipc, read32(pc+2), mod&0x1F, opsize, out);
							return 6;

						case 27: // Direct Address Deferred Indexed
							out_AM_DirectAddressDeferredIndexed(read32(pc+2), mod&0x1F, opsize, out);
							return 6;

						default:
							strcat(out, "!ERRAM3");
							return 1;
						}

				default:
					strcat(out, "!ERRAM2");
					return 1;
				}

		default:
			strcat(out, "!ERRAM1");
			return 1;
		}
	} else {
		switch(mod>>5) {
		case 0: // Displacement (8 bit)
			out_AM_Displacement(mod&0x1F, read8(pc+1), opsize, out);
			return 2;

		case 1: // Displacement (16 bit)
			out_AM_Displacement(mod&0x1F, read16(pc+1), opsize, out);
			return 3;

		case 2: // Displacement (32 bit)
			out_AM_Displacement(mod&0x1F, read32(pc+1), opsize, out);
			return 5;

		case 3: // Register indirect
			out_AM_RegisterIndirect(mod&0x1F, opsize, out);
			return 1;

		case 4: // Displacement indirect (8 bit)
			out_AM_DisplacementIndirect(mod&0x1F, read8(pc+1), opsize, out);
			return 2;

		case 5: // Displacement indirect (16 bit)
			out_AM_DisplacementIndirect(mod&0x1F, read16(pc+1), opsize, out);
			return 3;

		case 6: // Displacement indirect (32 bit)
			out_AM_DisplacementIndirect(mod&0x1F, read32(pc+1), opsize, out);
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
				out_AM_Immediate(mod&0x1F, opsize, out);
				return 1;

			case 16: // PC Displacement (8 bit)
				out_AM_PCDisplacement(ipc, read8(pc+1), opsize, out);
				return 2;

			case 17: // PC Displacement (16 bit)
				out_AM_PCDisplacement(ipc, read16(pc+1), opsize, out);
				return 3;

			case 18: // PC Displacement (32 bit)
				out_AM_PCDisplacement(ipc, read32(pc+1), opsize, out);
				return 5;

			case 19: // Direct Address
				out_AM_DirectAddress(read32(pc+1), opsize, out);
				return 5;


			case 20:
				switch(opsize&0x7F) {
				case 0: // Immediate (8 bit)
					out_AM_Immediate(read8(pc+1), opsize, out);
					return 2;

				case 1: // Immediate (16 bit)
					out_AM_Immediate(read16(pc+1), opsize, out);
					return 3;

				case 2: // Immediate (32 bit)
					out_AM_Immediate(read32(pc+1), opsize, out);
					return 5;

				default:
					strcat(out, "!ERRAM6");
					return 1;
				}

			case 24: // PC Displacement Indirect (8 bit)
				out_AM_PCDisplacementIndirect(ipc, read8(pc+1), opsize, out);
				return 2;

			case 25: // PC Displacement Indirect (16 bit)
				out_AM_PCDisplacementIndirect(ipc, read16(pc+1), opsize, out);
				return 3;

			case 26: // PC Displacement Indirect (32 bit)
				out_AM_PCDisplacementIndirect(ipc, read32(pc+1), opsize, out);
				return 5;

			case 27: // Direct Address Deferred
				out_AM_DirectAddressDeferred(read32(pc+1), opsize, out);
				return 5;

			case 28: // PC Double Displacement (8 bit)
				out_AM_PCDoubleDisplacement(ipc, read8(pc+1), read8(pc+2), opsize, out);
				return 3;

			case 29: // PC Double Displacement (16 bit)
				out_AM_PCDoubleDisplacement(ipc, read16(pc+1), read16(pc+3), opsize, out);
				return 5;

			case 30: // PC Double Displacement (32 bit)
				out_AM_PCDoubleDisplacement(ipc, read32(pc+1), read32(pc+5), opsize, out);
				return 9;

			default:
				strcat(out, "!ERRAM5");
				return 1;
			}

		default:
			strcat(out, "!ERRAM4");
			return 1;
		}
	}
}


static int decode_F1(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	unsigned char code = readop(pc);
	sprintf(out, "%-8s", opnm);
	if(code & 0x20) {
		int ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out)) + 2;
		strcat(out, ", ");
		out_AM_Register(code & 0x1f, out + strlen(out));
		return ret;
	} else {
		out_AM_Register(code & 0x1f, out + strlen(out));
		strcat(out, ", ");
		return decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out)) + 2;
	}
}

static int decode_F2(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	int ret;
	unsigned char code = readop(pc);
	sprintf(out, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out));
	strcat(out, ", ");
	ret += decode_AM(ipc, pc+1+ret, code & 0x20, opsize2, out + strlen(out));
	return ret+2;
}

static int decode_F1F2(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	if(readop(pc) & 0x80)
		return decode_F2(opnm, opsize1, opsize2, ipc, pc, out);
	else
		return decode_F1(opnm, opsize1, opsize2, ipc, pc, out);
}

static int decode_F3(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "%-8s", opnm);
	return decode_AM(ipc, pc, readop(pc-1) & 1, opsize1, out + strlen(out)) + 1;
}

static int decode_F4a(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "%-8s%X", opnm, ipc+read8(pc));
	return 2;
}

static int decode_F4b(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "%-8s%X", opnm, ipc+read16(pc));
	return 3;
}

static int decode_F5(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	strcpy(out, opnm);
	return 1;
}

static int decode_F6(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "%-8s%s, %X[PC]", opnm, v60_reg_names[readop(pc) & 0x1f], ipc+read16(pc+1));
	return 4;
}

static int decode_F7a(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	int ret;
	unsigned char code = readop(pc);
	unsigned char code2;

	sprintf(out, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out));
	strcat(out, ", ");

	code2 = readop(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, out + strlen(out));
	else
		out_AM_Immediate(code2, 1, out + strlen(out));
	strcat(out, ", ");

	ret += decode_AM(ipc, pc+2+ret, code & 0x20, opsize2, out + strlen(out));
	strcat(out, ", ");

	code2 = readop(pc+2+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, out + strlen(out));
	else
		out_AM_Immediate(code2, 1, out + strlen(out));

	return ret+4;
}

static int decode_F7b(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	int ret;
	unsigned char code = readop(pc);
	unsigned char code2;

	sprintf(out, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out));
	strcat(out, ", ");

	code2 = readop(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, out + strlen(out));
	else
		out_AM_Immediate(code2, 1, out + strlen(out));
	strcat(out, ", ");

	ret += decode_AM(ipc, pc+2+ret, code & 0x20, opsize2, out + strlen(out));

	return ret+3;
}

static int decode_F7c(const char *opnm, int opsize1, int opsize2, unsigned ipc, unsigned pc, char *out)
{
	int ret;
	unsigned char code = readop(pc);
	unsigned char code2;

	sprintf(out, "%-8s", opnm);
	ret = decode_AM(ipc, pc+1, code & 0x40, opsize1, out + strlen(out));
	strcat(out, ", ");

	ret += decode_AM(ipc, pc+1+ret, code & 0x20, opsize2, out + strlen(out));
	strcat(out, ", ");

	code2 = readop(pc+1+ret);
	if(code2 & 0x80)
		out_AM_Register(code2 & 0x1f, out + strlen(out));
	else
		out_AM_Immediate(code2, 1, out + strlen(out));

	return ret+3;
}

static int dopUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$%02X", readop(pc));
	return 1;
}

static int dop58UNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$58");
	return 1;
}

static int dop59UNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$59");
	return 1;
}

static int dop5AUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5A");
	return 1;
}

static int dop5BUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5B");
	return 1;
}

static int dop5CUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5C");
	return 1;
}

static int dop5DUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5D");
	return 1;
}

static int dop5EUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5E");
	return 1;
}

static int dop5FUNHANDLED(unsigned ipc, unsigned pc, char *out)
{
	sprintf(out, "$5F");
	return 1;
}

#define DEFINE_EASY_OPCODE(name, opnm, ftype, opsize1, opsize2) \
	static int dop ## name(unsigned ipc, unsigned pc, char *out) \
	{ \
		return decode_ ## ftype(opnm, opsize1, opsize2, ipc, pc, out); \
	}

#define DEFINE_EASY_OPCODE_EX(name, opnm, ftype, opsize1, opsize2, flags) \
	static int dop ## name(unsigned ipc, unsigned pc, char *out) \
	{ \
		return decode_ ## ftype(opnm, opsize1, opsize2, ipc, pc, out) | (flags); \
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
DEFINE_EASY_OPCODE_EX(BSR, "bsr", F4b, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(CALL, "call", F1F2, 0, 2, DASMFLAG_STEP_OVER)
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
DEFINE_EASY_OPCODE_EX(DBGT, "dbgt", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBGE, "dbge", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBLT, "dbgt", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBLE, "dbge", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBH, "dbh", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNL, "dbnl", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBL, "dbl", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNH, "dbnh", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBE, "dbe", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNE, "dbne", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBV, "dbe", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBNV, "dbne", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBN, "dbn", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBP, "dbp", F6, 0, 0, DASMFLAG_STEP_OVER)
DEFINE_EASY_OPCODE_EX(DBR, "dbr", F6, 0, 0, DASMFLAG_STEP_OVER)
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
DEFINE_EASY_OPCODE_EX(JSR, "jsr", F3, 0, 0, DASMFLAG_STEP_OVER)
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
DEFINE_EASY_OPCODE_EX(RET, "ret", F3, 2, 0, DASMFLAG_STEP_OUT)
DEFINE_EASY_OPCODE_EX(RETIU, "retiu", F3, 1, 0, DASMFLAG_STEP_OUT)
DEFINE_EASY_OPCODE_EX(RETIS, "retis", F3, 1, 0, DASMFLAG_STEP_OUT)
DEFINE_EASY_OPCODE(ROTB, "rot.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(ROTH, "rot.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(ROTW, "rot.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE(ROTCB, "rotc.b", F1F2, 0, 0)
DEFINE_EASY_OPCODE(ROTCH, "rotc.h", F1F2, 0, 1)
DEFINE_EASY_OPCODE(ROTCW, "rotc.w", F1F2, 0, 2)
DEFINE_EASY_OPCODE_EX(RSR, "rsr", F5, 0, 0, DASMFLAG_STEP_OUT)
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
DEFINE_EASY_OPCODE_EX(TRAP, "trap", F3, 0, 0, DASMFLAG_STEP_OVER)
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

static int (*const dasm_optable_58[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCMPCB,
	/* 0x01 */ dopCMPCFB,
	/* 0x02 */ dopCMPCSB,
	/* 0x03 */ dop58UNHANDLED,
	/* 0x04 */ dop58UNHANDLED,
	/* 0x05 */ dop58UNHANDLED,
	/* 0x06 */ dop58UNHANDLED,
	/* 0x07 */ dop58UNHANDLED,
	/* 0x08 */ dopMOVCUB,
	/* 0x09 */ dopMOVCDB,
	/* 0x0A */ dopMOVCFUB,
	/* 0x0B */ dopMOVCFDB,
	/* 0x0C */ dopMOVCSB,
	/* 0x0D */ dop58UNHANDLED,
	/* 0x0E */ dop58UNHANDLED,
	/* 0x0F */ dop58UNHANDLED,
	/* 0x10 */ dop58UNHANDLED,
	/* 0x11 */ dop58UNHANDLED,
	/* 0x12 */ dop58UNHANDLED,
	/* 0x13 */ dop58UNHANDLED,
	/* 0x14 */ dop58UNHANDLED,
	/* 0x15 */ dop58UNHANDLED,
	/* 0x16 */ dop58UNHANDLED,
	/* 0x17 */ dop58UNHANDLED,
	/* 0x18 */ dopSCHCUB,
	/* 0x19 */ dopSCHCDB,
	/* 0x1A */ dopSKPCUB,
	/* 0x1B */ dopSKPCDB,
	/* 0x1C */ dop58UNHANDLED,
	/* 0x1D */ dop58UNHANDLED,
	/* 0x1E */ dop58UNHANDLED,
	/* 0x1F */ dop58UNHANDLED
};

static int (*const dasm_optable_59[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopADDDC,
	/* 0x01 */ dopSUBDC,
	/* 0x02 */ dopSUBRDC,
	/* 0x03 */ dop59UNHANDLED,
	/* 0x04 */ dop59UNHANDLED,
	/* 0x05 */ dop59UNHANDLED,
	/* 0x06 */ dop59UNHANDLED,
	/* 0x07 */ dop59UNHANDLED,
	/* 0x08 */ dop59UNHANDLED,
	/* 0x09 */ dop59UNHANDLED,
	/* 0x0A */ dop59UNHANDLED,
	/* 0x0B */ dop59UNHANDLED,
	/* 0x0C */ dop59UNHANDLED,
	/* 0x0D */ dop59UNHANDLED,
	/* 0x0E */ dop59UNHANDLED,
	/* 0x0F */ dop59UNHANDLED,
	/* 0x10 */ dopCVTDPZ,
	/* 0x11 */ dop59UNHANDLED,
	/* 0x12 */ dop59UNHANDLED,
	/* 0x13 */ dop59UNHANDLED,
	/* 0x14 */ dop59UNHANDLED,
	/* 0x15 */ dop59UNHANDLED,
	/* 0x16 */ dop59UNHANDLED,
	/* 0x17 */ dop59UNHANDLED,
	/* 0x18 */ dopCVTDZP,
	/* 0x19 */ dop59UNHANDLED,
	/* 0x1A */ dop59UNHANDLED,
	/* 0x1B */ dop59UNHANDLED,
	/* 0x1C */ dop59UNHANDLED,
	/* 0x1D */ dop59UNHANDLED,
	/* 0x1E */ dop59UNHANDLED,
	/* 0x1F */ dop59UNHANDLED
};

static int (*const dasm_optable_5A[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCMPCH,
	/* 0x01 */ dopCMPCFH,
	/* 0x02 */ dopCMPCSH,
	/* 0x03 */ dop5AUNHANDLED,
	/* 0x04 */ dop5AUNHANDLED,
	/* 0x05 */ dop5AUNHANDLED,
	/* 0x06 */ dop5AUNHANDLED,
	/* 0x07 */ dop5AUNHANDLED,
	/* 0x08 */ dopMOVCUH,
	/* 0x09 */ dopMOVCDH,
	/* 0x0A */ dopMOVCFUH,
	/* 0x0B */ dopMOVCFDH,
	/* 0x0C */ dopMOVCSH,
	/* 0x0D */ dop5AUNHANDLED,
	/* 0x0E */ dop5AUNHANDLED,
	/* 0x0F */ dop5AUNHANDLED,
	/* 0x10 */ dop5AUNHANDLED,
	/* 0x11 */ dop5AUNHANDLED,
	/* 0x12 */ dop5AUNHANDLED,
	/* 0x13 */ dop5AUNHANDLED,
	/* 0x14 */ dop5AUNHANDLED,
	/* 0x15 */ dop5AUNHANDLED,
	/* 0x16 */ dop5AUNHANDLED,
	/* 0x17 */ dop5AUNHANDLED,
	/* 0x18 */ dopSCHCUH,
	/* 0x19 */ dopSCHCDH,
	/* 0x1A */ dopSKPCUH,
	/* 0x1B */ dopSKPCDH,
	/* 0x1C */ dop5AUNHANDLED,
	/* 0x1D */ dop5AUNHANDLED,
	/* 0x1E */ dop5AUNHANDLED,
	/* 0x1F */ dop5AUNHANDLED
};

static int (*const dasm_optable_5B[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopSCH0BSU,
	/* 0x01 */ dopSCH0BSD,
	/* 0x02 */ dopSCH1BSU,
	/* 0x03 */ dopSCH1BSD,
	/* 0x04 */ dop5BUNHANDLED,
	/* 0x05 */ dop5BUNHANDLED,
	/* 0x06 */ dop5BUNHANDLED,
	/* 0x07 */ dop5BUNHANDLED,
	/* 0x08 */ dopMOVBSU,
	/* 0x09 */ dopMOVBSD,
	/* 0x0A */ dopNOTBSU,
	/* 0x0B */ dopNOTBSD,
	/* 0x0C */ dop5BUNHANDLED,
	/* 0x0D */ dop5BUNHANDLED,
	/* 0x0E */ dop5BUNHANDLED,
	/* 0x0F */ dop5BUNHANDLED,
	/* 0x10 */ dopANDBSU,
	/* 0x11 */ dopANDBSD,
	/* 0x12 */ dopANDNBSU,
	/* 0x13 */ dopANDNBSD,
	/* 0x14 */ dopORBSU,
	/* 0x15 */ dopORBSD,
	/* 0x16 */ dopORNBSU,
	/* 0x17 */ dopORNBSD,
	/* 0x18 */ dopXORBSU,
	/* 0x19 */ dopXORBSD,
	/* 0x1A */ dopXORNBSU,
	/* 0x1B */ dopXORNBSD,
	/* 0x1C */ dop5BUNHANDLED,
	/* 0x1D */ dop5BUNHANDLED,
	/* 0x1E */ dop5BUNHANDLED,
	/* 0x1F */ dop5BUNHANDLED
};

static int (*const dasm_optable_5C[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCMPFS,
	/* 0x01 */ dop5CUNHANDLED,
	/* 0x02 */ dop5CUNHANDLED,
	/* 0x03 */ dop5CUNHANDLED,
	/* 0x04 */ dop5CUNHANDLED,
	/* 0x05 */ dop5CUNHANDLED,
	/* 0x06 */ dop5CUNHANDLED,
	/* 0x07 */ dop5CUNHANDLED,
	/* 0x08 */ dopMOVFS,
	/* 0x09 */ dopNEGFS,
	/* 0x0A */ dopABSFS,
	/* 0x0B */ dop5CUNHANDLED,
	/* 0x0C */ dop5CUNHANDLED,
	/* 0x0D */ dop5CUNHANDLED,
	/* 0x0E */ dop5CUNHANDLED,
	/* 0x0F */ dop5CUNHANDLED,
	/* 0x10 */ dopSCLFS,
	/* 0x11 */ dop5CUNHANDLED,
	/* 0x12 */ dop5CUNHANDLED,
	/* 0x13 */ dop5CUNHANDLED,
	/* 0x14 */ dop5CUNHANDLED,
	/* 0x15 */ dop5CUNHANDLED,
	/* 0x16 */ dop5CUNHANDLED,
	/* 0x17 */ dop5CUNHANDLED,
	/* 0x18 */ dopADDFS,
	/* 0x19 */ dopSUBFS,
	/* 0x1A */ dopMULFS,
	/* 0x1B */ dopDIVFS,
	/* 0x1C */ dop5CUNHANDLED,
	/* 0x1D */ dop5CUNHANDLED,
	/* 0x1E */ dop5CUNHANDLED,
	/* 0x1F */ dop5CUNHANDLED
};

static int (*const dasm_optable_5D[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCMPBFS,
	/* 0x01 */ dopCMPBFZ,
	/* 0x02 */ dopCMPBFL,
	/* 0x03 */ dop5DUNHANDLED,
	/* 0x04 */ dop5DUNHANDLED,
	/* 0x05 */ dop5DUNHANDLED,
	/* 0x06 */ dop5DUNHANDLED,
	/* 0x07 */ dop5DUNHANDLED,
	/* 0x08 */ dopEXTBFS,
	/* 0x09 */ dopEXTBFZ,
	/* 0x0A */ dopEXTBFL,
	/* 0x0B */ dop5DUNHANDLED,
	/* 0x0C */ dop5DUNHANDLED,
	/* 0x0D */ dop5DUNHANDLED,
	/* 0x0E */ dop5DUNHANDLED,
	/* 0x0F */ dop5DUNHANDLED,
	/* 0x10 */ dop5DUNHANDLED,
	/* 0x11 */ dop5DUNHANDLED,
	/* 0x12 */ dop5DUNHANDLED,
	/* 0x13 */ dop5DUNHANDLED,
	/* 0x14 */ dop5DUNHANDLED,
	/* 0x15 */ dop5DUNHANDLED,
	/* 0x16 */ dop5DUNHANDLED,
	/* 0x17 */ dop5DUNHANDLED,
	/* 0x18 */ dopINSBFR,
	/* 0x19 */ dopINSBFL,
	/* 0x1A */ dop5DUNHANDLED,
	/* 0x1B */ dop5DUNHANDLED,
	/* 0x1C */ dop5DUNHANDLED,
	/* 0x1D */ dop5DUNHANDLED,
	/* 0x1E */ dop5DUNHANDLED,
	/* 0x1F */ dop5DUNHANDLED
};

static int (*const dasm_optable_5E[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCMPFL,
	/* 0x01 */ dop5EUNHANDLED,
	/* 0x02 */ dop5EUNHANDLED,
	/* 0x03 */ dop5EUNHANDLED,
	/* 0x04 */ dop5EUNHANDLED,
	/* 0x05 */ dop5EUNHANDLED,
	/* 0x06 */ dop5EUNHANDLED,
	/* 0x07 */ dop5EUNHANDLED,
	/* 0x08 */ dopMOVFL,
	/* 0x09 */ dopNEGFL,
	/* 0x0A */ dopABSFL,
	/* 0x0B */ dop5EUNHANDLED,
	/* 0x0C */ dop5EUNHANDLED,
	/* 0x0D */ dop5EUNHANDLED,
	/* 0x0E */ dop5EUNHANDLED,
	/* 0x0F */ dop5EUNHANDLED,
	/* 0x10 */ dopSCLFL,
	/* 0x11 */ dop5EUNHANDLED,
	/* 0x12 */ dop5EUNHANDLED,
	/* 0x13 */ dop5EUNHANDLED,
	/* 0x14 */ dop5EUNHANDLED,
	/* 0x15 */ dop5EUNHANDLED,
	/* 0x16 */ dop5EUNHANDLED,
	/* 0x17 */ dop5EUNHANDLED,
	/* 0x18 */ dopADDFL,
	/* 0x19 */ dopSUBFL,
	/* 0x1A */ dopMULFL,
	/* 0x1B */ dopDIVFL,
	/* 0x1C */ dop5EUNHANDLED,
	/* 0x1D */ dop5EUNHANDLED,
	/* 0x1E */ dop5EUNHANDLED,
	/* 0x1F */ dop5EUNHANDLED
};

static int (*const dasm_optable_5F[32])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopCVTWS,
	/* 0x01 */ dopCVTSW,
	/* 0x02 */ dop5FUNHANDLED,
	/* 0x03 */ dop5FUNHANDLED,
	/* 0x04 */ dop5FUNHANDLED,
	/* 0x05 */ dop5FUNHANDLED,
	/* 0x06 */ dop5FUNHANDLED,
	/* 0x07 */ dop5FUNHANDLED,
	/* 0x08 */ dopCVTLS,
	/* 0x09 */ dopCVTLW,
	/* 0x0A */ dop5FUNHANDLED,
	/* 0x0B */ dop5FUNHANDLED,
	/* 0x0C */ dop5FUNHANDLED,
	/* 0x0D */ dop5FUNHANDLED,
	/* 0x0E */ dop5FUNHANDLED,
	/* 0x0F */ dop5FUNHANDLED,
	/* 0x10 */ dopCVTSL,
	/* 0x11 */ dopCVTWL,
	/* 0x12 */ dop5FUNHANDLED,
	/* 0x13 */ dop5FUNHANDLED,
	/* 0x14 */ dop5FUNHANDLED,
	/* 0x15 */ dop5FUNHANDLED,
	/* 0x16 */ dop5FUNHANDLED,
	/* 0x17 */ dop5FUNHANDLED,
	/* 0x18 */ dop5FUNHANDLED,
	/* 0x19 */ dop5FUNHANDLED,
	/* 0x1A */ dop5FUNHANDLED,
	/* 0x1B */ dop5FUNHANDLED,
	/* 0x1C */ dop5FUNHANDLED,
	/* 0x1D */ dop5FUNHANDLED,
	/* 0x1E */ dop5FUNHANDLED,
	/* 0x1F */ dop5FUNHANDLED
};

static int (*const dasm_optable_C6[8])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x0 */ dopDBV,
	/* 0x1 */ dopDBL,
	/* 0x2 */ dopDBE,
	/* 0x3 */ dopDBNH,
	/* 0x4 */ dopDBN,
	/* 0x5 */ dopDBR,
	/* 0x6 */ dopDBLT,
	/* 0x7 */ dopDBLE
};

static int (*const dasm_optable_C7[8])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x0 */ dopDBNV,
	/* 0x1 */ dopDBNL,
	/* 0x2 */ dopDBNE,
	/* 0x3 */ dopDBH,
	/* 0x4 */ dopDBP,
	/* 0x5 */ dopTB,
	/* 0x6 */ dopDBGE,
	/* 0x7 */ dopDBGT
};

static int dop58(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_58[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop59(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_59[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5A(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5A[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5B(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5B[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5C(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5C[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5D(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5D[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5E(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5E[readop(pc) & 0x1f](ipc, pc, out);
}

static int dop5F(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_5F[readop(pc) & 0x1f](ipc, pc, out);
}

static int dopC6(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_C6[readop(pc) >> 5](ipc, pc, out);
}

static int dopC7(unsigned ipc, unsigned pc, char *out)
{
	return dasm_optable_C7[readop(pc) >> 5](ipc, pc, out);
}

static int (*const dasm_optable[256])(unsigned ipc, unsigned pc, char *out) =
{
	/* 0x00 */ dopHALT,
	/* 0x01 */ dopLDTASK,
	/* 0x02 */ dopSTPR,
	/* 0x03 */ dopGETRA,
	/* 0x04 */ dopGETPTE,
	/* 0x05 */ dopGETATE,
	/* 0x06 */ dopUNHANDLED,
	/* 0x07 */ dopUNHANDLED,
	/* 0x08 */ dopRVBIT,
	/* 0x09 */ dopMOVB,
	/* 0x0A */ dopMOVSBH,
	/* 0x0B */ dopMOVZBH,
	/* 0x0C */ dopMOVSBW,
	/* 0x0D */ dopMOVZBW,
	/* 0x0E */ dopUNHANDLED,
	/* 0x0F */ dopUNHANDLED,
	/* 0x10 */ dopCLRTLBA,
	/* 0x11 */ dopUNHANDLED,
	/* 0x12 */ dopLDPR,
	/* 0x13 */ dopUPDPSWW,
	/* 0x14 */ dopUPDPTE,
	/* 0x15 */ dopUPDATE,
	/* 0x16 */ dopUNHANDLED,
	/* 0x17 */ dopUNHANDLED,
	/* 0x18 */ dopUNHANDLED,
	/* 0x19 */ dopMOVTHB,
	/* 0x1A */ dopUNHANDLED,
	/* 0x1B */ dopMOVH,
	/* 0x1C */ dopMOVSHW,
	/* 0x1D */ dopMOVZHW,
	/* 0x1E */ dopUNHANDLED,
	/* 0x1F */ dopUNHANDLED,
	/* 0x20 */ dopINB,
	/* 0x21 */ dopOUTB,
	/* 0x22 */ dopINH,
	/* 0x23 */ dopOUTH,
	/* 0x24 */ dopINW,
	/* 0x25 */ dopOUTW,
	/* 0x26 */ dopUNHANDLED,
	/* 0x27 */ dopUNHANDLED,
	/* 0x28 */ dopUNHANDLED,
	/* 0x29 */ dopMOVTWB,
	/* 0x2A */ dopUNHANDLED,
	/* 0x2B */ dopMOVTWH,
	/* 0x2C */ dopRVBYT,
	/* 0x2D */ dopMOVW,
	/* 0x2E */ dopUNHANDLED,
	/* 0x2F */ dopUNHANDLED,
	/* 0x30 */ dopUNHANDLED,
	/* 0x31 */ dopUNHANDLED,
	/* 0x32 */ dopUNHANDLED,
	/* 0x33 */ dopUNHANDLED,
	/* 0x34 */ dopUNHANDLED,
	/* 0x35 */ dopUNHANDLED,
	/* 0x36 */ dopUNHANDLED,
	/* 0x37 */ dopUNHANDLED,
	/* 0x38 */ dopNOTB,
	/* 0x39 */ dopNEGB,
	/* 0x3A */ dopNOTH,
	/* 0x3B */ dopNEGH,
	/* 0x3C */ dopNOTW,
	/* 0x3D */ dopNEGW,
	/* 0x3E */ dopUNHANDLED,
	/* 0x3F */ dopMOVD,
	/* 0x40 */ dopMOVEAB,
	/* 0x41 */ dopXCHB,
	/* 0x42 */ dopMOVEAH,
	/* 0x43 */ dopXCHH,
	/* 0x44 */ dopMOVEAW,
	/* 0x45 */ dopXCHW,
	/* 0x46 */ dopUNHANDLED,
	/* 0x47 */ dopSETF,
	/* 0x48 */ dopBSR,
	/* 0x49 */ dopCALL,
	/* 0x4A */ dopUPDPSWH,
	/* 0x4B */ dopCHLVL,
	/* 0x4C */ dopCAXI,
	/* 0x4D */ dopCHKAR,
	/* 0x4E */ dopCHKAW,
	/* 0x4F */ dopCHKAE,
	/* 0x50 */ dopREMB,
	/* 0x51 */ dopREMUB,
	/* 0x52 */ dopREMH,
	/* 0x53 */ dopREMUH,
	/* 0x54 */ dopREMW,
	/* 0x55 */ dopREMUW,
	/* 0x56 */ dopUNHANDLED,
	/* 0x57 */ dopUNHANDLED,
	/* 0x58 */ dop58,
	/* 0x59 */ dop59,
	/* 0x5A */ dop5A,
	/* 0x5B */ dop5B,
	/* 0x5C */ dop5C,
	/* 0x5D */ dop5D,
	/* 0x5E */ dop5E,
	/* 0x5F */ dop5F,
	/* 0x60 */ dopBV8,
	/* 0x61 */ dopBNV8,
	/* 0x62 */ dopBL8,
	/* 0x63 */ dopBNL8,
	/* 0x64 */ dopBE8,
	/* 0x65 */ dopBNE8,
	/* 0x66 */ dopBNH8,
	/* 0x67 */ dopBH8,
	/* 0x68 */ dopBN8,
	/* 0x69 */ dopBP8,
	/* 0x6A */ dopBR8,
	/* 0x6B */ dopUNHANDLED,
	/* 0x6C */ dopBLT8,
	/* 0x6D */ dopBGE8,
	/* 0x6E */ dopBLE8,
	/* 0x6F */ dopBGT8,
	/* 0x70 */ dopBV16,
	/* 0x71 */ dopBNV16,
	/* 0x72 */ dopBL16,
	/* 0x73 */ dopBNL16,
	/* 0x74 */ dopBE16,
	/* 0x75 */ dopBNE16,
	/* 0x76 */ dopBNH16,
	/* 0x77 */ dopBH16,
	/* 0x78 */ dopBN16,
	/* 0x79 */ dopBP16,
	/* 0x7A */ dopBR16,
	/* 0x7B */ dopUNHANDLED,
	/* 0x7C */ dopBLT16,
	/* 0x7D */ dopBGE16,
	/* 0x7E */ dopBLE16,
	/* 0x7F */ dopBGT16,
	/* 0x80 */ dopADDB,
	/* 0x81 */ dopMULB,
	/* 0x82 */ dopADDH,
	/* 0x83 */ dopMULH,
	/* 0x84 */ dopADDW,
	/* 0x85 */ dopMULW,
	/* 0x86 */ dopMULX,
	/* 0x87 */ dopTEST1,
	/* 0x88 */ dopORB,
	/* 0x89 */ dopROTB,
	/* 0x8A */ dopORH,
	/* 0x8B */ dopROTH,
	/* 0x8C */ dopORW,
	/* 0x8D */ dopROTW,
	/* 0x8E */ dopUNHANDLED,
	/* 0x8F */ dopUNHANDLED,
	/* 0x90 */ dopADDCB,
	/* 0x91 */ dopMULUB,
	/* 0x92 */ dopADDCH,
	/* 0x93 */ dopMULUH,
	/* 0x94 */ dopADDCW,
	/* 0x95 */ dopMULUW,
	/* 0x96 */ dopMULUX,
	/* 0x97 */ dopSET1,
	/* 0x98 */ dopSUBCB,
	/* 0x99 */ dopROTCB,
	/* 0x9A */ dopSUBCH,
	/* 0x9B */ dopROTCH,
	/* 0x9C */ dopSUBCW,
	/* 0x9D */ dopROTCW,
	/* 0x9E */ dopUNHANDLED,
	/* 0x9F */ dopUNHANDLED,
	/* 0xA0 */ dopANDB,
	/* 0xA1 */ dopDIVB,
	/* 0xA2 */ dopANDH,
	/* 0xA3 */ dopDIVH,
	/* 0xA4 */ dopANDW,
	/* 0xA5 */ dopDIVW,
	/* 0xA6 */ dopDIVX,
	/* 0xA7 */ dopCLR1,
	/* 0xA8 */ dopSUBB,
	/* 0xA9 */ dopSHLB,
	/* 0xAA */ dopSUBH,
	/* 0xAB */ dopSHLH,
	/* 0xAC */ dopSUBW,
	/* 0xAD */ dopSHLW,
	/* 0xAE */ dopUNHANDLED,
	/* 0xAF */ dopUNHANDLED,
	/* 0xB0 */ dopXORB,
	/* 0xB1 */ dopDIVUB,
	/* 0xB2 */ dopXORH,
	/* 0xB3 */ dopDIVUH,
	/* 0xB4 */ dopXORW,
	/* 0xB5 */ dopDIVUW,
	/* 0xB6 */ dopDIVUX,
	/* 0xB7 */ dopNOT1,
	/* 0xB8 */ dopCMPB,
	/* 0xB9 */ dopSHAB,
	/* 0xBA */ dopCMPH,
	/* 0xBB */ dopSHAH,
	/* 0xBC */ dopCMPW,
	/* 0xBD */ dopSHAW,
	/* 0xBE */ dopUNHANDLED,
	/* 0xBF */ dopUNHANDLED,
	/* 0xC0 */ dopUNHANDLED,
	/* 0xC1 */ dopUNHANDLED,
	/* 0xC2 */ dopUNHANDLED,
	/* 0xC3 */ dopUNHANDLED,
	/* 0xC4 */ dopUNHANDLED,
	/* 0xC5 */ dopUNHANDLED,
	/* 0xC6 */ dopC6,
	/* 0xC7 */ dopC7,
	/* 0xC8 */ dopBRK,
	/* 0xC9 */ dopBRKV,
	/* 0xCA */ dopRSR,
	/* 0xCB */ dopTRAPFL,
	/* 0xCC */ dopDISPOSE,
	/* 0xCD */ dopNOP,
	/* 0xCE */ dopUNHANDLED,
	/* 0xCF */ dopUNHANDLED,
	/* 0xD0 */ dopDECB,
	/* 0xD1 */ dopDECB,
	/* 0xD2 */ dopDECH,
	/* 0xD3 */ dopDECH,
	/* 0xD4 */ dopDECW,
	/* 0xD5 */ dopDECW,
	/* 0xD6 */ dopJMP,
	/* 0xD7 */ dopJMP,
	/* 0xD8 */ dopINCB,
	/* 0xD9 */ dopINCB,
	/* 0xDA */ dopINCH,
	/* 0xDB */ dopINCH,
	/* 0xDC */ dopINCW,
	/* 0xDD */ dopINCW,
	/* 0xDE */ dopPREPARE,
	/* 0xDF */ dopPREPARE,
	/* 0xE0 */ dopTASI,
	/* 0xE1 */ dopTASI,
	/* 0xE2 */ dopRET,
	/* 0xE3 */ dopRET,
	/* 0xE4 */ dopPOPM,
	/* 0xE5 */ dopPOPM,
	/* 0xE6 */ dopPOP,
	/* 0xE7 */ dopPOP,
	/* 0xE8 */ dopJSR,
	/* 0xE9 */ dopJSR,
	/* 0xEA */ dopRETIU,
	/* 0xEB */ dopRETIU,
	/* 0xEC */ dopPUSHM,
	/* 0xED */ dopPUSHM,
	/* 0xEE */ dopPUSH,
	/* 0xEF */ dopPUSH,
	/* 0xF0 */ dopTESTB,
	/* 0xF1 */ dopTESTB,
	/* 0xF2 */ dopTESTH,
	/* 0xF3 */ dopTESTH,
	/* 0xF4 */ dopTESTW,
	/* 0xF5 */ dopTESTW,
	/* 0xF6 */ dopGETPSW,
	/* 0xF7 */ dopGETPSW,
	/* 0xF8 */ dopTRAP,
	/* 0xF9 */ dopTRAP,
	/* 0xFA */ dopRETIS,
	/* 0xFB */ dopRETIS,
	/* 0xFC */ dopSTTASK,
	/* 0xFD */ dopSTTASK,
	/* 0xFE */ dopCLRTLB,
	/* 0xFF */ dopCLRTLB
};

#ifdef MAME_DEBUG
offs_t v60_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	rombase = oprom;
	pcbase = pc;
	return dasm_optable[oprom[0]](pc, pc+1, buffer) | DASMFLAG_SUPPORTED;
}

offs_t v70_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	rombase = oprom;
	pcbase = pc;
	return dasm_optable[oprom[0]](pc, pc+1, buffer) | DASMFLAG_SUPPORTED;
}
#endif
