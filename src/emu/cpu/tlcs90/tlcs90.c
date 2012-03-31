/*************************************************************************************************************

    Toshiba TLCS-90 Series MCU's

    emulation by Luca Elia, based on the Z80 core by Juergen Buchmueller

*************************************************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tlcs90.h"

typedef enum					{	UNKNOWN,	NOP,	EX,		EXX,	LD,		LDW,	LDA,	LDI,	LDIR,	LDD,	LDDR,	CPI,	CPIR,	CPD,	CPDR,	PUSH,	POP,	JP,		JR,		CALL,	CALLR,		RET,	RETI,	HALT,	DI,		EI,		SWI,	DAA,	CPL,	NEG,	LDAR,	RCF,	SCF,	CCF,	TSET,	BIT,	SET,	RES,	INC,	DEC,	INCX,	DECX,	INCW,	DECW,	ADD,	ADC,	SUB,	SBC,	AND,	XOR,	OR,		CP,		RLC,	RRC,	RL,		RR,		SLA,	SRA,	SLL,	SRL,	RLD,	RRD,	DJNZ,	MUL,	DIV		}	_e_op;
typedef UINT8 e_op;
static const char *const op_names[] =	{	"??",		"nop",	"ex",	"exx",	"ld",	"ldw",	"lda",	"ldi",	"ldir",	"ldd",	"lddr",	"cpi",	"cpir",	"cpd",	"cpdr",	"push",	"pop",	"jp",	"jr",	"call",	"callr",	"ret",	"reti",	"halt",	"di",	"ei",	"swi",	"daa",	"cpl",	"neg",	"ldar",	"rcf",	"scf",	"ccf",	"tset",	"bit",	"set",	"res",	"inc",	"dec",	"incx",	"decx",	"incw",	"decw",	"add",	"adc",	"sub",	"sbc",	"and",	"xor",	"or",	"cp",	"rlc",	"rrc",	"rl",	"rr",	"sla",	"sra",	"sll",	"srl",	"rld",	"rrd",	"djnz",	"mul",	"div"	};

typedef enum	{
	MODE_NONE,	MODE_BIT8,	MODE_CC,
	MODE_I8,	MODE_D8,	MODE_R8,
	MODE_I16,	MODE_D16,	MODE_R16,
	MODE_MI16,	MODE_MR16,	MODE_MR16D8,	MODE_MR16R8,
	MODE_R16D8,	MODE_R16R8
}	e_mode;
ALLOW_SAVE_TYPE(e_mode); // allow save_item on a non-fundamental type

typedef UINT16 e_r;

typedef struct
{
	PAIR		prvpc,pc,sp,af,bc,de,hl,ix,iy;
	PAIR		af2,bc2,de2,hl2;
	UINT8		halt, after_EI;
	UINT16		irq_state, irq_mask;
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	address_space *io;
	int		icount;
	int			extra_cycles;		// extra cycles for interrupts
	UINT8		internal_registers[48];
	UINT32		ixbase,iybase;

	// Timers: 4 x 8-bit + 1 x 16-bit
	emu_timer	*timer[4+1];
	UINT8		timer_value[4];
	UINT16		timer4_value;
	attotime	timer_period;

	// Work registers
	e_op		op;

	e_mode	mode1;
	e_r		r1,r1b;

	e_mode	mode2;
	e_r		r2,r2b;

	int	cyc_t,cyc_f;

	UINT32	addr;

}	t90_Regs;

INLINE t90_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMP90840 ||
		   device->type() == TMP90841 ||
		   device->type() == TMP91640 ||
		   device->type() == TMP91641);
	return (t90_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

enum	{
		T90_B,	T90_C,	T90_D,	T90_E,	T90_H,	T90_L,	T90_A,
		T90_BC,	T90_DE,	T90_HL,	T90_XX,	T90_IX,	T90_IY,	T90_SP,
		T90_AF, T90_PC
};

// Regs

#define	B	0
#define	C	1
#define	D	2
#define	E	3
#define	H	4
#define	L	5
#define	A	6

#define	BC	0
#define	DE	1
#define	HL	2
//          3
#define	IX	4
#define	IY	5
#define	SP	6

#define	AF	7
#define	AF2	8
#define	PC	9

#define F	cpustate->af.b.l

static const char *const r8_names[]	=	{	"b",	"c",	"d",	"e",	"h",	"l",	"a"								};
static const char *const r16_names[]	=	{	"bc",	"de",	"hl",	"??",	"ix",	"iy",	"sp",	"af",	"af'",	"pc"	};

// Condition Codes

#define FLS	0x0
#define LT	0x1
#define LE	0x2
#define ULE	0x3
#define OV	0x4
#define PE	0x4
#define MI	0x5
#define Z	0x6
#define EQ	0x6
#define CR	0x7
#define ULT	0x7
#define T	0x8
#define GE	0x9
#define GT	0xa
#define UGT	0xb
#define NOV	0xc
#define PO	0xc
#define PL	0xd
#define NZ	0xe
#define NE	0xe
#define NC	0xf
#define UGE	0xf

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	PF
#define XCF	0x08
#define HF	0x10
#define IF	0x20
#define ZF	0x40
#define SF	0x80

static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static const char *const cc_names[]	=	{	"f",	"lt",	"le",	"ule",	"ov",	"mi",	"z",	"c",	"",		"ge",	"gt",	"ugt",	"nov",	"pl",	"nz",	"nc"	};

// Opcodes

#define OP_16 0x80





#define OP(   X,CT )		cpustate->op = X;		cpustate->cyc_t = (CT*2);
#define OP16( X,CT )		OP( (X)|OP_16,CT )

#define OPCC(   X,CF,CT )	OP( X, CT )	cpustate->cyc_f = (CF*2);
#define OPCC16( X,CF,CT )	OPCC( (X)|OP_16,CF,CT )

#define BIT8( N,I )			cpustate->mode##N = MODE_BIT8;	cpustate->r##N = I;
#define I8( N,I )			cpustate->mode##N = MODE_I8;		cpustate->r##N = I;
#define D8( N,I )			cpustate->mode##N = MODE_D8;		cpustate->r##N = I;
#define I16( N,I )			cpustate->mode##N = MODE_I16;		cpustate->r##N = I;
#define D16( N,I )			cpustate->mode##N = MODE_D16;		cpustate->r##N = I;
#define R8( N,R )			cpustate->mode##N = MODE_R8;		cpustate->r##N = R;
#define R16( N,R )			cpustate->mode##N = MODE_R16;		cpustate->r##N = R;
#define Q16( N,R )			cpustate->mode##N = MODE_R16;		cpustate->r##N = R;	if (cpustate->r##N == SP) cpustate->r##N = AF;
#define MI16( N,I )			cpustate->mode##N = MODE_MI16;	cpustate->r##N = I;
#define MR16( N,R )			cpustate->mode##N = MODE_MR16;	cpustate->r##N = R;
#define MR16D8( N,R,I )		cpustate->mode##N = MODE_MR16D8;	cpustate->r##N = R;	cpustate->r##N##b = I;
#define MR16R8( N,R,g )		cpustate->mode##N = MODE_MR16R8;	cpustate->r##N = R;	cpustate->r##N##b = g;
#define NONE( N )			cpustate->mode##N = MODE_NONE;
#define CC( N,cc )			cpustate->mode##N = MODE_CC;		cpustate->r##N = cc;
#define R16D8( N,R,I )		cpustate->mode##N = MODE_R16D8;	cpustate->r##N = R;	cpustate->r##N##b = I;
#define R16R8( N,R,g )		cpustate->mode##N = MODE_R16R8;	cpustate->r##N = R;	cpustate->r##N##b = g;

INLINE UINT8  RM8 (t90_Regs *cpustate, UINT32 a)	{ return cpustate->program->read_byte( a ); }
INLINE UINT16 RM16(t90_Regs *cpustate, UINT32 a)	{ return RM8(cpustate,a) | (RM8( cpustate, (a+1) & 0xffff ) << 8); }

INLINE void WM8 (t90_Regs *cpustate, UINT32 a, UINT8  v)	{ cpustate->program->write_byte( a, v ); }
INLINE void WM16(t90_Regs *cpustate, UINT32 a, UINT16 v)	{ WM8(cpustate,a,v);	WM8( cpustate, (a+1) & 0xffff, v >> 8); }

INLINE UINT8  RX8 (t90_Regs *cpustate, UINT32 a, UINT32 base)	{ return cpustate->program->read_byte( base | a ); }
INLINE UINT16 RX16(t90_Regs *cpustate, UINT32 a, UINT32 base)	{ return RX8(cpustate,a,base) | (RX8( cpustate, (a+1) & 0xffff, base ) << 8); }

INLINE void WX8 (t90_Regs *cpustate, UINT32 a, UINT8  v, UINT32 base)	{ cpustate->program->write_byte( base | a, v ); }
INLINE void WX16(t90_Regs *cpustate, UINT32 a, UINT16 v, UINT32 base)	{ WX8(cpustate,a,v,base);	WX8( cpustate, (a+1) & 0xffff, v >> 8, base); }

INLINE UINT8  READ8(t90_Regs *cpustate)	{ UINT8 b0 = RM8( cpustate, cpustate->addr++ ); cpustate->addr &= 0xffff; return b0; }
INLINE UINT16 READ16(t90_Regs *cpustate)	{ UINT8 b0 = READ8(cpustate); return b0 | (READ8(cpustate) << 8); }

static void decode(t90_Regs *cpustate)
{
	UINT8  b0, b1, b2, b3;
	UINT16 imm16;

	b0 = READ8(cpustate);

	switch ( b0 )
	{
		case 0x00:
			OP( NOP,2 )			NONE( 1 )					NONE( 2 )						return;		// NOP

		case 0x01:
			OP( HALT,4 )		NONE( 1 )					NONE( 2 )						return;		// HALT
		case 0x02:
			OP( DI,2 )			NONE( 1 )					NONE( 2 )						return;		// DI
		case 0x03:
			OP( EI,2 )			NONE( 1 )					NONE( 2 )						return;		// EI

		case 0x07:
			OPCC( INCX,6,10 )	MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// INCX ($FF00+n)

		case 0x08:
			OP( EX,2 )			R16( 1, DE )				R16( 2, HL )					return;		// EX DE,HL
		case 0x09:
			OP( EX,2 )			R16( 1, AF )				R16( 2, AF2 )					return;		// EX AF,AF'
		case 0x0a:
			OP( EXX,2 )			NONE( 1 )					NONE( 2 )						return;		// EXX

		case 0x0b:
			OP( DAA,4 )			R8( 1, A )					NONE( 2 )						return;		// DAA A

		case 0x0c:
			OP( RCF,2 )			NONE( 1 )					NONE( 2 )						return;		// RCF
		case 0x0d:
			OP( SCF,2 )			NONE( 1 )					NONE( 2 )						return;		// SCF
		case 0x0e:
			OP( CCF,2 )			NONE( 1 )					NONE( 2 )						return;		// CCF

		case 0x0f:
			OPCC( DECX,6,10 )	MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// DECX ($FF00+n)

		case 0x10:
			OP( CPL,2 )			R8( 1, A )					NONE( 2 )						return;		// CPL A
		case 0x11:
			OP( NEG,2 )			R8( 1, A )					NONE( 2 )						return;		// NEG A

		case 0x12:																						// MUL HL,n
		case 0x13:																						// DIV HL,n
			OP( MUL+b0-0x12,16)	R16( 1, HL )				I8( 2, READ8(cpustate) )				return;

		case 0x14:	case 0x15:	case 0x16:
			OP16( ADD,6 )		R16( 1, IX+b0-0x14 )		I16( 2, READ16(cpustate) )				return;		// ADD ix,mn

		case 0x17:
			OP( LDAR,8 )		R16( 1, HL )				D16( 2, READ16(cpustate) )				return;		// LDAR HL,+cd

		case 0x18:
			OP( DJNZ,10 )		D8( 1, READ8(cpustate) )			NONE( 2 )						return;		// DJNZ +d
		case 0x19:
			OP16( DJNZ,10 )		R16( 1, BC )				D8( 2, READ8(cpustate) )				return;		// DJNZ BC,+d

		case 0x1a:
			OPCC( JP,8,8 )		CC( 1, T )					I16( 2, READ16(cpustate) )				return;		// JP T,mn
		case 0x1b:
			OPCC16( JR,10,10 )	CC( 1, T )					D16( 2, READ16(cpustate) )				return;		// JR T,+cd

		case 0x1c:
			OPCC( CALL,14,14 )	CC( 1, T )					I16( 2, READ16(cpustate) )				return;		// CALL T,mn
		case 0x1d:
			OP( CALLR,16 )		D16( 1, READ16(cpustate) )			NONE( 2 )						return;		// CALLR +cd

		case 0x1e:
			OPCC( RET,10,10 )	CC( 1, T )					NONE( 2 )						return;		// RET T
		case 0x1f:
			OP( RETI,14 )		NONE( 1 )					NONE( 2 )						return;		// RETI

		case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
			OP( LD,2 )			R8( 1, A )					R8( 2, b0 - 0x20 )				return;		// LD A,r

		case 0x27:
			OP( LD,8 )			R8( 1, A )					MI16( 2, 0xFF00|READ8(cpustate) )		return;		// LD A,($FF00+n)

		case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
			OP( LD,2 )			R8( 1, b0 - 0x28 )			R8( 2, A )						return;		// LD r,A

		case 0x2f:
			OP( LD,8 )			MI16( 1, 0xFF00|READ8(cpustate) )	R8( 2, A )						return;		// LD ($FF00+n), A

		case 0x30:	case 0x31:	case 0x32:	case 0x33:	case 0x34:	case 0x35:	case 0x36:
			OP( LD,4 )			R8( 1, b0 - 0x30 )			I8( 2, READ8(cpustate) )				return;		// LD r,n

		case 0x37:
			OP( LD,10 )			MI16( 1, 0xFF00|READ8(cpustate) )	I8( 2, READ8(cpustate) )				return;		// LD ($FF00+w),n

		case 0x38:	case 0x39:	case 0x3a:	/*case 0x3b:*/	case 0x3c:	case 0x3d:	case 0x3e:
			OP16( LD,6 )		R16( 1, b0 - 0x38 )			I16( 2, READ16(cpustate) )				return;		// LD rr,nn

		case 0x3f:
			OP( LDW,14 )		MI16( 1, 0xFF00|READ8(cpustate) )	I16( 2, READ16(cpustate) )				return;		// LDW ($FF00+w),mn

		case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
			OP16( LD,4 )		R16( 1, HL )				R16( 2, b0 - 0x40 )				return;		// LD HL,rr

		case 0x47:
			OP16( LD,10 )		R16( 1, HL )				MI16( 2, 0xFF00|READ8(cpustate) )		return;		// LD HL,($FF00+n)

		case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
			OP16( LD,4 )		R16( 1, b0 - 0x48 )			R16( 2, HL )					return;		// LD rr,HL

		case 0x4f:
			OP16( LD,10 )		MI16( 1, 0xFF00|READ8(cpustate) )	R16( 2, HL )					return;		// LD ($FF00+n), HL

		case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
			OP( PUSH,8 )		Q16( 1, b0 - 0x50 )			NONE( 2 )						return;		// PUSH qq
		case 0x58:	case 0x59:	case 0x5a:	/*case 0x5b:*/	case 0x5c:	case 0x5d:	case 0x5e:
			OP( POP,10 )		Q16( 1, b0 - 0x58 )			NONE( 2 )						return;		// POP qq

		case 0x60:																						// ADD A,($FF00+n)
		case 0x61:																						// ADC A,($FF00+n)
		case 0x62:																						// SUB A,($FF00+n)
		case 0x63:																						// SBC A,($FF00+n)
		case 0x64:																						// AND A,($FF00+n)
		case 0x65:																						// XOR A,($FF00+n)
		case 0x66:																						// OR  A,($FF00+n)
		case 0x67:																						// CP  A,($FF00+n)
			OP( ADD+b0-0x60,8 )	R8( 1, A )					MI16( 2, 0xFF00|READ8(cpustate) )		return;

		case 0x68:																						// ADD A,n
		case 0x69:																						// ADC A,n
		case 0x6a:																						// SUB A,n
		case 0x6b:																						// SBC A,n
		case 0x6c:																						// AND A,n
		case 0x6d:																						// XOR A,n
		case 0x6e:																						// OR  A,n
		case 0x6f:																						// CP  A,n
			OP( ADD+b0-0x68,4 )	R8( 1, A )					I8( 2, READ8(cpustate) )				return;

		case 0x70:																						// ADD HL,($FF00+n)
		case 0x71:																						// ADC HL,($FF00+n)
		case 0x72:																						// SUB HL,($FF00+n)
		case 0x73:																						// SBC HL,($FF00+n)
		case 0x74:																						// AND HL,($FF00+n)
		case 0x75:																						// XOR HL,($FF00+n)
		case 0x76:																						// OR  HL,($FF00+n)
		case 0x77:																						// CP  HL,($FF00+n)
			OP16( ADD+b0-0x70,10 )	R16( 1, HL )			MI16( 2, 0xFF00|READ8(cpustate) )		return;

		case 0x78:																						// ADD HL,mn
		case 0x79:																						// ADC HL,mn
		case 0x7a:																						// SUB HL,mn
		case 0x7b:																						// SBC HL,mn
		case 0x7c:																						// AND HL,mn
		case 0x7d:																						// XOR HL,mn
		case 0x7e:																						// OR  HL,mn
		case 0x7f:																						// CP  HL,mn
			OP16( ADD+b0-0x78,6 )	R16( 1, HL )			I16( 2, READ16(cpustate) )				return;

		case 0x80:	case 0x81:	case 0x82:	case 0x83:	case 0x84:	case 0x85:	case 0x86:
			OP( INC,2 )			R8( 1, b0 - 0x80 )			NONE( 2 )						return;		// INC r
		case 0x87:
			OP( INC,10 )		MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// INC ($FF00+n)

		case 0x88:	case 0x89:	case 0x8a:	case 0x8b:	case 0x8c:	case 0x8d:	case 0x8e:
			OP( DEC,2 )			R8( 1, b0 - 0x88 )			NONE( 2 )						return;		// DEC r
		case 0x8f:
			OP( DEC,10 )		MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// DEC ($FF00+n)

		case 0x90:	case 0x91:	case 0x92:	/*case 0x93:*/	case 0x94:	case 0x95:	case 0x96:
			OP16( INC,4 )		R16( 1, b0 - 0x90 )			NONE( 2 )						return;		// INC rr
		case 0x97:
			OP( INCW,14 )		MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// INCW ($FF00+n)
		case 0x98:	case 0x99:	case 0x9a:	/*case 0x9b:*/	case 0x9c:	case 0x9d:	case 0x9e:
			OP16( DEC,4 )		R16( 1, b0 - 0x98 )			NONE( 2 )						return;		// DEC rr
		case 0x9f:
			OP( DECW,14 )		MI16( 1, 0xFF00|READ8(cpustate) )	NONE( 2 )						return;		// DECW ($FF00+n)

		case 0xa0:																						// RLC A
		case 0xa1:																						// RRC A
		case 0xa2:																						// RL  A
		case 0xa3:																						// RR  A
		case 0xa4:																						// SLA A
		case 0xa5:																						// SRA A
		case 0xa6:																						// SLL A
		case 0xa7:																						// SRL A
			OP( RLC+b0-0xa0,2 )	R8( 1, A )					NONE( 2 )						return;

		case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
			OP( BIT,8 )			BIT8( 1, b0 - 0xa8 )		MI16( 2, 0xFF00|READ8(cpustate) )		return;		// BIT b,($FF00+n)
		case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
			OP( RES,12 )		BIT8( 1, b0 - 0xb0 )		MI16( 2, 0xFF00|READ8(cpustate) )		return;		// RES b,($FF00+n)
		case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
			OP( SET,12 )		BIT8( 1, b0 - 0xb8 )		MI16( 2, 0xFF00|READ8(cpustate) )		return;		// SET b,($FF00+n)

		case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:	case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
		case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:	case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
			OPCC( JR,4,8 )		CC( 1, b0 - 0xc0 )			D8( 2, READ8(cpustate) )				return;		// JR cc,+d

		case 0xe0:	case 0xe1:	case 0xe2:	/*case 0xe3:*/	case 0xe4:	case 0xe5:	case 0xe6:
			b1 = READ8(cpustate);
			switch ( b1 )	{
				case 0x10:																				// RLD (gg)
				case 0x11:																				// RRD (gg)
					OP( RLD+b1-0x10,12 )	MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;

				case 0x12:																				// MUL HL,(gg)
				case 0x13:																				// DIV HL,(gg)
					OP( MUL+b1-0x12,18 )	R16( 1, HL )			MR16( 2, b0 - 0xe0 )	return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,8 )			R16( 1, IX+b1-0x14 )	MR16( 2, b0 - 0xe0 )	return;		// ADD ix,(gg)

				case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
					OP( LD,6 )				R8( 1, b1 - 0x28 )		MR16( 2, b0 - 0xe0 )	return;		// LD r,(gg)
				case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
					OP16( LD,8 )			R16( 1, b1 - 0x48 )		MR16( 2, b0 - 0xe0 )	return;		// LD rr,(gg)

				case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
					OP( EX,14 )				MR16( 1, b0 - 0xe0 )	R16( 2, b1 - 0x50 )		return;		// EX (gg),rr

				case 0x60:																				// ADD A,(gg)
				case 0x61:																				// ADC A,(gg)
				case 0x62:																				// SUB A,(gg)
				case 0x63:																				// SBC A,(gg)
				case 0x64:																				// AND A,(gg)
				case 0x65:																				// XOR A,(gg)
				case 0x66:																				// OR  A,(gg)
				case 0x67:																				// CP  A,(gg)
					OP( ADD+b1-0x60,6 )		R8( 1, A )				MR16( 2, b0 - 0xe0 )	return;

				case 0x70:																				// ADD HL,(gg)
				case 0x71:																				// ADC HL,(gg)
				case 0x72:																				// SUB HL,(gg)
				case 0x73:																				// SBC HL,(gg)
				case 0x74:																				// AND HL,(gg)
				case 0x75:																				// XOR HL,(gg)
				case 0x76:																				// OR  HL,(gg)
				case 0x77:																				// CP  HL,(gg)
					OP16( ADD+b1-0x70,8 )	R16( 1, HL )			MR16( 2, b0 - 0xe0 )	return;

				case 0x87:
					OP( INC,8 )				MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;		// INC (gg)
				case 0x8f:
					OP( DEC,8 )				MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;		// DEC (gg)

				case 0x97:
					OP( INCW,12 )			MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;		// INCW (gg)
				case 0x9f:
					OP( DECW,12 )			MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;		// DECW (gg)

				case 0xa0:																				// RLC (gg)
				case 0xa1:																				// RRC (gg)
				case 0xa2:																				// RL  (gg)
				case 0xa3:																				// RR  (gg)
				case 0xa4:																				// SLA (gg)
				case 0xa5:																				// SRA (gg)
				case 0xa6:																				// SLL (gg)
				case 0xa7:																				// SRL (gg)
					OP( RLC+b1-0xa0,8 )		MR16( 1, b0 - 0xe0 )	NONE( 2 )				return;

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,12 )			BIT8( 1, b1 - 0x18 )	MR16( 2, b0 - 0xe0 )	return;		// TSET b,(gg)
				case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
					OP( BIT,6 )				BIT8( 1, b1 - 0xa8 )	MR16( 2, b0 - 0xe0 )	return;		// BIT b,(gg)
				case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
					OP( RES,10 )			BIT8( 1, b1 - 0xb0 )	MR16( 2, b0 - 0xe0 )	return;		// RES b,(gg)
				case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					OP( SET,10 )			BIT8( 1, b1 - 0xb8 )	MR16( 2, b0 - 0xe0 )	return;		// SET b,(gg)
			}	break;
		case 0xe3:
			imm16 = READ16(cpustate);
			b3 = READ8(cpustate);
			switch ( b3 )	{
				case 0x10:																				// RLD (mn)
				case 0x11:																				// RRD (mn)
					OP( RLD+b3-0x10,16 )	MI16( 1, imm16 )		NONE( 2 )				return;

				case 0x12:																				// MUL HL,(mn)
				case 0x13:																				// DIV HL,(mn)
					OP( MUL+b3-0x12,22 )	R16( 1, HL )			MI16( 2, imm16 )		return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,12 )			R16( 1, IX+b3-0x14 )	MI16( 2, imm16 )		return;		// ADD ix,(mn)

				case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
					OP( LD,10 )				R8( 1, b3 - 0x28 )		MI16( 2, imm16 )		return;		// LD r,(mn)
				case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
					OP16( LD,12 )			R16( 1, b3 - 0x48 )		MI16( 2, imm16 )		return;		// LD rr,(mn)

				case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
					OP( EX,18 )				MI16( 1, imm16 )		R16( 2, b3 - 0x50 )		return;		// EX (mn),rr

				case 0x60:																				// ADD A,(mn)
				case 0x61:																				// ADC A,(mn)
				case 0x62:																				// SUB A,(mn)
				case 0x63:																				// SBC A,(mn)
				case 0x64:																				// AND A,(mn)
				case 0x65:																				// XOR A,(mn)
				case 0x66:																				// OR  A,(mn)
				case 0x67:																				// CP  A,(mn)
					OP( ADD+b3-0x60,10 )	R8( 1, A )				MI16( 2, imm16 )		return;

				case 0x70:																				// ADD HL,(mn)
				case 0x71:																				// ADC HL,(mn)
				case 0x72:																				// SUB HL,(mn)
				case 0x73:																				// SBC HL,(mn)
				case 0x74:																				// AND HL,(mn)
				case 0x75:																				// XOR HL,(mn)
				case 0x76:																				// OR  HL,(mn)
				case 0x77:																				// CP  HL,(mn)
					OP16( ADD+b3-0x70,12 )	R16( 1, HL )			MI16( 2, imm16 )		return;

				case 0x87:
					OP( INC,12 )			MI16( 1, imm16 )		NONE( 2 )				return;		// INC (mn)
				case 0x8f:
					OP( DEC,12 )			MI16( 1, imm16 )		NONE( 2 )				return;		// DEC (mn)

				case 0x97:
					OP( INCW,16 )			MI16( 1, imm16 )		NONE( 2 )				return;		// INCW (mn)
				case 0x9f:
					OP( DECW,16 )			MI16( 1, imm16 )		NONE( 2 )				return;		// DECW (mn)

				case 0xa0:																				// RLC (mn)
				case 0xa1:																				// RRC (mn)
				case 0xa2:																				// RL  (mn)
				case 0xa3:																				// RR  (mn)
				case 0xa4:																				// SLA (mn)
				case 0xa5:																				// SRA (mn)
				case 0xa6:																				// SLL (mn)
				case 0xa7:																				// SRL (mn)
					OP( RLC+b3-0xa0,12 )	MI16( 1, imm16 )		NONE( 2 )				return;

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,16 )			BIT8( 1, b3 - 0x18 )	MI16( 2, imm16 )		return;		// TSET b,(mn)
				case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
					OP( BIT,10 )			BIT8( 1, b3 - 0xa8 )	MI16( 2, imm16 )		return;		// BIT b,(mn)
				case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
					OP( RES,14 )			BIT8( 1, b3 - 0xb0 )	MI16( 2, imm16 )		return;		// RES b,(mn)
				case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					OP( SET,14 )			BIT8( 1, b3 - 0xb8 )	MI16( 2, imm16 )		return;		// SET b,(mn)
			}	break;

		case 0xe7:
			b1 = READ8(cpustate);
			b2 = READ8(cpustate);
			switch ( b2 )	{
				case 0x10:																				// RLD ($FF00+n)
				case 0x11:																				// RRD ($FF00+n)
					OP( RLD+b2-0x10,14 )	MI16( 1, 0xFF00|b1 )	NONE( 2 )				return;

				case 0x12:																				// MUL HL,($FF00+n)
				case 0x13:																				// DIV HL,($FF00+n)
					OP( MUL+b2-0x12,20 )	R16( 1, HL )			MI16( 2, 0xFF00|b1 )	return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,10 )			R16( 1, IX+b2-0x14 )	MI16( 2, 0xFF00|b1 )	return;		// ADD ix,($FF00+n)

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,14 )			BIT8( 1, b2 - 0x18 )	MI16( 2, 0xFF00|b1 )	return;		// TSET b,($FF00+n)
				case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
					OP( LD,8 )				R8( 1, b2 - 0x28 )		MI16( 2, 0xFF00|b1 )	return;		// LD r,($FF00+n)
				case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
					OP16( LD,10 )			R16( 1, b2 - 0x48 )		MI16( 2, 0xFF00|b1 )	return;		// LD rr,($FF00+n)

				case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
					OP( EX,16 )				MI16( 1, 0xFF00|b1 )	R16( 2, b2 - 0x50 )		return;		// EX ($FF00+n),rr

				case 0xa0:																				// RLC ($FF00+n)
				case 0xa1:																				// RRC ($FF00+n)
				case 0xa2:																				// RL  ($FF00+n)
				case 0xa3:																				// RR  ($FF00+n)
				case 0xa4:																				// SLA ($FF00+n)
				case 0xa5:																				// SRA ($FF00+n)
				case 0xa6:																				// SLL ($FF00+n)
				case 0xa7:																				// SRL ($FF00+n)
					OP( RLC+b2-0xa0,10 )	MI16( 1, 0xFF00|b1 )	NONE( 2 )				return;
			}	break;

		case 0xe8:	case 0xe9:	case 0xea:	/*case 0xeb:*/	case 0xec:	case 0xed:	case 0xee:
			b1 = READ8(cpustate);
			switch ( b1 )	{
				case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
					OP( LD,6 )				MR16( 1, b0 - 0xe8 )	R8( 2, b1 - 0x20 )		return;		// LD (gg),r
				case 0x37:
					OP( LD,8 )				MR16( 1, b0 - 0xe8 )	I8( 2, READ8(cpustate) )		return;		// LD (gg),n
				case 0x3f:
					OP( LDW,12 )			MR16( 1, b0 - 0xe8 )	I16( 2, READ16(cpustate) )		return;		// LDW (gg),mn
				case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
					OP16( LD,8 )			MR16( 1, b0 - 0xe8 )	R16( 2, b1 - 0x40 )		return;		// LD (gg),rr

				case 0x68:																				// ADD (gg),n
				case 0x69:																				// ADC (gg),n
				case 0x6a:																				// SUB (gg),n
				case 0x6b:																				// SBC (gg),n
				case 0x6c:																				// AND (gg),n
				case 0x6d:																				// XOR (gg),n
				case 0x6e:																				// OR  (gg),n
					OP( ADD+b1-0x68,10 )	MR16( 1, b0 - 0xe8 )	I8( 2, READ8(cpustate) )		return;
				case 0x6f:																				// CP  (gg),n
					OP( CP,8 )				MR16( 1, b0 - 0xe8 )	I8( 2, READ8(cpustate) )		return;

				case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:	case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
				case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:	case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
					OPCC( JP,6,8 )			CC( 1, b1 - 0xc0 )		R16( 2, b0 - 0xe8 )		return;		// JP [cc,]gg
				case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
				case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:	case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
					OPCC( CALL,6,14 )		CC( 1, b1 - 0xd0 )		R16( 2, b0 - 0xe8 )		return;		// CALL [cc,]gg
			}	break;
		case 0xeb:
			imm16 = READ16(cpustate);
			b3 = READ8(cpustate);
			switch ( b3 )	{
				case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
					OP( LD,10 )				MI16( 1, imm16 )		R8( 2, b3 - 0x20 )		return;		// LD (mn),r
				case 0x37:
					OP( LD,12 )				MI16( 1, imm16 )		I8( 2, READ8(cpustate) )		return;		// LD (vw),n
				case 0x3f:
					OP( LDW,16 )			MI16( 1, imm16 )		I16( 2, READ16(cpustate) )		return;		// LDW (vw),mn
				case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
					OP16( LD,12 )			MI16( 1, imm16 )		R16( 2, b3 - 0x40 )		return;		// LD (mn),rr

				case 0x68:																				// ADD (vw),n
				case 0x69:																				// ADC (vw),n
				case 0x6a:																				// SUB (vw),n
				case 0x6b:																				// SBC (vw),n
				case 0x6c:																				// AND (vw),n
				case 0x6d:																				// XOR (vw),n
				case 0x6e:																				// OR  (vw),n
					OP( ADD+b3-0x68,14 )	MI16( 1, imm16 )		I8( 2, READ8(cpustate) )		return;
				case 0x6f:																				// CP  (vw),n
					OP( ADD+b3-0x68,12 )	MI16( 1, imm16 )		I8( 2, READ8(cpustate) )		return;

				case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:	case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
				case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:	case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
					OPCC( JP,10,12 )		CC( 1, b3 - 0xc0 )		I16( 2, imm16 )			return;		// JP cc,mn
				case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
				case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:	case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
					OPCC( CALL,10,18 )		CC( 1, b3 - 0xd0 )		I16( 2, imm16 )			return;		// CALL cc,mn
			}	break;

		case 0xef:
			b1 = READ8(cpustate);
			b2 = READ8(cpustate);
			switch ( b2 )	{
				case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
					OP( LD,8 )				MI16( 1, 0xFF00|b1 )	R8( 2, b2 - 0x20 )		return;		// LD ($FF00+n),r
				case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
					OP16( LD,10 )			MI16( 1, 0xFF00|b1 )	R16( 2, b2 - 0x40 )		return;		// LD ($FF00+n),rr

				case 0x68:																				// ADD ($FF00+w),n
				case 0x69:																				// ADC ($FF00+w),n
				case 0x6a:																				// SUB ($FF00+w),n
				case 0x6b:																				// SBC ($FF00+w),n
				case 0x6c:																				// AND ($FF00+w),n
				case 0x6d:																				// XOR ($FF00+w),n
				case 0x6e:																				// OR  ($FF00+w),n
					OP( ADD+b2-0x68,12 )	MI16( 1, 0xFF00|b1 )	I8( 2, READ8(cpustate) )		return;
				case 0x6f:																				// CP  ($FF00+w),n
					OP( ADD+b2-0x68,10 )	MI16( 1, 0xFF00|b1 )	I8( 2, READ8(cpustate) )		return;
			}	break;

		case 0xf0:	case 0xf1:	case 0xf2:
			b1 = READ8(cpustate);
			b2 = READ8(cpustate);
			switch ( b2 )	{
				case 0x10:																				// RLD (ix+d)
				case 0x11:																				// RRD (ix+d)
					OP( RLD+b2-0x10,16 )	MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;

				case 0x12:																				// MUL HL,(ix+d)
				case 0x13:																				// DIV HL,(ix+d)
					OP( MUL+b2-0x12,22 )	R16( 1, HL )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,12 )	R16( 1, IX+b2-0x14 )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;		// ADD ix,(jx+d)

				case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
					OP( LD,10 )		R8( 1, b2 - 0x28 )	MR16D8( 2, IX + b0 - 0xf0, b1 )		return;		// LD r,(ix+d)
				case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
					OP16( LD,12 )	R16( 1, b2 - 0x48 )	MR16D8( 2, IX + b0 - 0xf0, b1 )		return;		// LD rr,(ix+d)

				case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
					OP( EX,18 )		MR16D8( 1, IX + b0 - 0xf0, b1 )	R16( 2, b2 - 0x50 )		return;		// EX (ix+d),rr

				case 0x60:																				// ADD A,(ix+d)
				case 0x61:																				// ADC A,(ix+d)
				case 0x62:																				// SUB A,(ix+d)
				case 0x63:																				// SBC A,(ix+d)
				case 0x64:																				// AND A,(ix+d)
				case 0x65:																				// XOR A,(ix+d)
				case 0x66:																				// OR  A,(ix+d)
				case 0x67:																				// CP  A,(ix+d)
					OP( ADD+b2-0x60,10 )	R8( 1, A )	MR16D8( 2, IX + b0 - 0xf0, b1 )		return;

				case 0x70:																				// ADD HL,(ix+d)
				case 0x71:																				// ADC HL,(ix+d)
				case 0x72:																				// SUB HL,(ix+d)
				case 0x73:																				// SBC HL,(ix+d)
				case 0x74:																				// AND HL,(ix+d)
				case 0x75:																				// XOR HL,(ix+d)
				case 0x76:																				// OR  HL,(ix+d)
				case 0x77:																				// CP  HL,(ix+d)
					OP16( ADD+b2-0x70,12 )	R16( 1, HL )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;

				case 0x87:
					OP( INC,12 )			MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;		// INC (ix+d)
				case 0x8f:
					OP( DEC,12 )			MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;		// DEC (ix+d)

				case 0x97:
					OP( INCW,16 )			MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;		// INCW (ix+d)
				case 0x9f:
					OP( DECW,16 )			MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;		// DECW (ix+d)

				case 0xa0:																				// RLC (ix+d)
				case 0xa1:																				// RRC (ix+d)
				case 0xa2:																				// RL  (ix+d)
				case 0xa3:																				// RR  (ix+d)
				case 0xa4:																				// SLA (ix+d)
				case 0xa5:																				// SRA (ix+d)
				case 0xa6:																				// SLL (ix+d)
				case 0xa7:																				// SRL (ix+d)
					OP( RLC+b2-0xa0,12 )	MR16D8( 1, IX + b0 - 0xf0, b1 )		NONE( 2 )	return;

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,16 )	BIT8( 1, b2 - 0x18 )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;		// TSET b,(ix+d)
				case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
					OP( BIT,10 )	BIT8( 1, b2 - 0xa8 )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;		// BIT b,(ix+d)
				case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
					OP( RES,14 )	BIT8( 1, b2 - 0xb0 )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;		// RES b,(ix+d)
				case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					OP( SET,14 )	BIT8( 1, b2 - 0xb8 )	MR16D8( 2, IX + b0 - 0xf0, b1 )	return;		// SET b,(ix+d)
			}	break;

		case 0xf3:
			b1 = READ8(cpustate);
			switch ( b1 )	{
				case 0x10:																				// RLD (HL+A)
				case 0x11:																				// RRD (HL+A)
					OP( RLD+b1-0x10,20 )	MR16R8( 1, HL, A )			NONE( 2 )			return;

				case 0x12:																				// MUL HL,(HL+A)
				case 0x13:																				// DIV HL,(HL+A)
					OP( MUL+b1-0x12,26 )	R16( 1, HL )				MR16R8( 2, HL, A )	return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,16 )			R16( 1, IX+b1-0x14 )		MR16R8( 2, HL, A )	return;		// ADD ix,(HL+A)

				case 0x28:	case 0x29:	case 0x2a:	case 0x2b:	case 0x2c:	case 0x2d:	case 0x2e:
					OP( LD,14 )				R8( 1, b1 - 0x28 )			MR16R8( 2, HL, A )	return;		// LD r,(HL+A)
				case 0x48:	case 0x49:	case 0x4a:	/*case 0x4b:*/	case 0x4c:	case 0x4d:	case 0x4e:
					OP16( LD,16 )			R16( 1, b1 - 0x48 )			MR16R8( 2, HL, A )	return;		// LD rr,(HL+A)

				case 0x50:	case 0x51:	case 0x52:	/*case 0x53:*/	case 0x54:	case 0x55:	case 0x56:
					OP( EX,22 )				MR16R8( 1, HL, A )			R16( 2, b1 - 0x50 )	return;		// EX (HL+A),rr

				case 0x60:																				// ADD A,(HL+A)
				case 0x61:																				// ADC A,(HL+A)
				case 0x62:																				// SUB A,(HL+A)
				case 0x63:																				// SBC A,(HL+A)
				case 0x64:																				// AND A,(HL+A)
				case 0x65:																				// XOR A,(HL+A)
				case 0x66:																				// OR  A,(HL+A)
				case 0x67:																				// CP  A,(HL+A)
					OP( ADD+b1-0x60,14 )	R8( 1, A )					MR16R8( 2, HL, A )	return;

				case 0x70:																				// ADD HL,(HL+A)
				case 0x71:																				// ADC HL,(HL+A)
				case 0x72:																				// SUB HL,(HL+A)
				case 0x73:																				// SBC HL,(HL+A)
				case 0x74:																				// AND HL,(HL+A)
				case 0x75:																				// XOR HL,(HL+A)
				case 0x76:																				// OR  HL,(HL+A)
				case 0x77:																				// CP  HL,(HL+A)
					OP16( ADD+b1-0x70,16 )	R16( 1, HL )				MR16R8( 2, HL, A )	return;

				case 0x87:
					OP( INC,16 )			MR16R8( 1, HL, A )			NONE( 2 )			return;		// INC (HL+A)
				case 0x8f:
					OP( DEC,16 )			MR16R8( 1, HL, A )			NONE( 2 )			return;		// DEC (HL+A)

				case 0x97:
					OP( INCW,20 )			MR16R8( 1, HL, A )			NONE( 2 )			return;		// INCW (HL+A)
				case 0x9f:
					OP( DECW,20 )			MR16R8( 1, HL, A )			NONE( 2 )			return;		// DECW (HL+A)

				case 0xa0:																				// RLC (HL+A)
				case 0xa1:																				// RRC (HL+A)
				case 0xa2:																				// RL  (HL+A)
				case 0xa3:																				// RR  (HL+A)
				case 0xa4:																				// SLA (HL+A)
				case 0xa5:																				// SRA (HL+A)
				case 0xa6:																				// SLL (HL+A)
				case 0xa7:																				// SRL (HL+A)
					OP( RLC+b1-0xa0,16 )	MR16R8( 1, HL, A )			NONE( 2 )			return;

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,20 )			BIT8( 1, b1 - 0x18 )		MR16R8( 2, HL, A )	return;		// TSET b,(HL+A)
				case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
					OP( BIT,14 )			BIT8( 1, b1 - 0xa8 )		MR16R8( 2, HL, A )	return;		// BIT b,(HL+A)
				case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
					OP( RES,18 )			BIT8( 1, b1 - 0xb0 )		MR16R8( 2, HL, A )	return;		// RES b,(HL+A)
				case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					OP( SET,18 )			BIT8( 1, b1 - 0xb8 )		MR16R8( 2, HL, A )	return;		// SET b,(HL+A)
			}	break;

		case 0xf4:	case 0xf5:	case 0xf6:
			b1 = READ8(cpustate);
			b2 = READ8(cpustate);
			switch ( b2 )	{
				case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
					OP( LD,10 )		MR16D8( 1, IX + b0 - 0xf4, b1 )	R8( 2, b2 - 0x20 )		return;		// LD (ix+d),r
				case 0x37:
					OP( LD,12 )		MR16D8( 1, IX + b0 - 0xf4, b1 )	I8( 2, READ8(cpustate) )		return;		// LD (ix+d),n
				case 0x38:	case 0x39:	case 0x3a:	/*case 0x3b:*/	case 0x3c:	case 0x3d:	case 0x3e:
					OP( LDA,10 )	R16( 1, b2 - 0x38 )		R16D8( 2, IX + b0 - 0xf4, b1 )	return;		// LDA rr,ix+d
				case 0x3f:
					OP( LDW,16 )	MR16D8( 1, IX + b0 - 0xf4, b1 )	I16( 2, READ16(cpustate) )		return;		// LDW (ix+d),mn
				case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
					OP16( LD,12 )	MR16D8( 1, IX + b0 - 0xf4, b1 )	R16( 2, b2 - 0x40 )		return;		// LD (ix+d),rr

				case 0x68:																				// ADD (ix+d),n
				case 0x69:																				// ADC (ix+d),n
				case 0x6a:																				// SUB (ix+d),n
				case 0x6b:																				// SBC (ix+d),n
				case 0x6c:																				// AND (ix+d),n
				case 0x6d:																				// XOR (ix+d),n
				case 0x6e:																				// OR  (ix+d),n
					OP( ADD+b2-0x68,14)	MR16D8( 1, IX + b0 - 0xf4, b1 )	I8( 2, READ8(cpustate) )	return;
				case 0x6f:																				// CP  (ix+d),n
					OP( ADD+b2-0x68,12)	MR16D8( 1, IX + b0 - 0xf4, b1 )	I8( 2, READ8(cpustate) )	return;

				case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:	case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
				case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:	case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
					OPCC( JP,10,12 )	CC( 1, b2 - 0xc0 )	R16D8( 2, IX + b0 - 0xf4, b1 )	return;		// JP [cc,]ix+d
				case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
				case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:	case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
					OPCC( CALL,10,18 )	CC( 1, b2 - 0xd0 )	R16D8( 2, IX + b0 - 0xf4, b1 )	return;		// CALL [cc,]ix+d
			}	break;

		case 0xf7:
			b1 = READ8(cpustate);
			switch ( b1 )	{
				case 0x20:	case 0x21:	case 0x22:	case 0x23:	case 0x24:	case 0x25:	case 0x26:
					OP( LD,14 )			MR16R8( 1, HL, A )		R8( 2, b1 - 0x20 )			return;		// LD (HL+A),r
				case 0x37:
					OP( LD,16 )			MR16R8( 1, HL, A )		I8( 2, READ8(cpustate) )			return;		// LD (HL+A),n
				case 0x38:	case 0x39:	case 0x3a:	/*case 0x3b:*/	case 0x3c:	case 0x3d:	case 0x3e:
					OP( LDA,14 )		R16( 1, b1 - 0x38 )		R16R8( 2, HL, A )			return;		// LDA rr,HL+A
				case 0x3f:
					OP( LDW,20 )		MR16R8( 1, HL, A )		I16( 2, READ16(cpustate) )			return;		// LDW (HL+A),mn
				case 0x40:	case 0x41:	case 0x42:	/*case 0x43:*/	case 0x44:	case 0x45:	case 0x46:
					OP16( LD,16 )		MR16R8( 1, HL, A )		R16( 2, b1 - 0x40 )			return;		// LD (HL+A),rr

				case 0x68:																				// ADD (HL+A),n
				case 0x69:																				// ADC (HL+A),n
				case 0x6a:																				// SUB (HL+A),n
				case 0x6b:																				// SBC (HL+A),n
				case 0x6c:																				// AND (HL+A),n
				case 0x6d:																				// XOR (HL+A),n
				case 0x6e:																				// OR  (HL+A),n
					OP( ADD+b1-0x68,18)	MR16R8( 1, HL, A )		I8( 2, READ8(cpustate) )			return;
				case 0x6f:																				// CP  (HL+A),n
					OP( ADD+b1-0x68,16)	MR16R8( 1, HL, A )		I8( 2, READ8(cpustate) )			return;

				case 0xc0:	case 0xc1:	case 0xc2:	case 0xc3:	case 0xc4:	case 0xc5:	case 0xc6:	case 0xc7:
				case 0xc8:	case 0xc9:	case 0xca:	case 0xcb:	case 0xcc:	case 0xcd:	case 0xce:	case 0xcf:
					OPCC( JP,14,16 )	CC( 1, b1 - 0xc0 )		R16R8( 2, HL, A )			return;		// JP [cc,]HL+A
				case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
				case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:	case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
					OPCC( CALL,14,22 )	CC( 1, b1 - 0xd0 )		R16R8( 2, HL, A )			return;		// CALL [cc,]HL+A
			}	break;

		case 0xf8:	case 0xf9:	case 0xfa:	case 0xfb:	case 0xfc:	case 0xfd:	case 0xfe:
			b1 = READ8(cpustate);
			switch ( b1 )	{
				case 0x12:																				// MUL HL,g
				case 0x13:																				// DIV HL,g
					OP( MUL+b1-0x12,18)	R16( 1, HL )			R8( 2, b0 - 0xf8 )			return;

				case 0x14:	case 0x15:	case 0x16:
					OP16( ADD,8 )		R16( 1, IX+b1-0x14 )	R16( 2, b0 - 0xf8 )			return;		// ADD ix,gg

				case 0x30:	case 0x31:	case 0x32:	case 0x33:	case 0x34:	case 0x35:	case 0x36:
					OP( LD,4 )			R8( 1, b1 - 0x30 )		R8( 2, b0 - 0xf8 )			return;		// LD r,g
				case 0x38:	case 0x39:	case 0x3a:	/*case 0x3b:*/	case 0x3c:	case 0x3d:	case 0x3e:
					OP16( LD,6 )		R16( 1, b1 - 0x38 )		R16( 2, b0 - 0xf8 )			return;		// LD rr,gg

				case 0x58:																				// LDI
				case 0x59:																				// LDIR
				case 0x5a:																				// LDD
				case 0x5b:																				// LDDR
				case 0x5c:																				// CPI
				case 0x5d:																				// CPIR
				case 0x5e:																				// CPD
				case 0x5f:																				// CPDR
				if (b0 == 0xfe) {
					OPCC( LDI+b1-0x58,14,18 )	NONE( 1 )		NONE( 2 )					return;
				}

				case 0x60:																				// ADD A,g
				case 0x61:																				// ADC A,g
				case 0x62:																				// SUB A,g
				case 0x63:																				// SBC A,g
				case 0x64:																				// AND A,g
				case 0x65:																				// XOR A,g
				case 0x66:																				// OR  A,g
				case 0x67:																				// CP  A,g
					OP( ADD+b1-0x60,4 )	R8( 1, A )				R8( 2, b0 - 0xf8 )			return;

				case 0x68:																				// ADD g,n
				case 0x69:																				// ADC g,n
				case 0x6a:																				// SUB g,n
				case 0x6b:																				// SBC g,n
				case 0x6c:																				// AND g,n
				case 0x6d:																				// XOR g,n
				case 0x6e:																				// OR  g,n
				case 0x6f:																				// CP  g,n
					OP( ADD+b1-0x68,6 )	R8( 1, b0 - 0xf8 )		I8( 2, READ8(cpustate) )			return;

				case 0x70:																				// ADD HL,gg
				case 0x71:																				// ADC HL,gg
				case 0x72:																				// SUB HL,gg
				case 0x73:																				// SBC HL,gg
				case 0x74:																				// AND HL,gg
				case 0x75:																				// XOR HL,gg
				case 0x76:																				// OR  HL,gg
				case 0x77:																				// CP  HL,gg
					OP16( ADD+b1-0x70,8 )	R16( 1, HL )		R16( 2, b0 - 0xf8 )			return;

				case 0xa0:																				// RLC g
				case 0xa1:																				// RRC g
				case 0xa2:																				// RL  g
				case 0xa3:																				// RR  g
				case 0xa4:																				// SLA g
				case 0xa5:																				// SRA g
				case 0xa6:																				// SLL g
				case 0xa7:																				// SRL g
					OP( RLC+b1-0xa0,4 )	R8( 1, b0 - 0xf8 )		NONE( 2 )					return;

				case 0x18:	case 0x19:	case 0x1a:	case 0x1b:	case 0x1c:	case 0x1d:	case 0x1e:	case 0x1f:
					OP( TSET,8 )		BIT8( 1, b1 - 0x18 )	R8( 2, b0 - 0xf8 )			return;		// TSET b,g
				case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:	case 0xac:	case 0xad:	case 0xae:	case 0xaf:
					OP( BIT,4 )			BIT8( 1, b1 - 0xa8 )	R8( 2, b0 - 0xf8 )			return;		// BIT b,g
				case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:	case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
					OP( RES,4 )			BIT8( 1, b1 - 0xb0 )	R8( 2, b0 - 0xf8 )			return;		// RES b,g
				case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:	case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf:
					OP( SET,4 )			BIT8( 1, b1 - 0xb8 )	R8( 2, b0 - 0xf8 )			return;		// SET b,g

				case 0xd0:	case 0xd1:	case 0xd2:	case 0xd3:	case 0xd4:	case 0xd5:	case 0xd6:	case 0xd7:
				case 0xd8:	case 0xd9:	case 0xda:	case 0xdb:	case 0xdc:	case 0xdd:	case 0xde:	case 0xdf:
				if (b0 == 0xfe) {
					OPCC( RET,6,14 )	CC( 1, b1 - 0xd0 )		NONE( 2 )					return;		// RET cc
				}
			}	break;

		case 0xff:
			OP( SWI,20 )				NONE( 1 )				NONE( 2 )					return;		// SWI
	}

	OP( UNKNOWN,2 )		NONE( 1 )		NONE( 2 )
}

