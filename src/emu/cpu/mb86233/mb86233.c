/***************************************************************************

    mb86233.c
    Core implementation for the portable Fujitsu MB86233 series DSP emulator.

    Written by ElSemi
    MAME version by Ernesto Corvi

    TODO:
    - Properly emulate the TGP Tables from the ROM (See GETEXTERNAL)
    - Many unknown opcodes and addressing modes
    - Interrupts?

***************************************************************************/

#include "mb86233.h"
#include "deprecat.h"
#include "debugger.h"

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

typedef union
{
	INT32	i;
	UINT32	u;
	float	f;
} MB86233_REG;

typedef struct
{
	UINT16			pc;
	MB86233_REG		a;
	MB86233_REG		b;
	MB86233_REG		d;
	MB86233_REG		p;

	UINT16			reps;
	UINT16			pcs[4];
	UINT8			pcsp;
	UINT32			eb;
	UINT32			shift;
	UINT32			repcnt;
	UINT16			sr;

	UINT32			gpr[16];
	UINT32			extport[0x30];

	/* FIFO */
	int				fifo_wait;
	int				(*fifo_read_cb)( UINT32 *data );
	void			(*fifo_write_cb)( UINT32 data );

	/* internal RAM */
	UINT32			*RAM;
	UINT32			*ARAM, *BRAM;
} MB86233_REGS;

/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static MB86233_REGS mb86233;
static int mb86233_icount;

/***************************************************************************
    MACROS
***************************************************************************/

#define GETPC()				mb86233.pc
#define GETA()				mb86233.a
#define GETB()				mb86233.b
#define GETD()				mb86233.d
#define GETP()				mb86233.p
#define GETSR()				mb86233.sr
#define GETGPR(a)			mb86233.gpr[a]
#define GETSHIFT()			mb86233.shift
#define GETPCS()			mb86233.pcs
#define GETPCSP()			mb86233.pcsp
#define GETEB()				mb86233.eb
#define GETREPS()			mb86233.reps
#define GETEXTPORT()		mb86233.extport
#define GETFIFOWAIT()		mb86233.fifo_wait
#define GETARAM()			mb86233.ARAM
#define GETBRAM()			mb86233.BRAM
#define ALU(a)				mb86233_alu(a)
#define GETREPCNT()			mb86233.repcnt

#define ROPCODE(a)			cpu_readop32(a<<2)
#define RDMEM(a)			program_read_dword_32le((a<<2))
#define WRMEM(a,v)			program_write_dword_32le((a<<2),v)

/***************************************************************************
    Context Switching
***************************************************************************/

static void mb86233_get_context(void *dst)
{
	/* copy the context */
	*(MB86233_REGS *)dst = mb86233;
}

static void mb86233_set_context(void *src)
{
	/* copy the context */
	if (src)
		mb86233 = *(MB86233_REGS *)src;
	change_pc(GETPC());
}


/***************************************************************************
    Initialization and Shutdown
***************************************************************************/

static void mb86233_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	struct mb86233_config * _config = (struct mb86233_config *)config;
	(void)index;
	(void)clock;
	(void)irqcallback;

	memset(&mb86233, 0, sizeof( MB86233_REGS ) );

	if ( _config )
	{
		mb86233.fifo_read_cb = _config->fifo_read_cb;
		mb86233.fifo_write_cb = _config->fifo_write_cb;
	}

	mb86233.RAM = auto_malloc(2 * 0x200 * sizeof(UINT32));		/* 2x 2KB */
	memset( mb86233.RAM, 0, 2 * 0x200 * sizeof(UINT32) );
	mb86233.ARAM = &mb86233.RAM[0];
	mb86233.BRAM = &mb86233.RAM[0x200];

	state_save_register_global_pointer(mb86233.RAM,2 * 0x200 * sizeof(UINT32));
}

static void mb86233_reset(void)
{
	/* zero registers and flags */
	mb86233.pc = 0;
	mb86233.sr = 0;
	mb86233.pcsp = 0;
	mb86233.eb = 0;
	mb86233.shift = 0;
	mb86233.fifo_wait = 0;
}



/***************************************************************************
    Status Register
***************************************************************************/

#define ZERO_FLAG	(1 << 0)
#define SIGN_FLAG	(1 << 1)

static void FLAGSF( float v )
{
	GETSR() = 0;

	if ( v == 0 )
		GETSR() |= ZERO_FLAG;

	if ( v < 0 )
		GETSR() |= SIGN_FLAG;
}

static void FLAGSI( UINT32 v )
{
	GETSR() = 0;

	if ( v == 0 )
		GETSR() |= ZERO_FLAG;

	if ( v & 0x80000000 )
		GETSR() |= SIGN_FLAG;
}



/***************************************************************************
    Condition Codes
***************************************************************************/

static int COND( UINT32 cond )
{
	switch( cond )
	{
		case 0x00:	/* eq */
			if ( (GETSR() & ZERO_FLAG) ) return 1;
		break;

		case 0x01:	/* ge */
			if ( (GETSR() & ZERO_FLAG) || ((GETSR() & SIGN_FLAG)==0) ) return 1;
		break;

		case 0x02:	/* le */
			if ( (GETSR() & ZERO_FLAG) || (GETSR() & SIGN_FLAG) ) return 1;
		break;

		case 0x06:	/* never */
		break;

		case 0x10:	/* --r12 != 0 */
			GETGPR(12)--;
			if ( GETGPR(12) != 0 ) return 1;
		break;

		case 0x11:	/* --r13 != 0 */
			GETGPR(13)--;
			if ( GETGPR(13) != 0 ) return 1;
		break;

		case 0x16:	/* always */
			return 1;
		break;

		default:
			logerror( "TGP: Unknown condition code (cc=%d) at PC:%x\n", cond, GETPC());
		break;
	}

	return 0;
}



