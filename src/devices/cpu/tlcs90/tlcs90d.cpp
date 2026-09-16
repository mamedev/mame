// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

    Toshiba TLCS-90 Series MCU's disassembler

*************************************************************************************************************/

#include "emu.h"
#include "tlcs90d.h"

const char *const tlcs90_disassembler::op_names[] =
{
	"??",
	"nop",  "ex",   "exx",  "ld",   "ldw",  "lda",  "ldi",  "ldir",
	"ldd",  "lddr", "cpi",  "cpir", "cpd",  "cpdr", "push", "pop",
	"jp",   "jr",   "call", "callr","ret",  "reti", "halt", "di",
	"ei",   "swi",  "daa",  "cpl",  "neg",  "ldar", "rcf",  "scf",
	"ccf",  "tset", "bit",  "set",  "res",  "inc",  "dec",  "incx",
	"decx", "incw", "decw", "add",  "adc",  "sub",  "sbc",  "and",
	"xor",  "or",   "cp",   "rlc",  "rrc",  "rl",   "rr",   "sla",
	"sra",  "sll",  "srl",  "rld",  "rrd",  "djnz", "mul",  "div"
};

const char *const tlcs90_disassembler::r8_names[] =
{
	"b",    "c",    "d",    "e",    "h",    "l",    "a"
};
const char *const tlcs90_disassembler::r16_names[] =
{
	"bc",   "de",   "hl",   "??",   "ix",   "iy",   "sp",   "af",   "af'",  "pc"
};

const char *const tlcs90_disassembler::cc_names[] =
{
	"f",    "lt",   "le",   "ule",  "ov",   "mi",   "z",    "c",    "",     "ge",   "gt",   "ugt",  "nov",  "pl",   "nz",   "nc"
};

u32 tlcs90_disassembler::opcode_alignment() const
{
	return 1;
}

tlcs90_disassembler::tlcs90_disassembler(uint16_t iobase, const char *const ir_names[])
	: m_iobase(iobase), m_ir_names(ir_names)
{
}

const char *const tmp90840_disassembler::ir_names[0x40] =
{
	"P0",       "P1",       "P01CR/IRFL",   "IRFH",     "P2",       "P2CR",     "P3",       "P3CR",
	"P4",       "P4CR",     "P5",           "SMMOD",    "P6",       "P7",       "P67CR",    "SMCR",
	"P8",       "P8CR",     "WDMOD",        "WDCR",     "TREG0",    "TREG1",    "TREG2",    "TREG3",
	"TCLK",     "TFFCR",    "TMOD",         "TRUN",     "CAP1L",    "CAP1H",    "CAP2L",    "CAP2H",
	"TREG4L",   "TREG4H",   "TREG5L",       "TREG5H",   "T4MOD",    "T4FFCR",   "INTEL",    "INTEH",
	"DMAEH",    "SCMOD",    "SCCR",         "SCBUF",    "BX",       "BY",       "ADREG",    "ADMOD",
	nullptr,    nullptr,    nullptr,        nullptr,    nullptr,    nullptr,    nullptr,    nullptr,
	nullptr,    nullptr,    nullptr,        nullptr,    nullptr,    nullptr,    nullptr,    nullptr
};

tmp90840_disassembler::tmp90840_disassembler()
	: tlcs90_disassembler(0xffc0, ir_names)
{
}

const char *const tmp90844_disassembler::ir_names[0x40] =
{
	"P0",           "P0CR",         "P1",           "P1CR",         "P2",       "P2CR",     "P3",       "P3CR",
	"P4",           "P4CR",         "P5",           "P6",           "P7",       "P67CR",    "P23FR",    "P4FR",
	"P67FR",        "P25FR",        "WDMOD",        "WDCR",         "TREG0",    "TREG1",    "TREG2",    "TREG3",
	"T01MOD",       "T23MOD",       "TFFCR",        "TRDC",         "TRUN",     nullptr,    nullptr,    nullptr,
	"CAP1L/TREG4L", "CAP1H/TREG4H", "CAP2L/TREG5L", "CAP2H/TREG5H", "T4MOD",    "T4FFCR",   "SCMOD",    "SCCR",
	"SCBUF",        "BRGCR",        "IRFL",         "IRFH/P1FR",    nullptr,    nullptr,    "STATUS",   "ADMOD",
	"ADREG0",       "ADREG1",       "ADREG2",       "ADREG3",       "INTEL",    "INTEH",    "DMAEL",    "DMAEH",
	nullptr,        nullptr,        nullptr,        nullptr,        nullptr,    nullptr,    nullptr,    nullptr
};

tmp90844_disassembler::tmp90844_disassembler()
	: tlcs90_disassembler(0xffc0, ir_names)
{
}

const char *const tmp90c051_disassembler::ir_names[0x4c] =
{
											"P2",     "P2CR",   "P3",     "P3CR",
	"P3FR",   "P4",     "P5",     "P45CR",  "P45FR",  "P6",     "P6CR",   "P6FR",
	"TREG0",  "TREG1",  "TREG2",  "TREG3",  "T01MOD", "T23MOD", "TFFCR",  "TRDC",
	"TRUN",   "IRF0",   "IRF1",   "IRF2",   "INTE0",  "INTE1",  "INTE2",  "DMAE",
	"SCMOD0", "SCCR0",  "SCBUF0", "BRGCR0", "SCMOD1", "SCCR1",  "SCBUF1", "BRGCR1",
	"REFCR",  "MSAR",   "MSAMR",  "MACR",   nullptr,  "WDMOD",  "WDCR",   "P2FR",
	"SECR",   "MINR",   "HOUR",   "DAYR",   "DATER",  "MONTHR", "YEARR",  "PAGER",
	"RESTR",  "TPHBUF", "TPHMOD", "TPHSCR", "BX",     "BY",     "CLLARL", "CLLARH",
	"EXPA1L", "EXPA1H", "EXPA0L", "EXPA0H", nullptr,  nullptr,  "DMASB0", "DMADB0",
	"DMAV0",  "DMASB1", "DMADB1", "DMAV1",  nullptr,  nullptr,  nullptr,  nullptr
};

tmp90c051_disassembler::tmp90c051_disassembler()
	: tlcs90_disassembler(0xffb4, ir_names) // I/O base is actually FF80H
{
	// TODO: recognize TMP90C051-original LDC instruction needed to access certain DMA registers
}

const char *tlcs90_disassembler::internal_registers_names(uint16_t x) const
{
	if (x >= m_iobase)
		return m_ir_names[x - m_iobase];
	return nullptr;
}

#define B   0
#define C   1
#define D   2
#define E   3
#define H   4
#define L   5
#define A   6

#define BC  0
#define DE  1
#define HL  2
//          3
#define IX  4
#define IY  5
#define SP  6

#define AF  7
#define AF2 8
#define PC  9

#define FLS 0x0
#define LT  0x1
#define LE  0x2
#define ULE 0x3
#define OV  0x4
#define PE  0x4
#define MI  0x5
#define Z   0x6
#define EQ  0x6
#define CR  0x7
#define ULT 0x7
#define T   0x8
#define GE  0x9
#define GT  0xa
#define UGT 0xb
#define NOV 0xc
#define PO  0xc
#define PL  0xd
#define NZ  0xe
#define NE  0xe
#define NC  0xf
#define UGE 0xf