static const char *const ir_names[] =	{
	"P0",		"P1",		"P01CR/IRFL",	"IRFH",		"P2",		"P2CR",		"P3",		"P3CR",
	"P4",		"P4CR",		"P5",			"SMMOD",	"P6",		"P7",		"P67CR",	"SMCR",
	"P8",		"P8CR",		"WDMOD",		"WDCR",		"TREG0",	"TREG1",	"TREG2",	"TREG3",
	"TCLK",		"TFFCR",	"TMOD",			"TRUN",		"CAP1L",	"CAP1H",	"CAP2L",	"CAL2H",
	"TREG4L",	"TREG4H",	"TREG5L",		"TREG5H",	"T4MOD",	"T4FFCR",	"INTEL",	"INTEH",
	"DMAEH",	"SCMOD",	"SCCR",			"SCBUF",	"BX",		"BY",		"ADREG",	"ADMOD"
};

static const char *internal_registers_names(UINT16 x)
{
	int ir = x - T90_IOBASE;
	if ( ir >= 0 && ir < sizeof(ir_names)/sizeof(ir_names[0]) )
		return ir_names[ir];
	return NULL;
}
static int sprint_arg(char *buffer, UINT32 pc, const char *pre, const e_mode mode, const e_r r, const e_r rb)
{
	const char *reg_name;
	switch ( mode )
	{
		case MODE_NONE:		return 0;

		case MODE_BIT8:		return	sprintf( buffer, "%s%d",			pre,	r									);
		case MODE_I8:		return	sprintf( buffer, "%s$%02X",			pre,	r									);
		case MODE_D8:		return	sprintf( buffer, "%s$%04X",			pre,	(pc+2+(r&0x7f)-(r&0x80))&0xffff		);
		case MODE_I16:		return	sprintf( buffer, "%s$%04X",			pre,	r									);
		case MODE_D16:		return	sprintf( buffer, "%s$%04X",			pre,	(pc+2+(r&0x7fff)-(r&0x8000))&0xffff	);
		case MODE_MI16:
			reg_name = internal_registers_names(r);
			return	(reg_name) ?	sprintf( buffer, "%s(%s)",			pre,	reg_name							):
									sprintf( buffer, "%s($%04X)",		pre,	r									);
		case MODE_R8:		return	sprintf( buffer, "%s%s",			pre,	r8_names[r]							);
		case MODE_R16:		return	sprintf( buffer, "%s%s",			pre,	r16_names[r]						);
		case MODE_MR16:		return	sprintf( buffer, "%s(%s)",			pre,	r16_names[r]						);

		case MODE_MR16R8:	return	sprintf( buffer, "%s(%s+%s)",		pre,	r16_names[r],	r8_names[rb]		);
		case MODE_MR16D8:	return	sprintf( buffer, "%s(%s%c$%02X)",	pre,	r16_names[r],	(rb&0x80)?'-':'+',	(rb&0x80)?((rb^0xff)+1):rb	);

		case MODE_CC:		return	sprintf( buffer, "%s%s",			pre,	cc_names[r]							);

		case MODE_R16R8:	return	sprintf( buffer, "%s%s+%s",			pre,	r16_names[r],	r8_names[rb]		);
		case MODE_R16D8:	return	sprintf( buffer, "%s%s%c$%02X",		pre,	r16_names[r],	(rb&0x80)?'-':'+',	(rb&0x80)?((rb^0xff)+1):rb	);

		default:
			fatalerror("%04x: unimplemented addr mode = %d\n",pc,mode);
	}

	return 0;
}

