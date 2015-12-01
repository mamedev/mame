// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
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

#include "emu.h"
#include "debugger.h"
#include "mb86233.h"


const device_type MB86233 = &device_creator<mb86233_cpu_device>;


mb86233_cpu_device::mb86233_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86233, "MB86233", tag, owner, clock, "mb86233", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, -2)
	, m_data_config("data", ENDIANNESS_LITTLE, 32, 32, 0), m_pc(0), m_reps(0), m_pcsp(0), m_eb(0), m_shift(0), m_repcnt(0), m_sr(0),
	m_fpucontrol(0), m_program(nullptr), m_direct(nullptr), m_icount(0), m_fifo_wait(0)
		, m_fifo_read_cb(*this)
	, m_fifo_read_ok_cb(*this)
	, m_fifo_write_cb(*this)
	, m_tablergn(nullptr), m_ARAM(nullptr), m_BRAM(nullptr)
		, m_Tables(nullptr)
{
}


offs_t mb86233_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( mb86233 );
	return CPU_DISASSEMBLE_NAME(mb86233)(this, buffer, pc, oprom, opram, options);
}


/***************************************************************************
    MACROS
***************************************************************************/

#define ZERO_FLAG   (1 << 0)
#define SIGN_FLAG   (1 << 1)
#define EXTERNAL_FLAG   (1 << 2)        //This seems to be a flag coming from some external circuit??

#define GETPC()             m_pc
#define GETA()              m_a
#define GETB()              m_b
#define GETD()              m_d
#define GETP()              m_p
#define GETSR()             m_sr
#define GETGPR(a)           m_gpr[a]
#define GETSHIFT()          m_shift
#define GETPCS()            m_pcs
#define GETPCSP()           m_pcsp
#define GETEB()             m_eb
#define GETREPS()           m_reps
#define GETEXTPORT()        m_extport
#define GETFIFOWAIT()       m_fifo_wait
#define GETARAM()           m_ARAM
#define GETBRAM()           m_BRAM
#define GETREPCNT()         m_repcnt

#define ROPCODE(a)          m_direct->read_dword(a<<2)
#define RDMEM(a)            m_program->read_dword((a<<2))
#define WRMEM(a,v)          m_program->write_dword((a<<2), v)

/***************************************************************************
    Initialization and Shutdown
***************************************************************************/

void mb86233_cpu_device::device_start()
{
	m_pc = 0;
	m_a.u = 0;
	m_b.u = 0;
	m_d.u = 0;
	m_p.u = 0;
	m_reps = 0;
	m_pcs[0] = m_pcs[1] = m_pcs[2] = m_pcs[3] = 0;
	m_pcsp = 0;
	m_eb = 0;
	m_shift = 0;
	m_repcnt = 0;
	m_sr = 0;
	memset(m_gpr, 0, sizeof(m_gpr));
	memset(m_extport, 0, sizeof(m_extport));
	m_fifo_wait = 0;

	m_fifo_read_cb.resolve_safe(0);
	m_fifo_read_ok_cb.resolve_safe(0);
	m_fifo_write_cb.resolve_safe();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	if ( m_tablergn )
	{
		m_Tables = (UINT32*) machine().root_device().memregion(m_tablergn)->base();
	}

	memset( m_RAM, 0, 2 * 0x200 * sizeof(UINT32) );
	m_ARAM = &m_RAM[0];
	m_BRAM = &m_RAM[0x200];

	save_item(NAME(m_pc));
	save_item(NAME(m_a.u));
	save_item(NAME(m_b.u));
	save_item(NAME(m_d.u));
	save_item(NAME(m_p.u));
	save_item(NAME(m_reps));
	save_item(NAME(m_pcs));
	save_item(NAME(m_pcsp));
	save_item(NAME(m_eb));
	save_item(NAME(m_shift));
	save_item(NAME(m_repcnt));
	save_item(NAME(m_sr));
	save_item(NAME(m_gpr));
	save_item(NAME(m_extport));
	save_item(NAME(m_RAM));

	state_add( MB86233_PC,    "PC", m_pc).formatstr("%04X");
	state_add( MB86233_A,     "PA", m_a.u).formatstr("%08X");
	state_add( MB86233_B,     "PB", m_b.u).formatstr("%08X");
	state_add( MB86233_P,     "PP", m_p.u).formatstr("%08X");
	state_add( MB86233_D,     "PD", m_d.u).formatstr("%08X");
	state_add( MB86233_REP,   "REPS", m_reps).formatstr("%08X");
	state_add( MB86233_SP,    "PCSP", m_pcsp).mask(0xf).formatstr("%01X");
	state_add( MB86233_EB,    "EB", m_eb).formatstr("%08X");
	state_add( MB86233_SHIFT, "SHIFT", m_shift).formatstr("%08X");
	state_add( MB86233_R0,    "R0", m_gpr[0]).formatstr("%08X");
	state_add( MB86233_R1,    "R1", m_gpr[1]).formatstr("%08X");
	state_add( MB86233_R2,    "R2", m_gpr[2]).formatstr("%08X");
	state_add( MB86233_R3,    "R3", m_gpr[3]).formatstr("%08X");
	state_add( MB86233_R4,    "R4", m_gpr[4]).formatstr("%08X");
	state_add( MB86233_R5,    "R5", m_gpr[5]).formatstr("%08X");
	state_add( MB86233_R6,    "R6", m_gpr[6]).formatstr("%08X");
	state_add( MB86233_R7,    "R7", m_gpr[7]).formatstr("%08X");
	state_add( MB86233_R8,    "R8", m_gpr[8]).formatstr("%08X");
	state_add( MB86233_R9,    "R9", m_gpr[9]).formatstr("%08X");
	state_add( MB86233_R10,   "R10", m_gpr[10]).formatstr("%08X");
	state_add( MB86233_R11,   "R11", m_gpr[11]).formatstr("%08X");
	state_add( MB86233_R12,   "R12", m_gpr[12]).formatstr("%08X");
	state_add( MB86233_R13,   "R13", m_gpr[13]).formatstr("%08X");
	state_add( MB86233_R14,   "R14", m_gpr[14]).formatstr("%08X");
	state_add( MB86233_R15,   "R15", m_gpr[15]).formatstr("%08X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%2s").noshow();

	m_icountptr = &m_icount;
}