/***************************************************************************
    ALU
***************************************************************************/

static void ALU( UINT32 alu)
{
	float	ftmp;

	switch(alu)
	{
		case 0x00:	/* NOP */
		break;

		case 0x01:	/* D = D & A */
			GETD().u &= GETA().u;
			FLAGSI(GETD().u);
		break;

		case 0x02:	/* D = D | A */
			GETD().u |= GETA().u;
			FLAGSI(GETD().u);
		break;

		case 0x03:	/* D = D ^ A */
			GETD().u ^= GETA().u;
			FLAGSI(GETD().u);
		break;

		case 0x05:	/* CMP D,A */
			ftmp = GETD().f - GETA().f;
			FLAGSF(ftmp);
			mb86233_icount--;
		break;

		case 0x06:	/* D = D + A */
			GETD().f += GETA().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x07:	/* D = D - A */
			GETD().f -= GETA().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x08:	/* P = A * B */
			GETP().f = GETA().f * GETB().f;
			mb86233_icount--;
		break;

		case 0x09:	/* D = D + P; P = A * B */
			GETD().f += GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0A:	/* D = D - P; P = A * B */
			GETD().f -= GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0B:	/* D = fabs(D) */
			GETD().f = fabs( GETD().f );
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0C:	/* D = D + P */
			GETD().f += GETP().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0D:	/* D = P; P = A * B */
			GETD().f = GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0E:	/* D = float(D) */
			GETD().f = (float)GETD().i;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x0F:	/* D = int(D) */
			GETD().i = (INT32)GETD().f;
			FLAGSI(GETD().u);
		break;

		case 0x10:	/* D = D / A */
			if ( GETA().u != 0 )
				GETD().f = GETD().f / GETA().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x11:	/* D = -D */
			GETD().f = -GETD().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x13:	/* D = A + B */
			GETD().f = GETA().f + GETB().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x14:	/* D = B - A */
			GETD().f = GETB().f - GETA().f;
			FLAGSF(GETD().f);
			mb86233_icount--;
		break;

		case 0x16:	/* LSR D, SHIFT */
			GETD().u >>= GETSHIFT();
			FLAGSI(GETD().u);
		break;

		case 0x17:	/* LSL D, SHIFT */
			GETD().u <<= GETSHIFT();
			FLAGSI(GETD().u);
		break;

		case 0x18:	/* ASR D, SHIFT */
//          GETD().u = (GETD().u & 0x80000000) | (GETD().u >> GETSHIFT());
			GETD().i >>= GETSHIFT();
			FLAGSI(GETD().u);
		break;

		case 0x1A:	/* D = D + A */
			GETD().i += GETA().i;
			FLAGSI(GETD().u);
		break;

		case 0x1B:	/* D = D - A */
			GETD().i -= GETA().i;
			FLAGSI(GETD().u);
		break;

		default:
			logerror( "TGP: Unknown ALU op %x at PC:%04x\n", alu, GETPC() );
		break;
	}
}



/***************************************************************************
    Memory Access
***************************************************************************/

static UINT32 GETEXTERNAL( UINT32 EB, UINT32 offset )
{
	UINT32		addr;

	if ( EB == 0 && offset >= 0x20 && offset <= 0x2f )	/* TGP Tables in ROM - FIXME - */
	{
		if ( offset == 0x20 )	/* SIN from value at RAM(0x20) in 0x4000/PI steps */
		{
			MB86233_REG		r;
			INT32			a;

			r.u = GETEXTPORT()[0x20];
			a = r.i;
			if ( a == 0 || a == -32768 )
				r.f = 0;
			else if ( a == 16384 )
				r.f = 1.0;
			else if ( a == -16384 )
				r.f=-1.0;
			else
				r.f = (float)sin((float)a*(2.0*M_PI/65536.0));

			return r.u;
		}

		if ( offset == 0x21 )	/* COS from value at RAM(0x20) in 0x4000/PI steps */
		{
			MB86233_REG		r;
			INT32			a;

			r.u = GETEXTPORT()[0x20];
			a = r.i;
			if ( a == 16384 || a == -16384 )
				r.f = 0;
			else if ( a == 0 )
				r.f = 1.0;
			else if ( a == -32768 )
				r.f = -1.0;
			else
				r.f = (float)cos((float)a*(2.0*M_PI/65536.0));

			return r.u;
		}

#if 0
		if ( offset == 0x22 )	/* -SIN from value at RAM(0x20) in 0x4000/PI steps */
		{
			MB86233_REG		r;
			INT32			a;

			r.u = GETEXTPORT()[0x20];
			a = r.i;
			r.f = (float)-sin((float)a*(2.0*M_PI/65536.0));
			return r.u;
		}
#endif

		if ( offset == 0x27 )	/* atan */
		{
			MB86233_REG		r1, r2;

			r1.u = GETEXTPORT()[0x24]; /* A */
			r2.u = GETEXTPORT()[0x25]; /* B */

			if ( r2.f == 0 )
			{
				if ( r1.f >= 0 )
					GETEXTPORT()[0x27] = 0;
				else
					GETEXTPORT()[0x27] = 0xFFFF8000;
			}
			else if ( r1.f == 0 )
			{
				if ( r2.f >= 0 )
					GETEXTPORT()[0x27] = 0x4000;
				else
					GETEXTPORT()[0x27] = 0xFFFFC000;
			}
			else
			{
				r1.i = (INT32)((atan2(r2.f,r1.f))*32768.0/M_PI);
				GETEXTPORT()[0x27] = r1.u;
			}

			return GETEXTPORT()[0x27];
		}

		if ( offset == 0x28 )	/* 1/x - part 1 */
			return 0;

		if ( offset == 0x29 )	/* 1/x - part 2 */
		{
			MB86233_REG		r;
			r.u = GETEXTPORT()[0x28];
			if ( r.f==0 )
					r.u = 0x80000000;
			r.f = 1/r.f;
			return r.u;
		}

		if ( offset == 0x2a )	/* 1/sqrt(x) - part 1 */
			return 0;

		if ( offset == 0x2b)	/* 1/sqrt(x) - part 2 */
		{
			MB86233_REG		r;
			r.u = GETEXTPORT()[0x2a];
			if ( r.f == 0 )
				r.u=0x80000000;
			else
				r.f = 1/sqrt(r.f);
			return r.u;
		}

		return GETEXTPORT()[offset];
	}

	addr = ( EB & 0xFFFF0000 ) | ( offset & 0xFFFF );

	return RDMEM(addr);
}

