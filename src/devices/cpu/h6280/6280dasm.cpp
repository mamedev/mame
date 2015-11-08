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

#ifdef __OS2__
/* To avoid name clash of _brk */
#define __STRICT_ANSI__
#endif

#include "emu.h"

#define RDOP(addr)   (oprom[addr - pc])
#define RDBYTE(addr) (opram[addr - pc])
#define RDWORD(addr) (opram[addr - pc] | ( oprom[(addr) + 1 - pc] << 8 ))

enum addr_mode {
	_non=0,      /* no additional arguments */
	_acc,        /* accumulator */
	_imp,        /* implicit */
	_imm,        /* immediate */
	_abs,        /* absolute */
	_zpg,        /* zero page */
	_zpx,        /* zero page + X */
	_zpy,        /* zero page + Y */
	_zpi,        /* zero page indirect */
	_abx,        /* absolute + X */
	_aby,        /* absolute + Y */
	_rel,        /* relative */
	_idx,        /* zero page pre indexed */
	_idy,        /* zero page post indexed */
	_ind,        /* indirect */
	_iax,        /* indirect + X */
	_blk,        /* block */
	_zrl,        /* zero page relative */
	_imz,        /* immediate, zero page */
	_izx,        /* immediate, zero page + X */
	_ima,        /* immediate, absolute */
	_imx         /* immediate, absolute + X */
};

enum opcodes {
	/* 6502 opcodes */
	_adc=0,_and,  _asl,  _bcc,  _bcs,  _beq,  _bit,  _bmi,
	_bne,  _bpl,  _brk,  _bvc,  _bvs,  _clc,  _cld,  _cli,
	_clv,  _cmp,  _cpx,  _cpy,  _dec,  _dex,  _dey,  _eor,
	_inc,  _inx,  _iny,  _jmp,  _jsr,  _lda,  _ldx,  _ldy,
	_lsr,  _nop,  _ora,  _pha,  _php,  _pla,  _plp,  _rol,
	_ror,  _rti,  _rts,  _sbc,  _sec,  _sed,  _sei,  _sta,
	_stx,  _sty,  _tax,  _tay,  _tsx,  _txa,  _txs,  _tya,
	_ill,

	/* Hu6280 extensions */
	_bra,  _stz,  _trb,  _tsb,  _dea,  _ina,  _sax,  _bsr,
	_phx,  _phy,  _plx,  _ply,  _csh,  _csl,  _tam,  _tma,
	_cla,  _cly,  _clx,  _st0,  _st1,  _st2,  _tst,  _set,
	_tdd,  _tia,  _tii,  _tin,  _tai,  _say,  _sxy,

	_sm0,  _sm1,  _sm2,  _sm3,  _sm4,  _sm5,  _sm6,  _sm7,
	_rm0,  _rm1,  _rm2,  _rm3,  _rm4,  _rm5,  _rm6,  _rm7,

	_bs0,  _bs1,  _bs2,  _bs3,  _bs4,  _bs5,  _bs6,  _bs7,
	_br0,  _br1,  _br2,  _br3,  _br4,  _br5,  _br6,  _br7

};


static const char *const token[]=
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

static const unsigned char op6280[512]=
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

/*****************************************************************************
 *  Disassemble a single command and return the number of bytes it uses.
 *****************************************************************************/
CPU_DISASSEMBLE( h6280 )
{
	UINT32 flags = 0;
	int PC, OP, opc, arg;

	PC = pc;
	OP = RDOP(PC);
	OP = OP << 1;
	PC++;

	opc = op6280[OP];
	arg = op6280[OP+1];

	if (opc == _jsr || opc == _bsr)
		flags = DASMFLAG_STEP_OVER;
	else if (opc == _rts)
		flags = DASMFLAG_STEP_OUT;

	switch(arg)
	{
		case _acc:
			sprintf(buffer,"%-5sa", token[opc]);
			break;
		case _imp:
			sprintf(buffer,"%s", token[opc]);
			break;
		case _rel:
			sprintf(buffer,"%-5s$%04X", token[opc], (PC + 1 + (signed char)RDBYTE(PC)) & 0xffff);
			PC+=1;
			break;
		case _imm:
			sprintf(buffer,"%-5s#$%02X", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _zpg:
			sprintf(buffer,"%-5s$%02X", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _zpx:
			sprintf(buffer,"%-5s$%02X,x", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _zpy:
			sprintf(buffer,"%-5s$%02X,y", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _idx:
			sprintf(buffer,"%-5s($%02X,x)", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _idy:
			sprintf(buffer,"%-5s($%02X),y", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _zpi:
			sprintf(buffer,"%-5s($%02X)", token[opc], RDBYTE(PC));
			PC+=1;
			break;
		case _abs:
			sprintf(buffer,"%-5s$%04X", token[opc], RDWORD(PC));
			PC+=2;
			break;
		case _abx:
			sprintf(buffer,"%-5s$%04X,x", token[opc], RDWORD(PC));
			PC+=2;
			break;
		case _aby:
			sprintf(buffer,"%-5s$%04X,y", token[opc], RDWORD(PC));
			PC+=2;
			break;
		case _ind:
			sprintf(buffer,"%-5s($%04X)", token[opc], RDWORD(PC));
			PC+=2;
			break;
		case _iax:
			sprintf(buffer,"%-5s($%04X),X", token[opc], RDWORD(PC));
			PC+=2;
			break;
		case _blk:
			sprintf(buffer,"%-5s$%04X $%04X $%04X", token[opc], RDWORD(PC), RDWORD(PC+2), RDWORD(PC+4));
			PC+=6;
			break;
		case _zrl:
			sprintf(buffer,"%-5s$%02X $%04X", token[opc], RDBYTE(PC), (PC + 2 + (signed char)RDBYTE(PC+1)) & 0xffff);
			PC+=2;
			break;
		case _imz:
			sprintf(buffer,"%-5s#$%02X $%02X", token[opc], RDBYTE(PC), RDBYTE(PC+1));
			PC+=2;
			break;
		case _izx:
			sprintf(buffer,"%-5s#$%02X $%02X,x", token[opc], RDBYTE(PC), RDBYTE(PC+1));
			PC+=2;
			break;
		case _ima:
			sprintf(buffer,"%-5s#$%02X $%04X", token[opc], RDBYTE(PC), RDWORD(PC+1));
			PC+=3;
			break;
		case _imx:
			sprintf(buffer,"%-5s#$%02X $%04X,x", token[opc], RDBYTE(PC), RDWORD(PC+1));
			PC+=3;
			break;

		default:
			sprintf(buffer,"%-5s$%02X", token[opc], OP >> 1);
	}
	return (PC - pc) | flags | DASMFLAG_SUPPORTED;
}