CPU_DISASSEMBLE( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);
	int len;

	cpustate->addr = pc;

	decode(cpustate);
	cpustate->op &= ~OP_16;

	buffer	+=	sprintf		( buffer,			"%-5s",				op_names[ cpustate->op ] );	// strlen("callr") == 5
	len		=	sprint_arg	( buffer, pc,		" ",				cpustate->mode1, cpustate->r1, cpustate->r1b );
	buffer	+=	len;
	buffer	+=	sprint_arg	( buffer, pc,		(len>1)?",":"",		cpustate->mode2, cpustate->r2, cpustate->r2b );

	return (cpustate->addr - pc) | DASMFLAG_SUPPORTED;
}


INLINE UINT16 r8( t90_Regs *cpustate, const e_r r )
{
	switch( r )
	{
		case A:	return cpustate->af.b.h;
		case B:	return cpustate->bc.b.h;
		case C:	return cpustate->bc.b.l;
		case D:	return cpustate->de.b.h;
		case E:	return cpustate->de.b.l;
		case H:	return cpustate->hl.b.h;
		case L:	return cpustate->hl.b.l;

		default:
			fatalerror("%04x: unimplemented r8 register index = %d\n",cpustate->pc.w.l,r);
	}
}

INLINE void w8( t90_Regs *cpustate, const e_r r, UINT16 value )
{
	switch( r )
	{
		case A:	cpustate->af.b.h = value;	return;
		case B:	cpustate->bc.b.h = value;	return;
		case C:	cpustate->bc.b.l = value;	return;
		case D:	cpustate->de.b.h = value;	return;
		case E:	cpustate->de.b.l = value;	return;
		case H:	cpustate->hl.b.h = value;	return;
		case L:	cpustate->hl.b.l = value;	return;

		default:
			fatalerror("%04x: unimplemented w8 register index = %d\n",cpustate->pc.w.l,r);
	}
}

