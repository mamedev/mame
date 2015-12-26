// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#include "emu.h"
#include "debugger.h"
#include "hd61700.h"

#define EXT_ROM     (pc > 0x0c00)
#define INC_POS     pos += (type+1)
#define POS         (pos + type)

static const char *const reg_5b[4] =  {"sx", "sy", "sz", "sz"};
static const char *const reg_8b[8] =  {"pe", "pd", "ib", "ua", "ia", "ie", "tm", "tm"};
static const char *const reg_16b[8] = {"ix", "iy", "iz", "us", "ss", "ky", "ky", "ky"};
static const char *const jp_cond[8] = {"z", "nc", "lz", "uz", "nz", "c", "nlz"};

enum
{
	OP_NULL=0,
	OP_IM16,
	OP_IM16A,
	OP_IM3,
	OP_IM5,
	OP_IM7,
	OP_IM8,
	OP_IM8I,
	OP_IM8_,
	OP_IR_IM3,
	OP_IR_IM8,
	OP_IR_IM8_,
	OP_JX_COND,
	OP_MREG,
	OP_MREG2,
	OP_MR_SIR,
	OP_MR_SIRI,
	OP_REG16,
	OP_REG16_,
	OP_REG8,
	OP_REG8_,
	OP_REGIM8,
	OP_RMSIM3,
	OP_RSIR
};

struct hd61700_dasm
{
	const char *str;
	UINT8       arg1;
	UINT8       arg2;
	bool        optjr;
};

