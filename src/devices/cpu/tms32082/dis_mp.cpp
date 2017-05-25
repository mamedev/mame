// license:BSD-3-Clause
// copyright-holders:Ville Linde
// TMS32082 MP Disassembler

#include "emu.h"


#define SIMM15(v) (int32_t)((v & 0x4000) ? (v | 0xffffe000) : (v))
#define UIMM15(v) (v)

static const char *BCND_CONDITION[32] =
{
	"nev.b",    "gt0.b",    "eq0.b",    "ge0.b",    "lt0.b",    "ne0.b",    "le0.b",    "alw.b",
	"nev.h",    "gt0.h",    "eq0.h",    "ge0.h",    "lt0.h",    "ne0.h",    "le0.h",    "alw.h",
	"nev.w",    "gt0.w",    "eq0.w",    "ge0.w",    "lt0.w",    "ne0.w",    "le0.w",    "alw.w",
	"nev.d",    "gt0.d",    "eq0.d",    "ge0.d",    "lt0.d",    "ne0.d",    "le0.d",    "alw.d",
};

static const char *BITNUM_CONDITION[32] =
{
	"eq.b",     "ne.b",     "gt.b",     "le.b",     "lt.b",     "ge.b",     "hi.b",     "ls.b",
	"lo.b",     "hs.b",     "eq.h",     "ne.h",     "gt.h",     "le.h",     "lt.h",     "ge.h",
	"hi.h",     "ls.h",     "lo.h",     "hs.h",     "eq.w",     "ne.w",     "gt.w",     "le.w",
	"lt.w",     "ge.w",     "hi.w",     "ls.w",     "lo.w",     "hs.w",     "?",        "?",
};

static const char *MEMOP_S[2] =
{
	":s", ""
};

static const char *MEMOP_M[2] =
{
	":m", ""
};

static const char *FLOATOP_PRECISION[4] =
{
	"s", "d", "i", "u"
};

static const char *ACC_SEL[4] =
{
	"A0", "A1", "A2", "A3"
};

static const char *FLOATOP_ROUND[4] =
{
	"n", "z", "p", "m"
};

static std::ostream *output;
static const uint8_t *opdata;
static int opbytes;

static uint32_t fetch(void)
{
	uint32_t d = ((uint32_t)(opdata[0]) << 24) | ((uint32_t)(opdata[1]) << 16) | ((uint32_t)(opdata[2]) << 8) | opdata[3];
	opdata += 4;
	opbytes += 4;
	return d;
}

static char* get_creg_name(uint32_t reg)
{
	static char buffer[64];

	switch (reg)
	{
		case 0x0000:    sprintf(buffer, "EPC"); break;
		case 0x0001:    sprintf(buffer, "EIP"); break;
		case 0x0002:    sprintf(buffer, "CONFIG"); break;
		case 0x0004:    sprintf(buffer, "INTPEN"); break;
		case 0x0006:    sprintf(buffer, "IE"); break;
		case 0x0008:    sprintf(buffer, "FPST"); break;
		case 0x000a:    sprintf(buffer, "PPERROR"); break;
		case 0x000d:    sprintf(buffer, "PKTREQ"); break;
		case 0x000e:    sprintf(buffer, "TCOUNT"); break;
		case 0x000f:    sprintf(buffer, "TSCALE"); break;
		case 0x0010:    sprintf(buffer, "FLTOP"); break;
		case 0x0011:    sprintf(buffer, "FLTADR"); break;
		case 0x0012:    sprintf(buffer, "FLTTAG"); break;
		case 0x0013:    sprintf(buffer, "FLTDTL"); break;
		case 0x0014:    sprintf(buffer, "FLTDTH"); break;
		case 0x0020:    sprintf(buffer, "SYSSTK"); break;
		case 0x0021:    sprintf(buffer, "SYSTMP"); break;
		case 0x0030:    sprintf(buffer, "MPC"); break;
		case 0x0031:    sprintf(buffer, "MIP"); break;
		case 0x0033:    sprintf(buffer, "ECOMCNTL"); break;
		case 0x0034:    sprintf(buffer, "ANASTAT"); break;
		case 0x0039:    sprintf(buffer, "BRK1"); break;
		case 0x003a:    sprintf(buffer, "BRK2"); break;
		case 0x4000:    sprintf(buffer, "IN0P"); break;
		case 0x4001:    sprintf(buffer, "IN1P"); break;
		case 0x4002:    sprintf(buffer, "OUTP"); break;
		default:        sprintf(buffer, "CR %04X", reg);
	}

	return buffer;
}

