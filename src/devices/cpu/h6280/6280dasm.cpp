// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    6280dasm.c Hudsonsoft Hu6280 (HuC6280/Hu6280a) disassembler

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    Notes relating to Mame:

    The dasm window shows 'real' memory, as executed by the cpu
    The data windows show 'physical' memory, as defined in the memory map

******************************************************************************/

#include "emu.h"
#include "6280dasm.h"

const char *const h6280_disassembler::token[]=
{
	/* 6502 opcodes */
	"adc", "and", "asl", "bcc", "bcs", "beq", "bit", "bmi",
	"bne", "bpl", "brk", "bvc", "bvs", "clc", "cld", "cli",
	"clv", "cmp", "cpx", "cpy", "dec", "dex", "dey", "eor",
	"inc", "inx", "iny", "jmp", "jsr", "lda", "ldx", "ldy",
	"lsr", "nop", "ora", "pha", "php", "pla", "plp", "rol",
	"ror", "rti", "rts", "sbc", "sec", "sed", "sei", "sta",
	"stx", "sty", "tax", "tay", "tsx", "txa", "txs", "tya",
	"ill",

	/* Hu6280 extensions */
	"bra", "stz", "trb", "tsb", "dea", "ina", "sax", "bsr",
	"phx", "phy", "plx", "ply", "csh", "csl", "tam", "tma",
	"cla", "cly", "clx", "st0", "st1", "st2", "tst", "set",
	"tdd", "tia", "tii", "tin", "tai", "say", "sxy",

	"smb0", "smb1", "smb2", "smb3", "smb4", "smb5", "smb6", "smb7",
	"rmb0", "rmb1", "rmb2", "rmb3", "rmb4", "rmb5", "rmb6", "rmb7",

	"bbs0", "bbs1", "bbs2", "bbs3", "bbs4", "bbs5", "bbs6", "bbs7",
	"bbr0", "bbr1", "bbr2", "bbr3", "bbr4", "bbr5", "bbr6", "bbr7"
};

const unsigned char h6280_disassembler::op6280[512]=
{
	_brk,_imp, _ora,_idx, _sxy,_imp, _st0,_imm, _tsb,_zpg, _ora,_zpg, _asl,_zpg, _rm0,_zpg, /* 00 */
	_php,_imp, _ora,_imm, _asl,_acc, _ill,_non, _tsb,_abs, _ora,_abs, _asl,_abs, _br0,_zrl,
	_bpl,_rel, _ora,_idy, _ora,_zpi, _st1,_imm, _trb,_zpg, _ora,_zpx, _asl,_zpx, _rm1,_zpg, /* 10 */
	_clc,_imp, _ora,_aby, _ina,_imp, _ill,_non, _trb,_abs, _ora,_abx, _asl,_abx, _br1,_zrl,
	_jsr,_abs, _and,_idx, _sax,_imp, _st2,_imm, _bit,_zpg, _and,_zpg, _rol,_zpg, _rm2,_zpg, /* 20 */
	_plp,_imp, _and,_imm, _rol,_acc, _ill,_non, _bit,_abs, _and,_abs, _rol,_abs, _br2,_zrl,
	_bmi,_rel, _and,_idy, _and,_zpi, _ill,_non, _bit,_zpx, _and,_zpx, _rol,_zpx, _rm3,_zpg, /* 30 */
	_sec,_imp, _and,_aby, _dea,_imp, _ill,_non, _bit,_abx, _and,_abx, _rol,_abx, _br3,_zrl,
	_rti,_imp, _eor,_idx, _say,_imp, _tma,_imm, _bsr,_rel, _eor,_zpg, _lsr,_zpg, _rm4,_zpg, /* 40 */
	_pha,_imp, _eor,_imm, _lsr,_acc, _ill,_non, _jmp,_abs, _eor,_abs, _lsr,_abs, _br4,_zrl,
	_bvc,_rel, _eor,_idy, _eor,_zpi, _tam,_imm, _csl,_imp, _eor,_zpx, _lsr,_zpx, _rm5,_zpg, /* 50 */
	_cli,_imp, _eor,_aby, _phy,_imp, _ill,_non, _ill,_non, _eor,_abx, _lsr,_abx, _br5,_zrl,
	_rts,_imp, _adc,_idx, _cla,_imp, _ill,_non, _stz,_zpg, _adc,_zpg, _ror,_zpg, _rm6,_zpg, /* 60 */
	_pla,_imp, _adc,_imm, _ror,_acc, _ill,_non, _jmp,_ind, _adc,_abs, _ror,_abs, _br6,_zrl,
	_bvs,_rel, _adc,_idy, _adc,_zpi, _tii,_blk, _stz,_zpx, _adc,_zpx, _ror,_zpx, _rm7,_zpg, /* 70 */
	_sei,_imp, _adc,_aby, _ply,_imp, _ill,_non, _jmp,_iax, _adc,_abx, _ror,_abx, _br7,_zrl,
	_bra,_rel, _sta,_idx, _clx,_imp, _tst,_imz, _sty,_zpg, _sta,_zpg, _stx,_zpg, _sm0,_zpg, /* 80 */
	_dey,_imp, _bit,_imm, _txa,_imp, _ill,_non, _sty,_abs, _sta,_abs, _stx,_abs, _bs0,_zrl,
	_bcc,_rel, _sta,_idy, _sta,_zpi, _tst,_ima, _sty,_zpx, _sta,_zpx, _stx,_zpy, _sm1,_zpg, /* 90 */
	_tya,_imp, _sta,_aby, _txs,_imp, _ill,_non, _stz,_abs, _sta,_abx, _stz,_abx, _bs1,_zrl,
	_ldy,_imm, _lda,_idx, _ldx,_imm, _tst,_izx, _ldy,_zpg, _lda,_zpg, _ldx,_zpg, _sm2,_zpg, /* a0 */
	_tay,_imp, _lda,_imm, _tax,_imp, _ill,_non, _ldy,_abs, _lda,_abs, _ldx,_abs, _bs2,_zrl,
	_bcs,_rel, _lda,_idy, _lda,_zpi, _tst,_imx, _ldy,_zpx, _lda,_zpx, _ldx,_zpy, _sm3,_zpg, /* b0 */
	_clv,_imp, _lda,_aby, _tsx,_imp, _ill,_non, _ldy,_abx, _lda,_abx, _ldx,_aby, _bs3,_zrl,
	_cpy,_imm, _cmp,_idx, _cly,_imp, _tdd,_blk, _cpy,_zpg, _cmp,_zpg, _dec,_zpg, _sm4,_zpg, /* c0 */
	_iny,_imp, _cmp,_imm, _dex,_imp, _ill,_non, _cpy,_abs, _cmp,_abs, _dec,_abs, _bs4,_zrl,
	_bne,_rel, _cmp,_idy, _cmp,_zpi, _tin,_blk, _csh,_imp, _cmp,_zpx, _dec,_zpx, _sm5,_zpg, /* d0 */
	_cld,_imp, _cmp,_aby, _phx,_imp, _ill,_non, _ill,_non, _cmp,_abx, _dec,_abx, _bs5,_zrl,
	_cpx,_imm, _sbc,_idx, _ill,_non, _tia,_blk, _cpx,_zpg, _sbc,_zpg, _inc,_zpg, _sm6,_zpg, /* e0 */
	_inx,_imp, _sbc,_imm, _nop,_imp, _ill,_non, _cpx,_abs, _sbc,_abs, _inc,_abs, _bs6,_zrl,
	_beq,_rel, _sbc,_idy, _sbc,_zpi, _tai,_blk, _set,_imp, _sbc,_zpx, _inc,_zpx, _sm7,_zpg, /* f0 */
	_sed,_imp, _sbc,_aby, _plx,_imp, _ill,_non, _ill,_non, _sbc,_abx, _inc,_abx, _bs7,_zrl
};

