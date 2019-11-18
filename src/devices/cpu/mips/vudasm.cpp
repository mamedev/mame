// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 Vector Unit disassembler
*
*/

#include "emu.h"
#include "vudasm.h"

/*static*/ char const *const sonyvu_disassembler::DEST_STR[] =
{
	"    ", "w   ", "z   ", "zw  ", "y   ", "yw  ", "yz  ", "yzw ",
	"x   ", "xw  ", "xz  ", "xzw ", "xy  ", "xyw ", "xyz ", "xyzw"
};

/*static*/ char const *const sonyvu_disassembler::DEST_COMMA_STR[] =
{
	",",  "w,",  "z,",  "zw,",  "y,",  "yw,",  "yz,",  "yzw,",
	"x,", "xw,", "xz,", "xzw,", "xy,", "xyw,", "xyz,", "xyzw,"
};

/*static*/ char const *const sonyvu_disassembler::BC_STR[] = { "x", "y", "z", "w" };
/*static*/ char const *const sonyvu_disassembler::BC_COMMA_STR[] = { "x,", "y,", "z,", "w," };

/*static*/ char const *const sonyvu_disassembler::VFREG[] =
{
	"$vf00", "$vf01", "$vf02", "$vf03", "$vf04", "$vf05", "$vf06", "$vf07",
	"$vf08", "$vf09", "$vf10", "$vf11", "$vf12", "$vf13", "$vf14", "$vf15",
	"$vf16", "$vf17", "$vf18", "$vf19", "$vf20", "$vf21", "$vf22", "$vf23",
	"$vf24", "$vf25", "$vf26", "$vf27", "$vf28", "$vf29", "$vf30", "$vf31"
};

/*static*/ char const *const sonyvu_disassembler::VIREG[] =
{
	"$vi00",  "$vi01", "$vi02", "$vi03",  "$vi04", "$vi05",    "$vi06", "$vi07",
	"$vi08",  "$vi09", "$vi10", "$vi11",  "$vi12", "$vi13",    "$vi14", "$vi15",
	"STATUS", "MACF",  "CLIPF", "res19",  "R",     "I",        "Q",     "res23",
	"res24",  "res25", "TPC",   "CMSAR0", "FBRST", "VPU_STAT", "res30", "CMSAR1"
};

offs_t sonyvu_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint64_t op = opcodes.r64(pc & ~7);
	if (pc & 4)
	{
		if (op & 0x8000000000000000ULL)
		{
			util::stream_format(stream, "loi         %f", *reinterpret_cast<float*>(&op));
		}
		else
		{
			dasm_lower(pc, (uint32_t)op, stream);
		}
	}
	else
	{
		dasm_upper(pc, (uint32_t)(op >> 32), stream);
	}
	return 4 | SUPPORTED;
}