#define CF  0x01
#define NF  0x02
#define PF  0x04
#define VF  PF
#define XCF 0x08
#define HF  0x10
#define IF  0x20
#define ZF  0x40
#define SF  0x80

#define OP(   X,CT )       m_op = X;
#define OP16( X,CT )       m_op = X;

#define OPCC(   X,CF,CT )  m_op = X;
#define OPCC16( X,CF,CT )  m_op = X;

#define BIT8( N,I )        m_mode##N = e_mode::BIT8;    m_r##N = I;
#define I8( N,I )          m_mode##N = e_mode::I8;      m_r##N = I;
#define D8( N,I )          m_mode##N = e_mode::D8;      m_r##N = I;
#define I16( N,I )         m_mode##N = e_mode::I16;     m_r##N = I;
#define D16( N,I )         m_mode##N = e_mode::D16;     m_r##N = I;
#define R8( N,R )          m_mode##N = e_mode::R8;      m_r##N = R;
#define R16( N,R )         m_mode##N = e_mode::R16;     m_r##N = R;
#define Q16( N,R )         m_mode##N = e_mode::R16;     m_r##N = R; if (m_r##N == SP) m_r##N = AF;
#define MI16( N,I )        m_mode##N = e_mode::MI16;    m_r##N = I;
#define MR16( N,R )        m_mode##N = e_mode::MR16;    m_r##N = R;
#define MR16D8( N,R,I )    m_mode##N = e_mode::MR16D8;  m_r##N = R; m_r##N##b = I;
#define MR16R8( N,R,g )    m_mode##N = e_mode::MR16R8;  m_r##N = R; m_r##N##b = g;
#define NONE( N )          m_mode##N = e_mode::NONE;
#define CC( N,cc )         m_mode##N = e_mode::CC;      m_r##N = cc;
#define R16D8( N,R,I )     m_mode##N = e_mode::R16D8;   m_r##N = R; m_r##N##b = I;
#define R16R8( N,R,g )     m_mode##N = e_mode::R16R8;   m_r##N = R; m_r##N##b = g;

uint8_t  tlcs90_disassembler::READ8()
{
	uint8_t b0 = m_opcodes->r8( m_addr++ );
	m_addr &= 0xffff;
	return b0;
}
uint16_t tlcs90_disassembler::READ16()
{
	uint8_t b0 = READ8();
	return b0 | (READ8() << 8);
}