static const hd61700_dasm hd61700_ops[256] =
{
	// 0x00
	{ "adc",  OP_MREG,    OP_MR_SIR, 1 }, { "sbc",  OP_MREG,    OP_MR_SIR, 1 },
	{ "ld",   OP_MREG,    OP_MR_SIR, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "anc",  OP_MREG,    OP_MR_SIR, 1 }, { "nac",  OP_MREG,    OP_MR_SIR, 1 },
	{ "orc",  OP_MREG,    OP_MR_SIR, 1 }, { "xrc",  OP_MREG,    OP_MR_SIR, 1 },
	{ "ad",   OP_MREG,    OP_MR_SIR, 1 }, { "sb",   OP_MREG,    OP_MR_SIR, 1 },
	{ "adb",  OP_MREG,    OP_MR_SIR, 1 }, { "sbb",  OP_MREG,    OP_MR_SIR, 1 },
	{ "an",   OP_MREG,    OP_MR_SIR, 1 }, { "na",   OP_MREG,    OP_MR_SIR, 1 },
	{ "or",   OP_MREG,    OP_MR_SIR, 1 }, { "xr",   OP_MREG,    OP_MR_SIR, 1 },

	// 0x10
	{ "st",   OP_MREG,    OP_MR_SIRI,  1 }, { "ld",   OP_MREG,  OP_MR_SIRI,1 },
	{ "stl",  OP_MREG2,   OP_NULL,     1 }, { "ldl",  OP_MREG2, OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "psr",  OP_RSIR,  OP_MREG2,  1 },
	{ "pst",  OP_REG8,   OP_MREG2,    1 }, { "pst",  OP_REG8, OP_MREG2,  1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "illegal", OP_NULL, OP_NULL, 0 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "???",  OP_MREG2, OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "gsr",  OP_RSIR,  OP_MREG2,  1 },
	{ "gst",  OP_REG8,   OP_MREG2,    1 }, { "gst",  OP_REG8, OP_MREG2,  1 },

	// 0x20
	{ "st",   OP_MREG,    OP_IR_IM8_,  0 }, { "st",   OP_MREG,  OP_IR_IM8_,0 },
	{ "sti",  OP_MREG,    OP_IR_IM8_,  0 }, { "sti",  OP_MREG,  OP_IR_IM8_,0 },
	{ "std",  OP_MREG,    OP_IR_IM8_,  0 }, { "std",  OP_MREG,  OP_IR_IM8_,0 },
	{ "phs",  OP_MREG2,   OP_NULL,     0 }, { "phu",  OP_MREG2, OP_NULL,   0 },
	{ "ld",   OP_MREG,    OP_IR_IM8_,  0 }, { "ld",   OP_MREG,  OP_IR_IM8_,0 },
	{ "ldi",  OP_MREG,    OP_IR_IM8_,  0 }, { "ldi",  OP_MREG,  OP_IR_IM8_,0 },
	{ "ldd",  OP_MREG,    OP_IR_IM8_,  0 }, { "ldd",  OP_MREG,  OP_IR_IM8_,0 },
	{ "pps",  OP_MREG2,   OP_NULL,     0 }, { "ppu",  OP_MREG2, OP_NULL,   0 },

	// 0x30
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_IM16A,   OP_NULL,  0 },
	{ "adc",  OP_IR_IM8, OP_MREG2,   0 }, { "adc",  OP_IR_IM8, OP_MREG2, 0 },
	{ "sbc",  OP_IR_IM8, OP_MREG2,   0 }, { "sbc",  OP_IR_IM8, OP_MREG2, 0 },
	{ "ad",   OP_IR_IM8, OP_MREG2,   0 }, { "ad",   OP_IR_IM8, OP_MREG2, 0 },
	{ "sb",   OP_IR_IM8, OP_MREG2,   0 }, { "sb",   OP_IR_IM8, OP_MREG2, 0 },

	// 0x40
	{ "adc",  OP_MREG2,    OP_IM8,     1 }, { "sbc",  OP_MREG2,    OP_IM8, 1 },
	{ "ld",   OP_MREG2,    OP_IM8,     1 }, { "illegal", OP_NULL, OP_NULL, 0 },
	{ "anc",  OP_MREG2,    OP_IM8,     1 }, { "nac",  OP_MREG2,    OP_IM8, 1 },
	{ "orc",  OP_MREG2,    OP_IM8,     1 }, { "xrc",  OP_MREG2,    OP_IM8, 1 },
	{ "ad",   OP_MREG2,    OP_IM8,     1 }, { "sb",   OP_MREG2,    OP_IM8, 1 },
	{ "adb",  OP_MREG2,    OP_IM8,     1 }, { "sbb",  OP_MREG2,    OP_IM8, 1 },
	{ "an",   OP_MREG2,    OP_IM8,     1 }, { "na",   OP_MREG2,    OP_IM8, 1 },
	{ "or",   OP_MREG2,    OP_IM8,     1 }, { "xr",   OP_MREG2,    OP_IM8, 1 },

	// 0x50
	{ "st",   OP_IM8I,    OP_MREG2,  0 }, { "st",    OP_IM8,      OP_MREG2,0 },
	{ "stl",  OP_IM8_,    OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },
	{ "???",  OP_IM8_,    OP_NULL,   0 }, { "psr",   OP_RSIR,     OP_IM5,  0 },
	{ "pst",  OP_REG8_,   OP_IM8,    0 }, { "pst",   OP_REG8_,    OP_IM8,  0 },
	{ "bups", OP_IM8_,    OP_NULL,   0 }, { "bdns",  OP_IM8_,     OP_NULL, 0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },
	{ "sup",  OP_IM8_,    OP_NULL,   0 }, { "sdn",   OP_IM8_,     OP_NULL, 0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },

	// 0x60
	{ "st",   OP_MREG2,    OP_REGIM8, 0 }, { "st",   OP_MREG2,   OP_REGIM8, 0 },
	{ "sti",  OP_MREG2,    OP_REGIM8, 0 }, { "sti",  OP_MREG2,   OP_REGIM8, 0 },
	{ "std",  OP_MREG2,    OP_REGIM8, 0 }, { "std",  OP_MREG2,   OP_REGIM8, 0 },
	{ "illegal", OP_NULL,  OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ld",   OP_MREG2,    OP_REGIM8, 0 }, { "ld",   OP_MREG2,   OP_REGIM8, 0 },
	{ "ldi",  OP_MREG2,    OP_REGIM8, 0 }, { "ldi",  OP_MREG2,   OP_REGIM8, 0 },
	{ "ldd",  OP_MREG2,    OP_REGIM8, 0 }, { "ldd",  OP_MREG2,   OP_REGIM8, 0 },
	{ "illegal", OP_NULL,  OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },

	// 0x70
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_IM16A,   OP_NULL,   0 },
	{ "adc",  OP_REGIM8,   OP_MREG2,  0 }, { "adc",  OP_REGIM8,  OP_MREG2,  0 },
	{ "sbc",  OP_REGIM8,   OP_MREG2,  0 }, { "sbc",  OP_REGIM8,  OP_MREG2,  0 },
	{ "ad",   OP_REGIM8,   OP_MREG2,  0 }, { "ad ",  OP_REGIM8,  OP_MREG2,  0 },
	{ "sb",   OP_REGIM8,   OP_MREG2,  0 }, { "sb",   OP_REGIM8,  OP_MREG2,  0 },

	// 0x80
	{ "adcw", OP_MREG,    OP_MR_SIR, 1 }, { "sbcw", OP_MREG,    OP_MR_SIR, 1 },
	{ "ldw",  OP_MREG,    OP_MR_SIR, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ancw", OP_MREG,    OP_MR_SIR, 1 }, { "nacw", OP_MREG,    OP_MR_SIR, 1 },
	{ "orcw", OP_MREG,    OP_MR_SIR, 1 }, { "xrcw", OP_MREG,    OP_MR_SIR, 1 },
	{ "adw",  OP_MREG,    OP_MR_SIR, 1 }, { "sbw",  OP_MREG,    OP_MR_SIR, 1 },
	{ "adbw", OP_MREG,    OP_MR_SIR, 1 }, { "sbbw", OP_MREG,    OP_MR_SIR, 1 },
	{ "anw",  OP_MREG,    OP_MR_SIR, 1 }, { "naw",  OP_MREG,    OP_MR_SIR, 1 },
	{ "orw",  OP_MREG,    OP_MR_SIR, 1 }, { "xrw",  OP_MREG,    OP_MR_SIR, 1 },

	// 0x90
	{ "stw",  OP_MREG,    OP_MR_SIRI,1 }, { "ldw",  OP_MREG,    OP_MR_SIRI,1 },
	{ "stlw", OP_MREG2,   OP_NULL,   1 }, { "ldlw", OP_MREG2,   OP_NULL,   1 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "pre",  OP_REG16,   OP_MREG2,  1 }, { "pre",  OP_REG16,   OP_MREG2,  1 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "???",  OP_MREG2,   OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "gre",  OP_REG16,   OP_MREG2,  1 }, { "gre",  OP_REG16,   OP_MREG2,  1 },

	// 0xa0
	{ "stw",  OP_MREG,   OP_IR_IM8_, 0 }, { "stw",  OP_MREG,   OP_IR_IM8_, 0 },
	{ "stiw", OP_MREG,   OP_IR_IM8_, 0 }, { "stiw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "stdw", OP_MREG,   OP_IR_IM8_, 0 }, { "stdw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "phsw", OP_MREG2,  OP_NULL,    0 }, { "phuw", OP_MREG2,  OP_NULL,    0 },
	{ "ldw",  OP_MREG,   OP_IR_IM8_, 0 }, { "ldw",  OP_MREG,   OP_IR_IM8_, 0 },
	{ "ldiw", OP_MREG,   OP_IR_IM8_, 0 }, { "ldiw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "lddw", OP_MREG,   OP_IR_IM8_, 0 }, { "lddw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "ppsw", OP_MREG2,  OP_NULL,    0 }, { "ppuw", OP_MREG2,  OP_NULL,    0 },

	// 0xb0
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_IM7,     OP_NULL,   0 },
	{ "adcw", OP_IR_IM8, OP_MREG,   0 }, { "adcw", OP_IR_IM8, OP_MREG,   0 },
	{ "sbcw", OP_IR_IM8, OP_MREG,   0 }, { "sbcw", OP_IR_IM8, OP_MREG,   0 },
	{ "adw",  OP_IR_IM8, OP_MREG,   0 }, { "adw",  OP_IR_IM8, OP_MREG,   0 },
	{ "sbw",  OP_IR_IM8, OP_MREG,   0 }, { "sbw",  OP_IR_IM8, OP_MREG,   0 },

	// 0xc0
	{ "adbcm",OP_MREG,    OP_RMSIM3, 1 }, { "sdbcm",OP_MREG,    OP_RMSIM3, 1 },
	{ "ldm",  OP_MREG,    OP_RMSIM3, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ancm", OP_MREG,    OP_RMSIM3, 1 }, { "nacm", OP_MREG,    OP_RMSIM3, 1 },
	{ "orcm", OP_MREG,    OP_RMSIM3, 1 }, { "xrcm", OP_MREG,    OP_RMSIM3, 1 },
	{ "adbm", OP_MREG,    OP_RMSIM3, 1 }, { "sbbm", OP_MREG,    OP_RMSIM3, 1 },
	{ "adbm", OP_MREG,    OP_RMSIM3, 1 }, { "sbbm", OP_MREG,    OP_RMSIM3, 1 },
	{ "anm",  OP_MREG,    OP_RMSIM3, 1 }, { "nam",  OP_MREG,    OP_RMSIM3, 1 },
	{ "orm",  OP_MREG,    OP_RMSIM3, 1 }, { "xrm",  OP_MREG,    OP_RMSIM3, 1 },

	// 0xd0
	{ "stw",  OP_IM16,    OP_MR_SIR, 0 }, { "ldw",  OP_MREG2,   OP_IM16,   0 },
	{ "stlm", OP_MREG2,   OP_IM3,    0 }, { "ldlm", OP_MREG2,   OP_IM3,    0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "pre",  OP_REG16_,  OP_IM16,   0 }, { "pre",  OP_REG16_,  OP_IM16,   0 },
	{ "bup",  OP_NULL,    OP_NULL,   0 }, { "bnd",  OP_NULL,    OP_NULL,   0 },
	{ "???",  OP_MREG2,   OP_IM3,    0 }, { "???",  OP_MREG2,   OP_IM3,    0 },
	{ "sup",  OP_MREG,    OP_NULL,   0 }, { "sdn",  OP_MREG,    OP_NULL,   0 },
	{ "jp",   OP_MREG,    OP_NULL,   0 }, { "jp",   OP_MREG,    OP_NULL,   0 },

	// 0xe0
	{ "stm",  OP_MREG,  OP_IR_IM3,   0 }, { "stm",  OP_MREG,    OP_IR_IM3, 0 },
	{ "stim", OP_MREG,  OP_IR_IM3,   0 }, { "stim", OP_MREG,    OP_IR_IM3, 0 },
	{ "stdm", OP_MREG,  OP_IR_IM3,   0 }, { "stdm", OP_MREG,    OP_IR_IM3, 0 },
	{ "phsm", OP_MREG2, OP_IM3,      0 }, { "phum", OP_MREG2,   OP_IM3,    0 },
	{ "ldm",  OP_MREG,  OP_IR_IM3,   0 }, { "ldm",  OP_MREG,    OP_IR_IM3, 0 },
	{ "ldim", OP_MREG,  OP_IR_IM3,   0 }, { "ldim", OP_MREG,    OP_IR_IM3, 0 },
	{ "lddm", OP_MREG,  OP_IR_IM3,   0 }, { "lddm", OP_MREG,    OP_IR_IM3, 0 },
	{ "ppsm", OP_MREG2, OP_IM3,      0 }, { "ppum", OP_MREG2,   OP_IM3,    0 },

	// 0xf0
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_NULL,    OP_NULL,   0 },
	{ "nop",  OP_JX_COND, OP_NULL,   0 }, { "clt",  OP_NULL,    OP_NULL,   0 },
	{ "fst",  OP_NULL,    OP_NULL,   0 }, { "slw",  OP_NULL,    OP_NULL,   0 },
	{ "can",  OP_NULL,    OP_NULL,   0 }, { "rtni", OP_NULL,    OP_NULL,   0 },
	{ "off",  OP_NULL,    OP_NULL,   0 }, { "trp",  OP_NULL,    OP_NULL,   0 },
};