void mb86233_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c", (m_sr & SIGN_FLAG) ? 'N' : 'n', (m_sr & ZERO_FLAG) ? 'Z' : 'z');
			break;
	}
}


void mb86233_cpu_device::device_reset()
{
	/* zero registers and flags */
	m_pc = 0;
	m_sr = 0;
	m_pcsp = 0;
	m_eb = 0;
	m_shift = 0;
	m_fifo_wait = 0;
}



/***************************************************************************
    Status Register
***************************************************************************/

#define ZERO_FLAG   (1 << 0)
#define SIGN_FLAG   (1 << 1)
#define EXTERNAL_FLAG   (1 << 2)        //This seems to be a flag coming from some external circuit??

void mb86233_cpu_device::FLAGSF( float v )
{
	GETSR() &= ~(ZERO_FLAG|SIGN_FLAG);

	if ( v == 0 )
		GETSR() |= ZERO_FLAG;

	if ( v < 0 )
		GETSR() |= SIGN_FLAG;
}

void mb86233_cpu_device::FLAGSI( UINT32 v )
{
	GETSR() &= ~(ZERO_FLAG|SIGN_FLAG);

	if ( v == 0 )
		GETSR() |= ZERO_FLAG;

	if ( v & 0x80000000 )
		GETSR() |= SIGN_FLAG;
}



/***************************************************************************
    Condition Codes
***************************************************************************/

int mb86233_cpu_device::COND( UINT32 cond )
{
	switch( cond )
	{
		case 0x00:  /* eq */
			if ( (GETSR() & ZERO_FLAG) ) return 1;
		break;

		case 0x01:  /* ge */
			if ( (GETSR() & ZERO_FLAG) || ((GETSR() & SIGN_FLAG)==0) ) return 1;
		break;

		case 0x02:  /* le */
			if ( (GETSR() & ZERO_FLAG) || (GETSR() & SIGN_FLAG) ) return 1;
		break;

		case 0x06:  /* never */
		break;

		case 0x0a:
			if(GETSR() & EXTERNAL_FLAG) return 1;
		break;

		case 0x10:  /* --r12 != 0 */
			GETGPR(12)--;
			if ( GETGPR(12) != 0 ) return 1;
		break;

		case 0x11:  /* --r13 != 0 */
			GETGPR(13)--;
			if ( GETGPR(13) != 0 ) return 1;
		break;

		case 0x16:  /* always */
			return 1;

		default:
			logerror( "TGP: Unknown condition code (cc=%d) at PC:%x\n", cond, GETPC());
		break;
	}

	return 0;
}



/***************************************************************************
    ALU
***************************************************************************/