static void SETEXTERNAL( UINT32 EB, UINT32 offset, UINT32 value )
{
	UINT32	addr;

	if ( EB == 0 && offset >= 0x20 && offset <= 0x2f )	/* TGP Tables in ROM - FIXME - */
	{
		GETEXTPORT()[offset] = value;
		return;
	}

	addr = ( EB & 0xFFFF0000 ) | ( offset & 0xFFFF );

	WRMEM( addr, value );
}



/***************************************************************************
    Register Access
***************************************************************************/

static UINT32 GETREGS( UINT32 reg, int source )
{
	UINT32	mode = ( reg >> 6 ) & 0x07;

	reg &= 0x3f;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		if ( reg < 0x10 )
		{
			return GETGPR(reg);
		}

		switch( reg )
		{
			case 0x10:	/* A */
				return GETA().u;
			break;

			case 0x11:	/* A.e */
				return (GETA().u >> 23) & 0xff;
			break;

			case 0x12:	/* A.m */
				return (GETA().u & 0x7fffff) | ((GETA().u&0x80000000) >> 8);
			break;

			case 0x13:	/* B */
				return GETB().u;
			break;

			case 0x14:	/* B.e */
				return (GETB().u >> 23) & 0xff;
			break;

			case 0x15:	/* B.m */
				return (GETB().u & 0x7fffff) | ((GETB().u&0x80000000) >> 8);
			break;

			case 0x19:	/* D */
				return GETD().u;
			break;

			case 0x1A:	/* D.e */
				return (GETD().u >> 23) & 0xff;
			break;

			case 0x1B:	/* D.m */
				return (GETD().u & 0x7fffff) | ((GETD().u&0x80000000) >> 8);
			break;

			case 0x1C:	/* P */
				return GETP().u;
			break;

			case 0x1D:	/* P.e */
				return (GETP().u >> 23) & 0xff;
			break;

			case 0x1E:	/* P.m */
				return (GETP().u & 0x7fffff) | ((GETP().u&0x80000000) >> 8);
			break;

			case 0x1F:	/* Shift */
				return GETSHIFT();
			break;

			case 0x20:	/* Parallel Port */
				logerror( "TGP: Parallel port read at PC:%04x\n", GETPC() );
				return 0;
			break;

			case 0x21:	/* FIn */
			{
				UINT32	fifo_data;

				if ( mb86233.fifo_read_cb )
				{
					if ( mb86233.fifo_read_cb(&fifo_data) )
					{
						return fifo_data;
					}
				}

				GETFIFOWAIT() = 1;
				return 0;
			}
			break;

			case 0x22:	/* FOut */
				return 0;
			break;

			case 0x23:	/* EB */
				return GETEB();
			break;

			case 0x34:
				return GETREPCNT();
			break;

			default:
				logerror( "TGP: Unknown GETREG (%d) at PC=%04x\n", reg, GETPC() );
			break;
		}
	}
	else if ( mode == 2 )	/* Indexed */
	{
		UINT32	addr = reg & 0x1f;

		if ( source )
		{
			if( !( reg & 0x20 ) )
				addr += GETGPR(0);

			addr += GETGPR(2);
		}
		else
		{
			if( !( reg & 0x20 ) )
				addr += GETGPR(1);

			addr += GETGPR(3);
		}

		return addr;
	}
	else if( mode == 6 )	/* Indexed with postop */
	{
		UINT32	addr = 0;

		if ( source )
		{
			if( !( reg & 0x20 ) )
				addr += GETGPR(0);

			addr += GETGPR(2);
		}
		else
		{
			if( !( reg & 0x20 ) )
				addr += GETGPR(1);

			addr += GETGPR(3);
		}

		if ( reg & 0x10 )
		{
			if ( source )
				GETGPR(2) -= 0x20 - ( reg & 0x1f );
			else
				GETGPR(3) -= 0x20 - ( reg & 0x1f );
		}
		else
		{
			if ( source )
				GETGPR(2) += ( reg & 0x1f );
			else
				GETGPR(3) += ( reg & 0x1f );
		}

		return addr;
	}
	else
	{
		fatalerror( "TGP: Unknown GETREG mode %d at PC:%04x\n", mode, GETPC() );
	}

	return 0;
}