inline int dasm_im8(char *buffer, UINT16 pc, int arg, const UINT8 *oprom, int &pos, int type)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		INC_POS;
		UINT8 ret = sprintf( buffer, "0x%02x", oprom[POS] & 0x1f );
		return ret;
	}
	else
	{
		return sprintf( buffer, "%s", reg_5b[(arg>>5) & 0x03] );
	}
}


inline int dasm_im8(char *buffer, UINT16 pc, int arg, int arg1, const UINT8 *oprom, int &pos)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		return sprintf( buffer, "0x%02x", arg1 & 0x1f );
	}
	else
	{
		return sprintf( buffer, "%s", reg_5b[(arg>>5) & 0x03] );
	}
}


int dasm_arg(char *buffer, UINT8 op, UINT16 pc, int arg, const UINT8 *oprom, int &pos)
{
	char* buffer_start = buffer;
	int type = EXT_ROM;

	switch( arg )
	{
		case OP_MREG:
		case OP_MREG2:
			buffer += sprintf( buffer, "$%02u", oprom[POS] & 0x1f );
			if (arg == OP_MREG2) INC_POS;
			break;

		case OP_RSIR:
			buffer += sprintf( buffer, "%s", reg_5b[(oprom[POS]>>5) & 0x03] );
			break;

		case OP_REG8:
		case OP_REG8_:
			buffer += sprintf( buffer, "%s", reg_8b[(BIT(op,0)<<2) + ((oprom[POS]>>5&3))]);
			if (arg == OP_REG8_) INC_POS;
			break;

		case OP_MR_SIR:
			buffer += dasm_im8(buffer, pc, oprom[POS], oprom, pos, type);
			INC_POS;
			break;

		case OP_IR_IM8:
		case OP_IR_IM8_:
			buffer += sprintf( buffer, "(%s%s", (op&1) ? "iz": "ix", (oprom[POS]&0x80) ? "-": "+");
			buffer += dasm_im8(buffer, pc, oprom[POS], oprom, pos, type);
			buffer += sprintf( buffer, ")");
			if (arg == OP_IR_IM8_) INC_POS;
			break;

		case OP_IM8_:
			INC_POS;
		case OP_IM8:
			buffer += sprintf( buffer, "0x%02x", oprom[POS] );
			INC_POS;
			break;

		case OP_IM8I:
			buffer += sprintf( buffer, "(0x%02x)", oprom[POS] );
			INC_POS;
			break;

		case OP_REGIM8:
			buffer += sprintf( buffer, "(%s%s", (op&1) ? "iz": "ix", (oprom[POS]&0x80) ? "-": "+");
			buffer += sprintf( buffer, "%x)", oprom[POS] & 0x1f);
			INC_POS;
			break;

		case OP_JX_COND:
			buffer += sprintf( buffer, "%s", jp_cond[op & 0x07] );
			break;


		case OP_RMSIM3:
			{
				UINT8 tmp = oprom[POS];
				INC_POS;
				buffer += dasm_im8(buffer, pc, tmp, oprom[POS], oprom, pos);
				buffer += sprintf( buffer, ", 0x%02x", ((tmp>>5)&7)+1);
				INC_POS;
			}
			break;

		case OP_IR_IM3:
			{
				UINT8 tmp = oprom[POS];
				INC_POS;
				buffer += sprintf( buffer, "(%s%s", (op&1) ? "iz": "ix", (tmp&0x80) ? "-": "+");
				buffer += dasm_im8(buffer, pc, tmp, oprom[POS], oprom, pos);
				buffer += sprintf( buffer, "), 0x%02x", ((oprom[POS]>>5)&7)+1 );
				INC_POS;
			}
			break;

		case OP_IM3:
			buffer += sprintf( buffer, "0x%02x", ((oprom[POS]>>5)&7)+1 );
			INC_POS;
			break;

		case OP_MR_SIRI:
			buffer += sprintf( buffer, "(");
			buffer += dasm_im8(buffer, pc, oprom[POS], oprom, pos, type);
			buffer += sprintf( buffer, ")");
			INC_POS;
			break;

		case OP_IM7:
			{
				int tmp = oprom[POS];
				if (tmp&0x80)       tmp = 0x80 - tmp;

				buffer += sprintf( buffer, "0x%04x", (pc + tmp + EXT_ROM) & 0xffff );
				INC_POS;
			}
			break;

		case OP_IM5:
			buffer += sprintf( buffer, "0x%02x", oprom[POS]&0x1f );
			INC_POS;
			break;

		case OP_REG16:
		case OP_REG16_:
			buffer += sprintf( buffer, "%s", reg_16b[(BIT(op,0)<<2) + ((oprom[POS]>>5&3))]);
			if (arg == OP_REG16_) INC_POS;
			break;

		case OP_IM16:
		case OP_IM16A:
			{
				UINT8 tmp1 = oprom[POS];
				INC_POS;
				if (!EXT_ROM && arg == OP_IM16A)    INC_POS;
				UINT8 tmp2 = oprom[POS];
				buffer += sprintf( buffer, "0x%04x", ((tmp2<<8) | tmp1));
				INC_POS;
			}
			break;

		case OP_NULL:
			break;
	}

	return buffer - buffer_start;
}