void mb86233_cpu_device::ALU( UINT32 alu)
{
	float   ftmp;

	switch(alu)
	{
		case 0x00:  /* NOP */
		break;

		case 0x01:  /* D = D & A */
			GETD().u &= GETA().u;
			FLAGSI( GETD().u);
		break;

		case 0x02:  /* D = D | A */
			GETD().u |= GETA().u;
			FLAGSI( GETD().u);
		break;

		case 0x03:  /* D = D ^ A */
			GETD().u ^= GETA().u;
			FLAGSI( GETD().u);
		break;

		case 0x04:  /* D = D ~ A */
			GETD().u = ~GETA().u;
			FLAGSI( GETD().u);
		break;

		case 0x05:  /* CMP D,A */
			ftmp = GETD().f - GETA().f;
			FLAGSF( ftmp);
			m_icount--;
		break;

		case 0x06:  /* D = D + A */
			GETD().f += GETA().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x07:  /* D = D - A */
			GETD().f -= GETA().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x08:  /* P = A * B */
			GETP().f = GETA().f * GETB().f;
			m_icount--;
		break;

		case 0x09:  /* D = D + P; P = A * B */
			GETD().f += GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0A:  /* D = D - P; P = A * B */
			GETD().f -= GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0B:  /* D = fabs(D) */
			GETD().f = fabs( GETD().f );
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0C:  /* D = D + P */
			GETD().f += GETP().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0D:  /* D = P; P = A * B */
			GETD().f = GETP().f;
			GETP().f = GETA().f * GETB().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0E:  /* D = float(D) */
			GETD().f = (float)GETD().i;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x0F:  /* D = int(D) */
			switch((m_fpucontrol>>1)&3)
			{
				//case 0: GETD().i = floor(GETD().f+0.5f); break;
				//case 1: GETD().i = ceil(GETD().f); break;
				case 2: GETD().i = floor(GETD().f); break; // Manx TT
				case 3: GETD().i = (INT32)GETD().f; break;
				default: popmessage("TGP uses D = int(D) with FPU control = %02x, contact MAMEdev",m_fpucontrol>>1); break;
			}

			FLAGSI( GETD().i);
		break;

		case 0x10:  /* D = D / A */
			if ( GETA().u != 0 )
				GETD().f = GETD().f / GETA().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x11:  /* D = -D */
			GETD().f = -GETD().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x13:  /* D = A + B */
			GETD().f = GETA().f + GETB().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x14:  /* D = B - A */
			GETD().f = GETB().f - GETA().f;
			FLAGSF( GETD().f);
			m_icount--;
		break;

		case 0x16:  /* LSR D, SHIFT */
			GETD().u >>= GETSHIFT();
			FLAGSI( GETD().u);
		break;

		case 0x17:  /* LSL D, SHIFT */
			GETD().u <<= GETSHIFT();
			FLAGSI( GETD().u);
		break;

		case 0x18:  /* ASR D, SHIFT */
//          GETD().u = (GETD().u & 0x80000000) | (GETD().u >> GETSHIFT());
			GETD().i >>= GETSHIFT();
			FLAGSI( GETD().u);
		break;

		case 0x1A:  /* D = D + A */
			GETD().i += GETA().i;
			FLAGSI( GETD().u);
		break;

		case 0x1B:  /* D = D - A */
			GETD().i -= GETA().i;
			FLAGSI( GETD().u);
		break;

		default:
			fatalerror( "TGP: Unknown ALU op %x at PC:%04x\n", alu, GETPC() );
	}
}



/***************************************************************************
    Memory Access
***************************************************************************/

UINT32 mb86233_cpu_device::ScaleExp(unsigned int v,int scale)
{
	int exp=(v>>23)&0xff;
	exp+=scale;
	v&=~0x7f800000;
	return v|(exp<<23);
}