static char* format_vector_op(uint32_t op, uint32_t imm32)
{
	static char buffer[256];
	static char dest[64];
	char *b = buffer;

	int rd = (op >> 27) & 0x1f;
	int rs = (op >> 22) & 0x1f;
	int src1 = (op & 0x1f);
	int subop = (op >> 12) & 0xff;
	int vector_ls_bits = (((op >> 9) & 0x3) << 1) | ((op >> 6) & 1);

	int p1 = (op >> 5) & 1;
	int pd2 = (op >> 7) & 1;
	int pd4 = (op >> 7) & 3;

	int z = op & (1 << 8);

	int acc = (((op >> 16) << 1) & 2) | ((op >> 11) & 1);
	bool regdest = (op & (1 << 10)) == 0 && (op & (1 << 6)) == 0;

	// accumulator or register destination
	if (regdest)
		sprintf(dest, "R%d", rd);
	else
		sprintf(dest, "A%d", acc);

	// base op
	switch (subop)
	{
		case 0xc0:  b += sprintf(b, "vadd.%s%s     R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], src1, rs, rs); break;
		case 0xc1:  b += sprintf(b, "vadd.%s%s     0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], imm32, rs, rs); break;
		case 0xc2:  b += sprintf(b, "vsub.%s%s     R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], rs, src1, rs); break;
		case 0xc3:  b += sprintf(b, "vsub.%s%s     R%d, 0x%08X, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], rs, imm32, rs); break;
		case 0xc4:  b += sprintf(b, "vmpy.%s%s     R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], src1, rs, rs); break;
		case 0xc5:  b += sprintf(b, "vmpy.%s%s     0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd2], imm32, rs, rs); break;

		case 0xd6: case 0xc6:
					b += sprintf(b, "vmsub.s%s    R%d, %s, R%d", FLOATOP_PRECISION[pd2], src1, z ? "0" : ACC_SEL[acc], rs);
					break;
		case 0xd7: case 0xc7:
					b += sprintf(b, "vmsub.s%s    0x%08X, %s, R%d", FLOATOP_PRECISION[pd2], imm32, z ? "0" : ACC_SEL[acc], rs);
					break;
		case 0xd8: case 0xc8:
					b += sprintf(b, "vrnd.%s%s     R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd4], src1, rs);
					break;
		case 0xd9: case 0xc9:
					b += sprintf(b, "vrnd.%s%s     0x%08X, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd4], imm32, rs);
					break;

		case 0xca:  b += sprintf(b, "vrnd.%s%s     R%d, R%d", FLOATOP_PRECISION[2 + p1], FLOATOP_PRECISION[pd2],src1, rs); break;
		case 0xcb:  b += sprintf(b, "vrnd.%s%s     0x%08X, R%d", FLOATOP_PRECISION[2 + p1], FLOATOP_PRECISION[pd2], imm32, rs); break;

		case 0xcc: case 0xdc:
					b += sprintf(b, "vmac.ss%s    R%d, R%d, %s, %s", FLOATOP_PRECISION[(op >> 9) & 1], src1, rs, z ? "0" : ACC_SEL[acc], (regdest && rd == 0) ? ACC_SEL[acc] : dest);
					break;
		case 0xcd: case 0xdd:
					b += sprintf(b, "vmac.ss%s    0x%08X, R%d, %s, %s", FLOATOP_PRECISION[(op >> 9) & 1], imm32, rs, z ? "0" : ACC_SEL[acc], (regdest && rd == 0) ? ACC_SEL[acc] : dest);
					break;
		case 0xce: case 0xde:
					b += sprintf(b, "vmsc.ss%s    R%d, R%d, %s, %s", FLOATOP_PRECISION[(op >> 9) & 1], src1, rs, z ? "0" : ACC_SEL[acc], (regdest && rd == 0) ? ACC_SEL[acc] : dest);
					break;
		case 0xcf: case 0xdf:
					b += sprintf(b, "vmsc.ss%s    0x%08X, R%d, %s, %s", FLOATOP_PRECISION[(op >> 9) & 1], imm32, rs, z ? "0" : ACC_SEL[acc], (regdest && rd == 0) ? ACC_SEL[acc] : dest);
					break;

		default:    b += sprintf(b, "?"); break;
	}

	// align the line end
	int len = strlen(buffer);
	if (len < 30)
	{
		for (int i=0; i < (30-len); i++)
		{
			b += sprintf(b, " ");
		}
	}

	// optional load/store op
	switch (vector_ls_bits)
	{
		case 0x01:      b += sprintf(b, "|| vst.s   R%d", rd); break;
		case 0x03:      b += sprintf(b, "|| vst.d   R%d", rd); break;
		case 0x04:      b += sprintf(b, "|| vld0.s  R%d", rd); break;
		case 0x05:      b += sprintf(b, "|| vld1.s  R%d", rd); break;
		case 0x06:      b += sprintf(b, "|| vld0.d  R%d", rd); break;
		case 0x07:      b += sprintf(b, "|| vld1.d  R%d", rd); break;
	}

	return buffer;
}

static offs_t tms32082_disasm_mp(std::ostream &stream, offs_t pc, const uint8_t *oprom)
{
	output = &stream;
	opdata = oprom;
	opbytes = 0;
	uint32_t flags = 0;

	uint32_t op = fetch();

	int rd = (op >> 27) & 0x1f;
	int link = rd;
	int bitnum = rd ^ 0x1f;
	int rs = (op >> 22) & 0x1f;
	int endmask = (op >> 5) & 0x1f;
	int rotate = (op & 0x1f);
	int src1 = rotate;
	uint32_t uimm15 = op & 0x7fff;

	switch ((op >> 20) & 3)
	{
		case 0: case 1: case 2:     // Short immediate
		{
			int subop = (op >> 15) & 0x7f;
			int m = op & (1 << 17) ? 0 : 1;

			switch (subop)
			{
				case 0x00:  util::stream_format(*output, "illop0      "); break;
				case 0x01:  util::stream_format(*output, "trap        %d", UIMM15(uimm15)); break;
				case 0x02:  util::stream_format(*output, "cmnd        0x%04X", UIMM15(uimm15)); break;

				case 0x04:
					if (op == 0x00020000)
						util::stream_format(*output, "nop         ");
					else
						util::stream_format(*output, "rdcr        %s, R%d", get_creg_name(UIMM15(uimm15)), rd);
					break;

				case 0x05:  util::stream_format(*output, "swcr        R%d, %s, R%d", rd, get_creg_name(UIMM15(uimm15)), rs); break;
				case 0x06:  util::stream_format(*output, "brcr        %s", get_creg_name(UIMM15(uimm15))); break;
				case 0x08:  util::stream_format(*output, "shift%s.dz   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x09:  util::stream_format(*output, "shift%s.dm   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0a:  util::stream_format(*output, "shift%s.ds   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0b:  util::stream_format(*output, "shift%s.ez   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0c:  util::stream_format(*output, "shift%s.em   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0d:  util::stream_format(*output, "shift%s.es   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0e:  util::stream_format(*output, "shift%s.iz   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x0f:  util::stream_format(*output, "shift%s.im   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x11:  util::stream_format(*output, "and         0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x12:  util::stream_format(*output, "and.tf      0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x14:  util::stream_format(*output, "and.ft      0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x16:  util::stream_format(*output, "xor         0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x17:  util::stream_format(*output, "or          0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x18:  util::stream_format(*output, "and.ff      0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x19:  util::stream_format(*output, "xnor        0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x1b:  util::stream_format(*output, "or.tf       0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x1d:  util::stream_format(*output, "or.ft       0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;
				case 0x1e:  util::stream_format(*output, "or.ff       0x%04X, R%d, R%d", UIMM15(uimm15), rs, rd); break;

				case 0x24: case 0x20:
							util::stream_format(*output, "ld.b        0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;
				case 0x25: case 0x21:
							util::stream_format(*output, "ld.h        0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;
				case 0x26: case 0x22:
							util::stream_format(*output, "ld          0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;
				case 0x27: case 0x23:
							util::stream_format(*output, "ld.d        0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;
				case 0x2c: case 0x28:
							util::stream_format(*output, "ld.ub       0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;
				case 0x2d: case 0x29:
							util::stream_format(*output, "ld.uh       0x%04X(R%d%s), R%d", UIMM15(uimm15), rs, MEMOP_M[m], rd);
							break;

				case 0x34: case 0x30:
							util::stream_format(*output, "st.b        R%d, 0x%04X(R%d%s)", rd, UIMM15(uimm15), rs, MEMOP_M[m]);
							break;
				case 0x35: case 0x31:
							util::stream_format(*output, "st.h        R%d, 0x%04X(R%d%s)", rd, UIMM15(uimm15), rs, MEMOP_M[m]);
							break;
				case 0x36: case 0x32:
							util::stream_format(*output, "st          R%d, 0x%04X(R%d%s)", rd, UIMM15(uimm15), rs, MEMOP_M[m]);
							break;
				case 0x37: case 0x33:
							util::stream_format(*output, "st.d        R%d, 0x%04X(R%d%s)", rd, UIMM15(uimm15), rs, MEMOP_M[m]);
							break;

				case 0x40:  util::stream_format(*output, "bsr         0x%08X, R%d", pc + (SIMM15(uimm15) * 4), link); break;
				case 0x41:  util::stream_format(*output, "bsr.a       0x%08X, R%d", pc + (SIMM15(uimm15) * 4), link); break;
				case 0x44:  util::stream_format(*output, "jsr         0x%04X(R%d), R%d", SIMM15(uimm15), rs, link); break;
				case 0x45:  util::stream_format(*output, "jsr.a       0x%04X(R%d), R%d", SIMM15(uimm15), rs, link); break;
				case 0x48:  util::stream_format(*output, "bbz         0x%08X, R%d, %s (%d)", pc + (SIMM15(uimm15) * 4), rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x49:  util::stream_format(*output, "bbz.a       0x%08X, R%d, %s (%d)", pc + (SIMM15(uimm15) * 4), rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x4a:  util::stream_format(*output, "bbo         0x%08X, R%d, %s (%d)", pc + (SIMM15(uimm15) * 4), rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x4b:  util::stream_format(*output, "bbo.a       0x%08X, R%d, %s (%d)", pc + (SIMM15(uimm15) * 4), rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x4c:  util::stream_format(*output, "bcnd        0x%08X, R%d, %s", pc + (SIMM15(uimm15) * 4), rs, BCND_CONDITION[rd]); break;
				case 0x4d:  util::stream_format(*output, "bcnd.a      0x%08X, R%d, %s", pc + (SIMM15(uimm15) * 4), rs, BCND_CONDITION[rd]); break;
				case 0x50:  util::stream_format(*output, "cmp         0x%08X, R%d, R%d", SIMM15(uimm15), rs, rd); break;
				case 0x58:  util::stream_format(*output, "add         0x%08X, R%d, R%d", SIMM15(uimm15), rs, rd); break;
				case 0x59:  util::stream_format(*output, "addu        0x%08X, R%d, R%d", SIMM15(uimm15), rs, rd); break;
				case 0x5a:  util::stream_format(*output, "sub         0x%08X, R%d, R%d", SIMM15(uimm15), rs, rd); break;
				case 0x5b:  util::stream_format(*output, "subu        0x%08X, R%d, R%d", SIMM15(uimm15), rs, rd); break;

				default:    util::stream_format(*output, "?"); break;
			}
			break;
		}

		case 3:                     // Register / Long immediate
		{
			int subop = (op >> 12) & 0xff;

			uint32_t imm32 = 0;
			if (op & (1 << 12))     // fetch 32-bit immediate if needed
				imm32 = fetch();

			int m = op & (1 << 15) ? 0 : 1;
			int s = op & (1 << 11) ? 0 : 1;

			int p1 = (op >> 5) & 3;
			int p2 = (op >> 7) & 3;
			int pd = (op >> 9) & 3;

			int rndmode = (op >> 7) & 3;


			switch (subop)
			{
				case 0x02:  util::stream_format(*output, "trap        %d", src1); break;
				case 0x03:  util::stream_format(*output, "trap        %d", imm32); break;
				case 0x04:  util::stream_format(*output, "cmnd        R%d", src1); break;
				case 0x05:  util::stream_format(*output, "cmnd        0x%08X", imm32); break;
				case 0x08:  util::stream_format(*output, "rdcr        R%d, R%d,", src1, rd); break;
				case 0x09:  util::stream_format(*output, "rdcr        %s, R%d", get_creg_name(imm32), rd); break;
				case 0x0a:  util::stream_format(*output, "swcr        R%d, R%d, R%d", rd, src1, rs); break;
				case 0x0b:  util::stream_format(*output, "swcr        R%d, %s, R%d", rd, get_creg_name(imm32), rs); break;
				case 0x0c:  util::stream_format(*output, "brcr        R%d", src1); break;
				case 0x0d:  util::stream_format(*output, "brcr        %s", get_creg_name(imm32)); break;

				case 0x10:  util::stream_format(*output, "shift%s.dz   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x12:  util::stream_format(*output, "shift%s.dm   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x14:  util::stream_format(*output, "shift%s.ds   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x16:  util::stream_format(*output, "shift%s.ez   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x18:  util::stream_format(*output, "shift%s.em   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x1a:  util::stream_format(*output, "shift%s.es   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x1c:  util::stream_format(*output, "shift%s.iz   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;
				case 0x1e:  util::stream_format(*output, "shift%s.im   %d, %d, R%d, R%d", (op & (1 << 10)) ? "r" : "l", rotate, endmask, rs, rd); break;

				case 0x22:  util::stream_format(*output, "and         R%d, R%d, R%d", src1, rs, rd); break;
				case 0x23:  util::stream_format(*output, "and         0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x24:  util::stream_format(*output, "and.tf      R%d, R%d, R%d", src1, rs, rd); break;
				case 0x25:  util::stream_format(*output, "and.tf      0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x28:  util::stream_format(*output, "and.ft      R%d, R%d, R%d", src1, rs, rd); break;
				case 0x29:  util::stream_format(*output, "and.ft      0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x2c:  util::stream_format(*output, "xor         R%d, R%d, R%d", src1, rs, rd); break;
				case 0x2d:  util::stream_format(*output, "xor         0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x2e:  util::stream_format(*output, "or          R%d, R%d, R%d", src1, rs, rd); break;
				case 0x2f:  util::stream_format(*output, "or          0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x30:  util::stream_format(*output, "and.ff      R%d, R%d, R%d", src1, rs, rd); break;
				case 0x31:  util::stream_format(*output, "and.ff      0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x32:  util::stream_format(*output, "xnor        R%d, R%d, R%d", src1, rs, rd); break;
				case 0x33:  util::stream_format(*output, "xnor        0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x36:  util::stream_format(*output, "or.tf       R%d, R%d, R%d", src1, rs, rd); break;
				case 0x37:  util::stream_format(*output, "or.tf       0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x3a:  util::stream_format(*output, "or.ft       R%d, R%d, R%d", src1, rs, rd); break;
				case 0x3b:  util::stream_format(*output, "or.ft       0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0x3c:  util::stream_format(*output, "or.ff       R%d, R%d, R%d", src1, rs, rd); break;
				case 0x3d:  util::stream_format(*output, "or.ff       0x%08X, R%d, R%d", imm32, rs, rd); break;

				case 0x48: case 0x40:
							util::stream_format(*output, "ld.b        R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x49: case 0x41:
							util::stream_format(*output, "ld.b        0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x4a: case 0x42:
							util::stream_format(*output, "ld.h        R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x4b: case 0x43:
							util::stream_format(*output, "ld.h        0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x4c: case 0x44:
							util::stream_format(*output, "ld          R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x4d: case 0x45:
							util::stream_format(*output, "ld          0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs,MEMOP_M[m], rd);
							break;
				case 0x4e: case 0x46:
							util::stream_format(*output, "ld.d        R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x4f: case 0x47:
							util::stream_format(*output, "ld.d        0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x58: case 0x50:
							util::stream_format(*output, "ld.ub       R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x59: case 0x51:
							util::stream_format(*output, "ld.ub       0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x5a: case 0x52:
							util::stream_format(*output, "ld.uh       R%d%s(R%d%s), R%d", src1, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;
				case 0x5b: case 0x53:
							util::stream_format(*output, "ld.uh       0x%08X%s(R%d%s), R%d", imm32, MEMOP_S[s], rs, MEMOP_M[m], rd);
							break;

				case 0x68: case 0x60:
							util::stream_format(*output, "st.b        R%d, R%d%s(R%d%s)", rd, src1, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x69: case 0x61:
							util::stream_format(*output, "st.b        R%d, 0x%08X%s(R%d%s)", rd, imm32, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6a: case 0x62:
							util::stream_format(*output, "st.h        R%d, R%d%s(R%d%s)", rd, src1, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6b: case 0x63:
							util::stream_format(*output, "st.h        R%d, 0x%08X%s(R%d%s)", rd, imm32, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6c: case 0x64:
							util::stream_format(*output, "st          R%d, R%d%s(R%d%s)", rd, src1, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6d: case 0x65:
							util::stream_format(*output, "st          R%d, 0x%08X%s(R%d%s)", rd, imm32, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6e: case 0x66:
							util::stream_format(*output, "st.d        R%d, R%d%s(R%d%s)", rd, src1, MEMOP_S[s], rs, MEMOP_M[m]);
							break;
				case 0x6f: case 0x67:
							util::stream_format(*output, "st.d        R%d, 0x%08X%s(R%d%s)", rd, imm32, MEMOP_S[s], rs, MEMOP_M[m]);
							break;

				case 0x78: case 0x70:
							util::stream_format(*output, "dcache      R%d(R%d)", src1, rs);
							break;
				case 0x79: case 0x71:
							util::stream_format(*output, "dcache      0x%08X(R%d)", imm32, rs);
							break;

				case 0x80:  util::stream_format(*output, "bsr         R%d, R%d", src1, link); break;
				case 0x81:  util::stream_format(*output, "bsr         0x%08X, R%d", imm32, link); break;
				case 0x82:  util::stream_format(*output, "bsr.a       R%d, R%d", src1, rd); break;
				case 0x83:  util::stream_format(*output, "bsr.a       0x%08X, R%d", imm32, link); break;
				case 0x88:  util::stream_format(*output, "jsr         R%d, R%d", src1, link); break;
				case 0x89:  util::stream_format(*output, "jsr         0x%08X, R%d", imm32, link); break;
				case 0x8a:  util::stream_format(*output, "jsr.a       R%d, R%d", src1, link); break;
				case 0x8b:  util::stream_format(*output, "jsr.a       0x%08X, R%d", imm32, link); break;
				case 0x90:  util::stream_format(*output, "bbz         R%d, R%d, %s (%d)", src1, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x91:  util::stream_format(*output, "bbz         0x%08X, R%d, %s (%d)", imm32, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x92:  util::stream_format(*output, "bbz.a       R%d, R%d, %s (%d)", src1, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x93:  util::stream_format(*output, "bbz.a       0x%08X, R%d, %s (%d)", imm32, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x94:  util::stream_format(*output, "bbo         R%d, R%d, %s (%d)", src1, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x95:  util::stream_format(*output, "bbo         0x%08X, R%d, %s (%d)", imm32, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x96:  util::stream_format(*output, "bbo.a       R%d, R%d, %s (%d)", src1, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x97:  util::stream_format(*output, "bbo.a       0x%08X, R%d, %s (%d)", imm32, rs, BITNUM_CONDITION[bitnum], bitnum); break;
				case 0x98:  util::stream_format(*output, "bcnd        R%d, R%d, %s", src1, rs, BCND_CONDITION[rd]); break;
				case 0x99:  util::stream_format(*output, "bcnd        0x%08X, R%d, %s", imm32, rs, BCND_CONDITION[rd]); break;
				case 0x9a:  util::stream_format(*output, "bcnd.a      R%d, R%d, %s", src1, rs, BCND_CONDITION[rd]); break;
				case 0x9b:  util::stream_format(*output, "bcnd.a      0x%08X, R%d, %s", imm32, rs, BCND_CONDITION[rd]); break;
				case 0xa0:  util::stream_format(*output, "cmp         R%d, R%d, R%d", src1, rs, rd); break;
				case 0xa1:  util::stream_format(*output, "cmp         0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0xb0:  util::stream_format(*output, "add         R%d, R%d, R%d", src1, rs, rd); break;
				case 0xb1:  util::stream_format(*output, "add         0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0xb2:  util::stream_format(*output, "addu        R%d, R%d, R%d", src1, rs, rd); break;
				case 0xb3:  util::stream_format(*output, "addu        0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0xb4:  util::stream_format(*output, "sub         R%d, R%d, R%d", src1, rs, rd); break;
				case 0xb5:  util::stream_format(*output, "sub         0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0xb6:  util::stream_format(*output, "subu        R%d, R%d, R%d", src1, rs, rd); break;
				case 0xb7:  util::stream_format(*output, "subu        0x%08X, R%d, R%d", imm32, rs, rd); break;

				case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5:
				case 0xc6: case 0xd6: case 0xc7: case 0xd7: case 0xc8: case 0xd8: case 0xc9: case 0xd9:
				case 0xca: case 0xcb: case 0xcc: case 0xdc: case 0xcd: case 0xdd: case 0xce: case 0xde:
				case 0xcf: case 0xdf:
				{
					util::stream_format(*output, "%s", format_vector_op(op, imm32));
					break;
				}

				case 0xe0:  util::stream_format(*output, "fadd.%s%s%s    R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], src1, rs, rd); break;
				case 0xe1:  util::stream_format(*output, "fadd.%s%s%s    0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], imm32, rs, rd); break;
				case 0xe2:  util::stream_format(*output, "fsub.%s%s%s    R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], src1, rs, rd); break;
				case 0xe3:  util::stream_format(*output, "fsub.%s%s%s    0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], imm32, rs, rd); break;
				case 0xe4:  util::stream_format(*output, "fmpy.%s%s%s    R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], src1, rs, rd); break;
				case 0xe5:  util::stream_format(*output, "fmpy.%s%s%s    0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], imm32, rs, rd); break;
				case 0xe6:  util::stream_format(*output, "fdiv.%s%s%s    R%d, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], src1, rs, rd); break;
				case 0xe7:  util::stream_format(*output, "fdiv.%s%s%s    0x%08X, R%d, R%d", FLOATOP_PRECISION[p1], FLOATOP_PRECISION[p2], FLOATOP_PRECISION[pd], imm32, rs, rd); break;
				case 0xe8:  util::stream_format(*output, "frnd%s.%s%s    R%d, R%d", FLOATOP_ROUND[rndmode], FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd], src1, rd); break;
				case 0xe9:  util::stream_format(*output, "frnd%s.%s%s    0x%08X, R%d", FLOATOP_ROUND[rndmode], FLOATOP_PRECISION[p1], FLOATOP_PRECISION[pd], imm32, rd); break;
				case 0xea:  util::stream_format(*output, "fcmp        R%d, R%d, R%d", src1, rs, rd); break;
				case 0xeb:  util::stream_format(*output, "fcmp        0x%08X, R%d, R%d", imm32, rs, rd); break;
				case 0xee:  util::stream_format(*output, "fsqrt       R%d, R%d", src1, rd); break;
				case 0xef:  util::stream_format(*output, "fsqrt       0x%08X, R%d", imm32, rd); break;
				case 0xf0:  util::stream_format(*output, "lmo         R%d, R%d", rs, rd); break;
				case 0xf2:  util::stream_format(*output, "rmo         R%d, R%d", rs, rd); break;
				case 0xfc:  util::stream_format(*output, "estop       "); break;

				case 0xfe: case 0xff:
							util::stream_format(*output, "illopF      ");
							break;

				default:    util::stream_format(*output, "?"); break;
			}
			break;
		}
	}

	return opbytes | flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE(tms32082_mp)
{
	return tms32082_disasm_mp(stream, pc, oprom);
}