UINT32 get_dasmflags(UINT8 op)
{
	switch (op)
	{
		case 0x30: case 0x31: case 0x32: case 0x33: //jp
		case 0x34: case 0x35: case 0x36: case 0x37: //jp
		case 0x70: case 0x71: case 0x72: case 0x73: //cal
		case 0x74: case 0x75: case 0x76: case 0x77: //cal
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: //jr
		case 0xb4: case 0xb5: case 0xb6: case 0xb7: //jr
		case 0xde:                                  //jp
		case 0xdf:                                  //jp
			return DASMFLAG_STEP_OVER;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: //rtn
		case 0xf4: case 0xf5: case 0xf6: case 0xf7: //rtn
		case 0xfd:                                  //rtni
			return DASMFLAG_STEP_OUT;
	}

	return 0;
}


CPU_DISASSEMBLE( hd61700 )
{
	const hd61700_dasm *inst;
	UINT32 dasmflags;
	UINT8 op, op1;
	int pos = 0, type = EXT_ROM;

	op = oprom[POS];
	INC_POS;

	dasmflags = get_dasmflags(op);

	op1 = oprom[POS];

	inst = &hd61700_ops[op];

	buffer += sprintf(buffer,"%-8s", inst->str);

	//dasm first arg
	buffer += dasm_arg(buffer, op, pc, inst->arg1, oprom, pos);

	//if present dasm second arg
	if (inst->arg2 != OP_NULL)
	{
		buffer += sprintf(buffer,", ");
		buffer += dasm_arg(buffer, op, pc, inst->arg2, oprom, pos);
	}

	//if required add the optional jr
	if (inst->optjr == true && BIT(op1, 7))
	{
		buffer += sprintf( buffer, ", jr ");
		buffer += dasm_arg(buffer, op, pc+1, OP_IM7, oprom, pos);

		dasmflags = DASMFLAG_STEP_OVER;
	}

	if (pos&1) INC_POS;

	return (pos>>1) | dasmflags | DASMFLAG_SUPPORTED;
}