u32 h6280_disassembler::opcode_alignment() const
{
	return 1;
}

/*****************************************************************************
 *  Disassemble a single command and return the number of bytes it uses.
 *****************************************************************************/
offs_t h6280_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	offs_t PC;
	int OP, opc, arg;

	PC = pc;
	OP = opcodes.r8(PC);
	OP = OP << 1;
	PC++;

	opc = op6280[OP];
	arg = op6280[OP+1];

	if (opc == _jsr || opc == _bsr)
		flags = STEP_OVER;
	else if (opc == _rts)
		flags = STEP_OUT;

	switch(arg)
	{
		case _acc:
			util::stream_format(stream, "%-5sa", token[opc]);
			break;
		case _imp:
			util::stream_format(stream, "%s", token[opc]);
			break;
		case _rel:
			util::stream_format(stream, "%-5s$%04X", token[opc], (PC + 1 + (signed char)params.r8(PC)) & 0xffff);
			PC+=1;
			if (opc != _bra && opc != _bsr)
				flags = STEP_COND;
			break;
		case _imm:
			util::stream_format(stream, "%-5s#$%02X", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _zpg:
			util::stream_format(stream, "%-5s$%02X", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _zpx:
			util::stream_format(stream, "%-5s$%02X,x", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _zpy:
			util::stream_format(stream, "%-5s$%02X,y", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _idx:
			util::stream_format(stream, "%-5s($%02X,x)", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _idy:
			util::stream_format(stream, "%-5s($%02X),y", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _zpi:
			util::stream_format(stream, "%-5s($%02X)", token[opc], params.r8(PC));
			PC+=1;
			break;
		case _abs:
			util::stream_format(stream, "%-5s$%04X", token[opc], params.r16(PC));
			PC+=2;
			break;
		case _abx:
			util::stream_format(stream, "%-5s$%04X,x", token[opc], params.r16(PC));
			PC+=2;
			break;
		case _aby:
			util::stream_format(stream, "%-5s$%04X,y", token[opc], params.r16(PC));
			PC+=2;
			break;
		case _ind:
			util::stream_format(stream, "%-5s($%04X)", token[opc], params.r16(PC));
			PC+=2;
			break;
		case _iax:
			util::stream_format(stream, "%-5s($%04X),X", token[opc], params.r16(PC));
			PC+=2;
			break;
		case _blk:
			util::stream_format(stream, "%-5s$%04X $%04X $%04X", token[opc], params.r16(PC), params.r16(PC+2), params.r16(PC+4));
			PC+=6;
			break;
		case _zrl:
			util::stream_format(stream, "%-5s$%02X $%04X", token[opc], params.r8(PC), (PC + 2 + (signed char)params.r8(PC+1)) & 0xffff);
			PC+=2;
			break;
		case _imz:
			util::stream_format(stream, "%-5s#$%02X $%02X", token[opc], params.r8(PC), params.r8(PC+1));
			PC+=2;
			break;
		case _izx:
			util::stream_format(stream, "%-5s#$%02X $%02X,x", token[opc], params.r8(PC), params.r8(PC+1));
			PC+=2;
			break;
		case _ima:
			util::stream_format(stream, "%-5s#$%02X $%04X", token[opc], params.r8(PC), params.r16(PC+1));
			PC+=3;
			break;
		case _imx:
			util::stream_format(stream, "%-5s#$%02X $%04X,x", token[opc], params.r8(PC), params.r16(PC+1));
			PC+=3;
			break;

		default:
			util::stream_format(stream, "%-5s$%02X", token[opc], OP >> 1);
	}
	return (PC - pc) | flags | SUPPORTED;
}