UINT32 mb86233_cpu_device::GETEXTERNAL( UINT32 EB, UINT32 offset )
{
	UINT32      addr;

	if ( EB == 0 && offset >= 0x20 && offset <= 0x2f )  /* TGP Tables in ROM - FIXME - */
	{
		if(offset>=0x20 && offset<=0x23)    //SIN from value at RAM(0x20) in 0x4000/PI steps
		{
			UINT32 r;
			UINT32 value=GETEXTPORT()[0x20];
			UINT32 off;
			value+=(offset-0x20)<<14;
			off=value&0x3fff;
			if((value&0x7fff)==0)
				r=0;
			else if((value&0x7fff)==0x4000)
				r=0x3f800000;
			else
			{
				if(value&0x4000)
					off=0x4000-off;
				r=m_Tables[off];
			}
			if(value&0x8000)
				r|=1<<31;
			return r;
		}

		if(offset==0x27)
		{
			unsigned int value=GETEXTPORT()[0x27];
			int exp=(value>>23)&0xff;
			unsigned int res=0;
			unsigned int sign=0;
			MB86233_REG a,b;
			int index;

			a.u=GETEXTPORT()[0x24];
			b.u=GETEXTPORT()[0x25];


			if(!exp)
			{
				if((a.u&0x7fffffff)<=(b.u&0x7fffffff))
				{
					if(b.u&0x80000000)
						res=0xc000;
					else
						res=0x4000;
				}
				else
				{
					if(a.u&0x80000000)
						res=0x8000;
					else
						res=0x0000;
				}
				return res;
			}

			if((a.u^b.u)&0x80000000)
				sign=16;                //the negative values are in the high word

			if((exp&0x70)!=0x70)
				index=0;
			else if(exp<0x70 || exp>0x7e)
				index=0x3fff;
			else
			{
				int expdif=exp-0x71;
				int base;
				int mask;
				int shift;


				if(expdif<0)
					expdif=0;
				base=1<<expdif;
				mask=base-1;
				shift=23-expdif;

				index=base+((value>>shift)&mask);

			}

			res=(m_Tables[index+0x10000/4]>>sign)&0xffff;

			if((a.u&0x7fffffff)<=(b.u&0x7fffffff))
				res=0x4000-res;


			if((a.u&0x80000000) && (b.u&0x80000000))    //3rd quadrant
			{
				res=0x8000|res;
			}
			else if((a.u&0x80000000) && !(b.u&0x80000000))  //2nd quadrant
			{
				res=res&0x7fff;
			}
			else if(!(a.u&0x80000000) && (b.u&0x80000000))  //2nd quadrant
			{
				res=0x8000|res;
			}

			return res;

		}

		if(offset==0x28)
		{
			UINT32 offset=(GETEXTPORT()[0x28]>>10)&0x1fff;
			UINT32 value=m_Tables[offset*2+0x20000/4];
			UINT32 srcexp=(GETEXTPORT()[0x28]>>23)&0xff;

			value&=0x7FFFFFFF;

			return ScaleExp(value,0x7f-srcexp);
		}
		if(offset==0x29)
		{
			UINT32 offset=(GETEXTPORT()[0x28]>>10)&0x1fff;
			UINT32 value=m_Tables[offset*2+(0x20000/4)+1];
			UINT32 srcexp=(GETEXTPORT()[0x28]>>23)&0xff;

			value&=0x7FFFFFFF;
			if(GETEXTPORT()[0x28]&(1<<31))
				value|=1<<31;

			return ScaleExp(value,0x7f-srcexp);
		}
		if(offset==0x2a)
		{
			UINT32 offset=((GETEXTPORT()[0x2a]>>11)&0x1fff)^0x1000;
			UINT32 value=m_Tables[offset*2+0x30000/4];
			UINT32 srcexp=(GETEXTPORT()[0x2a]>>24)&0x7f;

			value&=0x7FFFFFFF;

			return ScaleExp(value,0x3f-srcexp);
		}
		if(offset==0x2b)
		{
			UINT32 offset=((GETEXTPORT()[0x2a]>>11)&0x1fff)^0x1000;
			UINT32 value=m_Tables[offset*2+(0x30000/4)+1];
			UINT32 srcexp=(GETEXTPORT()[0x2a]>>24)&0x7f;

			value&=0x7FFFFFFF;
			if(GETEXTPORT()[0x2a]&(1<<31))
				value|=1<<31;

			return ScaleExp(value,0x3f-srcexp);
		}

		return GETEXTPORT()[offset];
	}

	addr = ( EB & 0xFFFF0000 ) | ( offset & 0xFFFF );

	return RDMEM(addr);
}

void mb86233_cpu_device::SETEXTERNAL( UINT32 EB, UINT32 offset, UINT32 value )
{
	UINT32  addr;

	if ( EB == 0 && offset >= 0x20 && offset <= 0x2f )  /* TGP Tables in ROM - FIXME - */
	{
		GETEXTPORT()[offset] = value;

		if(offset==0x25 || offset==0x24)
		{
			if((GETEXTPORT()[0x24]&0x7fffffff)<=(GETEXTPORT()[0x25]&0x7fffffff))
			{
				GETSR()|=EXTERNAL_FLAG;
			}
			else
			{
				GETSR()&=~EXTERNAL_FLAG;
			}
		}
		return;
	}

	addr = ( EB & 0xFFFF0000 ) | ( offset & 0xFFFF );

	WRMEM( addr, value );
}



/***************************************************************************
    Register Access
***************************************************************************/