static void SETREGS( UINT32 reg, UINT32 val )
{
	int		mode = ( reg >> 6) & 0x07;

	reg &= 0x3f;

	if( mode == 0 || mode == 1 || mode == 3 )
	{
		if ( reg < 0x10 )
		{
			if ( reg == 12 || reg == 13 )
				val &= 0xff;

			GETGPR(reg) = val;
			return;
		}

		switch( reg )
		{
			case 0x10:	/* A */
				GETA().u = val;
			break;

			case 0x11:	/* A.e */
				GETA().u &= ~((0x0000007f) << 23);
				GETA().u |= (( val & 0xff ) << 23 );
			break;

			case 0x12:	/* A.m */
				GETA().u &= ~( 0x807fffff );
				GETA().u |= ( val & 0x7fffff );
				GETA().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x13:	/* B */
				GETB().u = val;
			break;

			case 0x14:	/* B.e */
				GETB().u &= ~((0x0000007f) << 23);
				GETB().u |= (( val & 0xff ) << 23 );
			break;

			case 0x15:	/* B.m */
				GETB().u &= ~( 0x807fffff );
				GETB().u |= ( val & 0x7fffff );
				GETB().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x19:	/* D */
				GETD().u = val;
			break;

			case 0x1A:	/* D.e */
				GETD().u &= ~((0x0000007f) << 23);
				GETD().u |= (( val & 0xff ) << 23 );
			break;

			case 0x1B:	/* B.m */
				GETD().u &= ~( 0x807fffff );
				GETD().u |= ( val & 0x7fffff );
				GETD().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x1C:	/* P */
				GETP().u = val;
			break;

			case 0x1D:	/* P.e */
				GETP().u &= ~((0x0000007f) << 23);
				GETP().u |= (( val & 0xff ) << 23 );
			break;

			case 0x1E:	/* P.m */
				GETP().u &= ~( 0x807fffff );
				GETP().u |= ( val & 0x7fffff );
				GETP().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x1F:
				GETSHIFT() = val;
			break;

			case 0x20: /* Parallel Port */
				logerror( "TGP: Parallel port write: %08x at PC:%04x\n", val, GETPC() );
			break;

			case 0x22: /* FOut */
				if ( mb86233.fifo_write_cb )
				{
					mb86233.fifo_write_cb( val );
				}
			break;

			case 0x23:
				GETEB() = val;
			break;

			case 0x34:
				GETREPCNT() = val;
			break;

			default:
			{
				fatalerror( "TGP: Unknown register write (r:%d, mode:%d) at PC:%04x\n", reg, mode, GETPC());
			}
			break;
		}
	}
	else
	{
		fatalerror( "TGP: Unknown register write (r:%d, mode:%d) at PC:%04x\n", reg, mode, GETPC());
	}
}


/***************************************************************************
    Addressing Modes
***************************************************************************/

static UINT32 INDIRECT( UINT32 reg, int source )
{
	UINT32	mode = ( reg >> 6 ) & 0x07;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		return reg;
	}
	else if ( mode == 2 )
	{
		UINT32	addr = reg & 0x1f;

		if ( source )
		{
			if( !(reg & 0x20) )
				addr += GETGPR(0);

			addr += GETGPR(2);
		}
		else
		{
			if( !(reg & 0x20) )
				addr += GETGPR(1);

			addr += GETGPR(3);
		}

		return addr;
	}
	else if ( mode == 6 || mode == 7 )
	{
		UINT32	addr = 0;

		if ( source )
		{
			if ( !( reg & 0x20 ) )
				addr += GETGPR(0);

			addr += GETGPR(2);
		}
		else
		{
			if ( !( reg & 0x20 ) )
				addr += GETGPR(1);

			addr += GETGPR(3);
		}

		if ( reg & 0x10 )
		{
			if ( source )
				GETGPR(2) -= 0x20 - ( reg & 0x1f );
			else
				GETGPR(3) -= 0x20 - ( reg & 0x1f );
		}
		else
		{
			if ( source )
				GETGPR(2) += ( reg & 0x1f );
			else
				GETGPR(3) += ( reg & 0x1f );
		}

		return addr;
	}
	else
	{
		fatalerror( "TGP: Unknown INDIRECT mode %d at PC:%04x\n", mode, GETPC() );
	}

	return 0;
}

/***************************************************************************
    Core Execution Loop
***************************************************************************/