INLINE UINT16 r16( t90_Regs *cpustate, const e_r r )
{
	switch( r )
	{
		case BC:	return cpustate->bc.w.l;
		case DE:	return cpustate->de.w.l;
		case HL:	return cpustate->hl.w.l;
		case IX:	return cpustate->ix.w.l;
		case IY:	return cpustate->iy.w.l;
		case SP:	return cpustate->sp.w.l;
		case AF:	return cpustate->af.w.l;
//      case AF2:   return cpustate->af2.w.l;
// one interrupt flip-flop? Needed by e.g. mjifb
		case AF2:	return (cpustate->af2.w.l & (~IF)) | (cpustate->af.w.l & IF);
		case PC:	return cpustate->pc.w.l;

		default:
			fatalerror("%04x: unimplemented r16 register index = %d\n",cpustate->pc.w.l,r);
	}
}

INLINE void w16( t90_Regs *cpustate, const e_r r, UINT16 value )
{
	switch( r )
	{
		case BC:	cpustate->bc.w.l  = value;	return;
		case DE:	cpustate->de.w.l  = value;	return;
		case HL:	cpustate->hl.w.l  = value;	return;
		case IX:	cpustate->ix.w.l  = value;	return;
		case IY:	cpustate->iy.w.l  = value;	return;
		case SP:	cpustate->sp.w.l  = value;	return;
		case AF:	cpustate->af.w.l  = value;	return;
		case AF2:	cpustate->af2.w.l = value;	return;
		case PC:	cpustate->pc.d = value;	return;

		default:
			fatalerror("%04x: unimplemented w16 register index = %d\n",cpustate->pc.w.l,r);
	}
}