void tlcs90_disassembler::decode()
{
	uint8_t  b0, b1, b2, b3;
	uint16_t imm16;

	b0 = READ8();

	switch ( b0 )
	{
		case 0x00:
			OP( NOP,2 )         NONE( 1 )                   NONE( 2 )                       return;     // NOP

		case 0x01:
			OP( HALT,4 )        NONE( 1 )                   NONE( 2 )                       return;     // HALT
		case 0x02:
			OP( DI,2 )          NONE( 1 )                   NONE( 2 )                       return;     // DI
		case 0x03:
			OP( EI,2 )          NONE( 1 )                   NONE( 2 )                       return;     // EI

		case 0x07:
			OPCC( INCX,6,10 )   MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // INCX ($FF00+n)

		case 0x08:
			OP( EX,2 )          R16( 1, DE )                R16( 2, HL )                    return;     // EX DE,HL
		case 0x09:
			OP( EX,2 )          R16( 1, AF )                R16( 2, AF2 )                   return;     // EX AF,AF'
		case 0x0a:
			OP( EXX,2 )         NONE( 1 )                   NONE( 2 )                       return;     // EXX

		case 0x0b:
			OP( DAA,4 )         R8( 1, A )                  NONE( 2 )                       return;     // DAA A

		case 0x0c:
			OP( RCF,2 )         NONE( 1 )                   NONE( 2 )                       return;     // RCF
		case 0x0d:
			OP( SCF,2 )         NONE( 1 )                   NONE( 2 )                       return;     // SCF
		case 0x0e:
			OP( CCF,2 )         NONE( 1 )                   NONE( 2 )                       return;     // CCF

		case 0x0f:
			OPCC( DECX,6,10 )   MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // DECX ($FF00+n)

		case 0x10:
			OP( CPL,2 )         R8( 1, A )                  NONE( 2 )                       return;     // CPL A
		case 0x11:
			OP( NEG,2 )         R8( 1, A )                  NONE( 2 )                       return;     // NEG A

		case 0x12:                                                                                      // MUL HL,n
		case 0x13:                                                                                      // DIV HL,n
			OP( MUL+b0-0x12,16) R16( 1, HL )                I8( 2, READ8() )                return;

		case 0x14:  case 0x15:  case 0x16:
			OP16( ADD,6 )       R16( 1, IX+b0-0x14 )        I16( 2, READ16() )              return;     // ADD ix,mn

		case 0x17:
			OP( LDAR,8 )        R16( 1, HL )                D16( 2, READ16() )              return;     // LDAR HL,+cd

		case 0x18:
			OP( DJNZ,10 )       D8( 1, READ8() )            NONE( 2 )                       return;     // DJNZ +d
		case 0x19:
			OP16( DJNZ,10 )     R16( 1, BC )                D8( 2, READ8() )                return;     // DJNZ BC,+d

		case 0x1a:
			OPCC( JP,8,8 )      CC( 1, T )                  I16( 2, READ16() )              return;     // JP T,mn
		case 0x1b:
			OPCC16( JR,10,10 )  CC( 1, T )                  D16( 2, READ16() )              return;     // JR T,+cd

		case 0x1c:
			OPCC( CALL,14,14 )  CC( 1, T )                  I16( 2, READ16() )              return;     // CALL T,mn
		case 0x1d:
			OP( CALLR,16 )      D16( 1, READ16() )          NONE( 2 )                       return;     // CALLR +cd

		case 0x1e:
			OPCC( RET,10,10 )   CC( 1, T )                  NONE( 2 )                       return;     // RET T
		case 0x1f:
			OP( RETI,14 )       NONE( 1 )                   NONE( 2 )                       return;     // RETI

		case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
			OP( LD,2 )          R8( 1, A )                  R8( 2, b0 - 0x20 )              return;     // LD A,r

		case 0x27:
			OP( LD,8 )          R8( 1, A )                  MI16( 2, 0xFF00|READ8() )       return;     // LD A,($FF00+n)

		case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
			OP( LD,2 )          R8( 1, b0 - 0x28 )          R8( 2, A )                      return;     // LD r,A

		case 0x2f:
			OP( LD,8 )          MI16( 1, 0xFF00|READ8() )   R8( 2, A )                      return;     // LD ($FF00+n), A

		case 0x30:  case 0x31:  case 0x32:  case 0x33:  case 0x34:  case 0x35:  case 0x36:
			OP( LD,4 )          R8( 1, b0 - 0x30 )          I8( 2, READ8() )                return;     // LD r,n

		case 0x37:
			OP( LD,10 )         MI16( 1, 0xFF00|READ8() )   I8( 2, READ8() )                return;     // LD ($FF00+w),n

		case 0x38:  case 0x39:  case 0x3a:  /*case 0x3b:*/  case 0x3c:  case 0x3d:  case 0x3e:
			OP16( LD,6 )        R16( 1, b0 - 0x38 )         I16( 2, READ16() )              return;     // LD rr,nn

		case 0x3f:
			OP( LDW,14 )        MI16( 1, 0xFF00|READ8() )   I16( 2, READ16() )              return;     // LDW ($FF00+w),mn

		case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
			OP16( LD,4 )        R16( 1, HL )                R16( 2, b0 - 0x40 )             return;     // LD HL,rr

		case 0x47:
			OP16( LD,10 )       R16( 1, HL )                MI16( 2, 0xFF00|READ8() )       return;     // LD HL,($FF00+n)

		case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
			OP16( LD,4 )        R16( 1, b0 - 0x48 )         R16( 2, HL )                    return;     // LD rr,HL

		case 0x4f:
			OP16( LD,10 )       MI16( 1, 0xFF00|READ8() )   R16( 2, HL )                    return;     // LD ($FF00+n), HL

		case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
			OP( PUSH,8 )        Q16( 1, b0 - 0x50 )         NONE( 2 )                       return;     // PUSH qq
		case 0x58:  case 0x59:  case 0x5a:  /*case 0x5b:*/  case 0x5c:  case 0x5d:  case 0x5e:
			OP( POP,10 )        Q16( 1, b0 - 0x58 )         NONE( 2 )                       return;     // POP qq

		case 0x60:                                                                                      // ADD A,($FF00+n)
		case 0x61:                                                                                      // ADC A,($FF00+n)
		case 0x62:                                                                                      // SUB A,($FF00+n)
		case 0x63:                                                                                      // SBC A,($FF00+n)
		case 0x64:                                                                                      // AND A,($FF00+n)
		case 0x65:                                                                                      // XOR A,($FF00+n)
		case 0x66:                                                                                      // OR  A,($FF00+n)
		case 0x67:                                                                                      // CP  A,($FF00+n)
			OP( ADD+b0-0x60,8 ) R8( 1, A )                  MI16( 2, 0xFF00|READ8() )       return;

		case 0x68:                                                                                      // ADD A,n
		case 0x69:                                                                                      // ADC A,n
		case 0x6a:                                                                                      // SUB A,n
		case 0x6b:                                                                                      // SBC A,n
		case 0x6c:                                                                                      // AND A,n
		case 0x6d:                                                                                      // XOR A,n
		case 0x6e:                                                                                      // OR  A,n
		case 0x6f:                                                                                      // CP  A,n
			OP( ADD+b0-0x68,4 ) R8( 1, A )                  I8( 2, READ8() )                return;

		case 0x70:                                                                                      // ADD HL,($FF00+n)
		case 0x71:                                                                                      // ADC HL,($FF00+n)
		case 0x72:                                                                                      // SUB HL,($FF00+n)
		case 0x73:                                                                                      // SBC HL,($FF00+n)
		case 0x74:                                                                                      // AND HL,($FF00+n)
		case 0x75:                                                                                      // XOR HL,($FF00+n)
		case 0x76:                                                                                      // OR  HL,($FF00+n)
		case 0x77:                                                                                      // CP  HL,($FF00+n)
			OP16( ADD+b0-0x70,10 )  R16( 1, HL )            MI16( 2, 0xFF00|READ8() )       return;

		case 0x78:                                                                                      // ADD HL,mn
		case 0x79:                                                                                      // ADC HL,mn
		case 0x7a:                                                                                      // SUB HL,mn
		case 0x7b:                                                                                      // SBC HL,mn
		case 0x7c:                                                                                      // AND HL,mn
		case 0x7d:                                                                                      // XOR HL,mn
		case 0x7e:                                                                                      // OR  HL,mn
		case 0x7f:                                                                                      // CP  HL,mn
			OP16( ADD+b0-0x78,6 )   R16( 1, HL )            I16( 2, READ16() )              return;

		case 0x80:  case 0x81:  case 0x82:  case 0x83:  case 0x84:  case 0x85:  case 0x86:
			OP( INC,2 )         R8( 1, b0 - 0x80 )          NONE( 2 )                       return;     // INC r
		case 0x87:
			OP( INC,10 )        MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // INC ($FF00+n)

		case 0x88:  case 0x89:  case 0x8a:  case 0x8b:  case 0x8c:  case 0x8d:  case 0x8e:
			OP( DEC,2 )         R8( 1, b0 - 0x88 )          NONE( 2 )                       return;     // DEC r
		case 0x8f:
			OP( DEC,10 )        MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // DEC ($FF00+n)

		case 0x90:  case 0x91:  case 0x92:  /*case 0x93:*/  case 0x94:  case 0x95:  case 0x96:
			OP16( INC,4 )       R16( 1, b0 - 0x90 )         NONE( 2 )                       return;     // INC rr
		case 0x97:
			OP( INCW,14 )       MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // INCW ($FF00+n)
		case 0x98:  case 0x99:  case 0x9a:  /*case 0x9b:*/  case 0x9c:  case 0x9d:  case 0x9e:
			OP16( DEC,4 )       R16( 1, b0 - 0x98 )         NONE( 2 )                       return;     // DEC rr
		case 0x9f:
			OP( DECW,14 )       MI16( 1, 0xFF00|READ8() )   NONE( 2 )                       return;     // DECW ($FF00+n)

		case 0xa0:                                                                                      // RLC A
		case 0xa1:                                                                                      // RRC A
		case 0xa2:                                                                                      // RL  A
		case 0xa3:                                                                                      // RR  A
		case 0xa4:                                                                                      // SLA A
		case 0xa5:                                                                                      // SRA A
		case 0xa6:                                                                                      // SLL A
		case 0xa7:                                                                                      // SRL A
			OP( RLC+b0-0xa0,2 ) R8( 1, A )                  NONE( 2 )                       return;

		case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
			OP( BIT,8 )         BIT8( 1, b0 - 0xa8 )        MI16( 2, 0xFF00|READ8() )       return;     // BIT b,($FF00+n)
		case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
			OP( RES,12 )        BIT8( 1, b0 - 0xb0 )        MI16( 2, 0xFF00|READ8() )       return;     // RES b,($FF00+n)
		case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
			OP( SET,12 )        BIT8( 1, b0 - 0xb8 )        MI16( 2, 0xFF00|READ8() )       return;     // SET b,($FF00+n)

		case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
		case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
			OPCC( JR,4,8 )      CC( 1, b0 - 0xc0 )          D8( 2, READ8() )                return;     // JR cc,+d

		case 0xe0:  case 0xe1:  case 0xe2:  /*case 0xe3:*/  case 0xe4:  case 0xe5:  case 0xe6:
			b1 = READ8();
			switch ( b1 )   {
				case 0x10:                                                                              // RLD (gg)
				case 0x11:                                                                              // RRD (gg)
					OP( RLD+b1-0x10,12 )    MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;

				case 0x12:                                                                              // MUL HL,(gg)
				case 0x13:                                                                              // DIV HL,(gg)
					OP( MUL+b1-0x12,18 )    R16( 1, HL )            MR16( 2, b0 - 0xe0 )    return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,8 )           R16( 1, IX+b1-0x14 )    MR16( 2, b0 - 0xe0 )    return;     // ADD ix,(gg)

				case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
					OP( LD,6 )              R8( 1, b1 - 0x28 )      MR16( 2, b0 - 0xe0 )    return;     // LD r,(gg)
				case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
					OP16( LD,8 )            R16( 1, b1 - 0x48 )     MR16( 2, b0 - 0xe0 )    return;     // LD rr,(gg)

				case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
					OP( EX,14 )             MR16( 1, b0 - 0xe0 )    R16( 2, b1 - 0x50 )     return;     // EX (gg),rr

				case 0x60:                                                                              // ADD A,(gg)
				case 0x61:                                                                              // ADC A,(gg)
				case 0x62:                                                                              // SUB A,(gg)
				case 0x63:                                                                              // SBC A,(gg)
				case 0x64:                                                                              // AND A,(gg)
				case 0x65:                                                                              // XOR A,(gg)
				case 0x66:                                                                              // OR  A,(gg)
				case 0x67:                                                                              // CP  A,(gg)
					OP( ADD+b1-0x60,6 )     R8( 1, A )              MR16( 2, b0 - 0xe0 )    return;

				case 0x70:                                                                              // ADD HL,(gg)
				case 0x71:                                                                              // ADC HL,(gg)
				case 0x72:                                                                              // SUB HL,(gg)
				case 0x73:                                                                              // SBC HL,(gg)
				case 0x74:                                                                              // AND HL,(gg)
				case 0x75:                                                                              // XOR HL,(gg)
				case 0x76:                                                                              // OR  HL,(gg)
				case 0x77:                                                                              // CP  HL,(gg)
					OP16( ADD+b1-0x70,8 )   R16( 1, HL )            MR16( 2, b0 - 0xe0 )    return;

				case 0x87:
					OP( INC,8 )             MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;     // INC (gg)
				case 0x8f:
					OP( DEC,8 )             MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;     // DEC (gg)

				case 0x97:
					OP( INCW,12 )           MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;     // INCW (gg)
				case 0x9f:
					OP( DECW,12 )           MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;     // DECW (gg)

				case 0xa0:                                                                              // RLC (gg)
				case 0xa1:                                                                              // RRC (gg)
				case 0xa2:                                                                              // RL  (gg)
				case 0xa3:                                                                              // RR  (gg)
				case 0xa4:                                                                              // SLA (gg)
				case 0xa5:                                                                              // SRA (gg)
				case 0xa6:                                                                              // SLL (gg)
				case 0xa7:                                                                              // SRL (gg)
					OP( RLC+b1-0xa0,8 )     MR16( 1, b0 - 0xe0 )    NONE( 2 )               return;

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,12 )           BIT8( 1, b1 - 0x18 )    MR16( 2, b0 - 0xe0 )    return;     // TSET b,(gg)
				case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
					OP( BIT,6 )             BIT8( 1, b1 - 0xa8 )    MR16( 2, b0 - 0xe0 )    return;     // BIT b,(gg)
				case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
					OP( RES,10 )            BIT8( 1, b1 - 0xb0 )    MR16( 2, b0 - 0xe0 )    return;     // RES b,(gg)
				case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					OP( SET,10 )            BIT8( 1, b1 - 0xb8 )    MR16( 2, b0 - 0xe0 )    return;     // SET b,(gg)
			}   break;
		case 0xe3:
			imm16 = READ16();
			b3 = READ8();
			switch ( b3 )   {
				case 0x10:                                                                              // RLD (mn)
				case 0x11:                                                                              // RRD (mn)
					OP( RLD+b3-0x10,16 )    MI16( 1, imm16 )        NONE( 2 )               return;

				case 0x12:                                                                              // MUL HL,(mn)
				case 0x13:                                                                              // DIV HL,(mn)
					OP( MUL+b3-0x12,22 )    R16( 1, HL )            MI16( 2, imm16 )        return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,12 )          R16( 1, IX+b3-0x14 )    MI16( 2, imm16 )        return;     // ADD ix,(mn)

				case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
					OP( LD,10 )             R8( 1, b3 - 0x28 )      MI16( 2, imm16 )        return;     // LD r,(mn)
				case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
					OP16( LD,12 )           R16( 1, b3 - 0x48 )     MI16( 2, imm16 )        return;     // LD rr,(mn)

				case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
					OP( EX,18 )             MI16( 1, imm16 )        R16( 2, b3 - 0x50 )     return;     // EX (mn),rr

				case 0x60:                                                                              // ADD A,(mn)
				case 0x61:                                                                              // ADC A,(mn)
				case 0x62:                                                                              // SUB A,(mn)
				case 0x63:                                                                              // SBC A,(mn)
				case 0x64:                                                                              // AND A,(mn)
				case 0x65:                                                                              // XOR A,(mn)
				case 0x66:                                                                              // OR  A,(mn)
				case 0x67:                                                                              // CP  A,(mn)
					OP( ADD+b3-0x60,10 )    R8( 1, A )              MI16( 2, imm16 )        return;

				case 0x70:                                                                              // ADD HL,(mn)
				case 0x71:                                                                              // ADC HL,(mn)
				case 0x72:                                                                              // SUB HL,(mn)
				case 0x73:                                                                              // SBC HL,(mn)
				case 0x74:                                                                              // AND HL,(mn)
				case 0x75:                                                                              // XOR HL,(mn)
				case 0x76:                                                                              // OR  HL,(mn)
				case 0x77:                                                                              // CP  HL,(mn)
					OP16( ADD+b3-0x70,12 )  R16( 1, HL )            MI16( 2, imm16 )        return;

				case 0x87:
					OP( INC,12 )            MI16( 1, imm16 )        NONE( 2 )               return;     // INC (mn)
				case 0x8f:
					OP( DEC,12 )            MI16( 1, imm16 )        NONE( 2 )               return;     // DEC (mn)

				case 0x97:
					OP( INCW,16 )           MI16( 1, imm16 )        NONE( 2 )               return;     // INCW (mn)
				case 0x9f:
					OP( DECW,16 )           MI16( 1, imm16 )        NONE( 2 )               return;     // DECW (mn)

				case 0xa0:                                                                              // RLC (mn)
				case 0xa1:                                                                              // RRC (mn)
				case 0xa2:                                                                              // RL  (mn)
				case 0xa3:                                                                              // RR  (mn)
				case 0xa4:                                                                              // SLA (mn)
				case 0xa5:                                                                              // SRA (mn)
				case 0xa6:                                                                              // SLL (mn)
				case 0xa7:                                                                              // SRL (mn)
					OP( RLC+b3-0xa0,12 )    MI16( 1, imm16 )        NONE( 2 )               return;

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,16 )           BIT8( 1, b3 - 0x18 )    MI16( 2, imm16 )        return;     // TSET b,(mn)
				case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
					OP( BIT,10 )            BIT8( 1, b3 - 0xa8 )    MI16( 2, imm16 )        return;     // BIT b,(mn)
				case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
					OP( RES,14 )            BIT8( 1, b3 - 0xb0 )    MI16( 2, imm16 )        return;     // RES b,(mn)
				case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					OP( SET,14 )            BIT8( 1, b3 - 0xb8 )    MI16( 2, imm16 )        return;     // SET b,(mn)
			}   break;

		case 0xe7:
			b1 = READ8();
			b2 = READ8();
			switch ( b2 )   {
				case 0x10:                                                                              // RLD ($FF00+n)
				case 0x11:                                                                              // RRD ($FF00+n)
					OP( RLD+b2-0x10,14 )    MI16( 1, 0xFF00|b1 )    NONE( 2 )               return;

				case 0x12:                                                                              // MUL HL,($FF00+n)
				case 0x13:                                                                              // DIV HL,($FF00+n)
					OP( MUL+b2-0x12,20 )    R16( 1, HL )            MI16( 2, 0xFF00|b1 )    return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,10 )          R16( 1, IX+b2-0x14 )    MI16( 2, 0xFF00|b1 )    return;     // ADD ix,($FF00+n)

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,14 )           BIT8( 1, b2 - 0x18 )    MI16( 2, 0xFF00|b1 )    return;     // TSET b,($FF00+n)
				case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
					OP( LD,8 )              R8( 1, b2 - 0x28 )      MI16( 2, 0xFF00|b1 )    return;     // LD r,($FF00+n)
				case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
					OP16( LD,10 )           R16( 1, b2 - 0x48 )     MI16( 2, 0xFF00|b1 )    return;     // LD rr,($FF00+n)

				case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
					OP( EX,16 )             MI16( 1, 0xFF00|b1 )    R16( 2, b2 - 0x50 )     return;     // EX ($FF00+n),rr

				case 0xa0:                                                                              // RLC ($FF00+n)
				case 0xa1:                                                                              // RRC ($FF00+n)
				case 0xa2:                                                                              // RL  ($FF00+n)
				case 0xa3:                                                                              // RR  ($FF00+n)
				case 0xa4:                                                                              // SLA ($FF00+n)
				case 0xa5:                                                                              // SRA ($FF00+n)
				case 0xa6:                                                                              // SLL ($FF00+n)
				case 0xa7:                                                                              // SRL ($FF00+n)
					OP( RLC+b2-0xa0,10 )    MI16( 1, 0xFF00|b1 )    NONE( 2 )               return;
			}   break;

		case 0xe8:  case 0xe9:  case 0xea:  /*case 0xeb:*/  case 0xec:  case 0xed:  case 0xee:
			b1 = READ8();
			switch ( b1 )   {
				case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
					OP( LD,6 )              MR16( 1, b0 - 0xe8 )    R8( 2, b1 - 0x20 )      return;     // LD (gg),r
				case 0x37:
					OP( LD,8 )              MR16( 1, b0 - 0xe8 )    I8( 2, READ8() )        return;     // LD (gg),n
				case 0x3f:
					OP( LDW,12 )            MR16( 1, b0 - 0xe8 )    I16( 2, READ16() )      return;     // LDW (gg),mn
				case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
					OP16( LD,8 )            MR16( 1, b0 - 0xe8 )    R16( 2, b1 - 0x40 )     return;     // LD (gg),rr

				case 0x68:                                                                              // ADD (gg),n
				case 0x69:                                                                              // ADC (gg),n
				case 0x6a:                                                                              // SUB (gg),n
				case 0x6b:                                                                              // SBC (gg),n
				case 0x6c:                                                                              // AND (gg),n
				case 0x6d:                                                                              // XOR (gg),n
				case 0x6e:                                                                              // OR  (gg),n
					OP( ADD+b1-0x68,10 )    MR16( 1, b0 - 0xe8 )    I8( 2, READ8() )        return;
				case 0x6f:                                                                              // CP  (gg),n
					OP( CP,8 )              MR16( 1, b0 - 0xe8 )    I8( 2, READ8() )        return;

				case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
				case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					OPCC( JP,6,8 )          CC( 1, b1 - 0xc0 )      R16( 2, b0 - 0xe8 )     return;     // JP [cc,]gg
				case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
				case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
					OPCC( CALL,6,14 )       CC( 1, b1 - 0xd0 )      R16( 2, b0 - 0xe8 )     return;     // CALL [cc,]gg
			}   break;
		case 0xeb:
			imm16 = READ16();
			b3 = READ8();
			switch ( b3 )   {
				case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
					OP( LD,10 )             MI16( 1, imm16 )        R8( 2, b3 - 0x20 )      return;     // LD (mn),r
				case 0x37:
					OP( LD,12 )             MI16( 1, imm16 )        I8( 2, READ8() )        return;     // LD (vw),n
				case 0x3f:
					OP( LDW,16 )            MI16( 1, imm16 )        I16( 2, READ16() )      return;     // LDW (vw),mn
				case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
					OP16( LD,12 )           MI16( 1, imm16 )        R16( 2, b3 - 0x40 )     return;     // LD (mn),rr

				case 0x68:                                                                              // ADD (vw),n
				case 0x69:                                                                              // ADC (vw),n
				case 0x6a:                                                                              // SUB (vw),n
				case 0x6b:                                                                              // SBC (vw),n
				case 0x6c:                                                                              // AND (vw),n
				case 0x6d:                                                                              // XOR (vw),n
				case 0x6e:                                                                              // OR  (vw),n
					OP( ADD+b3-0x68,14 )    MI16( 1, imm16 )        I8( 2, READ8() )        return;
				case 0x6f:                                                                              // CP  (vw),n
					OP( ADD+b3-0x68,12 )    MI16( 1, imm16 )        I8( 2, READ8() )        return;

				case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
				case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					OPCC( JP,10,12 )        CC( 1, b3 - 0xc0 )      I16( 2, imm16 )         return;     // JP cc,mn
				case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
				case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
					OPCC( CALL,10,18 )      CC( 1, b3 - 0xd0 )      I16( 2, imm16 )         return;     // CALL cc,mn
			}   break;

		case 0xef:
			b1 = READ8();
			b2 = READ8();
			switch ( b2 )   {
				case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
					OP( LD,8 )              MI16( 1, 0xFF00|b1 )    R8( 2, b2 - 0x20 )      return;     // LD ($FF00+n),r
				case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
					OP16( LD,10 )           MI16( 1, 0xFF00|b1 )    R16( 2, b2 - 0x40 )     return;     // LD ($FF00+n),rr

				case 0x68:                                                                              // ADD ($FF00+w),n
				case 0x69:                                                                              // ADC ($FF00+w),n
				case 0x6a:                                                                              // SUB ($FF00+w),n
				case 0x6b:                                                                              // SBC ($FF00+w),n
				case 0x6c:                                                                              // AND ($FF00+w),n
				case 0x6d:                                                                              // XOR ($FF00+w),n
				case 0x6e:                                                                              // OR  ($FF00+w),n
					OP( ADD+b2-0x68,12 )    MI16( 1, 0xFF00|b1 )    I8( 2, READ8() )        return;
				case 0x6f:                                                                              // CP  ($FF00+w),n
					OP( ADD+b2-0x68,10 )    MI16( 1, 0xFF00|b1 )    I8( 2, READ8() )        return;
			}   break;

		case 0xf0:  case 0xf1:  case 0xf2:
			b1 = READ8();
			b2 = READ8();
			switch ( b2 )   {
				case 0x10:                                                                              // RLD (ix+d)
				case 0x11:                                                                              // RRD (ix+d)
					OP( RLD+b2-0x10,16 )    MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;

				case 0x12:                                                                              // MUL HL,(ix+d)
				case 0x13:                                                                              // DIV HL,(ix+d)
					OP( MUL+b2-0x12,22 )    R16( 1, HL )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,12 )  R16( 1, IX+b2-0x14 )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;     // ADD ix,(jx+d)

				case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
					OP( LD,10 )     R8( 1, b2 - 0x28 )  MR16D8( 2, IX + b0 - 0xf0, b1 )     return;     // LD r,(ix+d)
				case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
					OP16( LD,12 )   R16( 1, b2 - 0x48 ) MR16D8( 2, IX + b0 - 0xf0, b1 )     return;     // LD rr,(ix+d)

				case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
					OP( EX,18 )     MR16D8( 1, IX + b0 - 0xf0, b1 ) R16( 2, b2 - 0x50 )     return;     // EX (ix+d),rr

				case 0x60:                                                                              // ADD A,(ix+d)
				case 0x61:                                                                              // ADC A,(ix+d)
				case 0x62:                                                                              // SUB A,(ix+d)
				case 0x63:                                                                              // SBC A,(ix+d)
				case 0x64:                                                                              // AND A,(ix+d)
				case 0x65:                                                                              // XOR A,(ix+d)
				case 0x66:                                                                              // OR  A,(ix+d)
				case 0x67:                                                                              // CP  A,(ix+d)
					OP( ADD+b2-0x60,10 )    R8( 1, A )  MR16D8( 2, IX + b0 - 0xf0, b1 )     return;

				case 0x70:                                                                              // ADD HL,(ix+d)
				case 0x71:                                                                              // ADC HL,(ix+d)
				case 0x72:                                                                              // SUB HL,(ix+d)
				case 0x73:                                                                              // SBC HL,(ix+d)
				case 0x74:                                                                              // AND HL,(ix+d)
				case 0x75:                                                                              // XOR HL,(ix+d)
				case 0x76:                                                                              // OR  HL,(ix+d)
				case 0x77:                                                                              // CP  HL,(ix+d)
					OP16( ADD+b2-0x70,12 )  R16( 1, HL )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;

				case 0x87:
					OP( INC,12 )            MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;     // INC (ix+d)
				case 0x8f:
					OP( DEC,12 )            MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;     // DEC (ix+d)

				case 0x97:
					OP( INCW,16 )           MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;     // INCW (ix+d)
				case 0x9f:
					OP( DECW,16 )           MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;     // DECW (ix+d)

				case 0xa0:                                                                              // RLC (ix+d)
				case 0xa1:                                                                              // RRC (ix+d)
				case 0xa2:                                                                              // RL  (ix+d)
				case 0xa3:                                                                              // RR  (ix+d)
				case 0xa4:                                                                              // SLA (ix+d)
				case 0xa5:                                                                              // SRA (ix+d)
				case 0xa6:                                                                              // SLL (ix+d)
				case 0xa7:                                                                              // SRL (ix+d)
					OP( RLC+b2-0xa0,12 )    MR16D8( 1, IX + b0 - 0xf0, b1 )     NONE( 2 )   return;

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,16 )   BIT8( 1, b2 - 0x18 )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;     // TSET b,(ix+d)
				case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
					OP( BIT,10 )    BIT8( 1, b2 - 0xa8 )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;     // BIT b,(ix+d)
				case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
					OP( RES,14 )    BIT8( 1, b2 - 0xb0 )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;     // RES b,(ix+d)
				case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					OP( SET,14 )    BIT8( 1, b2 - 0xb8 )    MR16D8( 2, IX + b0 - 0xf0, b1 ) return;     // SET b,(ix+d)
			}   break;

		case 0xf3:
			b1 = READ8();
			switch ( b1 )   {
				case 0x10:                                                                              // RLD (HL+A)
				case 0x11:                                                                              // RRD (HL+A)
					OP( RLD+b1-0x10,20 )    MR16R8( 1, HL, A )          NONE( 2 )           return;

				case 0x12:                                                                              // MUL HL,(HL+A)
				case 0x13:                                                                              // DIV HL,(HL+A)
					OP( MUL+b1-0x12,26 )    R16( 1, HL )                MR16R8( 2, HL, A )  return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,16 )          R16( 1, IX+b1-0x14 )        MR16R8( 2, HL, A )  return;     // ADD ix,(HL+A)

				case 0x28:  case 0x29:  case 0x2a:  case 0x2b:  case 0x2c:  case 0x2d:  case 0x2e:
					OP( LD,14 )             R8( 1, b1 - 0x28 )          MR16R8( 2, HL, A )  return;     // LD r,(HL+A)
				case 0x48:  case 0x49:  case 0x4a:  /*case 0x4b:*/  case 0x4c:  case 0x4d:  case 0x4e:
					OP16( LD,16 )           R16( 1, b1 - 0x48 )         MR16R8( 2, HL, A )  return;     // LD rr,(HL+A)

				case 0x50:  case 0x51:  case 0x52:  /*case 0x53:*/  case 0x54:  case 0x55:  case 0x56:
					OP( EX,22 )             MR16R8( 1, HL, A )          R16( 2, b1 - 0x50 ) return;     // EX (HL+A),rr

				case 0x60:                                                                              // ADD A,(HL+A)
				case 0x61:                                                                              // ADC A,(HL+A)
				case 0x62:                                                                              // SUB A,(HL+A)
				case 0x63:                                                                              // SBC A,(HL+A)
				case 0x64:                                                                              // AND A,(HL+A)
				case 0x65:                                                                              // XOR A,(HL+A)
				case 0x66:                                                                              // OR  A,(HL+A)
				case 0x67:                                                                              // CP  A,(HL+A)
					OP( ADD+b1-0x60,14 )    R8( 1, A )                  MR16R8( 2, HL, A )  return;

				case 0x70:                                                                              // ADD HL,(HL+A)
				case 0x71:                                                                              // ADC HL,(HL+A)
				case 0x72:                                                                              // SUB HL,(HL+A)
				case 0x73:                                                                              // SBC HL,(HL+A)
				case 0x74:                                                                              // AND HL,(HL+A)
				case 0x75:                                                                              // XOR HL,(HL+A)
				case 0x76:                                                                              // OR  HL,(HL+A)
				case 0x77:                                                                              // CP  HL,(HL+A)
					OP16( ADD+b1-0x70,16 )  R16( 1, HL )                MR16R8( 2, HL, A )  return;

				case 0x87:
					OP( INC,16 )            MR16R8( 1, HL, A )          NONE( 2 )           return;     // INC (HL+A)
				case 0x8f:
					OP( DEC,16 )            MR16R8( 1, HL, A )          NONE( 2 )           return;     // DEC (HL+A)

				case 0x97:
					OP( INCW,20 )           MR16R8( 1, HL, A )          NONE( 2 )           return;     // INCW (HL+A)
				case 0x9f:
					OP( DECW,20 )           MR16R8( 1, HL, A )          NONE( 2 )           return;     // DECW (HL+A)

				case 0xa0:                                                                              // RLC (HL+A)
				case 0xa1:                                                                              // RRC (HL+A)
				case 0xa2:                                                                              // RL  (HL+A)
				case 0xa3:                                                                              // RR  (HL+A)
				case 0xa4:                                                                              // SLA (HL+A)
				case 0xa5:                                                                              // SRA (HL+A)
				case 0xa6:                                                                              // SLL (HL+A)
				case 0xa7:                                                                              // SRL (HL+A)
					OP( RLC+b1-0xa0,16 )    MR16R8( 1, HL, A )          NONE( 2 )           return;

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,20 )           BIT8( 1, b1 - 0x18 )        MR16R8( 2, HL, A )  return;     // TSET b,(HL+A)
				case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
					OP( BIT,14 )            BIT8( 1, b1 - 0xa8 )        MR16R8( 2, HL, A )  return;     // BIT b,(HL+A)
				case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
					OP( RES,18 )            BIT8( 1, b1 - 0xb0 )        MR16R8( 2, HL, A )  return;     // RES b,(HL+A)
				case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					OP( SET,18 )            BIT8( 1, b1 - 0xb8 )        MR16R8( 2, HL, A )  return;     // SET b,(HL+A)
			}   break;

		case 0xf4:  case 0xf5:  case 0xf6:
			b1 = READ8();
			b2 = READ8();
			switch ( b2 )   {
				case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
					OP( LD,10 )     MR16D8( 1, IX + b0 - 0xf4, b1 ) R8( 2, b2 - 0x20 )      return;     // LD (ix+d),r
				case 0x37:
					OP( LD,12 )     MR16D8( 1, IX + b0 - 0xf4, b1 ) I8( 2, READ8() )        return;     // LD (ix+d),n
				case 0x38:  case 0x39:  case 0x3a:  /*case 0x3b:*/  case 0x3c:  case 0x3d:  case 0x3e:
					OP( LDA,10 )    R16( 1, b2 - 0x38 )     R16D8( 2, IX + b0 - 0xf4, b1 )  return;     // LDA rr,ix+d
				case 0x3f:
					OP( LDW,16 )    MR16D8( 1, IX + b0 - 0xf4, b1 ) I16( 2, READ16() )      return;     // LDW (ix+d),mn
				case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
					OP16( LD,12 )   MR16D8( 1, IX + b0 - 0xf4, b1 ) R16( 2, b2 - 0x40 )     return;     // LD (ix+d),rr

				case 0x68:                                                                              // ADD (ix+d),n
				case 0x69:                                                                              // ADC (ix+d),n
				case 0x6a:                                                                              // SUB (ix+d),n
				case 0x6b:                                                                              // SBC (ix+d),n
				case 0x6c:                                                                              // AND (ix+d),n
				case 0x6d:                                                                              // XOR (ix+d),n
				case 0x6e:                                                                              // OR  (ix+d),n
					OP( ADD+b2-0x68,14) MR16D8( 1, IX + b0 - 0xf4, b1 ) I8( 2, READ8() )    return;
				case 0x6f:                                                                              // CP  (ix+d),n
					OP( ADD+b2-0x68,12) MR16D8( 1, IX + b0 - 0xf4, b1 ) I8( 2, READ8() )    return;

				case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
				case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					OPCC( JP,10,12 )    CC( 1, b2 - 0xc0 )  R16D8( 2, IX + b0 - 0xf4, b1 )  return;     // JP [cc,]ix+d
				case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
				case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
					OPCC( CALL,10,18 )  CC( 1, b2 - 0xd0 )  R16D8( 2, IX + b0 - 0xf4, b1 )  return;     // CALL [cc,]ix+d
			}   break;

		case 0xf7:
			b1 = READ8();
			switch ( b1 )   {
				case 0x20:  case 0x21:  case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
					OP( LD,14 )         MR16R8( 1, HL, A )      R8( 2, b1 - 0x20 )          return;     // LD (HL+A),r
				case 0x37:
					OP( LD,16 )         MR16R8( 1, HL, A )      I8( 2, READ8() )            return;     // LD (HL+A),n
				case 0x38:  case 0x39:  case 0x3a:  /*case 0x3b:*/  case 0x3c:  case 0x3d:  case 0x3e:
					OP( LDA,14 )        R16( 1, b1 - 0x38 )     R16R8( 2, HL, A )           return;     // LDA rr,HL+A
				case 0x3f:
					OP( LDW,20 )        MR16R8( 1, HL, A )      I16( 2, READ16() )          return;     // LDW (HL+A),mn
				case 0x40:  case 0x41:  case 0x42:  /*case 0x43:*/  case 0x44:  case 0x45:  case 0x46:
					OP16( LD,16 )       MR16R8( 1, HL, A )      R16( 2, b1 - 0x40 )         return;     // LD (HL+A),rr

				case 0x68:                                                                              // ADD (HL+A),n
				case 0x69:                                                                              // ADC (HL+A),n
				case 0x6a:                                                                              // SUB (HL+A),n
				case 0x6b:                                                                              // SBC (HL+A),n
				case 0x6c:                                                                              // AND (HL+A),n
				case 0x6d:                                                                              // XOR (HL+A),n
				case 0x6e:                                                                              // OR  (HL+A),n
					OP( ADD+b1-0x68,18) MR16R8( 1, HL, A )      I8( 2, READ8() )            return;
				case 0x6f:                                                                              // CP  (HL+A),n
					OP( ADD+b1-0x68,16) MR16R8( 1, HL, A )      I8( 2, READ8() )            return;

				case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
				case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					OPCC( JP,14,16 )    CC( 1, b1 - 0xc0 )      R16R8( 2, HL, A )           return;     // JP [cc,]HL+A
				case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
				case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
					OPCC( CALL,14,22 )  CC( 1, b1 - 0xd0 )      R16R8( 2, HL, A )           return;     // CALL [cc,]HL+A
			}   break;

		case 0xf8:  case 0xf9:  case 0xfa:  case 0xfb:  case 0xfc:  case 0xfd:  case 0xfe:
			b1 = READ8();
			switch ( b1 )   {
				case 0x12:                                                                              // MUL HL,g
				case 0x13:                                                                              // DIV HL,g
					OP( MUL+b1-0x12,18) R16( 1, HL )            R8( 2, b0 - 0xf8 )          return;

				case 0x14:  case 0x15:  case 0x16:
					OP16( ADD,8 )       R16( 1, IX+b1-0x14 )    R16( 2, b0 - 0xf8 )         return;     // ADD ix,gg

				case 0x30:  case 0x31:  case 0x32:  case 0x33:  case 0x34:  case 0x35:  case 0x36:
					OP( LD,4 )          R8( 1, b1 - 0x30 )      R8( 2, b0 - 0xf8 )          return;     // LD r,g
				case 0x38:  case 0x39:  case 0x3a:  /*case 0x3b:*/  case 0x3c:  case 0x3d:  case 0x3e:
					OP16( LD,6 )        R16( 1, b1 - 0x38 )     R16( 2, b0 - 0xf8 )         return;     // LD rr,gg

				case 0x58:                                                                              // LDI
				case 0x59:                                                                              // LDIR
				case 0x5a:                                                                              // LDD
				case 0x5b:                                                                              // LDDR
				case 0x5c:                                                                              // CPI
				case 0x5d:                                                                              // CPIR
				case 0x5e:                                                                              // CPD
				case 0x5f:                                                                              // CPDR
				if (b0 == 0xfe) {
					OPCC( LDI+b1-0x58,14,18 )   NONE( 1 )       NONE( 2 )                   return;
				}
				break;

				case 0x60:                                                                              // ADD A,g
				case 0x61:                                                                              // ADC A,g
				case 0x62:                                                                              // SUB A,g
				case 0x63:                                                                              // SBC A,g
				case 0x64:                                                                              // AND A,g
				case 0x65:                                                                              // XOR A,g
				case 0x66:                                                                              // OR  A,g
				case 0x67:                                                                              // CP  A,g
					OP( ADD+b1-0x60,4 ) R8( 1, A )              R8( 2, b0 - 0xf8 )          return;

				case 0x68:                                                                              // ADD g,n
				case 0x69:                                                                              // ADC g,n
				case 0x6a:                                                                              // SUB g,n
				case 0x6b:                                                                              // SBC g,n
				case 0x6c:                                                                              // AND g,n
				case 0x6d:                                                                              // XOR g,n
				case 0x6e:                                                                              // OR  g,n
				case 0x6f:                                                                              // CP  g,n
					OP( ADD+b1-0x68,6 ) R8( 1, b0 - 0xf8 )      I8( 2, READ8() )            return;

				case 0x70:                                                                              // ADD HL,gg
				case 0x71:                                                                              // ADC HL,gg
				case 0x72:                                                                              // SUB HL,gg
				case 0x73:                                                                              // SBC HL,gg
				case 0x74:                                                                              // AND HL,gg
				case 0x75:                                                                              // XOR HL,gg
				case 0x76:                                                                              // OR  HL,gg
				case 0x77:                                                                              // CP  HL,gg
					OP16( ADD+b1-0x70,8 )   R16( 1, HL )        R16( 2, b0 - 0xf8 )         return;

				case 0xa0:                                                                              // RLC g
				case 0xa1:                                                                              // RRC g
				case 0xa2:                                                                              // RL  g
				case 0xa3:                                                                              // RR  g
				case 0xa4:                                                                              // SLA g
				case 0xa5:                                                                              // SRA g
				case 0xa6:                                                                              // SLL g
				case 0xa7:                                                                              // SRL g
					OP( RLC+b1-0xa0,4 ) R8( 1, b0 - 0xf8 )      NONE( 2 )                   return;

				case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
					OP( TSET,8 )        BIT8( 1, b1 - 0x18 )    R8( 2, b0 - 0xf8 )          return;     // TSET b,g
				case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:  case 0xac:  case 0xad:  case 0xae:  case 0xaf:
					OP( BIT,4 )         BIT8( 1, b1 - 0xa8 )    R8( 2, b0 - 0xf8 )          return;     // BIT b,g
				case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:  case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
					OP( RES,4 )         BIT8( 1, b1 - 0xb0 )    R8( 2, b0 - 0xf8 )          return;     // RES b,g
				case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:  case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					OP( SET,4 )         BIT8( 1, b1 - 0xb8 )    R8( 2, b0 - 0xf8 )          return;     // SET b,g

				case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
				case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
				if (b0 == 0xfe) {
					OPCC( RET,6,14 )    CC( 1, b1 - 0xd0 )      NONE( 2 )                   return;     // RET cc
				}
				break;
			}   break;

		case 0xff:
			OP( SWI,20 )                NONE( 1 )               NONE( 2 )                   return;     // SWI
	}

	OP( UNKNOWN,2 )     NONE( 1 )       NONE( 2 )
}