void sonyvu_disassembler::dasm_upper(uint32_t pc, uint32_t op, std::ostream &stream)
{
	const int rd   = (op >>  6) & 31;
	const int rs   = (op >> 11) & 31;
	const int rt   = (op >> 16) & 31;
	const char* bc = BC_STR[op & 3];
	const char* dest = DEST_STR[(op >> 21) & 15];
	const char* destc = DEST_COMMA_STR[(op >> 21) & 15];

	switch (op & 0x3f)
	{
		case 0x08: case 0x09: case 0x0a: case 0x0b:
			util::stream_format(stream, "madd%s.%s  %s%s %s%s %s%s", bc, dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], bc); break;
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "msub%s.%s  %s%s %s%s %s%s", bc, dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], bc); break;
		case 0x10: case 0x11: case 0x12: case 0x13:
			util::stream_format(stream, "max%s.%s   %s%s %s%s %s%s", bc, dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], bc); break;
		case 0x14: case 0x15: case 0x16: case 0x17:
			util::stream_format(stream, "mini%s.%s  %s%s %s%s %s%s", bc, dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], bc); break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			util::stream_format(stream, "mul%s.%s   %s%s %s%s %s%s", bc, dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], bc); break;
		case 0x1c: util::stream_format(stream, "mulq.%s   %s%s %s%s Q", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x1d: util::stream_format(stream, "maxi.%s   %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x1e: util::stream_format(stream, "muli.%s   %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x1f: util::stream_format(stream, "minii.%s  %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x20: util::stream_format(stream, "addq.%s   %s%s %s%s Q", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x21: util::stream_format(stream, "maddq.%s  %s%s %s%s Q", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x22: util::stream_format(stream, "addi.%s   %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x23: util::stream_format(stream, "maddi.%s  %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x24: util::stream_format(stream, "subq.%s   %s%s %s%s Q", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x25: util::stream_format(stream, "msubq.%s  %s%s %s%s Q", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x26: util::stream_format(stream, "subi.%s   %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x27: util::stream_format(stream, "msubi.%s  %s%s %s%s I", dest, VFREG[rd], destc, VFREG[rs], destc); break;
		case 0x28: util::stream_format(stream, "add.%s    %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x29: util::stream_format(stream, "madd.%s   %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x2a: util::stream_format(stream, "mul.%s    %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x2b: util::stream_format(stream, "max.%s    %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x2c: util::stream_format(stream, "sub.%s    %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x2d: util::stream_format(stream, "msub.%s   %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x2e: util::stream_format(stream, "opmsub.xyz  %sxyz, %sxyz, %sxyz", VFREG[rd], VFREG[rs], VFREG[rt]); break;
		case 0x2f: util::stream_format(stream, "mini.%s   %s%s %s%s %s%s", dest, VFREG[rd], destc, VFREG[rs], destc, VFREG[rt], dest); break;
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		{
			const uint8_t type2_op = ((op & 0x3c0) >> 4) | (op & 3);
			switch (type2_op)
			{
				case 0x00: case 0x01: case 0x02: case 0x03:
					util::stream_format(stream, "adda%s.%s  ACC%s %s%s %s%s", bc, dest, destc, VFREG[rs], destc, VFREG[rt], bc); break;
				case 0x04: case 0x05: case 0x06: case 0x07:
					util::stream_format(stream, "suba%s.%s  ACC%s %s%s %s%s", bc, dest, destc, VFREG[rs], destc, VFREG[rt], bc); break;
				case 0x08: case 0x09: case 0x0a: case 0x0b:
					util::stream_format(stream, "madda%s.%s ACC%s %s%s %s%s", bc, dest, destc, VFREG[rs], destc, VFREG[rt], bc); break;
				case 0x0c: case 0x0d: case 0x0e: case 0x0f:
					util::stream_format(stream, "msuba%s.%s ACC%s %s%s %s%s", bc, dest, destc, VFREG[rs], destc, VFREG[rt], bc); break;
				case 0x10: util::stream_format(stream, "itof0.%s  %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x11: util::stream_format(stream, "itof4.%s  %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x12: util::stream_format(stream, "itof12.%s %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x13: util::stream_format(stream, "itof15.%s %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x14: util::stream_format(stream, "ftoi0.%s  %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x15: util::stream_format(stream, "ftoi4.%s  %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x16: util::stream_format(stream, "ftoi12.%s %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x17: util::stream_format(stream, "ftoi15.%s %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x18: case 0x19: case 0x1a: case 0x1b:
					util::stream_format(stream, "mula%s.%s  ACC%s %s%s %s%s", bc, dest, destc, VFREG[rs], destc, VFREG[rt], bc); break;
				case 0x1c: util::stream_format(stream, "mulaq.%s  ACC%s %s%s Q", dest, destc, VFREG[rs], destc); break;
				case 0x1d: util::stream_format(stream, "abs.%s    %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest); break;
				case 0x1e: util::stream_format(stream, "mulai.%s  ACC%s %s%s I", dest, destc, VFREG[rs], destc); break;
				case 0x1f: util::stream_format(stream, "clipw.xyz   %sxyz, %sw", VFREG[rs], VFREG[rt]); break;
				case 0x20: util::stream_format(stream, "addaq.%s  ACC%s %s%s Q", dest, destc, VFREG[rs], destc); break;
				case 0x21: util::stream_format(stream, "maddaq.%s ACC%s %s%s Q", dest, destc, VFREG[rs], destc); break;
				case 0x22: util::stream_format(stream, "addai.%s  ACC%s %s%s I", dest, destc, VFREG[rs], destc); break;
				case 0x23: util::stream_format(stream, "maddai.%s ACC%s %s%s I", dest, destc, VFREG[rs], destc); break;
				case 0x24: util::stream_format(stream, "subaq.%s  ACC%s %s%s Q", dest, destc, VFREG[rs], destc); break;
				case 0x25: util::stream_format(stream, "msubaq.%s ACC%s %s%s Q", dest, destc, VFREG[rs], destc); break;
				case 0x26: util::stream_format(stream, "subai.%s  ACC%s %s%s I", dest, destc, VFREG[rs], destc); break;
				case 0x27: util::stream_format(stream, "msubai.%s ACC%s %s%s I", dest, destc, VFREG[rs], destc); break;
				case 0x28: util::stream_format(stream, "adda.%s   ACC%s %s%s %s%s", dest, destc, VFREG[rs], destc, VFREG[rt], dest); break;
				case 0x29: util::stream_format(stream, "madda.%s  ACC%s %s%s %s%s", dest, destc, VFREG[rs], destc, VFREG[rt], dest); break;
				case 0x2a: util::stream_format(stream, "mula.%s   ACC%s %s%s %s%s", dest, destc, VFREG[rs], destc, VFREG[rt], dest); break;
				// 2b?
				case 0x2c: util::stream_format(stream, "suba.%s   ACC%s %s%s %s%s", dest, destc, VFREG[rs], destc, VFREG[rt], dest); break;
				case 0x2d: util::stream_format(stream, "msuba.%s  ACC%s %s%s %s%s", dest, destc, VFREG[rs], destc, VFREG[rt], dest); break;
				case 0x2e: util::stream_format(stream, "opmula.xyz  ACCxyz, %sxyz, %sxyz", VFREG[rs], VFREG[rt]); break;
				case 0x2f: util::stream_format(stream, "nop"); break;
				default:
					util::stream_format(stream, "invalid");
					break;
			}
			break;
		}
		default:
			util::stream_format(stream, "invalid");
			break;
	}
}

void sonyvu_disassembler::dasm_lower(uint32_t pc, uint32_t op, std::ostream &stream)
{
	const int rd   = (op >>  6) & 31;
	const int rs   = (op >> 11) & 31;
	const int rt   = (op >> 16) & 31;
	const char* dest = DEST_STR[(op >> 21) & 15];
	const char* destc = DEST_COMMA_STR[(op >> 21) & 15];
	const char* ftf = BC_STR[(op >> 23) & 3];
	const char* fsf = BC_STR[(op >> 21) & 3];
	const char* fsfc = BC_COMMA_STR[(op >> 21) & 3];

	switch ((op >> 25) & 0x7f)
	{
		case 0x00: // LQ
			util::stream_format(stream, "lq.%s     %s%s %s(%s)", dest, VFREG[rt], destc, signed_11bit(op), VIREG[rs]);
			break;
		case 0x01: // SQ
			util::stream_format(stream, "sq.%s     %s%s %s(%s)", dest, VFREG[rs], destc, signed_11bit(op), VIREG[rt]);
			break;
		case 0x04: // ILW
			util::stream_format(stream, "ilw.%s    %s, %s(%s)%s", dest, VIREG[rt], destc, signed_11bit(op), VIREG[rs], dest);
			break;
		case 0x05: // ISW
			util::stream_format(stream, "isw.%s    %s, %s(%s)%s", dest, VIREG[rt], destc, signed_11bit(op), VIREG[rs], dest);
			break;
		case 0x08: // IADDIU
			util::stream_format(stream, "iaddiu      %s, %s, %s", VIREG[rt], VIREG[rs], signed_11bit(op));
			break;
		case 0x09: // ISUBIU
			util::stream_format(stream, "isubiu      %s, %s, %s", VIREG[rt], VIREG[rs], signed_11bit(op));
			break;
		case 0x10: // FCEQ
			util::stream_format(stream, "fceq        $vi01, %06x", op & 0xffffff);
			break;
		case 0x11: // FCSET
			util::stream_format(stream, "fcset       %06x", op & 0xffffff);
			break;
		case 0x12: // FCAND
			util::stream_format(stream, "fcand       $vi01, %06x", op & 0xffffff);
			break;
		case 0x13: // FCOR
			util::stream_format(stream, "fcor        $vi01, %06x", op & 0xffffff);
			break;
		case 0x14: // FSEQ
			util::stream_format(stream, "fseq        %s, %s", VIREG[rt], op & 0xfff);
			break;
		case 0x15: // FSSET
			util::stream_format(stream, "fsset       %03x", op & 0xfff);
			break;
		case 0x16: // FSAND
			util::stream_format(stream, "fsand       %s, %s", VIREG[rt], op & 0xfff);
			break;
		case 0x17: // FSOR
			util::stream_format(stream, "fsor        %s, %s", VIREG[rt], op & 0xfff);
			break;
		case 0x18: // FMEQ
			util::stream_format(stream, "fmeq        %s, %s", VIREG[rt], VIREG[rs]);
			break;
		case 0x1a: // FMAND
			util::stream_format(stream, "fmand       %s, %s", VIREG[rt], VIREG[rs]);
			break;
		case 0x1b: // FMOR
			util::stream_format(stream, "fmor        %s, %s", VIREG[rt], VIREG[rs]);
			break;
		case 0x1c: // FCGET
			util::stream_format(stream, "fcget       %s", VIREG[rt]);
			break;
		case 0x20: // B
			util::stream_format(stream, "b           %s", signed_11bit_x8(op));
			break;
		case 0x21: // BAL
			util::stream_format(stream, "bal         %s, %s", VIREG[rt], signed_11bit_x8(op));
			break;
		case 0x24: // JR
			util::stream_format(stream, "jr          %s", VIREG[rs]);
			break;
		case 0x25: // JALR
			util::stream_format(stream, "jalr        %s, %s", VIREG[rt], VIREG[rs]);
			break;
		case 0x28: // IBEQ
			util::stream_format(stream, "ibeq        %s, %s, %s", VIREG[rt], VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x29: // IBNE
			util::stream_format(stream, "ibne        %s, %s, %s", VIREG[rt], VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x2c: // IBLTZ
			util::stream_format(stream, "ibltz       %s, %s", VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x2d: // IBGTZ
			util::stream_format(stream, "ibgtz       %s, %s", VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x2e: // IBLEZ
			util::stream_format(stream, "iblez       %s, %s", VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x2f: // IBGEZ
			util::stream_format(stream, "ibgez       %s, %s", VIREG[rs], signed_11bit_x8(op));
			break;
		case 0x40: // SPECIAL
		{
			if ((op & 0x3c) == 0x3c)
			{
				uint8_t type4_op = ((op & 0x7c0) >> 4) | (op & 3);
				switch (type4_op)
				{
					case 0x30: // MOVE
						if (((op >> 21) & 15) == 0 && rt == 0 && rs == 0)
							util::stream_format(stream, "nop");
						else
							util::stream_format(stream, "move.%s   %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest);
						break;
					case 0x31: // MR32
						util::stream_format(stream, "mr32.%s   %s%s %s%s", dest, VFREG[rt], destc, VFREG[rs], dest);
						break;
					case 0x34: // LQI
						util::stream_format(stream, "lqi.%s    %s%s (%s++)", dest, VFREG[rt], destc, VIREG[rs]);
						break;
					case 0x35: // SQI
						util::stream_format(stream, "sqi.%s    %s%s (%s++)", dest, VFREG[rs], destc, VIREG[rt]);
						break;
					case 0x36: // LQD
						util::stream_format(stream, "lqi.%s    %s%s (--%s)", dest, VFREG[rt], destc, VIREG[rs]);
						break;
					case 0x37: // SQD
						util::stream_format(stream, "sqi.%s    %s%s (--%s)", dest, VFREG[rs], destc, VIREG[rt]);
						break;
					case 0x38: // DIV
						util::stream_format(stream, "div         Q, %s%s %s%s", VFREG[rs], fsfc, VFREG[rt], ftf);
						break;
					case 0x39: // SQRT
						util::stream_format(stream, "sqrt        Q, %s%s", dest, VFREG[rt], ftf);
						break;
					case 0x3a: // RSQRT
						util::stream_format(stream, "rsqrt       Q, %s%s %s%s", VFREG[rs], fsfc, VFREG[rt], ftf);
						break;
					case 0x3b: // WAITQ
						util::stream_format(stream, "waitq");
						break;
					case 0x3c: // MTIR
						util::stream_format(stream, "mtir        %s, %s%s", VIREG[rt], VFREG[rs], fsf);
						break;
					case 0x3d: // MFIR
						util::stream_format(stream, "mtir.%s   %s%s %s", dest, VFREG[rt], destc, VIREG[rs]  );
						break;
					case 0x3e: // ILWR
						util::stream_format(stream, "ilwr.%s   %s, (%s)%s", dest, VIREG[rt], VIREG[rs], dest);
						break;
					case 0x3f: // ISWR
						util::stream_format(stream, "iswr.%s   %s, (%s)%s", dest, VIREG[rt], VIREG[rs], dest);
						break;
					case 0x40: // RNEXT
						util::stream_format(stream, "rnext.%s  %s%s R", dest, VFREG[rt], destc);
						break;
					case 0x41: // RGET
						util::stream_format(stream, "rget.%s   %s%s R", dest, VFREG[rt], destc);
						break;
					case 0x42: // RINIT
						util::stream_format(stream, "rinit       R, %s%s", dest, VFREG[rs], dest);
						break;
					case 0x43: // RXOR
						util::stream_format(stream, "rxor        R, %s%s", dest, VFREG[rs], dest);
						break;
					case 0x64: // MFP
						util::stream_format(stream, "mfp.%s    %s%s P", dest, VFREG[rt], destc);
						break;
					case 0x68: // XTOP
						util::stream_format(stream, "xtop        %s", VIREG[rt]);
						break;
					case 0x69: // XITOP
						util::stream_format(stream, "xitop       %s", VIREG[rt]);
						break;
					case 0x6c: // XGKICK
						util::stream_format(stream, "xgkick      %s", VIREG[rs]);
						break;
					case 0x70: // ESADD
						util::stream_format(stream, "esadd       P, %s", VFREG[rs]);
						break;
					case 0x71: // ERSADD
						util::stream_format(stream, "ersadd      P, %s", VFREG[rs]);
						break;
					case 0x72: // ELENG
						util::stream_format(stream, "eleng       P, %s", VFREG[rs]);
						break;
					case 0x73: // ERLENG
						util::stream_format(stream, "erleng      P, %s", VFREG[rs]);
						break;
					case 0x74: // EATANxy
						util::stream_format(stream, "eatanxy     P, %s", VFREG[rs]);
						break;
					case 0x75: // EATANxz
						util::stream_format(stream, "eatanxz     P, %s", VFREG[rs]);
						break;
					case 0x76: // ESUM
						util::stream_format(stream, "esum        P, %s", VFREG[rs]);
						break;
					case 0x78: // ESQRT
						util::stream_format(stream, "esqrt       P, %s%s", VFREG[rs], fsf);
						break;
					case 0x79: // ERSQRT
						util::stream_format(stream, "ersqrt      P, %s%s", VFREG[rs], fsf);
						break;
					case 0x7a: // ERCPR
						util::stream_format(stream, "ercpr       P, %s%s", VFREG[rs], fsf);
						break;
					case 0x7b: // WAITP
						util::stream_format(stream, "waitp");
						break;
					case 0x7c: // ESIN
						util::stream_format(stream, "esin        P, %s%s", VFREG[rs], fsf);
						break;
					case 0x7d: // EATAN
						util::stream_format(stream, "eatan       P, %s%s", VFREG[rs], fsf);
						break;
					case 0x7e: // EEXP
						util::stream_format(stream, "eexp        P, %s%s", VFREG[rs], fsf);
						break;
					default:
						util::stream_format(stream, "invalid");
						break;
				}
				break;
			}
			else
			{
				switch (op & 0x3f)
				{
					case 0x30: // IADD
						util::stream_format(stream, "iadd        %s, %s, %s", VIREG[rd], VIREG[rs], VIREG[rt]);
						break;
					case 0x31: // ISUB
						util::stream_format(stream, "isub        %s, %s, %s", VIREG[rd], VIREG[rs], VIREG[rt]);
						break;
					case 0x32: // IADDI
						util::stream_format(stream, "iaddi       %s, %s, %s", VIREG[rt], VIREG[rs], signed_5bit_rd(op));
						break;
					case 0x34: // IAND
						util::stream_format(stream, "iand        %s, %s, %s", VIREG[rd], VIREG[rs], VIREG[rt]);
						break;
					case 0x35: // IOR
						util::stream_format(stream, "ior         %s, %s, %s", VIREG[rd], VIREG[rs], VIREG[rt]);
						break;
					default:
						util::stream_format(stream, "invalid");
						break;
				}
			}
			break;
		}
		default:
			util::stream_format(stream, "invalid");
			break;
	}
}

uint32_t sonyvu_disassembler::opcode_alignment() const
{
	return 4;
}

std::string sonyvu_disassembler::signed_5bit(uint16_t val)
{
	int16_t sval = (int32_t)val;
	sval <<= 11;
	sval >>= 11;

	if (sval < 0)
		return util::string_format("-$%x", -sval);
	else
		return util::string_format("$%x", sval);
}

std::string sonyvu_disassembler::signed_5bit_rd(uint16_t val)
{
	int16_t sval = (int32_t)((val >> 6) & 0x1f);
	sval <<= 11;
	sval >>= 11;

	if (sval < 0)
		return util::string_format("-$%x", -sval);
	else
		return util::string_format("$%x", sval);
}

std::string sonyvu_disassembler::unsigned_11bit(uint16_t val)
{
	val <<= 5;
	val >>= 5;
	return util::string_format("$%x", val);
}

std::string sonyvu_disassembler::signed_11bit(uint16_t val)
{
	int16_t sval = (int32_t)val;
	sval <<= 5;
	sval >>= 5;

	if (sval < 0)
		return util::string_format("-$%x", -sval);
	else
		return util::string_format("$%x", sval);
}

std::string sonyvu_disassembler::signed_11bit_x8(uint16_t val)
{
	int16_t sval = (int32_t)val;
	sval <<= 5;
	sval >>= 5;

	if (sval < 0)
		return util::string_format("-$%x", -sval * 8);
	else
		return util::string_format("$%x", sval * 8);
}