#define READ_FN( N ) \
INLINE UINT8 Read##N##_8(t90_Regs *cpustate)	{ \
	switch ( cpustate->mode##N )	{ \
		case MODE_CC: \
		case MODE_BIT8: \
		case MODE_I8:		return (UINT8)cpustate->r##N; \
		case MODE_D8:		return (UINT8)cpustate->r##N; \
		case MODE_R8:		return (UINT8)r8(cpustate, cpustate->r##N); \
		case MODE_MI16: 	return RM8(cpustate, cpustate->r##N); \
		case MODE_MR16R8:	return RM8(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)r8(cpustate, cpustate->r##N##b))); \
		case MODE_MR16: \
			switch( cpustate->r##N ) { \
				case IX:	return RX8(cpustate, cpustate->ix.w.l,cpustate->ixbase); \
				case IY:	return RX8(cpustate, cpustate->iy.w.l,cpustate->iybase); \
			} \
			return RM8(cpustate, r16(cpustate, cpustate->r##N)); \
		case MODE_MR16D8: \
			switch( cpustate->r##N ) { \
				case IX:	return RX8(cpustate, (UINT16)(cpustate->ix.w.l + (INT8)cpustate->r##N##b),cpustate->ixbase); \
				case IY:	return RX8(cpustate, (UINT16)(cpustate->iy.w.l + (INT8)cpustate->r##N##b),cpustate->iybase); \
			} \
			return RM8(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)cpustate->r##N##b)); \
		default: \
			fatalerror("%04x: unimplemented Read%d_8 mode = %d\n",cpustate->pc.w.l,N,cpustate->mode##N); \
	} \
	return 0; \
} \
INLINE UINT16 Read##N##_16(t90_Regs *cpustate)	{ \
	switch ( cpustate->mode##N )	{ \
		case MODE_I16:		return cpustate->r##N; \
		case MODE_D16:		return cpustate->r##N - 1; \
		case MODE_R16:		return r16(cpustate, cpustate->r##N); \
		case MODE_R16D8:	return r16(cpustate, cpustate->r##N) + (INT8)cpustate->r##N##b; \
		case MODE_R16R8:	return r16(cpustate, cpustate->r##N) + (INT8)r8(cpustate, cpustate->r##N##b); \
		case MODE_MI16: 	return RM16(cpustate, cpustate->r##N); \
		case MODE_MR16R8:	return RM16(cpustate,(UINT16)(r16(cpustate, cpustate->r##N) + (INT8)r8(cpustate, cpustate->r##N##b))); \
		case MODE_MR16: \
			switch( cpustate->r##N ) { \
				case IX:	return RX16(cpustate, cpustate->ix.w.l,cpustate->ixbase); \
				case IY:	return RX16(cpustate, cpustate->iy.w.l,cpustate->iybase); \
			} \
			return RM16(cpustate,r16(cpustate, cpustate->r##N)); \
		case MODE_MR16D8: \
			switch( cpustate->r##N ) { \
				case IX:	return RX16(cpustate, (UINT16)(cpustate->ix.w.l + (INT8)cpustate->r##N##b),cpustate->ixbase); \
				case IY:	return RX16(cpustate, (UINT16)(cpustate->iy.w.l + (INT8)cpustate->r##N##b),cpustate->iybase); \
			} \
			return RM16(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)cpustate->r##N##b)); \
		default: \
			fatalerror("%04x: unimplemented Read%d_16 modes = %d\n",cpustate->pc.w.l,N,cpustate->mode##N); \
	} \
	return 0; \
}



#define WRITE_FN( N ) \
INLINE void Write##N##_8( t90_Regs *cpustate, UINT8 value )	{ \
	switch ( cpustate->mode##N )	{ \
		case MODE_R8:		w8(cpustate, cpustate->r##N,value);		return; \
		case MODE_MI16: 	WM8(cpustate, cpustate->r##N, value);	return; \
		case MODE_MR16R8:	WM8(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)r8(cpustate, cpustate->r##N##b)), value);	return; \
		case MODE_MR16: \
			switch( cpustate->r##N ) { \
				case IX:	WX8(cpustate, cpustate->ix.w.l,value,cpustate->ixbase);	return; \
				case IY:	WX8(cpustate, cpustate->iy.w.l,value,cpustate->iybase);	return; \
			} \
			WM8(cpustate, r16(cpustate, cpustate->r##N), value);	return; \
		case MODE_MR16D8: \
			switch( cpustate->r##N ) { \
				case IX:	WX8(cpustate, (UINT16)(cpustate->ix.w.l + (INT8)cpustate->r##N##b),value,cpustate->ixbase);	return; \
				case IY:	WX8(cpustate, (UINT16)(cpustate->iy.w.l + (INT8)cpustate->r##N##b),value,cpustate->iybase);	return; \
			} \
			WM8(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)cpustate->r##N##b), value);	return; \
		default: \
			fatalerror("%04x: unimplemented Write%d_8 mode = %d\n",cpustate->pc.w.l,N,cpustate->mode##N); \
	} \
} \
INLINE void Write##N##_16( t90_Regs *cpustate, UINT16 value ) \
{ \
	switch ( cpustate->mode##N )	{ \
		case MODE_R16:		w16(cpustate, cpustate->r##N,value);	return; \
		case MODE_MI16: 	WM16(cpustate, cpustate->r##N, value);	return; \
		case MODE_MR16R8:	WM16(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)r8(cpustate, cpustate->r##N##b)), value);	return; \
		case MODE_MR16: \
			switch( cpustate->r##N ) { \
				case IX:	WX16(cpustate, cpustate->ix.w.l,value,cpustate->ixbase);	return; \
				case IY:	WX16(cpustate, cpustate->iy.w.l,value,cpustate->iybase);	return; \
			} \
			WM16(cpustate, r16(cpustate, cpustate->r##N), value);	return; \
		case MODE_MR16D8: \
			switch( cpustate->r##N ) { \
				case IX:	WX16(cpustate, (UINT16)(cpustate->ix.w.l + (INT8)cpustate->r##N##b),value,cpustate->ixbase);	return; \
				case IY:	WX16(cpustate, (UINT16)(cpustate->iy.w.l + (INT8)cpustate->r##N##b),value,cpustate->iybase);	return; \
			} \
			WM16(cpustate, (UINT16)(r16(cpustate, cpustate->r##N) + (INT8)cpustate->r##N##b), value);	return; \
		default: \
			fatalerror("%04x: unimplemented Write%d_16 mode = %d\n",cpustate->pc.w.l,N,cpustate->mode##N); \
	} \
}

READ_FN(1)
READ_FN(2)
WRITE_FN(1)
WRITE_FN(2)

INLINE int Test( t90_Regs *cpustate, UINT8 cond )
{
	int s,v;
	switch ( cond )
	{
		case FLS:	return 0;
		case LT:	s = F & SF;	v = F & VF;	return (s && !v) || (!s && v);
		case LE:	s = F & SF;	v = F & VF;	return (F & ZF) || (s && !v) || (!s && v);
		case ULE:	return (F & CF) || (F & ZF);
		case OV:	return F & VF;
		case MI:	return F & SF;
		case Z:		return F & ZF;
		case CR:	return F & CF;
		case T:		return 1;
		case GE:	s = F & SF;	v = F & VF;	return (s && v) || (!s && !v);
		case GT:	s = F & SF;	v = F & VF;	return !((F & ZF) || (s && !v) || (!s && v));
		case UGT:	return !(F & CF) && !(F & ZF);
		case NOV:	return !(F & VF);
		case PL:	return !(F & SF);
		case NZ:	return !(F & ZF);
		case NC:	return !(F & CF);
		default:
			fatalerror("%04x: unimplemented condition = %d\n",cpustate->pc.w.l,cond);
	}
	return 0;
}

INLINE void Push( t90_Regs *cpustate, e_r rr )
{
	cpustate->sp.w.l -= 2;
	WM16( cpustate, cpustate->sp.w.l, r16(cpustate, rr) );
}
INLINE void Pop( t90_Regs *cpustate, e_r rr )
{
	w16( cpustate, rr, RM16( cpustate, cpustate->sp.w.l ) );
	cpustate->sp.w.l += 2;
}

/*************************************************************************************************************

Interrupts

----------------------------------------------------------------------------------------------------------------
Priority    Type            Interrupt Source            Vector/8    Vector  Address         uDMA Address
----------------------------------------------------------------------------------------------------------------
1           Non Maskable    SWI                         -           10      0010            -
2           ""              NMI                         -           18      0018            -
3           ""              INTWD   Watch Dog           -           20      0020            -
4           Maskable        INT0    External 0          05          28      0028            FF28
5           ""              INTT0   Timer 0             06          30      0030            FF30
6           ""              INTT1   Timer 1             07          38      0038            FF38
7           ""              INTAD   A/D                 08          40      0040            FF48
""          ""              INTT2   Timer 2             ""          ""      ""              ""
8           ""              INTT3   Timer 3             09          48      0048            FF48
9           ""              INTT4   Timer 4             0A          50      0050            FF50
10          ""              INT1    External 1          0B          58      0058            FF58
11          ""              INTT5   Timer 5             0C          60      0060            FF60
12          ""              INT2    External 2          0D          68      0068            FF68
13          ""              INTRX   End Serial Receive  0E          70      0070            FF70
14          ""              INTTX   End Serial Transmit 0F          78      0078            FF78

----------------------------------------------------------------------------------------------------------------
Interrupt   Terminal    Mode            How To Set
----------------------------------------------------------------------------------------------------------------
NMI         -           Falling Edge    -
INT0        P80         Level           P8CR<EDGE> = 0
                        Rising Edge     P8CR<EDGE> = 1
INT1        P81         Rising Edge     T4MOD<CAPM1,0> = 0,0 or 0,1 or 1,1
                        Falling Edge    T4MOD<CAPM1,0> = 1,0
INT2        P82         Rising Edge     -

*************************************************************************************************************/

typedef enum	{	INTSWI = 0,	INTNMI,	INTWD,	INT0,	INTT0,	INTT1,	INTT2,	INTT3,	INTT4,	INT1,	INTT5,	INT2,	INTRX,	INTTX,	INTMAX	}	e_irq;
DECLARE_ENUM_OPERATORS(e_irq)

INLINE void leave_halt(t90_Regs *cpustate)
{
	if( cpustate->halt )
	{
		cpustate->halt = 0;
		cpustate->pc.w.l++;
	}
}

static void take_interrupt(t90_Regs *cpustate, e_irq irq)
{
	cpustate->irq_state &= ~(1 << irq);

	leave_halt(cpustate);

	Push( cpustate, PC );
	Push( cpustate, AF );

	F &= ~IF;

	cpustate->pc.w.l = 0x10 + irq * 8;

	cpustate->extra_cycles += 20*2;
}

static void check_interrupts(t90_Regs *cpustate)
{
	e_irq irq;

	if (!(F & IF))
		return;

	for (irq = INT0; irq < INTMAX; irq++)
	{
		if ( cpustate->irq_state & cpustate->irq_mask & (1 << irq) )
		{
			take_interrupt( cpustate, irq );
			return;
		}
	}
}

static void set_irq_line(t90_Regs *cpustate, int irq, int state)
{
	if ( ((cpustate->irq_state >> irq)&1) == state ) return;

	if (state)
	{
		cpustate->irq_state |= 1 << irq;
		check_interrupts(cpustate);
	}
	else
	{
		cpustate->irq_state &= ~(1 << irq);
	}
}

INLINE void Cyc(t90_Regs *cpustate)	{	cpustate->icount -= cpustate->cyc_t;	}
INLINE void Cyc_f(t90_Regs *cpustate)	{	cpustate->icount -= cpustate->cyc_f;	}

static CPU_EXECUTE( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);
	UINT8    a8,b8;
	UINT16   a16,b16;
	unsigned a32;
	PAIR tmp;

	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	do
	{
		cpustate->prvpc.d = cpustate->pc.d;
		debugger_instruction_hook(device, cpustate->pc.d);

		check_interrupts(cpustate);

		cpustate->addr = cpustate->pc.d;
		decode(cpustate);
		cpustate->pc.d = cpustate->addr;

		switch ( cpustate->op )
		{
			case NOP:
				Cyc(cpustate);
				break;

			case EX:
				a16 = Read1_16(cpustate);
				Write1_16( cpustate, Read2_16(cpustate) );
				Write2_16( cpustate, a16 );
				Cyc(cpustate);
				break;
			case EXX:
				tmp = cpustate->bc;	cpustate->bc = cpustate->bc2;	cpustate->bc2 = tmp;
				tmp = cpustate->de;	cpustate->de = cpustate->de2;	cpustate->de2 = tmp;
				tmp = cpustate->hl;	cpustate->hl = cpustate->hl2;	cpustate->hl2 = tmp;
				Cyc(cpustate);
				break;

			case LD:
				Write1_8( cpustate, Read2_8(cpustate) );
				Cyc(cpustate);
				break;
			case LDW:
			case LD | OP_16:
				Write1_16( cpustate, Read2_16(cpustate) );
				Cyc(cpustate);
				break;

//          case LDA:
//              Cyc(cpustate);
//              break;

			case LDI:
#define _LDI											\
				WM8( cpustate, cpustate->de.w.l, RM8(cpustate, cpustate->hl.w.l) );		\
				cpustate->de.w.l++;							\
				cpustate->hl.w.l++;							\
				cpustate->bc.w.l--;							\
				F &= SF | ZF | IF | XCF | CF;			\
				if ( cpustate->bc.w.l )	F |= VF;

				_LDI
				Cyc(cpustate);
				break;
			case LDIR:
				_LDI
				if ( cpustate->bc.w.l )
				{
					cpustate->pc.w.l -= 2;
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;

			case LDD:
#define _LDD											\
				WM8( cpustate, cpustate->de.w.l, RM8(cpustate, cpustate->hl.w.l) );		\
				cpustate->de.w.l--;							\
				cpustate->hl.w.l--;							\
				cpustate->bc.w.l--;							\
				F &= SF | ZF | IF | XCF | CF;			\
				if ( cpustate->bc.w.l )	F |= VF;

				_LDD
				Cyc(cpustate);
				break;
			case LDDR:
				_LDD
				if ( cpustate->bc.w.l )
				{
					cpustate->pc.w.l -= 2;
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;


//          case CPD:
//              Cyc(cpustate);
//              break;
//          case CPDR:
//              Cyc(cpustate);
//              break;
			case CPI:
				a8 = RM8(cpustate, cpustate->hl.w.l);
				b8 = cpustate->af.b.h - a8;
				cpustate->hl.w.l++;
				cpustate->bc.w.l--;
				F = (F & (IF | CF)) | SZ[b8] | ((cpustate->af.b.h^a8^b8)&HF) | NF;
				if ( cpustate->bc.w.l ) F |= VF;
				Cyc(cpustate);
				break;
			case CPIR:
				a8 = RM8(cpustate, cpustate->hl.w.l);
				b8 = cpustate->af.b.h - a8;
				cpustate->hl.w.l++;
				cpustate->bc.w.l--;
				F = (F & (IF | CF)) | SZ[b8] | ((cpustate->af.b.h^a8^b8)&HF) | NF;
				if ( cpustate->bc.w.l )
				{
					F |= VF;
					cpustate->pc.w.l -= 2;
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;

			case PUSH:
				Push( cpustate, cpustate->r1 );
				Cyc(cpustate);
				break;
			case POP:
				Pop( cpustate, cpustate->r1 );
				Cyc(cpustate);
				break;

			case JP:
				if ( Test( cpustate, Read1_8(cpustate) ) )
				{
					cpustate->pc.w.l = Read2_16(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case JR:
				if ( Test( cpustate, Read1_8(cpustate) ) )
				{
					cpustate->pc.w.l += /*2 +*/ (INT8)Read2_8(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case JR | OP_16:
				if ( Test( cpustate, Read1_8(cpustate) ) )
				{
					cpustate->pc.w.l += /*2 +*/ Read2_16(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;


			case CALL:
				if ( Test( cpustate, Read1_8(cpustate) ) )
				{
					Push( cpustate, PC );
					cpustate->pc.w.l = Read2_16(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case CALLR:
				Push( cpustate, PC );
				cpustate->pc.w.l += /*2 +*/ Read1_16(cpustate);
				Cyc(cpustate);
				break;

			case RET:
				if ( Test( cpustate, Read1_8(cpustate) ) )
				{
					Pop( cpustate, PC );
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case RETI:
				Pop( cpustate, AF );
				Pop( cpustate, PC );
				Cyc(cpustate);
				break;

//          case HALT:
//              Cyc(cpustate);
//              break;
			case DI:
				cpustate->after_EI = 0;
				F &= ~IF;
				Cyc(cpustate);
				break;
			case EI:
				cpustate->after_EI = !(F & IF);
				Cyc(cpustate);
				break;

			case SWI:
				Cyc(cpustate);
				take_interrupt( cpustate, INTSWI );
				break;

			case DAA:
			{
				UINT8 cf, nf, hf, lo, hi, diff;
				cf = F & CF;
				nf = F & NF;
				hf = F & HF;
				lo = cpustate->af.b.h & 15;
				hi = cpustate->af.b.h / 16;

				if (cf)
				{
					diff = (lo <= 9 && !hf) ? 0x60 : 0x66;
				}
				else
				{
					if (lo >= 10)
					{
						diff = hi <= 8 ? 0x06 : 0x66;
					}
					else
					{
						if (hi >= 10)
						{
							diff = hf ? 0x66 : 0x60;
						}
						else
						{
							diff = hf ? 0x06 : 0x00;
						}
					}
				}
				if (nf) cpustate->af.b.h -= diff;
				else cpustate->af.b.h += diff;

				F = SZP[A] | (F & (IF | NF));
				if (cf || (lo <= 9 ? hi >= 10 : hi >= 9)) F |= XCF | CF;
				if (nf ? hf && lo <= 5 : lo >= 10)	F |= HF;
			}
				Cyc(cpustate);
				break;


			case CPL:
				cpustate->af.b.h ^= 0xff;
				F |= HF | NF;
				Cyc(cpustate);
				break;
			case NEG:
				a8 = 0;
				b8 = cpustate->af.b.h;
				a32 = a8 - b8;
				F = (F & IF) | SZ[(UINT8)a32] | NF;
				if (a32 & 0x100)			F |= CF | XCF;	//X?
				if ((a8 ^ a32 ^ b8) & 0x10)	F |= HF;
				if ((b8 ^ a8) & (a8 ^ a32) & 0x80)	F |= VF;
				cpustate->af.b.h = a32;
				Cyc(cpustate);
				break;

			case LDAR:
				cpustate->hl.w.l = cpustate->pc.w.l + /*2 +*/ Read2_16(cpustate);
				Cyc(cpustate);
				break;

			case RCF:
				F &= SF | ZF | IF | VF;
				Cyc(cpustate);
				break;
			case SCF:
				F = (F & (SF | ZF | IF | VF)) | XCF | CF;
				Cyc(cpustate);
				break;
			case CCF:
				F = (F & (SF | ZF | IF | VF)) | ((F & CF)?HF:(XCF | CF));
				Cyc(cpustate);
				break;

//          case TSET:
//              Cyc(cpustate);
//              break;
			case BIT:
				F = (F & (IF | CF)) | HF | SZ_BIT[ Read2_8(cpustate) & (1 << Read1_8(cpustate)) ];
				Cyc(cpustate);
				break;
			case SET:
				Write2_8( cpustate, Read2_8(cpustate) | (1 << Read1_8(cpustate)) );
				Cyc(cpustate);
				break;
			case RES:
				Write2_8( cpustate, Read2_8(cpustate) & (~(1 << Read1_8(cpustate))) );
				Cyc(cpustate);
				break;

			case INC:
				a8 = Read1_8(cpustate) + 1;
				Write1_8( cpustate, a8 );
				F = (F & (IF | CF)) | SZHV_inc[a8];
				if (a8 == 0)	F |= XCF;
				Cyc(cpustate);
				break;
			case INCX:
				if ( F & XCF )
				{
					a8 = Read1_8(cpustate) + 1;
					Write1_8( cpustate, a8 );
					F = (F & (IF | CF)) | SZHV_inc[a8];
					if (a8 == 0)	F |= XCF;
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case INC | OP_16:
				a16 = Read1_16(cpustate) + 1;
				Write1_16( cpustate, a16 );
				if (a16 == 0)	F |=  XCF;
				else			F &= ~XCF;
				Cyc(cpustate);
				break;
			case INCW:
				a16 = Read1_16(cpustate);
				a32 = a16 + 1;
				Write1_16( cpustate, a32 );
				F &= IF | CF;
				if ((UINT16)a32 == 0)	F |= ZF | XCF;
				if (a32 & 0x8000)		F |= SF;
				if ((a16 ^ 0x8000) & a32 & 0x8000)	F |= VF;
				if ((a16 ^ a32 ^ 1) & 0x1000)	F |= HF;	//??
				Cyc(cpustate);
				break;


			case DEC:
				a8 = Read1_8(cpustate) - 1;
				Write1_8( cpustate, a8 );
				F = (F & (IF | CF)) | SZHV_dec[a8];
				if (a8 == 0)	F |= XCF;
				Cyc(cpustate);
				break;
			case DECX:
				if ( F & XCF )
				{
					a8 = Read1_8(cpustate) - 1;
					Write1_8( cpustate, a8 );
					F = (F & (IF | CF)) | SZHV_dec[a8];
					if (a8 == 0)	F |= XCF;
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case DEC | OP_16:
				a16 = Read1_16(cpustate) - 1;
				Write1_16( cpustate, a16 );
				if (a16 == 0)	F |=  XCF;
				else			F &= ~XCF;
				Cyc(cpustate);
				break;
			case DECW:
				a16 = Read1_16(cpustate);
				a32 = a16 - 1;
				Write1_16( cpustate, a32 );
				F = (F & (IF | CF)) | NF;
				if ((UINT16)a32 == 0)	F |= ZF | XCF;
				if (a32 & 0x8000)		F |= SF;
				if (a16 == 0x8000)		F |= VF;
				if ((a16 ^ a32 ^ 1) & 0x1000)	F |= HF;	//??
				Cyc(cpustate);
				break;

			case ADD:
			case ADC:
				a8 = Read1_8(cpustate);
				b8 = Read2_8(cpustate);
				a32 = a8 + b8;
				if ( (cpustate->op == ADC) && (F & CF) )	a32 += 1;
				Write1_8( cpustate, a32 );
				F = (F & IF) | SZ[(UINT8)a32];
				if (a32 & 0x100)			F |= CF | XCF;	//X?
				if ((a8 ^ a32 ^ b8) & 0x10)	F |= HF;
				if ((b8 ^ a8 ^ 0x80) & (b8 ^ a32) & 0x80)	F |= VF;
				Cyc(cpustate);
				break;
			case ADD | OP_16:
			case ADC | OP_16:
				a16 = Read1_16(cpustate);
				b16 = Read2_16(cpustate);
				a32 = a16 + b16;
				if ( (cpustate->op == (ADC | OP_16)) && (F & CF) )	a32 += 1;
				Write1_16( cpustate, a32 );
				if ( (cpustate->op == (ADD | OP_16)) && cpustate->mode2 == MODE_R16 )
				{
					F &= SF | ZF | IF | VF;
				}
				else
				{
					F &= IF;
					if ((UINT16)a32 == 0)			F |= ZF;
					if (a32 & 0x8000)				F |= SF;
					if ((b16 ^ a16 ^ 0x8000) & (b16 ^ a32) & 0x8000)	F |= VF;
				}
				if (a32 & 0x10000)				F |= CF | XCF;	//X?
				if ((a16 ^ a32 ^ b16) & 0x1000)	F |= HF;	//??
				Cyc(cpustate);
				break;

			case CP:
			case SUB:
			case SBC:
				a8 = Read1_8(cpustate);
				b8 = Read2_8(cpustate);
				a32 = a8 - b8;
				if ( (cpustate->op == SBC) && (F & CF) )	a32 -= 1;
				F = (F & IF) | SZ[(UINT8)a32] | NF;
				if (a32 & 0x100)			F |= CF | XCF;	//X?
				if ((a8 ^ a32 ^ b8) & 0x10)	F |= HF;
				if ((b8 ^ a8) & (a8 ^ a32) & 0x80)	F |= VF;
				if (cpustate->op != CP)
					Write1_8( cpustate, a32 );
				Cyc(cpustate);
				break;
			case CP | OP_16:
			case SUB | OP_16:
			case SBC | OP_16:
				a16 = Read1_16(cpustate);
				b16 = Read2_16(cpustate);
				a32 = a16 - b16;
				if ( (cpustate->op == (SBC | OP_16)) && (F & CF) )	a32 -= 1;
				F = (F & IF) | NF;
				if ((UINT16)a32 == 0)			F |= ZF;
				if (a32 & 0x8000)				F |= SF;
				if (a32 & 0x10000)				F |= CF | XCF;	//X?
				if ((a16 ^ a32 ^ b16) & 0x1000)	F |= HF;	//??
				if ((b16 ^ a16) & (a16 ^ a32) & 0x8000)	F |= VF;
				if (cpustate->op != (CP | OP_16))
					Write1_16( cpustate, a32 );
				Cyc(cpustate);
				break;

			case AND:
				a8 = Read1_8(cpustate) & Read2_8(cpustate);
				Write1_8( cpustate, a8 );
				F = (F & IF) | SZP[a8] | HF;
				Cyc(cpustate);
				break;
			case AND | OP_16:
				a16 = Read1_16(cpustate) & Read2_16(cpustate);
				Write1_16( cpustate, a16 );
				F = (F & IF) | HF;
				if (a16 == 0)		F |= ZF;
				if (a16 & 0x8000)	F |= SF;
				Cyc(cpustate);
				break;
			case XOR:
				a8 = Read1_8(cpustate) ^ Read2_8(cpustate);
				Write1_8( cpustate, a8 );
				F = (F & IF) | SZP[a8];
				Cyc(cpustate);
				break;
			case XOR | OP_16:
				a16 = Read1_16(cpustate) ^ Read2_16(cpustate);
				Write1_16( cpustate, a16 );
				F &= IF;
				if (a16 == 0)		F |= ZF;
				if (a16 & 0x8000)	F |= SF;
				Cyc(cpustate);
				break;
			case OR:
				a8 = Read1_8(cpustate) | Read2_8(cpustate);
				Write1_8( cpustate, a8 );
				F = (F & IF) | SZP[a8];
				Cyc(cpustate);
				break;
			case OR | OP_16:
				a16 = Read1_16(cpustate) | Read2_16(cpustate);
				Write1_16( cpustate, a16 );
				F &= IF;
				if (a16 == 0)		F |= ZF;
				if (a16 & 0x8000)	F |= SF;
				Cyc(cpustate);
				break;

			case RLC:
				a8 = Read1_8(cpustate);
				a8 = (a8 << 1) | (a8 >> 7);
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (a8 & 0x01)						F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case RRC:
				a8 = Read1_8(cpustate);
				a8 = (a8 >> 1) | (a8 << 7);
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (a8 & 0x80)						F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case RL:
				a8 = Read1_8(cpustate);
				b8 = a8 & 0x80;
				a8 <<= 1;
				if (F & CF)	a8 |= 0x01;
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (b8)								F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case RR:
				a8 = Read1_8(cpustate);
				b8 = a8 & 0x01;
				a8 >>= 1;
				if (F & CF)	a8 |= 0x80;
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (b8)								F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;

			case SLA:
			case SLL:
				a8 = Read1_8(cpustate);
				b8 = a8 & 0x80;
				a8 <<= 1;
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (b8)								F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case SRA:
				a8 = Read1_8(cpustate);
				b8 = a8 & 0x01;
				a8 = (a8 & 0x80) | (a8 >> 1);
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (b8)								F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case SRL:
				a8 = Read1_8(cpustate);
				b8 = a8 & 0x01;
				a8 >>= 1;
				Write1_8( cpustate, a8 );
				if ( cpustate->mode1 == MODE_R8 && cpustate->r1 == A )	F &= SF | ZF | IF | PF;
				else								F = (F & IF) | SZP[a8];
				if (b8)								F |= CF | XCF;	// X?
				Cyc(cpustate);
				break;
			case RLD:
				a8 = cpustate->af.b.h;
				b8 = Read1_8(cpustate);
				Write1_8( cpustate, (b8 << 4) | (a8 & 0x0f) );
				a8 = (a8 & 0xf0) | (b8 >> 4);
				F = (F & (IF | CF)) | SZP[a8];
				cpustate->af.b.h = a8;
				Cyc(cpustate);
				break;
			case RRD:
				a8 = cpustate->af.b.h;
				b8 = Read1_8(cpustate);
				Write1_8( cpustate, (b8 >> 4) | (a8 << 4) );
				a8 = (a8 & 0xf0) | (b8 & 0x0f);
				F = (F & (IF | CF)) | SZP[a8];
				cpustate->af.b.h = a8;
				Cyc(cpustate);
				break;

			case DJNZ:
				if ( --cpustate->bc.b.h )
				{
					cpustate->pc.w.l += /*2 +*/ (INT8)Read1_8(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;
			case DJNZ | OP_16:
				if ( --cpustate->bc.w.l )
				{
					cpustate->pc.w.l += /*2 +*/ (INT8)Read2_8(cpustate);
					Cyc(cpustate);
				}
				else	Cyc_f(cpustate);
				break;

			case MUL:
				cpustate->hl.w.l = (UINT16)cpustate->hl.b.l * (UINT16)Read2_8(cpustate);
				Cyc(cpustate);
				break;
			case DIV:
				a16 = cpustate->hl.w.l;
				b16 = (UINT16)Read2_8(cpustate);
				if (b16 == 0)
				{
					F |= VF;
					cpustate->hl.w.l = (a16 << 8) | ((a16 >> 8) ^ 0xff);
				}
				else
				{
					cpustate->hl.b.h = a16 % b16;
					a16 /= b16;
					if (a16 > 0xff)	F |=  VF;
					else			F &= ~VF;
					cpustate->hl.b.l = a16;
				}
				Cyc(cpustate);
				break;

			default:
				fatalerror("%04x: unimplemented opcode, op=%02x\n",cpu_get_pc(device),cpustate->op);
		}

		if ( cpustate->op != EI )
			if (cpustate->after_EI)
			{
				F |= IF;
				cpustate->after_EI = 0;
			}

	} while( cpustate->icount > 0 );

	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;
}

static CPU_RESET( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);
	cpustate->irq_state = 0;
	cpustate->irq_mask = 0;
	cpustate->pc.d = 0x0000;
	F &= ~IF;
/*
    P0/D0-D7 P1/A0-A7 P2/A8-A15 P6 P7 = INPUT
    P35/~RD P36/~WR CLK = 1 (ALWAYS OUTPUTS)
    P4/A16-A19 P83 = 0
    dedicated input ports and registers remain unchanged,
    but PC IFF BX BY = 0, A undefined
*/
}

static CPU_EXIT( t90 )
{
}

static CPU_BURN( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);
	cpustate->icount -= 4 * ((cycles + 3) / 4);
}


/*************************************************************************************************************


----------------------------------------------------------------------------------------------------------------
FFC0    P0      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     P07-P00 R W     IN      I/O Port 0

----------------------------------------------------------------------------------------------------------------
FFC1    P1      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     P17-P10 R W     IN      I/O Port 1

----------------------------------------------------------------------------------------------------------------
FFC2    P01CR/  R/W     Reset   Description                 * Prohibit Read-Modify-Write *
        IRFL
----------------------------------------------------------------------------------------------------------------
 7      -       -       0       -
 6      IRF0    R       0       INT0    interrupt request (1 = asserted)
 5      IRFT0   R       0       INTT0   ""
 4      IRFT1   R       0       INTT1   ""
 3      -       -       0       -
 2      EXT       W     0       P1/P2 control:  0 = Port                        1 = Address Bus
 1      P1CR      W     0       P1 control:     0 = IN                          1 = OUT     |
 0      P0CR      W     0       P0 control:     0 = IN                          1 = OUT     |
                                                |                                           |
                            Port 0 also functions as  data bus (D0-D7),so           P1: regardless of <P1CR>
                            P0CR is reset when external memory is accessed          P2: only if P2CR is set to 1 (output)
                                                                                    TMP90C841A: always address bus

----------------------------------------------------------------------------------------------------------------
FFC3    IRFH    R/W     Reset   Description                 * Prohibit Read-Modify-Write *
----------------------------------------------------------------------------------------------------------------
 7      IRFT2   R W     0       INTT2   interrupt request (1 = asserted)
 6      IRFT3   R W     0       INTT3   ""
 5      IRFT4   R W     0       INTT4   ""
 4      IRF1    R W     0       INT1    ""
 3      IRFT5   R W     0       INTT5   ""
 2      IRF2    R W     0       INT2    ""
 1      IRFRX   R W     0       INTRX   ""
 0      IRFTX   R W     0       INTTX   ""
                  |__ Writing Vector/8 clears the request flip-flop for that interrupt

----------------------------------------------------------------------------------------------------------------
FFC4    P2      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     P27-P20 R W     IN      I/O Port 2

----------------------------------------------------------------------------------------------------------------
FFC5    P2CR    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     P27C-     W     0       Port 2 control: 0 = IN                          1 = OUT
        P20C

----------------------------------------------------------------------------------------------------------------
FFC6    P3      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      P37     R       IN      P37 input
 6      P36     R W     1       ~WD
 5      P35     R W     1       ~RD
 4      P34     R       IN      P34 input
 3      P33     R W     1       TxD / P33 output
 2      P32     R W     1       TxD / P32 output
 1      P31     R       IN      RxD / P31 input
 0      P30     R       IN      RxD / P30 input

----------------------------------------------------------------------------------------------------------------
FFC7    P3CR    R/W     Reset   Description             * Prohibit Read-Modify-Write *
----------------------------------------------------------------------------------------------------------------
7-6     WAITC1-0R W     00      Wait control:   00 = 2state     01 = normal     10 = no wait    11 = reserved
 5      RDE     R W     0       RD control:     0 = only external access        1 = always
 4      ODE     R W     0       P33 control:    0 = CMOS                        1 = Open Drain
3-2     TXDC1-0 R W     00      P33-P32:        00 = OUT-OUT    01 = OUT-TxD    10 = TxD-OUT    11 = TxD-~RTS/SCLK
1-0     RXDC1-0 R W     00      P31-P30:        00 = IN-IN      01 = IN-RxD     10 = RxD-IN     11 = not used

----------------------------------------------------------------------------------------------------------------
FFC8    P4      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-4     -
3-0     P43-P40 R W     0       I/O Port 4 bits 3-0 or address bus A16-A19

----------------------------------------------------------------------------------------------------------------
FFC9    P4CR    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-4     -
3-0     P43C-     W     0       Port 4 control: 0 = OUT                         1 = Address Output
        P40C

----------------------------------------------------------------------------------------------------------------
FFCA    P5      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7       -       R       0       Test bit, must be set to 0
6       -       R       0
5-0     P55-P50 R       0       I/O Port 5 bits 5-0 / AN5-AN0 analog inputs

----------------------------------------------------------------------------------------------------------------
FFCB    SMMOD   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      -       R W     -
 6      SM7M0   R W     0       Motor Step:         0 = Full                        1 = Half
 5      P70C1   R W     0       Port 7 control:     0 = IN-OUT      0 = IN-OUT,TO3  1 = IN / M1     1 = Timer 4
 4      P70C0   R W     0                           0               1               0   Timer 2,3   1
 3      -       R W     -
 2      SM6M0   R W     0       Motor Step:         0 = Full                        1 = Half
 1      P60C1   R W     0       Port 6 control:     0 = IN-OUT      0 = IN-OUT,TO1  1 = IN / M0
 0      P60C0   R W     0                           0               1               X   Timer 0,1

----------------------------------------------------------------------------------------------------------------
FFCC    P6      R/W     Reset   Description             * Read-Modify-Write not available in Stepping Motor control *
----------------------------------------------------------------------------------------------------------------
7-4     SA60-63   W     Undef   Stepping motor Port 0 (M0) Shifter alternate reg.
3-0     P63-P60 R W     IN      Bits 3-0 of I/O Port 6 or Stepping motor Port 0 (M0)

----------------------------------------------------------------------------------------------------------------
FFCD    P7      R/W     Reset   Description             * Read-Modify-Write not available in Stepping Motor control *
----------------------------------------------------------------------------------------------------------------
7-4     SA70-73   W     Undef   Stepping motor Port 1 (M1) Shifter alternate reg.
3-0     P73-P70 R W     IN      Bits 3-0 of I/O Port 7 or Stepping motor Port 1 (M1)

----------------------------------------------------------------------------------------------------------------
FFCE    P67CR   R/W     Reset   Description             * Prohibit Read-Modify-Write *
----------------------------------------------------------------------------------------------------------------
7-4     P73-70C   W     0       Port 7:             0 = IN                          1 = OUT
3-0     P63-60C   W     0       Port 6:             0 = IN                          1 = OUT

----------------------------------------------------------------------------------------------------------------
FFD0    P8      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-4     -
 3      P83     R W     0       P83 / TIO3 / TIO4 output
 2      P82     R       IN      P82 + INT2 input
 1      P81     R       IN      P81 + INT1 input
 0      P80     R       IN      P80 + INT0 input

----------------------------------------------------------------------------------------------------------------
FFD1    P8CR    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-4     -
 3      P83OC     W     0       P83 out control:    0 = P83                         1 = TO3 / TO4
 2      ZCE2      W     0       INT2/TI5 control:   0 = ZCD disable                 1 = ZCD enable
 1      ZCE1      W     0       INT1/TI4 control:   0 = ZCD disable                 1 = ZCD enable
 0      EDGE      W     0       INT0 control:       0 = Level                       1 = Rising edge

----------------------------------------------------------------------------------------------------------------
FFD2    WDMOD   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      WDTE    R W     1       1 = WDT Enable
 6      WDTP1   R W     0       WDT detection time: 0 = 2^14/fc     0 = 2^16/fc     1 = 2^18/fc     1 = 2^20/fc
 5      WDTP0   R W     0                           0               1               0               1
 4      WARM    R W     0       Warming up time:    0 = 2^14/fc                     1 = 2^16/fc
 3      HALTM1  R W     0       Standby mode:       0 = RUN         0 = STOP        1 = IDLE1       1 = IDLE2
 2      HALTM0  R W     0                           0               1               0               1
 1      EXF     R       Undef   Invert each time EXX instruction is executed
 0      DRIVE   R W     0       1 to drive pins in stop mode

----------------------------------------------------------------------------------------------------------------
FFD4    TREG0   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     -       W       0       Timer 0 match value

----------------------------------------------------------------------------------------------------------------
FFD5    TREG1   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     -       W       0       Timer 0 match value

----------------------------------------------------------------------------------------------------------------
FFD6    TREG2   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     -       W       0       Timer 0 match value

----------------------------------------------------------------------------------------------------------------
FFD7    TREG3   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-0     -       W       0       Timer 0 match value

----------------------------------------------------------------------------------------------------------------
FFD8    TCLK    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-6     T3CLK1-0R W     00      Timer 3 clock:      00 = Timer 2    01 = clock      10 = clock/16   11 = clock/256  <- Timer 2 overflow output in 16 bit mode
5-4     T2CLK1-0R W     00      Timer 2 clock:      00 = -          01 = clock      10 = clock/16   11 = clock/256
3-2     T1CLK1-0R W     00      Timer 1 clock:      00 = Timer 0    01 = clock      10 = clock/16   11 = clock/256  <- Timer 0 overflow output in 16 bit mode
1-0     T0CLK1-0R W     00      Timer 0 clock:      00 = -          01 = clock      10 = clock/16   11 = clock/256

----------------------------------------------------------------------------------------------------------------
FFD9    TFFCR   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-6     TFF3C1-0  W     -       Timer 3 flip-flop:  00 = Clear      01 = Set        10 = Invert     11 = Don't care <-  Always read as 11
5       TFF3IE  R W     0       Timer 3 Invert:     0 = Disable                     1 = Enable
4       TFF3IS  R W     0       Timer 3 Invert:     0 = Timer 2                     1 = Timer 3
3-2     TFF1C1-0  W     -       Timer 1 Flip-Flop:  00 = Clear      01 = Set        10 = Invert     11 = Don't care <-  Always read as 11
1       TFF1IE  R W     0       Timer 1 Invert:     0 = Disable                     1 = Enable
0       TFF1IS  R W     0       Timer 1 Invert:     0 = Timer 0                     1 = Timer 1

----------------------------------------------------------------------------------------------------------------
FFDA    TMOD    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-6     T32M1-0 R W     00      Timers 2 & 3:       00 = 8b x 2     01 = 16b(T3+T2) 10 = 8b PPG(T3) 11 = 8b PWM (T3) + 8b (T2)
5-4     PWM21-0 R W     00      Timer 3 PWM freq:   00 = -          01 = 63/fc      10 = 127/fc     11 = 255/fc
3-2     T10M1-0 R W     00      Timers 1 & 0:       00 = 8b x 2     01 = 16b(T1+T0) 10 = 8b PPG(T1) 11 = 8b PWM (T1) + 8b (T0)
1-0     PWM01-0 R W     00      Timer 1 PWM freq:   00 = -          01 = 63/fc      10 = 127/fc     11 = 255/fc

----------------------------------------------------------------------------------------------------------------
FFDB    TRUN    R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
7-6     BRATE1-0R W     00      Serial baud rate:   00 = 300/150    01 = 1200/600   10 = 4800/2400  11 = 19200/9600
 5      PRRUN   R W     0       Prescaler control:  0 = Stop & Clear                1 = Run
 4      T4RUN   R W     0       Timer 4 control:    0 = Stop & Clear                1 = Run
3-0     T4RUN-0 R W     00      Timers 3-0 control: 0 = Stop & Clear                1 = Run

----------------------------------------------------------------------------------------------------------------
FFE6    INTEL   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      DET2    R W     0       INTT2/INTAD interrupt enable flag (1 = enable)
 6      DET3    R W     0       INTT3       ""
 5      DET4    R W     0       INTT4       ""
 4      DE1     R W     0       INT1        ""
 3      DET5    R W     0       INTT5       ""
 2      DE2     R W     0       INT2        ""
 1      DERX    R W     0       INTRX       ""
 0      DETX    R W     0       INTTX       ""

----------------------------------------------------------------------------------------------------------------
FFE7    INTEH   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      -       R W     0       Write "0"
 6      DE0     R W     0       INT0        DMA enable flag (1 = enable)
 5      DET0    R W     0       INTT0       ""
 4      DET1    R W     0       INTT1       ""
 3      ADIS    R W     0       INTT2/INTAD selection (1 = INTAD)
 2      IE0     R W     0       INT0        interrupt enable flag (1 = enable)
 1      IET0    R W     0       INTT0       ""
 0      IET1    R W     0       INTT1       ""

----------------------------------------------------------------------------------------------------------------
FFE8    DMAEH   R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      DET2    R W     0       INTT2/INTAD DMA enable flag (1 = enable)
 6      DET3    R W     0       INTT3       ""
 5      DET4    R W     0       INTT4       ""
 4      DE1     R W     0       INT1        ""
 3      DET5    R W     0       INTT5       ""
 2      DE2     R W     0       INT2        ""
 1      DERX    R W     0       INTRX       ""
 0      DETX    R W     0       INTTX       ""

----------------------------------------------------------------------------------------------------------------
FFEC    BX      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      -       R       1
 6      -       R       1
 5      -       R       1
 4      -       R       1
 3      BX3     R W     0       IX bank register bit 3
 2      BX2     R W     0       IX bank register bit 2
 1      BX1     R w     0       IX bank register bit 1
 0      BX0     R W     0       IX bank register bit 0

----------------------------------------------------------------------------------------------------------------
FFED    BX      R/W     Reset   Description
----------------------------------------------------------------------------------------------------------------
 7      -       R       1
 6      -       R       1
 5      -       R       1
 4      -       R       1
 3      BY3     R W     0       IY bank register bit 3
 2      BY2     R W     0       IY bank register bit 2
 1      BY1     R w     0       IY bank register bit 1
 0      BY0     R W     0       IY bank register bit 0

*************************************************************************************************************/

static READ8_HANDLER( t90_internal_registers_r )
{
	t90_Regs *cpustate = get_safe_token(&space->device());

	#define RIO		cpustate->io->read_byte( T90_IOBASE+offset )

	UINT8 data = cpustate->internal_registers[offset];
	switch ( T90_IOBASE + offset )
	{
		case T90_P3:	// 7,4,1,0
			return (data & 0x6c) | (RIO & 0x93);

		case T90_P4:	// only output
			return data & 0x0f;

		case T90_P5:
			return (RIO & 0x3f);

		case T90_P6:
		case T90_P7:
			return (data & 0xf0) | (RIO & 0x0f);

		case T90_P8:	// 2,1,0
			return (data & 0x08) | (RIO & 0x07);

		case T90_BX:
		case T90_BY:
			return 0xf0 | data;
	}
	return data;
}

static void t90_start_timer(t90_Regs *cpustate, int i)
{
	int prescaler;
	attotime period;

	cpustate->timer_value[i] = 0;

	switch((cpustate->internal_registers[ T90_TMOD - T90_IOBASE ] >> (i * 2)) & 0x03)
	{
		case 0:
			// 8-bit mode
			break;
		case 1:
			// 16-bit mode
			if (i & 1)
			{
				logerror("%04X: CPU Timer %d clocked by Timer %d overflow signal\n", cpustate->pc.w.l, i,i-1);
				return;
			}
			break;
		case 2:
			logerror("%04X: CPU Timer %d, unsupported PPG mode\n", cpustate->pc.w.l, i);
			return;
		case 3:
			logerror("%04X: CPU Timer %d, unsupported PWM mode\n", cpustate->pc.w.l, i);
			return;
	}

	switch((cpustate->internal_registers[ T90_TCLK - T90_IOBASE ] >> (i * 2)) & 0x03)
	{
		case 0:	if (i & 1)	logerror("%04X: CPU Timer %d clocked by Timer %d match signal\n", cpustate->pc.w.l, i,i-1);
				else		logerror("%04X: CPU Timer %d, unsupported TCLK = 0\n", cpustate->pc.w.l, i);
				return;
		case 2:	prescaler =  16;	break;
		case 3:	prescaler = 256;	break;
		default:
		case 1:	prescaler =   1;	break;
	}


	period = cpustate->timer_period * prescaler;

	cpustate->timer[i]->adjust(period, i, period);

	logerror("%04X: CPU Timer %d started at %lf Hz\n", cpustate->pc.w.l, i, 1.0 / period.as_double());
}

static void t90_start_timer4(t90_Regs *cpustate)
{
	int prescaler;
	attotime period;

	cpustate->timer4_value = 0;

	switch(cpustate->internal_registers[ T90_T4MOD - T90_IOBASE ] & 0x03)
	{
		case 1:		prescaler =   1;	break;
		case 2:		prescaler =  16;	break;
		default:	logerror("%04X: CPU Timer 4, unsupported T4MOD = %d\n", cpustate->pc.w.l,cpustate->internal_registers[ T90_T4MOD - T90_IOBASE ] & 0x03);
					return;
	}

	period = cpustate->timer_period * prescaler;

	cpustate->timer[4]->adjust(period, 4, period);

	logerror("%04X: CPU Timer 4 started at %lf Hz\n", cpustate->pc.w.l, 1.0 / period.as_double());
}


static void t90_stop_timer(t90_Regs *cpustate, int i)
{
	cpustate->timer[i]->adjust(attotime::never, i);
	logerror("%04X: CPU Timer %d stopped\n", cpustate->pc.w.l, i);
}

static void t90_stop_timer4(t90_Regs *cpustate)
{
	t90_stop_timer(cpustate, 4);
}

static TIMER_CALLBACK( t90_timer_callback )
{
	t90_Regs *cpustate = (t90_Regs *)ptr;
	int is16bit;
	int i = param;

	if ( (cpustate->internal_registers[ T90_TRUN - T90_IOBASE ] & (1 << i)) == 0 )
		return;

//  logerror("CPU Timer %d fired! value = %d\n", i,(unsigned)cpustate->timer_value[i]);

	cpustate->timer_value[i]++;

	is16bit = ((cpustate->internal_registers[ T90_TMOD - T90_IOBASE ] >> (i/2 * 2 + 2)) & 0x03) == 1;

	// Match

	if ( cpustate->timer_value[i] == cpustate->internal_registers[ T90_TREG0+i - T90_IOBASE ] )
	{
//      logerror("CPU Timer %d match\n", i);

		if (is16bit)
		{
			if (i & 1)
			{
				if ( cpustate->timer_value[i-1] == cpustate->internal_registers[ T90_TREG0+i-1 - T90_IOBASE ] )
				{
					cpustate->timer_value[i]   = 0;
					cpustate->timer_value[i-1] = 0;

					set_irq_line(cpustate, INTT0 + i, 1);
				}
			}
			else
				set_irq_line(cpustate, INTT0 + i, 1);
		}
		else
		{
			cpustate->timer_value[i] = 0;
			set_irq_line(cpustate, INTT0 + i, 1);
		}

		switch (i)
		{
			case 0:
			case 2:
				if ( !is16bit )
					if ( (cpustate->internal_registers[ T90_TCLK - T90_IOBASE ] & (0x03 << (i * 2 + 2))) == 0 )	// T0/T1 match signal clocks T1/T3
						t90_timer_callback(machine, cpustate, i+1);
				break;
		}
	}

	// Overflow

	if ( cpustate->timer_value[i] == 0 )
	{
//      logerror("CPU Timer %d overflow\n", i);

		switch (i)
		{
			case 0:
			case 2:
				if ( is16bit )	// T0/T1 overflow signal clocks T1/T3
					t90_timer_callback(machine, cpustate, i+1);
				break;
		}
	}
}

static TIMER_CALLBACK( t90_timer4_callback )
{
//  logerror("CPU Timer 4 fired! value = %d\n", (unsigned)cpustate->timer_value[4]);

	t90_Regs *cpustate = (t90_Regs *)ptr;
	cpustate->timer4_value++;

	// Match

	if ( cpustate->timer4_value == (cpustate->internal_registers[ T90_TREG4L - T90_IOBASE ] + (cpustate->internal_registers[ T90_TREG4H - T90_IOBASE ] << 8)) )
	{
//      logerror("CPU Timer 4 matches TREG4\n");
		set_irq_line(cpustate, INTT4, 1);
	}
	if ( cpustate->timer4_value == (cpustate->internal_registers[ T90_TREG5L - T90_IOBASE ] + (cpustate->internal_registers[ T90_TREG5H - T90_IOBASE ] << 8)) )
	{
//      logerror("CPU Timer 4 matches TREG5\n");
		set_irq_line(cpustate, INTT5, 1);
		if (cpustate->internal_registers[ T90_T4MOD - T90_IOBASE ] & 0x04)
			cpustate->timer4_value = 0;
	}

	// Overflow

	if ( cpustate->timer_value == 0 )
	{
//      logerror("CPU Timer 4 overflow\n");
	}
}

static WRITE8_HANDLER( t90_internal_registers_w )
{
	#define WIO		cpustate->io->write_byte( T90_IOBASE+offset, data )

	t90_Regs *cpustate = get_safe_token(&space->device());
	UINT8 out_mask;
	UINT8 old = cpustate->internal_registers[offset];
	switch ( T90_IOBASE + offset )
	{
		case T90_TRUN:
		{
			int i;
			// Timers 0-3
			for (i = 0; i < 4; i++)
			{
				if ( (old ^ data) & (0x20 | (1 << i)) )	// if timer bit or prescaler bit changed
				{
					if ( data == (0x20 | (1 << i)) )	t90_start_timer(cpustate, i);
					else								t90_stop_timer(cpustate, i);
				}
			}
			// Timer 4
			if ( (old ^ data) & (0x20 | 0x10) )
			{
				if ( data == (0x20 | 0x10) )	t90_start_timer4(cpustate);
				else							t90_stop_timer4(cpustate);
			}
			break;
		}

		case T90_INTEL:
			cpustate->irq_mask	&=	~(	(1 << INTT2 ) |
									(1 << INTT3 ) |
									(1 << INTT4 ) |
									(1 << INT1  ) |
									(1 << INTT5 ) |
									(1 << INT2  ) |
									(1 << INTRX ) |
									(1 << INTTX )	);

			cpustate->irq_mask	|=	((data & 0x80) ? (1 << INTT2 ) : 0) |
								((data & 0x40) ? (1 << INTT3 ) : 0) |
								((data & 0x20) ? (1 << INTT4 ) : 0) |
								((data & 0x10) ? (1 << INT1  ) : 0) |
								((data & 0x08) ? (1 << INTT5 ) : 0) |
								((data & 0x04) ? (1 << INT2  ) : 0) |
								((data & 0x02) ? (1 << INTRX ) : 0) |
								((data & 0x01) ? (1 << INTTX ) : 0) ;
			break;

		case T90_INTEH:
			cpustate->irq_mask	&=	~(	(1 << INT0 ) |
									(1 << INTT0) |
									(1 << INTT1)	);

			cpustate->irq_mask	|=	((data & 0x04) ? (1 << INT0 ) : 0) |
								((data & 0x02) ? (1 << INTT0) : 0) |
								((data & 0x01) ? (1 << INTT1) : 0) ;
			break;

		case T90_P3:
			data &= 0x6c;
			WIO;
			break;

		case T90_P4:
			data &= 0x0f;
			out_mask = (~cpustate->internal_registers[ T90_P4CR - T90_IOBASE ]) & 0x0f;
			if (out_mask)
			{
				data &= out_mask;
				WIO;
			}
			break;

		case T90_P6:
			out_mask = cpustate->internal_registers[ T90_P67CR - T90_IOBASE ] & 0x0f;
			switch (cpustate->internal_registers[ T90_SMMOD - T90_IOBASE ] & 0x03)
			{
				case 1:
					data &= ~0x01;
					// add TO1 here
					break;
				case 2:
				case 3:
					data &= ~0x0f;
					// add M0 here
					break;
			}

			if (out_mask)
			{
				data &= out_mask;
				WIO;
			}
			break;

		case T90_P7:
			out_mask = (cpustate->internal_registers[ T90_P67CR - T90_IOBASE ] & 0xf0) >> 4;
			switch ((cpustate->internal_registers[ T90_SMMOD - T90_IOBASE ]>>4) & 0x03)
			{
				case 1:
					data &= ~0x01;
					// add TO3 here
					break;
				case 2:
				case 3:
					data &= ~0x0f;
					// add M1 here
					break;
			}

			if (out_mask)
			{
				data &= out_mask;
				WIO;
			}
			break;

		case T90_P8:
			data &= 0x0f;
			out_mask = (~cpustate->internal_registers[ T90_P8CR - T90_IOBASE ]) & 0x08;
			if (out_mask)
			{
				data &= out_mask;
				WIO;
			}
			break;

		case T90_BX:
			cpustate->ixbase = (data & 0xf) << 16;
			break;
		case T90_BY:
			cpustate->iybase = (data & 0xf) << 16;
			break;
	}
	cpustate->internal_registers[offset] = data;
}

static void state_register( legacy_cpu_device *device )
{
	t90_Regs *cpustate = get_safe_token(device);

	device->save_item(NAME(cpustate->prvpc.w.l));
	device->save_item(NAME(cpustate->pc.w.l));
	device->save_item(NAME(cpustate->sp.w.l));
	device->save_item(NAME(cpustate->af.w.l));
	device->save_item(NAME(cpustate->bc.w.l));
	device->save_item(NAME(cpustate->de.w.l));
	device->save_item(NAME(cpustate->hl.w.l));
	device->save_item(NAME(cpustate->ix.w.l));
	device->save_item(NAME(cpustate->iy.w.l));
	device->save_item(NAME(cpustate->af2.w.l));
	device->save_item(NAME(cpustate->bc2.w.l));
	device->save_item(NAME(cpustate->de2.w.l));
	device->save_item(NAME(cpustate->hl2.w.l));
	device->save_item(NAME(cpustate->halt));
	device->save_item(NAME(cpustate->after_EI));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->irq_mask));
	device->save_item(NAME(cpustate->icount));
	device->save_item(NAME(cpustate->extra_cycles));

	device->save_item(NAME(cpustate->internal_registers));
	device->save_item(NAME(cpustate->ixbase));
	device->save_item(NAME(cpustate->iybase));

	device->save_item(NAME(cpustate->timer_value));
	device->save_item(NAME(cpustate->timer4_value));

	// Work registers
	device->save_item(NAME(cpustate->op));
	device->save_item(NAME(cpustate->mode1));
	device->save_item(NAME(cpustate->r1));
	device->save_item(NAME(cpustate->r1b));
	device->save_item(NAME(cpustate->mode2));
	device->save_item(NAME(cpustate->r2));
	device->save_item(NAME(cpustate->r2b));

	device->save_item(NAME(cpustate->cyc_t));
	device->save_item(NAME(cpustate->cyc_f));
	device->save_item(NAME(cpustate->addr));
}


static CPU_INIT( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);
	int i, p;

	state_register(device);

	for (i = 0; i < 256; i++)
	{
		p = 0;
		if( i&0x01 ) ++p;
		if( i&0x02 ) ++p;
		if( i&0x04 ) ++p;
		if( i&0x08 ) ++p;
		if( i&0x10 ) ++p;
		if( i&0x20 ) ++p;
		if( i&0x40 ) ++p;
		if( i&0x80 ) ++p;
		SZ[i] = i ? i & SF : ZF;
//      SZ[i] |= (i & (YF | XF));       /* undocumented flag bits 5+3 */
		SZ_BIT[i] = i ? i & SF : ZF | PF;
//      SZ_BIT[i] |= (i & (YF | XF));   /* undocumented flag bits 5+3 */
		SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
		SZHV_inc[i] = SZ[i];
		if( i == 0x80 ) SZHV_inc[i] |= VF;
		if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
		SZHV_dec[i] = SZ[i] | NF;
		if( i == 0x7f ) SZHV_dec[i] |= VF;
		if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
	}

	memset(cpustate, 0, sizeof(t90_Regs));
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);

	cpustate->timer_period = attotime::from_hz(device->unscaled_clock()) * 8;

	// Reset registers to their initial values

//  IX = IY = 0xffff;
//  F = ZF;

	// Timers

	for (i = 0; i < 4; i++)
		cpustate->timer[i] = device->machine().scheduler().timer_alloc(FUNC(t90_timer_callback), cpustate);

	cpustate->timer[4] = device->machine().scheduler().timer_alloc(FUNC(t90_timer4_callback), cpustate);
}

static ADDRESS_MAP_START(tmp90840_mem, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(	0x0000,		0x1fff			)	AM_ROM	// 8KB ROM (internal)
	AM_RANGE(	0xfec0,		0xffc0			)	AM_RAM	// 256b RAM (internal)
	AM_RANGE(	T90_IOBASE,	T90_IOBASE+47	)	AM_READWRITE( t90_internal_registers_r, t90_internal_registers_w )
ADDRESS_MAP_END
static ADDRESS_MAP_START(tmp90841_mem, AS_PROGRAM, 8, legacy_cpu_device)
//  AM_RANGE(   0x0000,     0x1fff          )   AM_ROM  // rom-less
	AM_RANGE(	0xfec0,		0xffc0			)	AM_RAM	// 256b RAM (internal)
	AM_RANGE(	T90_IOBASE,	T90_IOBASE+47	)	AM_READWRITE( t90_internal_registers_r, t90_internal_registers_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tmp91640_mem, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE(	0x0000,		0x3fff			) AM_ROM	// 16KB ROM (internal)
	AM_RANGE(	0xfdc0,		0xffc0			) AM_RAM	// 512b RAM (internal)
	AM_RANGE(	T90_IOBASE, T90_IOBASE+47	) AM_READWRITE( t90_internal_registers_r, t90_internal_registers_w )
ADDRESS_MAP_END
static ADDRESS_MAP_START(tmp91641_mem, AS_PROGRAM, 8, legacy_cpu_device )
//  AM_RANGE(   0x0000,     0x3fff          ) AM_ROM    // rom-less
	AM_RANGE(	0xfdc0,		0xffc0			) AM_RAM	// 512b RAM (internal)
	AM_RANGE(	T90_IOBASE, T90_IOBASE+47	) AM_READWRITE( t90_internal_registers_r, t90_internal_registers_w )
ADDRESS_MAP_END

static CPU_SET_INFO( t90 )
{
	t90_Regs *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(cpustate, INTNMI, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:		set_irq_line(cpustate, INT0,   info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ1:		set_irq_line(cpustate, INT1,   info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ2:		set_irq_line(cpustate, INT2,   info->i);				break;

		case CPUINFO_INT_PC:								cpustate->pc.d = info->i;			break;
		case CPUINFO_INT_REGISTER + T90_PC:					cpustate->pc.w.l = info->i;						break;
		case CPUINFO_INT_SP:								cpustate->sp.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_SP:					cpustate->sp.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_A:					cpustate->af.b.h = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_B:					cpustate->bc.b.h = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_C:					cpustate->bc.b.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_D:					cpustate->de.b.h = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_E:					cpustate->de.b.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_H:					cpustate->hl.b.h = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_L:					cpustate->hl.b.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_AF:					cpustate->af.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_BC:					cpustate->bc.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_DE:					cpustate->de.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_HL:					cpustate->hl.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_IX:					cpustate->ix.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + T90_IY:					cpustate->iy.w.l = info->i;						break;
	}
}

CPU_GET_INFO( tmp90840 )
{
	t90_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		case CPUINFO_INT_CONTEXT_SIZE:								info->i = sizeof(t90_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:								info->i = 1;					break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:						info->i = 0xff;					break;
		case DEVINFO_INT_ENDIANNESS:								info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:							info->i = 1;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:								info->i = 1;					break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:						info->i = 1;					break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:						info->i = 6;					break;
		case CPUINFO_INT_MIN_CYCLES:								info->i = 2;					break;
		case CPUINFO_INT_MAX_CYCLES:								info->i = 26;					break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;					break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:		info->i = 0;					break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:			info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:			info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:			info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:				info->i = cpustate->irq_state & (1 << INTNMI);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:				info->i = cpustate->irq_state & (1 << INT0);		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ1:				info->i = cpustate->irq_state & (1 << INT1);		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ2:				info->i = cpustate->irq_state & (1 << INT2);		break;

		case CPUINFO_INT_PREVIOUSPC:								info->i = cpustate->prvpc.w.l;		break;
		case CPUINFO_INT_PC:										info->i = cpustate->pc.d;				break;
		case CPUINFO_INT_REGISTER + T90_PC:							info->i = cpustate->pc.w.l;			break;
		case CPUINFO_INT_SP:										info->i = cpustate->sp.d;				break;
		case CPUINFO_INT_REGISTER + T90_SP:							info->i = cpustate->sp.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_A:							info->i = cpustate->af.b.h;			break;
		case CPUINFO_INT_REGISTER + T90_B:							info->i = cpustate->bc.b.h;			break;
		case CPUINFO_INT_REGISTER + T90_C:							info->i = cpustate->bc.b.l;			break;
		case CPUINFO_INT_REGISTER + T90_D:							info->i = cpustate->de.b.h;			break;
		case CPUINFO_INT_REGISTER + T90_E:							info->i = cpustate->de.b.l;			break;
		case CPUINFO_INT_REGISTER + T90_H:							info->i = cpustate->hl.b.h;			break;
		case CPUINFO_INT_REGISTER + T90_L:							info->i = cpustate->hl.b.l;			break;
		case CPUINFO_INT_REGISTER + T90_AF:							info->i = cpustate->af.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_BC:							info->i = cpustate->bc.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_DE:							info->i = cpustate->de.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_HL:							info->i = cpustate->hl.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_IX:							info->i = cpustate->ix.w.l;			break;
		case CPUINFO_INT_REGISTER + T90_IY:							info->i = cpustate->iy.w.l;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */

		case CPUINFO_FCT_SET_INFO:									info->setinfo = CPU_SET_INFO_NAME(t90);		break;
		case CPUINFO_FCT_INIT:										info->init = CPU_INIT_NAME(t90);				break;
		case CPUINFO_FCT_RESET:										info->reset = CPU_RESET_NAME(t90);			break;
		case CPUINFO_FCT_EXIT:										info->exit = CPU_EXIT_NAME(t90);				break;
		case CPUINFO_FCT_EXECUTE:									info->execute = CPU_EXECUTE_NAME(t90);		break;
		case CPUINFO_FCT_BURN:										info->burn = CPU_BURN_NAME(t90);				break;
		case CPUINFO_FCT_DISASSEMBLE:								info->disassemble = CPU_DISASSEMBLE_NAME(t90);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:						info->icount = &cpustate->icount;			break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(tmp90840_mem); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */

		case DEVINFO_STR_NAME:				strcpy(info->s, "TMP90840");			break;
		case DEVINFO_STR_FAMILY:		strcpy(info->s, "Toshiba TLCS-90");	break;
		case DEVINFO_STR_VERSION:		strcpy(info->s, "1.0");				break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);			break;
		case DEVINFO_STR_CREDITS:		strcpy(info->s, "Luca Elia");			break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				F & 0x80 ? 'S':'.',
				F & 0x40 ? 'Z':'.',
				F & 0x20 ? 'I':'.',
				F & 0x10 ? 'H':'.',
				F & 0x08 ? 'X':'.',
				F & 0x04 ? 'P':'.',
				F & 0x02 ? 'N':'.',
				F & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + T90_PC:		sprintf(info->s, "PC:%04X", cpustate->pc.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_SP:		sprintf(info->s, "SP:%04X", cpustate->sp.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_A:		sprintf(info->s, "~A:%02X", cpustate->af.b.h);	break;
		case CPUINFO_STR_REGISTER + T90_B:		sprintf(info->s, "~B:%02X", cpustate->bc.b.h);	break;
		case CPUINFO_STR_REGISTER + T90_C:		sprintf(info->s, "~C:%02X", cpustate->bc.b.l);	break;
		case CPUINFO_STR_REGISTER + T90_D:		sprintf(info->s, "~D:%02X", cpustate->de.b.h);	break;
		case CPUINFO_STR_REGISTER + T90_E:		sprintf(info->s, "~E:%02X", cpustate->de.b.l);	break;
		case CPUINFO_STR_REGISTER + T90_H:		sprintf(info->s, "~H:%02X", cpustate->hl.b.h);	break;
		case CPUINFO_STR_REGISTER + T90_L:		sprintf(info->s, "~L:%02X", cpustate->hl.b.l);	break;
		case CPUINFO_STR_REGISTER + T90_AF:		sprintf(info->s, "AF:%04X", cpustate->af.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_BC:		sprintf(info->s, "BC:%04X", cpustate->bc.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_DE:		sprintf(info->s, "DE:%04X", cpustate->de.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_HL:		sprintf(info->s, "HL:%04X", cpustate->hl.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_IX:		sprintf(info->s, "IX:%04X", cpustate->ix.w.l);	break;
		case CPUINFO_STR_REGISTER + T90_IY:		sprintf(info->s, "IY:%04X", cpustate->iy.w.l);	break;
	}
}

CPU_GET_INFO( tmp90841 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(tmp90841_mem); return;

		/* --- the following bits of info are returned as NULL-terminated strings --- */

		case DEVINFO_STR_NAME:				strcpy(info->s, "TMP90841");			return;
	}

	CPU_GET_INFO_CALL(tmp90840);
}

CPU_GET_INFO( tmp91640 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(tmp91640_mem); return;

		/* --- the following bits of info are returned as NULL-terminated strings --- */

		case DEVINFO_STR_NAME:				strcpy(info->s, "TMP91640");			return;
	}

	CPU_GET_INFO_CALL(tmp90840);
}

CPU_GET_INFO( tmp91641 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(tmp91641_mem); return;

		/* --- the following bits of info are returned as NULL-terminated strings --- */

		case DEVINFO_STR_NAME:				strcpy(info->s, "TMP91641");			return;
	}

	CPU_GET_INFO_CALL(tmp90840);
}

DEFINE_LEGACY_CPU_DEVICE(TMP90840, tmp90840);
DEFINE_LEGACY_CPU_DEVICE(TMP90841, tmp90841);
DEFINE_LEGACY_CPU_DEVICE(TMP91640, tmp91640);
DEFINE_LEGACY_CPU_DEVICE(TMP91641, tmp91641);