static int mb86233_execute(int cycles)
{

	mb86233_icount = cycles;

	while( mb86233_icount > 0 )
	{
		UINT32		val;
		UINT32		opcode;

		CALL_MAME_DEBUG;

		opcode = ROPCODE(GETPC());

		GETFIFOWAIT() = 0;

		switch( (opcode >> 26) & 0x3f )
		{
			case 0x00:	/* dual move */
			{
				UINT32		r1 = opcode & 0x1ff;
				UINT32		r2 = ( opcode >> 9 ) & 0x7f;
				UINT32		alu = ( opcode >> 21 ) & 0x1f;
				UINT32		op = ( opcode >> 16 ) & 0x1f;

				ALU( alu );

				switch( op )
				{
					case 0x0C:
						GETA().u = GETARAM()[r1];
						GETB().u = GETBRAM()[r2];
					break;

					case 0x0D:
						GETA().u = GETARAM()[INDIRECT(r1,0)];
						GETB().u = GETBRAM()[INDIRECT(r2|2<<6,0)];
					break;

					case 0x0F:
						GETA().u = GETARAM()[r1];
						GETB().u = GETBRAM()[INDIRECT(r2|6<<6,0)];
					break;

					case 0x10:
						GETA().u = GETBRAM()[INDIRECT(r1,1)];
						GETB().u = GETARAM()[r2];
					break;

					case 0x11:
						GETA().u = GETARAM()[INDIRECT(r1,1)];
						GETB().u = GETBRAM()[INDIRECT(r2|(2<<6),0)];
					break;

					default:
						logerror( "TGP: Unknown TGP double move (op=%d) at PC:%x\n", op, GETPC());
					break;
				}
			}
			break;

			case 0x7:	/* LD/MOV */
			{
				UINT32		r1 = opcode & 0x1ff;
				UINT32		r2 = ( opcode >> 9 ) & 0x7f;
				UINT32		alu = ( opcode >> 21 ) & 0x1f;
				UINT32		op = ( opcode >> 16 ) & 0x1f;

				switch( op )
				{
					case 0x04:	/* MOV RAM->External */
					{
						SETEXTERNAL(GETEB(), r2, GETARAM()[r1]);
						ALU(alu);
					}
					break;

					case 0x0c:	/* MOV RAM->BRAM */
					{
						GETBRAM()[r2] = GETARAM()[r1];
						ALU(alu);
					}
					break;

					case 0x1d:	/* MOV RAM->Reg */
					{
						if ( r1 & 0x180 )
						{
							val = GETARAM()[GETREGS(r1,0)];
						}
						else
						{
							val = GETARAM()[r1];
						}

						/* if we're waiting for data, don't complete the instruction */
						if ( GETFIFOWAIT() )
							break;

						ALU(alu);
						SETREGS(r2,val);
					}
					break;

					case 0x1c:  /* MOV Reg->RAMInd */
					{
						val = GETREGS(r2,1);

						/* if we're waiting for data, don't complete the instruction */
						if ( GETFIFOWAIT() )
							break;

						ALU(alu);

						if ( ( r2 >> 6 ) & 0x01)
						{
							SETEXTERNAL(GETEB(),INDIRECT(r1,0),val);
						}
						else
						{
							GETARAM()[INDIRECT(r1,0)] = val;
						}
					}
					break;

					case 0x1f:	/* MOV Reg->Reg */
					{
						if ( r1 == 0x10 && r2 == 0xf )
						{
							/* NOP */
							ALU(alu);
						}
						else
						{
							val = GETREGS(r1,1);

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							ALU(alu);
							SETREGS( r2, val );
						}
					}
					break;

					case 0x0f:	/* MOV RAMInd->BRAMInd */
					{
						val = GETARAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETBRAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					case 0x13:	/* MOV BRAMInd->RAMInd */
					{
						val = GETBRAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETARAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					case 0x10:	/* MOV RAMInd->RAM  */
					{
						val = GETBRAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETARAM()[r2] = val;
					}
					break;

					case 0x1e:	/* External->Reg */
					{
						UINT32	offset;

						if ( ( r2 >> 6 ) & 1 )
							offset = INDIRECT(r1,1);
						else
							offset = INDIRECT(r1,0);

						val = GETEXTERNAL(GETEB(),offset);
						ALU(alu);
						SETREGS(r2,val);
					}
					break;

					case 0x03:	/* RAM->External Ind */
					{
						val = GETARAM()[r1];
						ALU(alu);
						SETEXTERNAL(GETEB(),INDIRECT(r2|(6<<6),0),val);
					}
					break;

					case 0x07:	/* RAMInd->External */
					{
						val = GETARAM()[INDIRECT(r1,1)];
						ALU(alu);
						SETEXTERNAL(GETEB(),INDIRECT(r2|(6<<6),0),val);
					}
					break;

					case 0x08:	/* External->RAM */
					{
						val = GETEXTERNAL(GETEB(),INDIRECT(r1,1));
						ALU(alu);
						GETARAM()[r2] = val;
					}
					break;

					case 0x0b:	/* External->RAMInd */
					{
						val = GETEXTERNAL(GETEB(),INDIRECT(r1,1));
						ALU(alu);
						GETARAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					default:
						logerror( "TGP: Unknown TGP move (op=%d) at PC:%x\n", op, GETPC());
					break;
				}
			}
			break;

			case 0x0e:	/* LDIMM24 */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				/* sign extend 24->32 */
				if ( imm & 0x800000 )
					imm |= 0xFF000000;

				switch( sub )
				{
					case 0x00: /* P */
						GETP().u = imm;
					break;

					case 0x01: /* A */
						GETA().u = imm;
					break;

					case 0x02: /* B */
						GETB().u = imm;
					break;

					case 0x03: /* D */
						GETD().u = imm;
					break;
				}
			}
			break;

			case 0x0f:	/* REP/CLEAR/FLAGS */
			{
				UINT32		alu = ( opcode >> 20 ) & 0x1f;
				UINT32		sub2 = ( opcode >> 16 ) & 0x0f;

				ALU( alu );

				if( sub2 == 0x00 )			/* CLEAR reg */
				{
					UINT32	reg = opcode & 0x1f;

					switch( reg )
					{
						case 0x10:
							GETD().u = 0;
						break;

						case 0x08:
							GETB().u = 0;
						break;

						case 0x04:
							GETA().u = 0;
						break;
					}
				}
				else if ( sub2 == 0x04 )	/* REP xxx */
				{
					UINT32		sub3 = ( opcode >> 12 ) & 0x0f;

					if ( sub3 == 0 )
					{
						GETREPS() = opcode & 0xfff;

						if ( GETREPS() == 0 )
							GETREPS() = 0x100;

						GETPC()++;
					}
					else if ( sub3 == 8 )
					{
						GETREPS() = GETREGS( opcode & 0xfff, 0 );
						GETPC()++;
					}
				}
				else if ( sub2 == 0x02 )	/* CLRFLAGS */
				{
					GETSR() &= ~(opcode&0xfff);
				}
				else if ( sub2 == 0x06 )	/* SETFLAGS */
				{
					GETSR() |= (opcode&0xfff);
				}
			}
			break;

			case 0x10:	/* LDIMM rx */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffff;

				GETGPR(sub) = imm;
			}
			break;

			case 0x13:	/* LDIMM r1x */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 0 ) /* R12 */
					GETGPR(12) = imm & 0xff;
				else if ( sub == 1 ) /* R13 */
					GETGPR(13) = imm & 0xff;
				else
					logerror( "TGP: Unknown LDIMM r12 (sub=%d) at PC:%04x\n", sub, GETPC() );
			}
			break;

			case 0x14:	/* LDIMM m,e */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 0 ) /* A */
				{
					GETA().u = imm;
				}
				else if ( sub == 1 ) /* A.e */
				{
					GETA().u = (imm & 0xff) << 23;
				}
				else if ( sub == 2 ) /* A.m */
				{
					GETA().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					logerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x15:	/* LDIMM m,e */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 0 ) /* B ? */
				{
					GETB().u = ((imm & 0x7f) << 23) | ((imm & 0xff) << 8) | ( imm & 0xff );
				}
				else if ( sub == 1 ) /* B.e */
				{
					GETB().u = (imm & 0xff) << 23;
				}
				else if ( sub == 2 ) /* B.m */
				{
					GETB().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					logerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x16:	/* LDIMM m,e */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 1 ) /* clear + D.m */
				{
					GETD().u = (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else if ( sub == 2 ) /* D.e */
				{
					GETD().u = (imm & 0xff) << 23;
				}
				else if ( sub == 3 ) /* D.m */
				{
					GETD().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					logerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x17:	/* LDIMM special reg */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 0x03 )
				{
					GETSHIFT() = imm;
				}
				else
				{
					logerror( "TGP: Unknown LDIMM special reg (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x18:	/* LDIMM external reg */
			{
				UINT32	sub = (opcode>>24) & 0x03;
				UINT32	imm = opcode & 0xffffff;

				if ( sub == 0x03 )
				{
					GETEB() = imm;
				}
				else
				{
					logerror( "TGP: Unknown LDIMM external reg (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x1d:	//LDIMM to Rep regs
			{
				UINT32 sub = (opcode>>24)&0x3;
				UINT32 imm = opcode&0xffffff;
				if(sub == 0x00)
				{
					GETREPCNT() = imm;
				}
				else
				{
					logerror( "TGP: Unknown LDIMM REPCnt (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x2f:	/* Conditional Branches */
			{
				UINT32	cond = ( opcode >> 20 ) & 0x1f;
				UINT32	subtype = ( opcode >> 16 ) & 0x0f;
				UINT32	data = opcode & 0xffff;

				if( COND(cond) )
				{
					switch( subtype )
					{
						case 0x00:	/* BRIF <addr> */
							GETPC() = data - 1;
							change_pc( GETPC() );
						break;

						case 0x02:	/* BRIF indirect */
							if ( data & 0x4000 )
								data = GETREGS(data&0x3f,0) - 1;
							else
								data = ((GETARAM()[data&0x3ff])&0xffff)-1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
							change_pc( GETPC() );
						break;

						case 0x04:	/* BSIF <addr> */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							GETPC() = data - 1;
							change_pc( GETPC() );
						break;

						case 0x06:	/* BSIF indirect */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							if ( data & 0x4000 )
								data = GETREGS(data&0x3f,0) - 1;
							else
								data = ((GETARAM()[data&0x3ff])&0xffff)-1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
							change_pc( GETPC() );
						break;

						case 0x0a:	/* RTIF */
							--GETPCSP();
							GETPC() = GETPCS()[GETPCSP()];
							change_pc( GETPC() );
						break;

						case 0x0c:	/* LDIF */
							SETREGS(((data>>9)&0x3f), GETARAM()[data&0x1FF] );
						break;

						case 0x0e:	/* RIIF */
							logerror( "TGP: RIIF unimplemented at PC:%04x\n", GETPC() );
						break;

						default:
							logerror( "TGP: Unknown Branch opcode (subtype=%d) at PC:%04x\n", subtype, GETPC() );
						break;
					}
				}
			}
			break;

			case 0x3f:	/* Inverse Conditional Branches */
			{
				UINT32	cond = ( opcode >> 20 ) & 0x1f;
				UINT32	subtype = ( opcode >> 16 ) & 0x0f;
				UINT32	data = opcode & 0xffff;

				if( !COND(cond) )
				{
					switch( subtype )
					{
						case 0x00:	/* BRUL <addr> */
							GETPC() = data - 1;
						break;

						case 0x02:	/* BRUL indirect */
							if ( data & 0x4000 )
								data = GETREGS(data&0x3f,0) - 1;
							else
								data = ((GETARAM()[data&0x3ff])&0xffff)-1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
							change_pc( GETPC() );
						break;

						case 0x04:	/* BSUL <addr> */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							GETPC() = data - 1;
							change_pc( GETPC() );
						break;

						case 0x06:	/* BSUL indirect */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							if ( data & 0x4000 )
								data = GETREGS(data&0x3f,0) - 1;
							else
								data = ((GETARAM()[data&0x3ff])&0xffff)-1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
							change_pc( GETPC() );
						break;

						case 0x0a:	/* RTUL */
							--GETPCSP();
							GETPC() = GETPCS()[GETPCSP()];
							change_pc( GETPC() );
						break;

						case 0x0c:	/* LDUL */
							SETREGS(((data>>9)&0x3f), GETARAM()[data&0x1FF] );
						break;

						case 0x0e:	/* RIUL */
							logerror( "TGP: RIIF unimplemented at PC:%04x\n", GETPC() );
						break;

						default:
							logerror( "TGP: Unknown Branch opcode (subtype=%d) at PC:%04x\n", subtype, GETPC() );
						break;
					}
				}
			}
			break;

			default:
				logerror( "TGP: unknown opcode %08x at PC:%04x\n", opcode, GETPC() );
			break;
		}

		if ( GETFIFOWAIT() == 0 )
		{
			if( GETREPS() == 0 )
				GETPC()++;
			else
				--GETREPS();

			mb86233_icount--;
		}
		else
		{
			mb86233_icount = 0;
		}
	}

	return cycles - mb86233_icount;
}

/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

#ifdef MAME_DEBUG
static offs_t mb86233_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	extern UINT32 dasm_mb86233(char *, UINT32);

	UINT32 op = *(UINT32 *)oprom;
	op = LITTLE_ENDIANIZE_INT32(op);
	return dasm_mb86233(buffer, op);
}
#endif /* MAME_DEBUG */

/***************************************************************************
    Information Setters
***************************************************************************/

static void mb86233_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MB86233_PC:			GETPC() = info->i; change_pc(GETPC());	break;
		case CPUINFO_INT_REGISTER + MB86233_A:			GETA().u = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_B:			GETB().u = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_P:			GETP().u = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_D:			GETD().u = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_REP:		GETREPS() = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB86233_SP:			GETPCSP() = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_EB:			GETEB() = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_SHIFT:		GETSHIFT() = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_FLAGS:		GETSR() = info->i;						break;
		case CPUINFO_INT_REGISTER + MB86233_R0:			GETGPR(0) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R1:			GETGPR(1) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R2:			GETGPR(2) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R3:			GETGPR(3) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R4:			GETGPR(4) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R5:			GETGPR(5) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R6:			GETGPR(6) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R7:			GETGPR(7) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R8:			GETGPR(8) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R9:			GETGPR(9) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R10:		GETGPR(10) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R11:		GETGPR(11) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R12:		GETGPR(12) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R13:		GETGPR(13) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R14:		GETGPR(14) = info->i;					break;
		case CPUINFO_INT_REGISTER + MB86233_R15:		GETGPR(15) = info->i;					break;
	}
}

/***************************************************************************
    Information Getters
***************************************************************************/

void mb86233_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(MB86233_REGS);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -2;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MB86233_PC: 		info->i = GETPC();						break;
		case CPUINFO_INT_REGISTER + MB86233_A:			info->i = GETA().u;						break;
		case CPUINFO_INT_REGISTER + MB86233_B:			info->i = GETB().u;						break;
		case CPUINFO_INT_REGISTER + MB86233_P:			info->i = GETP().u;						break;
		case CPUINFO_INT_REGISTER + MB86233_D:			info->i = GETD().u;						break;
		case CPUINFO_INT_REGISTER + MB86233_REP:		info->i = GETREPS();					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB86233_SP:			info->i = GETPCSP();					break;
		case CPUINFO_INT_REGISTER + MB86233_EB:			info->i = GETEB();						break;
		case CPUINFO_INT_REGISTER + MB86233_SHIFT:		info->i = GETSHIFT();					break;
		case CPUINFO_INT_REGISTER + MB86233_FLAGS: 		info->i = GETSR();						break;
		case CPUINFO_INT_REGISTER + MB86233_R0:			info->i = GETGPR(0);					break;
		case CPUINFO_INT_REGISTER + MB86233_R1:			info->i = GETGPR(1);					break;
		case CPUINFO_INT_REGISTER + MB86233_R2:			info->i = GETGPR(2);					break;
		case CPUINFO_INT_REGISTER + MB86233_R3:			info->i = GETGPR(3);					break;
		case CPUINFO_INT_REGISTER + MB86233_R4:			info->i = GETGPR(4);					break;
		case CPUINFO_INT_REGISTER + MB86233_R5:			info->i = GETGPR(5);					break;
		case CPUINFO_INT_REGISTER + MB86233_R6:			info->i = GETGPR(6);					break;
		case CPUINFO_INT_REGISTER + MB86233_R7:			info->i = GETGPR(7);					break;
		case CPUINFO_INT_REGISTER + MB86233_R8:			info->i = GETGPR(8);					break;
		case CPUINFO_INT_REGISTER + MB86233_R9:			info->i = GETGPR(9);					break;
		case CPUINFO_INT_REGISTER + MB86233_R10:		info->i = GETGPR(10);					break;
		case CPUINFO_INT_REGISTER + MB86233_R11:		info->i = GETGPR(11);					break;
		case CPUINFO_INT_REGISTER + MB86233_R12:		info->i = GETGPR(12);					break;
		case CPUINFO_INT_REGISTER + MB86233_R13:		info->i = GETGPR(13);					break;
		case CPUINFO_INT_REGISTER + MB86233_R14:		info->i = GETGPR(14);					break;
		case CPUINFO_INT_REGISTER + MB86233_R15:		info->i = GETGPR(15);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = mb86233_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = mb86233_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = mb86233_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = mb86233_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = mb86233_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = NULL;						break;
		case CPUINFO_PTR_EXECUTE:						info->execute = mb86233_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = mb86233_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mb86233_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB86233");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Fujitsu MB86233");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Miguel Angel Horna and Ernesto Corvi"); break;

		case CPUINFO_STR_FLAGS:
    		sprintf(info->s, "%c%c", (GETSR()&SIGN_FLAG) ? 'N' : 'n', (GETSR()&ZERO_FLAG) ? 'Z' : 'z' );
	        break;

        case CPUINFO_STR_REGISTER + MB86233_FLAGS:
    		sprintf(info->s, "FL:%c%c", (GETSR()&SIGN_FLAG) ? 'N' : 'n', (GETSR()&ZERO_FLAG) ? 'Z' : 'z' );
	        break;

		case CPUINFO_STR_REGISTER + MB86233_PC:			sprintf(info->s, "PC:%04X", GETPC());					break;
		case CPUINFO_STR_REGISTER + MB86233_A:			sprintf(info->s, "PA:%08X (%f)", GETA().u, GETA().f);	break;
		case CPUINFO_STR_REGISTER + MB86233_B:			sprintf(info->s, "PB:%08X (%f)", GETB().u, GETB().f);	break;
		case CPUINFO_STR_REGISTER + MB86233_P:			sprintf(info->s, "PP:%08X (%f)", GETP().u, GETP().f);	break;
		case CPUINFO_STR_REGISTER + MB86233_D:			sprintf(info->s, "PD:%08X (%f)", GETD().u, GETD().f);	break;
        case CPUINFO_STR_REGISTER + MB86233_REP:		sprintf(info->s, "REPS:%08X", GETREPS());				break;
		case CPUINFO_STR_REGISTER + MB86233_SP:			sprintf(info->s, "PCSP:%1X", GETPCSP());				break;
		case CPUINFO_STR_REGISTER + MB86233_EB:			sprintf(info->s, "EB:%08X", GETEB());					break;
		case CPUINFO_STR_REGISTER + MB86233_SHIFT:		sprintf(info->s, "SHIFT:%08X", GETSHIFT());				break;
		case CPUINFO_STR_REGISTER + MB86233_R0:			sprintf(info->s, "R0:%08X", GETGPR(0));					break;
		case CPUINFO_STR_REGISTER + MB86233_R1:			sprintf(info->s, "R1:%08X", GETGPR(1));					break;
		case CPUINFO_STR_REGISTER + MB86233_R2:			sprintf(info->s, "R2:%08X", GETGPR(2));					break;
		case CPUINFO_STR_REGISTER + MB86233_R3:			sprintf(info->s, "R3:%08X", GETGPR(3));					break;
		case CPUINFO_STR_REGISTER + MB86233_R4:			sprintf(info->s, "R4:%08X", GETGPR(4));					break;
		case CPUINFO_STR_REGISTER + MB86233_R5:			sprintf(info->s, "R5:%08X", GETGPR(5));					break;
		case CPUINFO_STR_REGISTER + MB86233_R6:			sprintf(info->s, "R6:%08X", GETGPR(6));					break;
		case CPUINFO_STR_REGISTER + MB86233_R7:			sprintf(info->s, "R7:%08X", GETGPR(7));					break;
		case CPUINFO_STR_REGISTER + MB86233_R8:			sprintf(info->s, "R8:%08X", GETGPR(8));					break;
		case CPUINFO_STR_REGISTER + MB86233_R9:			sprintf(info->s, "R9:%08X", GETGPR(9));					break;
		case CPUINFO_STR_REGISTER + MB86233_R10:		sprintf(info->s, "R10:%08X", GETGPR(10));				break;
		case CPUINFO_STR_REGISTER + MB86233_R11:		sprintf(info->s, "R11:%08X", GETGPR(11));				break;
		case CPUINFO_STR_REGISTER + MB86233_R12:		sprintf(info->s, "R12:%08X", GETGPR(12));				break;
		case CPUINFO_STR_REGISTER + MB86233_R13:		sprintf(info->s, "R13:%08X", GETGPR(13));				break;
		case CPUINFO_STR_REGISTER + MB86233_R14:		sprintf(info->s, "R14:%08X", GETGPR(14));				break;
		case CPUINFO_STR_REGISTER + MB86233_R15:		sprintf(info->s, "R15:%08X", GETGPR(15));				break;
	}
}