UINT32 mb86233_cpu_device::GETREGS( UINT32 reg, int source )
{
	UINT32  mode = ( reg >> 6 ) & 0x07;

	reg &= 0x3f;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		if ( reg < 0x10 )
		{
			return GETGPR(reg);
		}

		switch( reg )
		{
			case 0x10:  /* A */
				return GETA().u;

			case 0x11:  /* A.e */
				return (GETA().u >> 23) & 0xff;

			case 0x12:  /* A.m */
				return (GETA().u & 0x7fffff) | ((GETA().u&0x80000000) >> 8);

			case 0x13:  /* B */
				return GETB().u;

			case 0x14:  /* B.e */
				return (GETB().u >> 23) & 0xff;

			case 0x15:  /* B.m */
				return (GETB().u & 0x7fffff) | ((GETB().u&0x80000000) >> 8);

			case 0x19:  /* D */
				return GETD().u;

			case 0x1A:  /* D.e */
				return (GETD().u >> 23) & 0xff;

			case 0x1B:  /* D.m */
				return (GETD().u & 0x7fffff) | ((GETD().u&0x80000000) >> 8);

			case 0x1C:  /* P */
				return GETP().u;

			case 0x1D:  /* P.e */
				return (GETP().u >> 23) & 0xff;

			case 0x1E:  /* P.m */
				return (GETP().u & 0x7fffff) | ((GETP().u&0x80000000) >> 8);

			case 0x1F:  /* Shift */
				return GETSHIFT();

			case 0x20:  /* Parallel Port */
				logerror( "TGP: Parallel port read at PC:%04x\n", GETPC() );
				return 0;

			case 0x21:  /* FIn */
			{
				if ( m_fifo_read_ok_cb() == ASSERT_LINE )
				{
					return m_fifo_read_cb();
				}

				GETFIFOWAIT() = 1;
				return 0;
			}

			case 0x22:  /* FOut */
				return 0;

			case 0x23:  /* EB */
				return GETEB();

			case 0x34:
				return GETREPCNT();

			default:
				fatalerror( "TGP: Unknown GETREG (%d) at PC=%04x\n", reg, GETPC() );
		}
	}
	else if ( mode == 2 )   /* Indexed */
	{
		UINT32  addr = reg & 0x1f;

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
	else if( mode == 6 )    /* Indexed with postop */
	{
		UINT32  addr = 0;

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

	// never executed
	//return 0;
}

void mb86233_cpu_device::SETREGS( UINT32 reg, UINT32 val )
{
	int     mode = ( reg >> 6) & 0x07;

	reg &= 0x3f;

	if( mode == 0 || mode == 1 || mode == 3 )
	{
		if(reg==12 || reg==13) // counter regs seem to be 8 bit only
			val&=0xff;

		if ( reg < 0x10 )
		{
			GETGPR(reg) = val;
			return;
		}

		switch( reg )
		{
			case 0x10:  /* A */
				GETA().u = val;
			break;

			case 0x11:  /* A.e */
				GETA().u &= ~((0x0000007f) << 23);
				GETA().u |= (( val & 0xff ) << 23 );
			break;

			case 0x12:  /* A.m */
				GETA().u &= ~( 0x807fffff );
				GETA().u |= ( val & 0x7fffff );
				GETA().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x13:  /* B */
				GETB().u = val;
			break;

			case 0x14:  /* B.e */
				GETB().u &= ~((0x0000007f) << 23);
				GETB().u |= (( val & 0xff ) << 23 );
			break;

			case 0x15:  /* B.m */
				GETB().u &= ~( 0x807fffff );
				GETB().u |= ( val & 0x7fffff );
				GETB().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x19:  /* D */
				GETD().u = val;
			break;

			case 0x1A:  /* D.e */
				GETD().u &= ~((0x0000007f) << 23);
				GETD().u |= (( val & 0xff ) << 23 );
			break;

			case 0x1B:  /* B.m */
				GETD().u &= ~( 0x807fffff );
				GETD().u |= ( val & 0x7fffff );
				GETD().u |= ( val & 0x800000 ) << 8;
			break;

			case 0x1C:  /* P */
				GETP().u = val;
			break;

			case 0x1D:  /* P.e */
				GETP().u &= ~((0x000000ff) << 23);
				GETP().u |= (( val & 0xff ) << 23 );
			break;

			case 0x1E:  /* P.m */
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
				m_fifo_write_cb( val );
			break;

			case 0x23:
				GETEB() = val;
			break;

			case 0x34:
				GETREPCNT() = val;
			break;

			default:
				fatalerror( "TGP: Unknown register write (r:%d, mode:%d) at PC:%04x\n", reg, mode, GETPC());
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

UINT32 mb86233_cpu_device::INDIRECT( UINT32 reg, int source )
{
	UINT32  mode = ( reg >> 6 ) & 0x07;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		return reg;
	}
	else if ( mode == 2 )
	{
		UINT32  addr = reg & 0x3f;

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
		UINT32  addr = 0;

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
		if( mode == 7)
		{
			if ( source )
				GETGPR(2)&=0x3f;
			else
				GETGPR(3)&=0x3f;
		}

		return addr;
	}
	else
	{
		fatalerror( "TGP: Unknown INDIRECT mode %d at PC:%04x\n", mode, GETPC() );
	}

	// never executed
	//return 0;
}

/***************************************************************************
    Core Execution Loop
***************************************************************************/

void mb86233_cpu_device::execute_run()
{
	while( m_icount > 0 )
	{
		UINT32      val;
		UINT32      opcode;

		debugger_instruction_hook(this, GETPC());

		opcode = ROPCODE(GETPC());

		GETFIFOWAIT() = 0;

		switch( (opcode >> 26) & 0x3f )
		{
			case 0x00:  /* dual move */
			{
				UINT32      r1 = opcode & 0x1ff;
				UINT32      r2 = ( opcode >> 9 ) & 0x7f;
				UINT32      alu = ( opcode >> 21 ) & 0x1f;
				UINT32      op = ( opcode >> 16 ) & 0x1f;

				ALU( alu );

				switch( op )
				{
					case 0x01:
						GETA().u = GETARAM()[INDIRECT(r1,1)];
						GETB().u = GETEXTERNAL( GETEB(),INDIRECT(r2|(2<<6), 0));
					break;

					case 0x04: // ?
						GETA().u = GETARAM()[r1];
						GETB().u = GETEXTERNAL( GETEB(),r2);
					break;

					case 0x0C:
						GETA().u = GETARAM()[INDIRECT(r1,1)];
						GETB().u = GETBRAM()[r2];
					break;

					case 0x0D: // VF2 shadows
						GETA().u = GETARAM()[INDIRECT(r1,1)];
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
						GETA().u = GETBRAM()[INDIRECT(r1,1)];
						GETB().u = GETARAM()[INDIRECT(r2|(2<<6),0)];
					break;

					default:
						logerror( "TGP: Unknown TGP double move (op=%d) at PC:%x\n", op, GETPC());
					break;
				}
			}
			break;

			case 0x7:   /* LD/MOV */
			{
				UINT32      r1 = opcode & 0x1ff;
				UINT32      r2 = ( opcode >> 9 ) & 0x7f;
				UINT32      alu = ( opcode >> 21 ) & 0x1f;
				UINT32      op = ( opcode >> 16 ) & 0x1f;

				switch( op )
				{
					case 0x04:  /* MOV RAM->External */
					{
						SETEXTERNAL( GETEB(), r2, GETARAM()[r1]);
						ALU(alu);
					}
					break;

					case 0x0c:  /* MOV RAM->BRAM */
					{
						GETBRAM()[r2] = GETARAM()[r1];
						ALU(alu);
					}
					break;

					case 0x0d: /* Move RAM->BRAM indirect? */
					{
						val = GETARAM()[r1];
						ALU(alu);
						GETBRAM()[INDIRECT(r2|(2<<6),0)] = val;
					}
					break;

					case 0x1d:  /* MOV RAM->Reg */
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
							SETEXTERNAL( GETEB(),INDIRECT(r1,0),val);
						}
						else
						{
							GETARAM()[INDIRECT(r1,0)] = val;
						}
					}
					break;

					case 0x1f:  /* MOV Reg->Reg */
					{
						if ( r1 == 0x10 && r2 == 0xf )
						{
							/* NOP */
							ALU( alu);
						}
						else
						{
							val = GETREGS(r1,1);

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							ALU(alu);
							SETREGS(r2, val);
						}
					}
					break;

					case 0x0f:  /* MOV RAMInd->BRAMInd */
					{
						val = GETARAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETBRAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					case 0x13:  /* MOV BRAMInd->RAMInd */
					{
						val = GETBRAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETARAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					case 0x10:  /* MOV RAMInd->RAM  */
					{
						val = GETBRAM()[INDIRECT(r1,1)];
						ALU(alu);
						GETARAM()[r2] = val;
					}
					break;

					case 0x1e:  /* External->Reg */
					{
						UINT32  offset;

						if ( (( r2 >> 6 ) & 7) == 1 )
						{
							offset = INDIRECT(r1,1);
							val = GETEXTERNAL(0,offset);
						}
						else
						{
							offset = INDIRECT(r1,0);
							val = GETEXTERNAL(GETEB(),offset);
						}

						ALU(alu);
						SETREGS(r2,val);
					}
					break;

					case 0x03:  /* RAM->External Ind */
					{
						val = GETARAM()[r1];
						ALU(alu);
						SETEXTERNAL(GETEB(),INDIRECT(r2|(6<<6),0),val);
					}
					break;

					case 0x07:  /* RAMInd->External */
					{
						val = GETARAM()[INDIRECT(r1,1)];
						ALU(alu);
						SETEXTERNAL( GETEB(),INDIRECT(r2|(6<<6),0),val);
					}
					break;

					case 0x08:  /* External->RAM */
					{
						val = GETEXTERNAL( GETEB(),INDIRECT(r1,1));
						ALU(alu);
						GETARAM()[r2] = val;
					}
					break;

					case 0x0b:  /* External->RAMInd */
					{
						val = GETEXTERNAL( GETEB(),INDIRECT(r1,1));
						ALU( alu);
						GETARAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;

					case 0x17: /* External r2-> RAMInd r3 */
					{
						UINT32  offset;

						offset = INDIRECT(r1,1);

						val = GETEXTERNAL( GETEB(), offset);
						ALU(alu);
						GETARAM()[INDIRECT(r2|(6<<6),0)] = val;
					}
					break;
					case 0x14:
					{
						UINT32  offset;

						offset = INDIRECT(r1,1);

						val = GETEXTERNAL( 0, offset);
						ALU(alu);
						GETARAM()[r2] = val;
					}
					break;

					default:
						fatalerror( "TGP: Unknown TGP move (op=%02x) at PC:%x\n", op, GETPC());
				}
			}
			break;

			case 0x0d: /* CONTROL? */
			{
				UINT32  sub = (opcode>>16) & 0xff;

				switch(sub)
				{
					case 0x0a: // FPU Round Control opcode
						m_fpucontrol = opcode & 0xff;
						logerror( "TGP: FPU Round CONTROL sets %02x at PC:%x\n", m_fpucontrol, GETPC());
						break;
					default:
						logerror( "TGP: Unknown CONTROL sub-type %02x at PC:%x\n", sub, GETPC());
						break;
				}

				break;
			}

			case 0x0e:  /* LDIMM24 */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

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

			case 0x0f:  /* REP/CLEAR/FLAGS */
			{
				UINT32      alu = ( opcode >> 20 ) & 0x1f;
				UINT32      sub2 = ( opcode >> 16 ) & 0x0f;

				ALU(alu);

				if( sub2 == 0x00 )          /* CLEAR reg */
				{
					UINT32  reg = opcode & 0x1f;

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
				else if ( sub2 == 0x04 )    /* REP xxx */
				{
					UINT32 sub3 = ( opcode >> 12 ) & 0x0f;

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
				else if ( sub2 == 0x02 )    /* CLRFLAGS */
				{
					GETSR() &= ~(opcode&0xfff);
				}
				else if ( sub2 == 0x06 )    /* SETFLAGS */
				{
					GETSR() |= (opcode&0xfff);
				}
			}
			break;

			case 0x10:  /* LDIMM rx */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffff;

				GETGPR(sub) = imm;
			}
			break;

			case 0x13:  /* LDIMM r1x */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

				if ( sub == 0 ) /* R12 */
					GETGPR(12) = imm;
				else if ( sub == 1 ) /* R13 */
					GETGPR(13) = imm;
				else
					logerror( "TGP: Unknown LDIMM r12 (sub=%d) at PC:%04x\n", sub, GETPC() );
			}
			break;

			case 0x14:  /* LDIMM m,e */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

				if ( sub == 0 ) /* A */
				{
					GETA().u = imm;
				}
				else if ( sub == 1 ) /* A.e */
				{
					GETA().u &= ~0x7f800000;
					GETA().u |= (imm & 0xff) << 23;
				}
				else if ( sub == 2 ) /* A.m */
				{
					GETA().u &= 0x7f800000;
					GETA().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					fatalerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x15:  /* LDIMM m,e */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

				if ( sub == 0 ) /* B.e again? */
				{
					//GETB().u = ((imm & 0x7f) << 23) | ((imm & 0xff) << 8) | ( imm & 0xff );
					GETB().u &= ~0x7f800000;
					GETB().u |= (imm & 0xff) << 23;
				}
				else if ( sub == 1 ) /* B.e */
				{
					GETB().u &= ~0x7f800000;
					GETB().u |= (imm & 0xff) << 23;
				}
				else if ( sub == 2 ) /* B.m */
				{
					GETB().u &= 0x7f800000;
					GETB().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					fatalerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x16:  /* LDIMM m,e */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

				if ( sub == 1 ) /* clear + D.m */
				{
					GETD().u = (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else if ( sub == 2 ) /* D.e */
				{
					GETD().u &= ~0x7f800000;
					GETD().u |= (imm & 0xff) << 23;
				}
				else if ( sub == 3 ) /* D.m */
				{
					GETD().u &= 0x7f800000;
					GETD().u |= (imm & 0x7fffff ) | ((imm & 0x800000) << 8);
				}
				else
				{
					fatalerror( "TGP: Unknown LDIMM m,e (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x17:  /* LDIMM special reg */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

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

			case 0x18:  /* LDIMM external reg */
			{
				UINT32  sub = (opcode>>24) & 0x03;
				UINT32  imm = opcode & 0xffffff;

				if ( sub == 0x03 )
				{
					GETEB() = imm;
				}
				else
				{
					fatalerror( "TGP: Unknown LDIMM external reg (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x1d:  //LDIMM to Rep regs
			{
				UINT32 sub = (opcode>>24)&0x3;
				UINT32 imm = opcode&0xffffff;
				if(sub == 0x00)
				{
					GETREPCNT() = imm;
				}
				else
				{
					fatalerror( "TGP: Unknown LDIMM REPCnt (sub=%d) at PC:%04x\n", sub, GETPC() );
				}
			}
			break;

			case 0x2f:  /* Conditional Branches */
			{
				UINT32  cond = ( opcode >> 20 ) & 0x1f;
				UINT32  subtype = ( opcode >> 16 ) & 0x0f;
				UINT32  data = opcode & 0xffff;

				if( COND( cond) )
				{
					switch( subtype )
					{
						case 0x00:  /* BRIF <addr> */
							GETPC() = data - 1;
						break;

						case 0x02:  /* BRIF indirect */
							data = GETREGS(data&0x7f,0) - 1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
						break;

						case 0x04:  /* BSIF <addr> */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							GETPC() = data - 1;
						break;

						case 0x06:  /* BSIF indirect */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							if ( data & 0x4000 )
								data = GETREGS(data&0x7f,0) - 1;
							else
								data = ((GETARAM()[data&0x3ff])&0xffff)-1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
						break;

						case 0x0a:  /* RTIF */
							--GETPCSP();
							GETPC() = GETPCS()[GETPCSP()];
						break;

						case 0x0c:  /* LDIF */
							SETREGS(((data>>9)&0x7f), GETARAM()[data&0x1FF] );
						break;

						case 0x0e:  /* RIIF */
							fatalerror( "TGP: RIIF unimplemented at PC:%04x\n", GETPC() );

						default:
							fatalerror( "TGP: Unknown Branch opcode (subtype=%d) at PC:%04x\n", subtype, GETPC() );
					}
				}
			}
			break;

			case 0x3f:  /* Inverse Conditional Branches */
			{
				UINT32  cond = ( opcode >> 20 ) & 0x1f;
				UINT32  subtype = ( opcode >> 16 ) & 0x0f;
				UINT32  data = opcode & 0xffff;

				if( !COND( cond) )
				{
					switch( subtype )
					{
						case 0x00:  /* BRUL <addr> */
							GETPC() = data - 1;
						break;

						case 0x02:  /* BRUL indirect */
							data = GETREGS(data&0x7f,0) - 1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
						break;

						case 0x04:  /* BSUL <addr> */
							GETPCS()[GETPCSP()] = GETPC();
							GETPCSP()++;
							GETPC() = data - 1;
						break;

						case 0x06:  /* BSUL indirect */
							data = GETARAM()[data] - 1;

							/* if we're waiting for data, don't complete the instruction */
							if ( GETFIFOWAIT() )
								break;

							GETPC() = data;
						break;

						case 0x0a:  /* RTUL */
							--GETPCSP();
							GETPC() = GETPCS()[GETPCSP()];
						break;

						case 0x0c:  /* LDUL */
							SETREGS(((data>>9)&0x7f), GETARAM()[data&0x1FF] );
						break;

						case 0x0e:  /* RIUL */
							fatalerror( "TGP: RIUL unimplemented at PC:%04x\n", GETPC() );

						default:
							fatalerror( "TGP: Unknown Branch opcode (subtype=%d) at PC:%04x\n", subtype, GETPC() );
					}
				}
			}
			break;

			case 0x1f:
			case 0x12:
				logerror( "TGP: unknown opcode %08x at PC:%04x (%02x)\n", opcode, GETPC(),(opcode >> 26) & 0x3f );
			break;

			default:
				fatalerror( "TGP: unknown opcode %08x at PC:%04x (%02x)\n", opcode, GETPC(),(opcode >> 26) & 0x3f );
		}

		if ( GETFIFOWAIT() == 0 )
		{
			if( GETREPS() == 0 )
				GETPC()++;
			else
				--GETREPS();

			m_icount--;
		}
		else
		{
			m_icount = 0;
		}
	}
}
