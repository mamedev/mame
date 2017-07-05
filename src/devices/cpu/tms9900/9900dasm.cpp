// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 *   9900dasm.c
 *   TMS 9900 family disassembler
 *
 *****************************************************************************/


#include "emu.h"
#include "tms99com.h"
#include "9900dasm.h"

#define MASK    0x0000ffff
#define BITS(val,n1,n2) ((val>>(15-(n2))) & (MASK>>(15-((n2)-(n1)))))


const tms9900_disassembler::description_t tms9900_disassembler::descriptions[144+3+1] =
{
	/* basic instruction set */
	{ "a",      format_1,   ps_any },           { "ab",     format_1,   ps_any },
	{ "c",      format_1,   ps_any },           { "cb",     format_1,   ps_any },
	{ "s",      format_1,   ps_any },           { "sb",     format_1,   ps_any },
	{ "soc",    format_1,   ps_any },           { "socb",   format_1,   ps_any },
	{ "szc",    format_1,   ps_any },           { "szcb",   format_1,   ps_any },
	{ "mov",    format_1,   ps_any },           { "movb",   format_1,   ps_any },
	{ "coc",    format_3_9, ps_any },           { "czc",    format_3_9, ps_any },
	{ "xor",    format_3_9, ps_any },           { "mpy",    format_3_9, ps_any },
	{ "div",    format_3_9, ps_any },           { "xop",    format_9,   ps_any },
	{ "b",      format_6,   ps_any },           { "bl",     format_6,   ps_any },
	{ "blwp",   format_6,   ps_any },           { "clr",    format_6,   ps_any },
	{ "seto",   format_6,   ps_any },           { "inv",    format_6,   ps_any },
	{ "neg",    format_6,   ps_any },           { "abs",    format_6,   ps_any },
	{ "swpb",   format_6,   ps_any },           { "inc",    format_6,   ps_any },
	{ "inct",   format_6,   ps_any },           { "dec",    format_6,   ps_any },
	{ "dect",   format_6,   ps_any },           { "x",      format_6,   ps_any },
	{ "ldcr",   format_4,   ps_any },           { "stcr",   format_4,   ps_any },
	{ "sbo",    format_2b,  ps_any },           { "sbz",    format_2b,  ps_any },
	{ "tb",     format_2b,  ps_any },           { "jeq",    format_2a,  ps_any },
	{ "jgt",    format_2a,  ps_any },           { "jh",     format_2a,  ps_any },
	{ "jhe",    format_2a,  ps_any },           { "jl",     format_2a,  ps_any },
	{ "jle",    format_2a,  ps_any },           { "jlt",    format_2a,  ps_any },
	{ "jmp",    format_2a,  ps_any },           { "jnc",    format_2a,  ps_any },
	{ "jne",    format_2a,  ps_any },           { "jno",    format_2a,  ps_any },
	{ "joc",    format_2a,  ps_any },           { "jop",    format_2a,  ps_any },
	{ "sla",    format_5,   ps_any },           { "sra",    format_5,   ps_any },
	{ "src",    format_5,   ps_any },           { "srl",    format_5,   ps_any },
	{ "ai",     format_8a,  ps_any|sd_11 },     { "andi",   format_8a,  ps_any|sd_11 },
	{ "ci",     format_8a,  ps_any|sd_11 },     { "li",     format_8a,  ps_any|sd_11 },
	{ "ori",    format_8a,  ps_any|sd_11 },     { "lwpi",   format_8b,  ps_any|sd_11|sd_11_15 },
	{ "limi",   format_8b,  ps_any|sd_11|sd_11_15 },    { "stst",   format_18,  ps_any|sd_11 },
	{ "stwp",   format_18,  ps_any|sd_11 },     { "rtwp",   format_7,   ps_any|sd_11|sd_11_15 },
	{ "idle",   format_7,   ps_any|sd_11|sd_11_15 },    { "rset",   format_7,   ps_any|sd_11|sd_11_15 },
	{ "ckof",   format_7,   ps_any|sd_11|sd_11_15 },    { "ckon",   format_7,   ps_any|sd_11|sd_11_15 },
	{ "lrex",   format_7,   ps_any|sd_11|sd_11_15 },

	/* mapper instruction set */
	{ "lds",    format_6,   ps_mapper },        { "ldd",    format_6,   ps_mapper },
	{ "lmf",    format_10,  ps_mapper },

	/* tms9995 instruction set */
	{ "divs",   format_6,   ps_tms9995 },       { "mpys",   format_6,   ps_tms9995 },
	{ "lst",    format_18,  ps_tms9995 },       { "lwp",    format_18,  ps_tms9995 },

	/* tms99000 instruction set */
	{ "bind",   format_6,   ps_tms99000 },

	/* ti990/12 instruction set */
	{ "sram",   format_13,  ps_ti990_12 },      { "slam",   format_13,  ps_ti990_12 },
	{ "rto",    format_11,  ps_ti990_12 },      { "lto",    format_11,  ps_ti990_12 },
	{ "cnto",   format_11,  ps_ti990_12 },      { "slsl",   format_20,  ps_ti990_12 },
	{ "slsp",   format_20,  ps_ti990_12 },      { "bdc",    format_11,  ps_ti990_12 },
	{ "dbc",    format_11,  ps_ti990_12 },      { "swpm",   format_11,  ps_ti990_12 },
	{ "xorm",   format_11,  ps_ti990_12 },      { "orm",    format_11,  ps_ti990_12 },
	{ "andm",   format_11,  ps_ti990_12 },      { "sm",     format_11,  ps_ti990_12 },
	{ "am",     format_11,  ps_ti990_12 },      { "mova",   format_19,  ps_ti990_12 },
	{ "emd",    format_7,   ps_ti990_12 },      { "eint",   format_7,   ps_ti990_12 },
	{ "dint",   format_7,   ps_ti990_12 },      { "stpc",   format_18,  ps_ti990_12 },
	{ "cs",     format_12,  ps_ti990_12 },      { "seqb",   format_12,  ps_ti990_12 },
	{ "movs",   format_12,  ps_ti990_12 },      { "lim",    format_18,  ps_ti990_12 },
	{ "lcs",    format_18,  ps_ti990_12 },      { "blsk",   format_8a,  ps_ti990_12 },
	{ "mvsr",   format_12,  ps_ti990_12 },      { "mvsk",   format_12,  ps_ti990_12 },
	{ "pops",   format_12,  ps_ti990_12 },      { "pshs",   format_12,  ps_ti990_12 },
	{ "cri",    format_7,   ps_ti990_12 },      { "cdi",    format_7,   ps_ti990_12 },
	{ "negr",   format_7,   ps_ti990_12 },      { "negd",   format_7,   ps_ti990_12 },
	{ "cre",    format_7,   ps_ti990_12 },      { "cde",    format_7,   ps_ti990_12 },
	{ "cer",    format_7,   ps_ti990_12 },      { "ced",    format_7,   ps_ti990_12 },
	{ "nrm",    format_11,  ps_ti990_12 },      { "tmb",    format_14,  ps_ti990_12 },
	{ "tcmb",   format_14,  ps_ti990_12 },      { "tsmb",   format_14,  ps_ti990_12 },
	{ "srj",    format_17,  ps_ti990_12 },      { "arj",    format_17,  ps_ti990_12 },
	{ "xit",    format_7,   ps_ti990_12 },      { "insf",   format_16,  ps_ti990_12 },
	{ "xv",     format_16,  ps_ti990_12 },      { "xf",     format_16,  ps_ti990_12 },
	{ "ar",     format_6,   ps_ti990_12 },      { "cir",    format_6,   ps_ti990_12 },
	{ "sr",     format_6,   ps_ti990_12 },      { "mr",     format_6,   ps_ti990_12 },
	{ "dr",     format_6,   ps_ti990_12 },      { "lr",     format_6,   ps_ti990_12 },
	{ "str",    format_6,   ps_ti990_12 },      { "iof",    format_15,  ps_ti990_12 },
	{ "sneb",   format_12,  ps_ti990_12 },      { "crc",    format_12,  ps_ti990_12 },
	{ "ts",     format_12,  ps_ti990_12 },      { "ad",     format_6,   ps_ti990_12 },
	{ "cid",    format_6,   ps_ti990_12 },      { "sd",     format_6,   ps_ti990_12 },
	{ "md",     format_6,   ps_ti990_12 },      { "dd",     format_6,   ps_ti990_12 },
	{ "ld",     format_6,   ps_ti990_12 },      { "std",    format_6,   ps_ti990_12 },
	{ "ep",     format_21,  ps_ti990_12 },

	/* tms9940-only instruction set */
	/* these instructions are said to be format 9 (xop), but since the xop
	level is interpreted as part of the opcode, dca and dcs should be handled
	like format 6.  liim looks like format 18, but slightly different,
	therefore it is handled like a special format. */
	{ "liim",   format_liim,/*ps_tms9940*/0 },  { "dca",    format_6,   /*ps_tms9940*/0 },
	{ "dcs",    format_6,   /*ps_tms9940*/0 },

	{ nullptr,     illegal,    ps_any }
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_4000_ffff_s12[12]=
{
	_szc,   _szcb,  _s,     _sb,                                    /*4000-7000*/
	_c,     _cb,    _a,     _ab,    _mov,   _movb,  _soc,   _socb   /*8000-f000*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_2000_3fff_s10[8]=
{
	_coc,   _czc,   _xor,   _xop,   _ldcr,  _stcr,  _mpy,   _div    /*2000-3800*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_1000_1fff_s8[16]=
{
	_jmp,   _jlt,   _jle,   _jeq,   _jhe,   _jgt,   _jne,   _jnc,   /*1000-1700*/
	_joc,   _jno,   _jl,    _jh,    _jop,   _sbo,   _sbz,   _tb     /*1800-1f00*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0e40_0fff_s6[7]=
{
			_ad,    _cid,   _sd,    _md,    _dd,    _ld,    _std    /*0e40-0fc0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0e00_0e3f_s4[4]=
{
	_iof,   _sneb,  _crc,   _ts                                     /*0e00-0e30*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0c40_0dff_s6[7]=
{
			_ar,    _cir,   _sr,    _mr,    _dr,    _lr,    _str    /*0c40-0dc0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0c10_0c3f_s4[3]=
{
			_insf,  _xv,    _xf                                     /*0c10-0c30*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0c00_0c0f_s0[16]=
{
	_cri,   _cdi,   _negr,  _negd,  _cre,   _cde,   _cer,   _ced,   /*0c00-0c07*/
	_nrm,   _tmb,   _tcmb,  _tsmb,  _srj,   _arj,   _xit,   _xit    /*0c08-0c0f*/
};



const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0800_0bff_s8[4]=
{
	_sra,   _srl,   _sla,   _src                                    /*0800-0b00*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0400_07ff_s6[16]=
{
	_blwp,  _b,     _x,     _clr,   _neg,   _inv,   _inc,   _inct,  /*0400-05c0*/
	_dec,   _dect,  _bl,    _swpb,  _seto,  _abs,   _lds,   _ldd    /*0600-07c0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0200_03ff_s5[16]=
{
	_li,    _ai,    _andi,  _ori,   _ci,    _stwp,  _stst,  _lwpi,  /*0200-02e0*/
	_limi,  _lmf,   _idle,  _rset,  _rtwp,  _ckon,  _ckof,  _lrex   /*0300-03e0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0100_01ff_s6[4]=
{
	_ill,   _bind,  _divs,  _mpys                                   /*0100-01c0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_0030_00ff_s4[13]=
{
							_stpc,  _cs,    _seqb,  _movs,  _lim,   /*0030-0070*/
	_lst,   _lwp,   _lcs,   _blsk,  _mvsr,  _mvsk,  _pops,  _pshs   /*0080-00f0*/
};


const enum tms9900_disassembler::opcodes tms9900_disassembler::ops_001c_002f_s0[20]=
{
									_sram,  _slam,  _rto,   _lto,   /*001c-001f*/
	_cnto,  _slsl,  _slsp,  _bdc,   _dbc,   _swpm,  _xorm,  _orm,   /*0020-0027*/
	_andm,  _sm,    _am,    _mova,  _ill,   _emd,   _eint,  _dint   /*0028-002f*/
};


inline uint16_t tms9900_disassembler::readop_arg(const data_buffer &params, offs_t &PC)
{
	uint16_t result = params.r16(PC);
	PC += 2;;
	return result;
}

void tms9900_disassembler::print_arg (std::ostream &stream, int mode, int arg, const data_buffer &params, offs_t &PC)
{
	int base;

	switch (mode)
	{
		case 0x0:   /* workspace register */
			util::stream_format(stream, "R%d", arg);
			break;
		case 0x1:   /* workspace register indirect */
			util::stream_format(stream, "*R%d", arg);
			break;
		case 0x2:   /* symbolic|indexed */
			base = readop_arg(params, PC);
			if (arg)    /* indexed */
				util::stream_format(stream, "@>%04x(R%d)", base, arg);
			else        /* symbolic (direct) */
				util::stream_format(stream, "@>%04x", base);
			break;
		case 0x3:   /* workspace register indirect auto increment */
			util::stream_format(stream, "*R%d+", arg);
			break;
	}
}

tms9900_disassembler::tms9900_disassembler(int model) : m_model_id(model)
{
}

u32 tms9900_disassembler::opcode_alignment() const
{
	return 2;
}

/*****************************************************************************
 *  Disassemble a single command and return the number of bytes it uses.
 *****************************************************************************/
offs_t tms9900_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int OP, OP2, opc;
	int sarg, darg, smode, dmode;
	signed char displacement;
	int byte_count, checkpoint;
	int bit_position, bit_width;
	unsigned dasmflags = 0;

	const char *mnemonic;
	format_t format;
	int flags;

	/*
	    Under tms9900, opcodes >0400->07FF are incompletely decoded: bits 11 is ignored, and so are
	    bits 12-15 for instructions which do not require a register.  On the other hand, ti990/10
	    generates an illegal instruction error when bit 11 is set, but still ignores bits 12-15.
	    Additionally, ti990/12 and tms9995 will generate an illegal error when bits 12-15 are
	    non-zero.
	*/
	#define BETTER_0200_DECODING (m_model_id == TI990_10_ID)
	#define COMPLETE_0200_DECODING (/*(m_model_id == TI990_12_ID) ||*/ (m_model_id >= TMS9995_ID))

	int processor_mask = ps_any;

	if ((m_model_id == TI990_10_ID) /*|| (m_model_id == TI990_12_ID)*/ || (m_model_id >= TMS99000_ID))
		processor_mask |= ps_mapper;        /* processors with memory mapper (ti990/10, ti990/12,
		                                        and tms99000 with mapper coprocessor) */
	if (/*(m_model_id == TI990_12_ID) ||*/ (m_model_id >= TMS9995_ID))
		processor_mask |= ps_tms9995;       /* ti990/12, tms9995, and later */

	if (/*(m_model_id == TI990_12_ID) ||*/ (m_model_id >= TMS99000_ID))
		processor_mask |= ps_tms99000;      /* ti990/12, tms99000, and later */

	/*if ((m_model_id == TI990_12_ID))
	    processor_mask |= ps_ti990_12;*/    /* ti990/12, tms99000, and later */

	offs_t PC = pc;
	OP = opcodes.r16(PC);
	PC += 2;

	/* let's identify the opcode */
	if (OP >= 0x4000)
		opc = ops_4000_ffff_s12[(OP - 0x4000) >> 12];
	else if (OP >= 0x2000)
		opc = ops_2000_3fff_s10[(OP - 0x2000) >> 10];
	else if (OP >= 0x1000)
		opc = ops_1000_1fff_s8[(OP - 0x1000) >> 8];
	else if (OP >= 0x0C00)
	{
		if (OP >= 0x0E40)
			opc = ops_0e40_0fff_s6[(OP - 0x0E40) >> 6];
		else if (OP >= 0x0E00)
			opc = ops_0e00_0e3f_s4[(OP - 0x0E00) >> 4];
		else if (OP >= 0x0C40)
			opc = ops_0c40_0dff_s6[(OP - 0x0C40) >> 6];
		else if (OP >= 0x0C10)
			opc = ops_0c10_0c3f_s4[(OP - 0x0C10) >> 4];
		else
			opc = ops_0c00_0c0f_s0[OP - 0x0C00];
	}
	else if (OP >= 0x0800)
		opc = ops_0800_0bff_s8[(OP - 0x0800) >> 8];
	else if (OP >= 0x0400)
		opc = ops_0400_07ff_s6[(OP - 0x0400) >> 6];
	else if (OP >= 0x0200)
	{
		opc = ops_0200_03ff_s5[(OP - 0x0200) >> 5];
		if (BETTER_0200_DECODING || COMPLETE_0200_DECODING)
		{
			flags = descriptions[opc].flags;
			if ( (COMPLETE_0200_DECODING && (flags & sd_11_15) && (OP & 0x001f))
					|| ((flags & sd_11) && (OP & 0x0010)) )
			{
				opc = _ill;
				if (OP >= 0x03f0)
					opc = _ep;  /* the ep opcode is located in a "hole" */
			}
		}
	}
	else if (OP >= 0x0100)
		opc = ops_0100_01ff_s6[(OP - 0x0100) >> 6];
	else if (OP >= 0x0030)
		opc = ops_0030_00ff_s4[(OP - 0x0030) >> 4];
	else if (OP >= 0x001C)
		opc = ops_001c_002f_s0[OP - 0x001C];
	else
		opc = _ill;

	/* read flags */
	flags = descriptions[opc].flags;
	/* set as illegal if the processor does not implement this instruction */
	if (! (flags & processor_mask))
	{
		opc = _ill;
		flags = descriptions[opc].flags;    /* read new flags */
	}

	/* tms9940 replace a few xops with custom instructions */
	if ((opc == _xop) && ((m_model_id == TMS9940_ID) || (m_model_id == TMS9985_ID)))
	{
		switch (BITS(OP,6,9))
		{
		case 0:
			/* opcode is dca */
			opc = _dca;
			break;

		case 1:
			/* opcode is dcs */
			opc = _dcs;
			break;

		case 2:
		case 3: /* should be 2, but instruction decoding is incomplete */
			/* opcode is liim */
			if (BITS(OP,12,15) == 0)
				/* ts must be == 0 */
				opc = _liim;
			else
				/* I don't know what happens when ts != 0.  Maybe the CPU does
				the complete address decoding, and liim gets a bogus value
				instead of the immediate.  Since I do not know, I handle this
				as an illegal instruction. */
				opc = _ill;
			break;

		default:
			/* this is still a software xop */
			break;
		}
	}

	mnemonic = descriptions[opc].mnemonic;
	format = descriptions[opc].format;

	/* bl and blwp instructions are subroutines */
	if (mnemonic != nullptr && mnemonic[0] == 'b' && mnemonic[1] == 'l')
		dasmflags = STEP_OVER;

	/* b *r11 and rtwp are returns */
	else if (opc == 0x045b || (mnemonic != nullptr && strcmp(mnemonic, "rtwp") == 0))
		dasmflags = STEP_OUT;

	switch (format)
	{
	case format_1:      /* 2 address instructions */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);
		dmode = BITS(OP,4,5);
		darg = BITS(OP,6,9);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
		break;

	case format_2a:     /* jump instructions */
		displacement = (signed char)BITS(OP,8,15);
		util::stream_format(stream, "%-4s >%04x", mnemonic, 0xffff & (PC + displacement * 2));
		break;

	case format_2b:     /* bit I/O instructions */
		displacement = (signed char)BITS(OP,8,15);
		util::stream_format(stream, "%-4s >%04x", mnemonic, 0xffff & displacement);
		break;

	case format_3_9:    /* logical, multiply, and divide instructions */
	case format_4:      /* CRU instructions */
	case format_9:      /* extended operation instruction */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);
		darg = BITS(OP,6,9);

		if ((darg == 0) && (format == format_4))
			darg = 16;

		if (format == format_3_9)
		{
			util::stream_format(stream, "%-4s ", mnemonic);
			print_arg(stream, smode, sarg, params, PC);
			util::stream_format(stream, ",R%d", darg);
		}
		else
		{
			util::stream_format(stream, "%-4s ", mnemonic);
			print_arg(stream, smode, sarg, params, PC);
			util::stream_format(stream, ",%d", darg);
		}
		break;

	case format_5:      /* register shift instructions */
		sarg = BITS(OP,12,15);
		darg = BITS(OP,8,11);

		util::stream_format(stream, darg ? "%-4s R%d,%d" : "%-4s R%d,R%d", mnemonic, sarg, darg);
		break;

	case format_6:      /* single address instructions */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		break;

	case format_7:      /* instructions without operands */
		util::stream_format(stream, "%s", mnemonic);
		break;

	case format_8a:     /* immediate instructions (destination register) */
		darg = BITS(OP,12,15);
		sarg = readop_arg(params, PC);

		util::stream_format(stream, "%-4s R%d,>%04x", mnemonic, darg, sarg);
		break;

	case format_8b:     /* immediate instructions (no destination register) */
		sarg = readop_arg(params, PC);

		util::stream_format(stream, "%-4s >%04x", mnemonic, sarg);
		break;

	case format_10:     /* memory map file instruction */
		sarg = BITS(OP,12,15);
		darg = BITS(OP,11,11);

		util::stream_format(stream, "%-4s R%d,%d", mnemonic, sarg, darg);
		break;

	case format_11:     /* multiple precision instructions */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
		util::stream_format(stream, byte_count ? ",%d" : ",R%d", byte_count);
		break;

	case format_12:     /* string instructions */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);
		checkpoint = BITS(OP,12,15);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
		util::stream_format(stream, byte_count ? ",%d,R%d" : ",R%d,R%d", byte_count, checkpoint);
		break;

	case format_13:     /* multiple precision shift instructions */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, byte_count ? ",%d" : ",R%d", byte_count);
		util::stream_format(stream, darg ? ",%d" : ",R%d", darg);
		break;

	case format_14:     /* bit testing instructions */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		darg = BITS(OP2,0,9);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		if (darg == 0x3ff)
			util::stream_format(stream, ",R0");
		else
			util::stream_format(stream, ",%d", darg);
		break;

	case format_15:     /* invert order of field instruction */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		bit_position = BITS(OP2,0,3);
		bit_width = BITS(OP,12,15);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, bit_position ? ",(%d," : ",(R%d,", bit_position);
		util::stream_format(stream, bit_width ? "%d)" : "R%d)", bit_width);
		break;

	case format_16:     /* field instructions */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		bit_position = BITS(OP2,0,3);
		bit_width = BITS(OP,12,15);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
		util::stream_format(stream, bit_position ? ",(%d," : ",(%d,", bit_position);
		util::stream_format(stream, bit_width ? "%d)" : "R%d)", bit_width);
		break;

	case format_17:     /* alter register and jump instructions */
		OP2 = readop_arg(params, PC);

		displacement = (signed char)BITS(OP2,8,15);
		sarg = BITS(OP2,4,7);
		darg = BITS(OP2,0,3);
		if (darg)
		util::stream_format(stream, darg ? "%-4s >%04x,%d,R%d" : "%-4s >%04x,R%d,R%d",
							mnemonic, 0xffff & (PC + displacement * 2), sarg, darg);
		break;

	case format_18:     /* single register operand instructions */
		sarg = BITS(OP,12,15);

		util::stream_format(stream, "%-4s R%d", mnemonic, sarg);
				break;

	case format_liim:   /* liim instruction */
		sarg = BITS(OP,14,15);

		util::stream_format(stream, "%-4s %d", mnemonic, sarg);
		break;

	case format_19:     /* move address instruction */
		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
				break;

	case format_20:     /* list search instructions */
	{
			const char *condition_code;

			OP2 = readop_arg(params, PC);

			smode = BITS(OP2,10,11);
			sarg = BITS(OP2,12,15);
			dmode = BITS(OP2,4,5);
			darg = BITS(OP2,6,9);

			switch (BITS(OP2,0,3))
			{
			case 0:
				condition_code = "eq";
				break;
			case 1:
				condition_code = "ne";
				break;
			case 2:
				condition_code = "he";
				break;
			case 3:
				condition_code = "l";
				break;
			case 4:
				condition_code = "ge";
				break;
			case 5:
				condition_code = "lt";
				break;
			case 6:
				condition_code = "le";
				break;
			case 7:
				condition_code = "h";
				break;
			case 8:
				condition_code = "lte";
				break;
			case 9:
				condition_code = "gt";
				break;
			default:
				condition_code = "??";
				break;
			}

			util::stream_format(stream, "%-4s %s,", mnemonic, condition_code);
			print_arg(stream, smode, sarg, params, PC);
			util::stream_format(stream, ",");
			print_arg(stream, dmode, darg, params, PC);
			break;
	}

	case format_21:     /* extend precision instruction */
	{
		int dest_byte_count;

		OP2 = readop_arg(params, PC);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);
		dest_byte_count = BITS(OP,12,15);

		util::stream_format(stream, "%-4s ", mnemonic);
		print_arg(stream, smode, sarg, params, PC);
		util::stream_format(stream, ",");
		print_arg(stream, dmode, darg, params, PC);
		util::stream_format(stream, byte_count ? ",%d" : ",R%d", byte_count);
		util::stream_format(stream, dest_byte_count ? ",%d" : ",R%d", dest_byte_count);
		break;
	}

	default:
		osd_printf_error("debbugger internal error, file %s, line %d\n", __FILE__, __LINE__);
	case illegal:
		util::stream_format(stream, "data >%04x", OP);
		break;
	}

	return (PC - pc) | SUPPORTED | dasmflags;
}