bool tlcs90_disassembler::stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const e_mode mode, const uint16_t r, const uint16_t rb)
{
	const char *reg_name;
	switch ( mode )
	{
	case e_mode::NONE:   return false;

	case e_mode::BIT8:   util::stream_format(stream, "%s%d",            pre,    r                                   );   return true;
	case e_mode::I8:     util::stream_format(stream, "%s$%02X",         pre,    r                                   );   return true;
	case e_mode::D8:     util::stream_format(stream, "%s$%04X",         pre,    (pc+2+(r&0x7f)-(r&0x80))&0xffff     );   return true;
	case e_mode::I16:    util::stream_format(stream, "%s$%04X",         pre,    r                                   );   return true;
	case e_mode::D16:    util::stream_format(stream, "%s$%04X",         pre,    (pc+2+(r&0x7fff)-(r&0x8000))&0xffff );   return true;
	case e_mode::MI16:
		reg_name = internal_registers_names(r);
		if (reg_name)
			util::stream_format(stream, "%s(%s)", pre, reg_name);
		else
			util::stream_format(stream, "%s($%04X)", pre, r);
		return true;
	case e_mode::R8:     util::stream_format(stream, "%s%s",            pre,    r8_names[r]                         );   return true;
	case e_mode::R16:    util::stream_format(stream, "%s%s",            pre,    r16_names[r]                        );   return true;
	case e_mode::MR16:   util::stream_format(stream, "%s(%s)",          pre,    r16_names[r]                        );   return true;

	case e_mode::MR16R8: util::stream_format(stream, "%s(%s+%s)",       pre,    r16_names[r],   r8_names[rb]        );   return true;
	case e_mode::MR16D8: util::stream_format(stream, "%s(%s%c$%02X)",   pre,    r16_names[r],   (rb&0x80)?'-':'+',  (rb&0x80)?((rb^0xff)+1):rb  );   return true;

	case e_mode::CC:     util::stream_format(stream, "%s%s",            pre,    cc_names[r]                         );   return true;

	case e_mode::R16R8:  util::stream_format(stream, "%s%s+%s",         pre,    r16_names[r],   r8_names[rb]        );   return true;
	case e_mode::R16D8:  util::stream_format(stream, "%s%s%c$%02X",     pre,    r16_names[r],   (rb&0x80)?'-':'+',  (rb&0x80)?((rb^0xff)+1):rb  );   return true;
	}

	// never executed
	throw std::logic_error(util::string_format("%04x: unimplemented addr mode = %d\n",pc,std::underlying_type_t<e_mode>(mode)));
}

offs_t tlcs90_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	m_addr = pc;
	m_opcodes = &opcodes;

	decode();

	util::stream_format         (stream, "%-5s",                op_names[ m_op ] ); // strlen("callr") == 5
	bool streamed = stream_arg  (stream, pc,       " ",         m_mode1, m_r1, m_r1b );
	stream_arg                  (stream, pc, streamed ?",":"",  m_mode2, m_r2, m_r2b );

	return (m_addr - pc) | SUPPORTED;
}
