// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   sh2.c
 *   Portable Hitachi SH-2 (SH7600 family) emulator
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was adapted to the MAME CPU core requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/

/*****************************************************************************
    Changes
    20130129 Angelo Salese
    - added illegal opcode exception handling, side effect of some Saturn games
      on loading like Feda or Falcom Classics Vol. 1
      (i.e. Master CPU Incautiously transfers memory from CD to work RAM H, and
            wipes out Slave CPU program code too while at it).

    20051129 Mariusz Wojcieszek
    - introduced memory_decrypted_read_word() for opcode fetching

    20050813 Mariusz Wojcieszek
    - fixed 64 bit / 32 bit division in division unit

    20031015 O. Galibert
    - dma fixes, thanks to sthief

    20031013 O. Galibert, A. Giles
    - timer fixes
    - multi-cpu simplifications

    20030915 O. Galibert
    - fix DMA1 irq vector
    - ignore writes to DRCRx
    - fix cpu number issues
    - fix slave/master recognition
    - fix wrong-cpu-in-context problem with the timers

    20021020 O. Galibert
    - DMA implementation, lightly tested
    - delay slot in debugger fixed
    - add divide box mirrors
    - Nicola-ify the indentation
    - Uncrapify sh2_internal_*
    - Put back nmi support that had been lost somehow

    20020914 R. Belmont
    - Initial SH2 internal timers implementation, based on code by O. Galibert.
      Makes music work in galspanic4/s/s2, panic street, cyvern, other SKNS games.
    - Fix to external division, thanks to "spice" on the E2J board.
      Corrects behavior of s1945ii turret boss.

    20020302 Olivier Galibert (galibert@mame.net)
    - Fixed interrupt in delay slot
    - Fixed rotcr
    - Fixed div1
    - Fixed mulu
    - Fixed negc

    20020301 R. Belmont
    - Fixed external division

    20020225 Olivier Galibert (galibert@mame.net)
    - Fixed interrupt handling

    20010207 Sylvain Glaize (mokona@puupuu.org)

    - Bug fix in void MOVBM(UINT32 m, UINT32 n) (see comment)
    - Support of full 32 bit addressing (RB, RW, RL and WB, WW, WL functions)
        reason : when the two high bits of the address are set, access is
        done directly in the cache data array. The SUPER KANEKO NOVA SYSTEM
        sets the stack pointer here, using these addresses as usual RAM access.

        No real cache support has been added.
    - Read/Write memory format correction (_bew to _bedw) (see also SH2
        definition in cpuintrf.c and DasmSH2(..) in sh2dasm.c )

    20010623 James Forshaw (TyRaNiD@totalise.net)

    - Modified operation of sh2_exception. Done cause mame irq system is stupid, and
      doesnt really seem designed for any more than 8 interrupt lines.

    20010701 James Forshaw (TyRaNiD@totalise.net)

    - Fixed DIV1 operation. Q bit now correctly generated

    20020218 Added save states (mokona@puupuu.org)

 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh2.h"
#include "sh2comn.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DISABLE_FAST_REGISTERS              (0) // set to 1 to turn off usage of register caching
#define SINGLE_INSTRUCTION_MODE             (0)

#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* size of the execution code cache */
#define CACHE_SIZE                  (32 * 1024 * 1024)

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         64
#define COMPILE_FORWARDS_BYTES          256
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/2) + (COMPILE_FORWARDS_BYTES/2))
#define COMPILE_MAX_SEQUENCE            64


const device_type SH1 = &device_creator<sh1_device>;
const device_type SH2 = &device_creator<sh2_device>;
const device_type SH2A = &device_creator<sh2a_device>;

/*-------------------------------------------------
    sh2_internal_a5 - read handler for
    SH2 internal map
-------------------------------------------------*/

READ32_MEMBER(sh2_device::sh2_internal_a5)
{
	return 0xa5a5a5a5;
}


/*-------------------------------------------------
    sh2_internal_map - maps SH2 built-ins
-------------------------------------------------*/

static ADDRESS_MAP_START( sh7604_map, AS_PROGRAM, 32, sh2_device )
	AM_RANGE(0x40000000, 0xbfffffff) AM_READ(sh2_internal_a5)
/*! 
  @todo: cps3boot breaks with this enabled. Needs customization ...
  */
//	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM // cache data array
//	AM_RANGE(0xffffff88, 0xffffff8b) AM_READWRITE(dma_dtcr0_r,dma_dtcr0_w)
	AM_RANGE(0xe0000000, 0xe00001ff) AM_MIRROR(0x1ffffe00) AM_READWRITE(sh7604_r, sh7604_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh7021_map, AS_PROGRAM, 32, sh2a_device )
//  overrides
	AM_RANGE(0x05ffff40, 0x05ffff43) AM_READWRITE(dma_sar0_r, dma_sar0_w)
	AM_RANGE(0x05ffff44, 0x05ffff47) AM_READWRITE(dma_dar0_r, dma_dar0_w)
	AM_RANGE(0x05ffff48, 0x05ffff4b) AM_READWRITE16(dmaor_r, dmaor_w,0xffff0000)
	AM_RANGE(0x05ffff48, 0x05ffff4b) AM_READWRITE16(dma_tcr0_r, dma_tcr0_w,0x0000ffff)
	AM_RANGE(0x05ffff4c, 0x05ffff4f) AM_READWRITE16(dma_chcr0_r, dma_chcr0_w, 0x0000ffff)
//  fall-back
 	AM_RANGE(0x05fffe00, 0x05ffffff) AM_READWRITE16(sh7021_r,sh7021_w,0xffffffff) // SH-7032H internal i/o
//	AM_RANGE(0x07000000, 0x070003ff) AM_RAM AM_SHARE("oram")// on-chip RAM, actually at 0xf000000 (1 kb)
//	AM_RANGE(0x0f000000, 0x0f0003ff) AM_RAM AM_SHARE("oram")// on-chip RAM, actually at 0xf000000 (1 kb)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh7032_map, AS_PROGRAM, 32, sh1_device )
//  fall-back
 	AM_RANGE(0x05fffe00, 0x05ffffff) AM_READWRITE16(sh7032_r,sh7032_w,0xffffffff) // SH-7032H internal i/o
ADDRESS_MAP_END

sh2_device::sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SH2, "SH-2", tag, owner, clock, "sh2", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, ADDRESS_MAP_NAME(sh7604_map))
	, m_decrypted_program_config("decrypted_opcodes", ENDIANNESS_BIG, 32, 32, 0)
	, m_is_slave(0)
	, m_cpu_type(CPU_TYPE_SH2)
	, m_cache(CACHE_SIZE + sizeof(internal_sh2_state))
	, m_drcuml(NULL)
//  , m_drcuml(*this, m_cache, 0, 1, 32, 1)
	, m_drcfe(NULL)
	, m_drcoptions(0)
	, m_sh2_state(NULL)
	, m_entry(NULL)
	, m_read8(NULL)
	, m_write8(NULL)
	, m_read16(NULL)
	, m_write16(NULL)
	, m_read32(NULL)
	, m_write32(NULL)
	, m_interrupt(NULL)
	, m_nocode(NULL)
	, m_out_of_cycles(NULL)
	, m_debugger_temp(0)
{
	m_isdrc = (mconfig.options().drc() && !mconfig.m_force_no_drc) ? true : false;
}


void sh2_device::device_stop()
{
	/* clean up the DRC */
	if ( m_drcuml )
	{
		auto_free(machine(), m_drcuml);
	}
}


sh2_device::sh2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int cpu_type, address_map_constructor internal_map, int addrlines )
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 32, addrlines, 0, internal_map)
	, m_decrypted_program_config("decrypted_opcodes", ENDIANNESS_BIG, 32, addrlines, 0)
	, m_is_slave(0)
	, m_cpu_type(cpu_type)
	, m_cache(CACHE_SIZE + sizeof(internal_sh2_state))
	, m_drcuml(NULL)
//  , m_drcuml(*this, m_cache, 0, 1, 32, 1)
	, m_drcfe(NULL)
	, m_drcoptions(0)
	, m_sh2_state(NULL)
	, m_entry(NULL)
	, m_read8(NULL)
	, m_write8(NULL)
	, m_read16(NULL)
	, m_write16(NULL)
	, m_read32(NULL)
	, m_write32(NULL)
	, m_interrupt(NULL)
	, m_nocode(NULL)
	, m_out_of_cycles(NULL)
{
	m_isdrc = (mconfig.options().drc() && !mconfig.m_force_no_drc) ? true : false;
}

sh2a_device::sh2a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh2_device(mconfig, SH1, "SH-2A", tag, owner, clock, "sh2a", __FILE__, CPU_TYPE_SH2, ADDRESS_MAP_NAME(sh7021_map), 28 )
{
}

sh1_device::sh1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sh2_device(mconfig, SH1, "SH-1", tag, owner, clock, "sh1", __FILE__, CPU_TYPE_SH1, ADDRESS_MAP_NAME(sh7032_map), 28 )
{
}

const address_space_config *sh2_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &m_decrypted_program_config : NULL;
	default:                   return NULL;
	}
}

offs_t sh2_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sh2 );
	return CPU_DISASSEMBLE_NAME( sh2 )(this, buffer, pc, oprom, opram, options);
}


/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS     1

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

UINT8 sh2_device::RB(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_byte(A & AM);

	return m_program->read_byte(A);
}

UINT16 sh2_device::RW(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_word(A & AM);

	return m_program->read_word(A);
}

UINT32 sh2_device::RL(offs_t A)
{
	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_dword(A & AM);

	return m_program->read_dword(A);
}

void sh2_device::WB(offs_t A, UINT8 V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_byte(A & AM,V);
		return;
	}
	
	m_program->write_byte(A,V);
}

void sh2_device::WW(offs_t A, UINT16 V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_word(A & AM,V);
		return;
	}
	
	m_program->write_word(A,V);
}

void sh2_device::WL(offs_t A, UINT32 V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_dword(A & AM,V);
		return;
	}
	
	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	m_program->write_dword(A,V);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
void sh2_device::ADD(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] += m_sh2_state->r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
void sh2_device::ADDI(UINT32 i, UINT32 n)
{
	m_sh2_state->r[n] += (INT32)(INT16)(INT8)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
void sh2_device::ADDC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = m_sh2_state->r[n] + m_sh2_state->r[m];
	tmp0 = m_sh2_state->r[n];
	m_sh2_state->r[n] = tmp1 + (m_sh2_state->sr & T);
	if (tmp0 > tmp1)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	if (tmp1 > m_sh2_state->r[n])
		m_sh2_state->sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
void sh2_device::ADDV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) m_sh2_state->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) m_sh2_state->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_sh2_state->r[n] += m_sh2_state->r[m];
	if ((INT32) m_sh2_state->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
void sh2_device::AND(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] &= m_sh2_state->r[m];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
void sh2_device::ANDI(UINT32 i)
{
	m_sh2_state->r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
void sh2_device::ANDM(UINT32 i)
{
	UINT32 temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = i & RB( m_sh2_state->ea );
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
void sh2_device::BF(UINT32 d)
{
	if ((m_sh2_state->sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
void sh2_device::BFS(UINT32 d)
{
	if ((m_sh2_state->sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		m_delay = m_sh2_state->pc;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
void sh2_device::BRA(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		UINT32 next_opcode = RW( m_sh2_state->ppc & AM );
		/* BRA  $
		 * NOP
		 */
		if (next_opcode == 0x0009)
			m_sh2_state->icount %= 3;   /* cycles for BRA $ and NOP taken (3) */
	}
#endif
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
void sh2_device::BRAF(UINT32 m)
{
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc += m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
void sh2_device::BSR(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
void sh2_device::BSRF(UINT32 m)
{
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc += m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
void sh2_device::BT(UINT32 d)
{
	if ((m_sh2_state->sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
void sh2_device::BTS(UINT32 d)
{
	if ((m_sh2_state->sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		m_delay = m_sh2_state->pc;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
void sh2_device::CLRMAC()
{
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
void sh2_device::CLRT()
{
	m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
void sh2_device::CMPEQ(UINT32 m, UINT32 n)
{
	if (m_sh2_state->r[n] == m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
void sh2_device::CMPGE(UINT32 m, UINT32 n)
{
	if ((INT32) m_sh2_state->r[n] >= (INT32) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
void sh2_device::CMPGT(UINT32 m, UINT32 n)
{
	if ((INT32) m_sh2_state->r[n] > (INT32) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
void sh2_device::CMPHI(UINT32 m, UINT32 n)
{
	if ((UINT32) m_sh2_state->r[n] > (UINT32) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
void sh2_device::CMPHS(UINT32 m, UINT32 n)
{
	if ((UINT32) m_sh2_state->r[n] >= (UINT32) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
void sh2_device::CMPPL(UINT32 n)
{
	if ((INT32) m_sh2_state->r[n] > 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
void sh2_device::CMPPZ(UINT32 n)
{
	if ((INT32) m_sh2_state->r[n] >= 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
void sh2_device::CMPSTR(UINT32 m, UINT32 n)
	{
	UINT32 temp;
	INT32 HH, HL, LH, LL;
	temp = m_sh2_state->r[n] ^ m_sh2_state->r[m];
	HH = (temp >> 24) & 0xff;
	HL = (temp >> 16) & 0xff;
	LH = (temp >> 8) & 0xff;
	LL = temp & 0xff;
	if (HH && HL && LH && LL)
	m_sh2_state->sr &= ~T;
	else
	m_sh2_state->sr |= T;
	}


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
void sh2_device::CMPIM(UINT32 i)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)i;

	if (m_sh2_state->r[0] == imm)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
void sh2_device::DIV0S(UINT32 m, UINT32 n)
{
	if ((m_sh2_state->r[n] & 0x80000000) == 0)
		m_sh2_state->sr &= ~Q;
	else
		m_sh2_state->sr |= Q;
	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->sr &= ~M;
	else
		m_sh2_state->sr |= M;
	if ((m_sh2_state->r[m] ^ m_sh2_state->r[n]) & 0x80000000)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
void sh2_device::DIV0U()
{
	m_sh2_state->sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
void sh2_device::DIV1(UINT32 m, UINT32 n)
{
	UINT32 tmp0;
	UINT32 old_q;

	old_q = m_sh2_state->sr & Q;
	if (0x80000000 & m_sh2_state->r[n])
		m_sh2_state->sr |= Q;
	else
		m_sh2_state->sr &= ~Q;

	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);

	if (!old_q)
	{
		if (!(m_sh2_state->sr & M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
			}
			else
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(m_sh2_state->sr & M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			else
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
		}
	}

	tmp0 = (m_sh2_state->sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  DMULS.L Rm,Rn */
void sh2_device::DMULS(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) m_sh2_state->r[n];
	tempm = (INT32) m_sh2_state->r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (m_sh2_state->r[n] ^ m_sh2_state->r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	m_sh2_state->mach = Res2;
	m_sh2_state->macl = Res0;
	m_sh2_state->icount--;
}

/*  DMULU.L Rm,Rn */
void sh2_device::DMULU(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = m_sh2_state->r[n] & 0x0000ffff;
	RnH = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	RmL = m_sh2_state->r[m] & 0x0000ffff;
	RmH = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	m_sh2_state->mach = Res2;
	m_sh2_state->macl = Res0;
	m_sh2_state->icount--;
}

/*  DT      Rn */
void sh2_device::DT(UINT32 n)
{
	m_sh2_state->r[n]--;
	if (m_sh2_state->r[n] == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW( m_sh2_state->ppc & AM );
		/* DT   Rn
		 * BF   $-2
		 */
		if (next_opcode == 0x8bfd)
		{
			while (m_sh2_state->r[n] > 1 && m_sh2_state->icount > 4)
			{
				m_sh2_state->r[n]--;
				m_sh2_state->icount -= 4;   /* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
void sh2_device::EXTSB(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = ((INT32)m_sh2_state->r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
void sh2_device::EXTSW(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = ((INT32)m_sh2_state->r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
void sh2_device::EXTUB(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
void sh2_device::EXTUW(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x0000ffff;
}

/*  ILLEGAL */
void sh2_device::ILLEGAL()
{
	logerror("SH2.%s: Illegal opcode at %08x\n", tag(), m_sh2_state->pc - 2);
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc - 2 ); /* push PC onto stack */

	/* fetch PC */
	m_sh2_state->pc = RL( m_sh2_state->vbr + 4 * 4 );

	/* TODO: timing is a guess */
	m_sh2_state->icount -= 5;
}


/*  JMP     @Rm */
void sh2_device::JMP(UINT32 m)
{
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  JSR     @Rm */
void sh2_device::JSR(UINT32 m)
{
	m_delay = m_sh2_state->pc;
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--;
}


/*  LDC     Rm,SR */
void sh2_device::LDCSR(UINT32 m)
{
	m_sh2_state->sr = m_sh2_state->r[m] & FLAGS;
	m_test_irq = 1;
}

/*  LDC     Rm,GBR */
void sh2_device::LDCGBR(UINT32 m)
{
	m_sh2_state->gbr = m_sh2_state->r[m];
}

/*  LDC     Rm,VBR */
void sh2_device::LDCVBR(UINT32 m)
{
	m_sh2_state->vbr = m_sh2_state->r[m];
}

/*  LDC.L   @Rm+,SR */
void sh2_device::LDCMSR(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->sr = RL( m_sh2_state->ea ) & FLAGS;
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
	m_test_irq = 1;
}

/*  LDC.L   @Rm+,GBR */
void sh2_device::LDCMGBR(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->gbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
void sh2_device::LDCMVBR(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->vbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDS     Rm,MACH */
void sh2_device::LDSMACH(UINT32 m)
{
	m_sh2_state->mach = m_sh2_state->r[m];
}

/*  LDS     Rm,MACL */
void sh2_device::LDSMACL(UINT32 m)
{
	m_sh2_state->macl = m_sh2_state->r[m];
}

/*  LDS     Rm,PR */
void sh2_device::LDSPR(UINT32 m)
{
	m_sh2_state->pr = m_sh2_state->r[m];
}

/*  LDS.L   @Rm+,MACH */
void sh2_device::LDSMMACH(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->mach = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
void sh2_device::LDSMMACL(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->macl = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
void sh2_device::LDSMPR(UINT32 m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->pr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
void sh2_device::MAC_L(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL( m_sh2_state->r[n] );
	m_sh2_state->r[n] += 4;
	tempm = (INT32) RL( m_sh2_state->r[m] );
	m_sh2_state->r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (m_sh2_state->sr & S)
	{
		Res0 = m_sh2_state->macl + Res0;
		if (m_sh2_state->macl > Res0)
			Res2++;
		Res2 += (m_sh2_state->mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		m_sh2_state->mach = Res2;
		m_sh2_state->macl = Res0;
	}
	else
	{
		Res0 = m_sh2_state->macl + Res0;
		if (m_sh2_state->macl > Res0)
			Res2++;
		Res2 += m_sh2_state->mach;
		m_sh2_state->mach = Res2;
		m_sh2_state->macl = Res0;
	}
	m_sh2_state->icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
void sh2_device::MAC_W(UINT32 m, UINT32 n)
{
	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW( m_sh2_state->r[n] );
	m_sh2_state->r[n] += 2;
	tempm = (INT32) RW( m_sh2_state->r[m] );
	m_sh2_state->r[m] += 2;
	templ = m_sh2_state->macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) m_sh2_state->macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	m_sh2_state->macl += tempm;
	if ((INT32) m_sh2_state->macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (m_sh2_state->sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					m_sh2_state->macl = 0x7fffffff;
				if (src == 2)
					m_sh2_state->macl = 0x80000000;
			}
	}
	else
	{
		m_sh2_state->mach += tempn;
		if (templ > m_sh2_state->macl)
			m_sh2_state->mach += 1;
	}
	m_sh2_state->icount -= 2;
}

/*  MOV     Rm,Rn */
void sh2_device::MOV(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m];
}

/*  MOV.B   Rm,@Rn */
void sh2_device::MOVBS(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
void sh2_device::MOVWS(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
void sh2_device::MOVLS(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @Rm,Rn */
void sh2_device::MOVBL(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( m_sh2_state->ea );
}

/*  MOV.W   @Rm,Rn */
void sh2_device::MOVWL(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16) RW( m_sh2_state->ea );
}

/*  MOV.L   @Rm,Rn */
void sh2_device::MOVLL(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   Rm,@-Rn */
void sh2_device::MOVBM(UINT32 m, UINT32 n)
{
	/* SMG : bug fix, was reading m_sh2_state->r[n] */
	UINT32 data = m_sh2_state->r[m] & 0x000000ff;

	m_sh2_state->r[n] -= 1;
	WB( m_sh2_state->r[n], data );
}

/*  MOV.W   Rm,@-Rn */
void sh2_device::MOVWM(UINT32 m, UINT32 n)
{
	UINT32 data = m_sh2_state->r[m] & 0x0000ffff;

	m_sh2_state->r[n] -= 2;
	WW( m_sh2_state->r[n], data );
}

/*  MOV.L   Rm,@-Rn */
void sh2_device::MOVLM(UINT32 m, UINT32 n)
{
	UINT32 data = m_sh2_state->r[m];

	m_sh2_state->r[n] -= 4;
	WL( m_sh2_state->r[n], data );
}

/*  MOV.B   @Rm+,Rn */
void sh2_device::MOVBP(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
void sh2_device::MOVWP(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16) RW( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
void sh2_device::MOVLP(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = RL( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
void sh2_device::MOVBS0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
void sh2_device::MOVWS0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
void sh2_device::MOVLS0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
void sh2_device::MOVBL0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( m_sh2_state->ea );
}

/*  MOV.W   @(R0,Rm),Rn */
void sh2_device::MOVWL0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16) RW( m_sh2_state->ea );
}

/*  MOV.L   @(R0,Rm),Rn */
void sh2_device::MOVLL0(UINT32 m, UINT32 n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV     #imm,Rn */
void sh2_device::MOVI(UINT32 i, UINT32 n)
{
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16)(INT8) i;
}

/*  MOV.W   @(disp8,PC),Rn */
void sh2_device::MOVWI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->r[n] = (UINT32)(INT32)(INT16) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,PC),Rn */
void sh2_device::MOVLI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
void sh2_device::MOVBLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	m_sh2_state->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
void sh2_device::MOVWLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	m_sh2_state->r[0] = (INT32)(INT16) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
void sh2_device::MOVLLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	m_sh2_state->r[0] = RL( m_sh2_state->ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
void sh2_device::MOVBSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
void sh2_device::MOVWSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
void sh2_device::MOVLSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
void sh2_device::MOVBS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
void sh2_device::MOVWS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
void sh2_device::MOVLS4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
void sh2_device::MOVBL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp;
	m_sh2_state->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
void sh2_device::MOVWL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 2;
	m_sh2_state->r[0] = (UINT32)(INT32)(INT16) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
void sh2_device::MOVLL4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOVA    @(disp8,PC),R0 */
void sh2_device::MOVA(UINT32 d)
{
	UINT32 disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[0] = m_sh2_state->ea;
}

/*  MOVT    Rn */
void sh2_device::MOVT(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->sr & T;
}

/*  MUL.L   Rm,Rn */
void sh2_device::MULL(UINT32 m, UINT32 n)
{
	m_sh2_state->macl = m_sh2_state->r[n] * m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  MULS    Rm,Rn */
void sh2_device::MULS(UINT32 m, UINT32 n)
{
	m_sh2_state->macl = (INT16) m_sh2_state->r[n] * (INT16) m_sh2_state->r[m];
}

/*  MULU    Rm,Rn */
void sh2_device::MULU(UINT32 m, UINT32 n)
{
	m_sh2_state->macl = (UINT16) m_sh2_state->r[n] * (UINT16) m_sh2_state->r[m];
}

/*  NEG     Rm,Rn */
void sh2_device::NEG(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = 0 - m_sh2_state->r[m];
}

/*  NEGC    Rm,Rn */
void sh2_device::NEGC(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = m_sh2_state->r[m];
	m_sh2_state->r[n] = -temp - (m_sh2_state->sr & T);
	if (temp || (m_sh2_state->sr & T))
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  NOP */
void sh2_device::NOP(void)
{
}

/*  NOT     Rm,Rn */
void sh2_device::NOT(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] = ~m_sh2_state->r[m];
}

/*  OR      Rm,Rn */
void sh2_device::OR(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] |= m_sh2_state->r[m];
}

/*  OR      #imm,R0 */
void sh2_device::ORI(UINT32 i)
{
	m_sh2_state->r[0] |= i;
}

/*  OR.B    #imm,@(R0,GBR) */
void sh2_device::ORM(UINT32 i)
{
	UINT32 temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp |= i;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  ROTCL   Rn */
void sh2_device::ROTCL(UINT32 n)
{
	UINT32 temp;

	temp = (m_sh2_state->r[n] >> 31) & T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | temp;
}

/*  ROTCR   Rn */
void sh2_device::ROTCR(UINT32 n)
{
	UINT32 temp;
	temp = (m_sh2_state->sr & T) << 31;
	if (m_sh2_state->r[n] & T)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
void sh2_device::ROTL(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->r[n] >> 31);
}

/*  ROTR    Rn */
void sh2_device::ROTR(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | (m_sh2_state->r[n] << 31);
}

/*  RTE */
void sh2_device::RTE()
{
	m_sh2_state->ea = m_sh2_state->r[15];
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc = RL( m_sh2_state->ea );
	m_sh2_state->r[15] += 4;
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->sr = RL( m_sh2_state->ea ) & FLAGS;
	m_sh2_state->r[15] += 4;
	m_sh2_state->icount -= 3;
	m_test_irq = 1;
}

/*  RTS */
void sh2_device::RTS()
{
	m_delay = m_sh2_state->pc;
	m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pr;
	m_sh2_state->icount--;
}

/*  SETT */
void sh2_device::SETT()
{
	m_sh2_state->sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
void sh2_device::SHAL(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHAR    Rn */
void sh2_device::SHAR(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (UINT32)((INT32)m_sh2_state->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
void sh2_device::SHLL(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHLL2   Rn */
void sh2_device::SHLL2(UINT32 n)
{
	m_sh2_state->r[n] <<= 2;
}

/*  SHLL8   Rn */
void sh2_device::SHLL8(UINT32 n)
{
	m_sh2_state->r[n] <<= 8;
}

/*  SHLL16  Rn */
void sh2_device::SHLL16(UINT32 n)
{
	m_sh2_state->r[n] <<= 16;
}

/*  SHLR    Rn */
void sh2_device::SHLR(UINT32 n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] >>= 1;
}

/*  SHLR2   Rn */
void sh2_device::SHLR2(UINT32 n)
{
	m_sh2_state->r[n] >>= 2;
}

/*  SHLR8   Rn */
void sh2_device::SHLR8(UINT32 n)
{
	m_sh2_state->r[n] >>= 8;
}

/*  SHLR16  Rn */
void sh2_device::SHLR16(UINT32 n)
{
	m_sh2_state->r[n] >>= 16;
}

/*  SLEEP */
void sh2_device::SLEEP()
{
	if(m_sh2_state->sleep_mode != 2)
		m_sh2_state->pc -= 2;
	m_sh2_state->icount -= 2;
	/* Wait_for_exception; */
	if(m_sh2_state->sleep_mode == 0)
		m_sh2_state->sleep_mode = 1;
	else if(m_sh2_state->sleep_mode == 2)
		m_sh2_state->sleep_mode = 0;
}

/*  STC     SR,Rn */
void sh2_device::STCSR(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->sr;
}

/*  STC     GBR,Rn */
void sh2_device::STCGBR(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->gbr;
}

/*  STC     VBR,Rn */
void sh2_device::STCVBR(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->vbr;
}

/*  STC.L   SR,@-Rn */
void sh2_device::STCMSR(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->sr );
	m_sh2_state->icount--;
}

/*  STC.L   GBR,@-Rn */
void sh2_device::STCMGBR(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->gbr );
	m_sh2_state->icount--;
}

/*  STC.L   VBR,@-Rn */
void sh2_device::STCMVBR(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->vbr );
	m_sh2_state->icount--;
}

/*  STS     MACH,Rn */
void sh2_device::STSMACH(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->mach;
}

/*  STS     MACL,Rn */
void sh2_device::STSMACL(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->macl;
}

/*  STS     PR,Rn */
void sh2_device::STSPR(UINT32 n)
{
	m_sh2_state->r[n] = m_sh2_state->pr;
}

/*  STS.L   MACH,@-Rn */
void sh2_device::STSMMACH(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->mach );
}

/*  STS.L   MACL,@-Rn */
void sh2_device::STSMMACL(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->macl );
}

/*  STS.L   PR,@-Rn */
void sh2_device::STSMPR(UINT32 n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->pr );
}

/*  SUB     Rm,Rn */
void sh2_device::SUB(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] -= m_sh2_state->r[m];
}

/*  SUBC    Rm,Rn */
void sh2_device::SUBC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = m_sh2_state->r[n] - m_sh2_state->r[m];
	tmp0 = m_sh2_state->r[n];
	m_sh2_state->r[n] = tmp1 - (m_sh2_state->sr & T);
	if (tmp0 < tmp1)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	if (tmp1 < m_sh2_state->r[n])
		m_sh2_state->sr |= T;
}

/*  SUBV    Rm,Rn */
void sh2_device::SUBV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) m_sh2_state->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) m_sh2_state->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_sh2_state->r[n] -= m_sh2_state->r[m];
	if ((INT32) m_sh2_state->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
	else
		m_sh2_state->sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
void sh2_device::SWAPB(UINT32 m, UINT32 n)
{
	UINT32 temp0, temp1;

	temp0 = m_sh2_state->r[m] & 0xffff0000;
	temp1 = (m_sh2_state->r[m] & 0x000000ff) << 8;
	m_sh2_state->r[n] = (m_sh2_state->r[m] >> 8) & 0x000000ff;
	m_sh2_state->r[n] = m_sh2_state->r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
void sh2_device::SWAPW(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] = (m_sh2_state->r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
void sh2_device::TAS(UINT32 n)
{
	UINT32 temp;
	m_sh2_state->ea = m_sh2_state->r[n];
	/* Bus Lock enable */
	temp = RB( m_sh2_state->ea );
	if (temp == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 3;
}

/*  TRAPA   #imm */
void sh2_device::TRAPA(UINT32 i)
{
	UINT32 imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->vbr + imm * 4;

	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc );

	m_sh2_state->pc = RL( m_sh2_state->ea );

	m_sh2_state->icount -= 7;
}

/*  TST     Rm,Rn */
void sh2_device::TST(UINT32 m, UINT32 n)
{
	if ((m_sh2_state->r[n] & m_sh2_state->r[m]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST     #imm,R0 */
void sh2_device::TSTI(UINT32 i)
{
	UINT32 imm = i & 0xff;

	if ((imm & m_sh2_state->r[0]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
void sh2_device::TSTM(UINT32 i)
{
	UINT32 imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	if ((imm & RB( m_sh2_state->ea )) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->icount -= 2;
}

/*  XOR     Rm,Rn */
void sh2_device::XOR(UINT32 m, UINT32 n)
{
	m_sh2_state->r[n] ^= m_sh2_state->r[m];
}

/*  XOR     #imm,R0 */
void sh2_device::XORI(UINT32 i)
{
	UINT32 imm = i & 0xff;
	m_sh2_state->r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
void sh2_device::XORM(UINT32 i)
{
	UINT32 imm = i & 0xff;
	UINT32 temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp ^= imm;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  XTRCT   Rm,Rn */
void sh2_device::XTRCT(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (m_sh2_state->r[m] << 16) & 0xffff0000;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] |= temp;
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

void sh2_device::op0000(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: ILLEGAL();                       break;
	case 0x01: ILLEGAL();                       break;
	case 0x02: STCSR(Rn);                  break;
	case 0x03: BSRF(Rn);                   break;
	case 0x04: MOVBS0(Rm, Rn);             break;
	case 0x05: MOVWS0(Rm, Rn);             break;
	case 0x06: MOVLS0(Rm, Rn);             break;
	case 0x07: MULL(Rm, Rn);               break;
	case 0x08: CLRT();                       break;
	case 0x09: NOP();                           break;
	case 0x0a: STSMACH(Rn);                break;
	case 0x0b: RTS();                        break;
	case 0x0c: MOVBL0(Rm, Rn);             break;
	case 0x0d: MOVWL0(Rm, Rn);             break;
	case 0x0e: MOVLL0(Rm, Rn);             break;
	case 0x0f: MAC_L(Rm, Rn);              break;

	case 0x10: ILLEGAL();                       break;
	case 0x11: ILLEGAL();                       break;
	case 0x12: STCGBR(Rn);                 break;
	case 0x13: ILLEGAL();                       break;
	case 0x14: MOVBS0(Rm, Rn);             break;
	case 0x15: MOVWS0(Rm, Rn);             break;
	case 0x16: MOVLS0(Rm, Rn);             break;
	case 0x17: MULL(Rm, Rn);               break;
	case 0x18: SETT();                       break;
	case 0x19: DIV0U();                  break;
	case 0x1a: STSMACL(Rn);                break;
	case 0x1b: SLEEP();                  break;
	case 0x1c: MOVBL0(Rm, Rn);             break;
	case 0x1d: MOVWL0(Rm, Rn);             break;
	case 0x1e: MOVLL0(Rm, Rn);             break;
	case 0x1f: MAC_L(Rm, Rn);              break;

	case 0x20: ILLEGAL();                       break;
	case 0x21: ILLEGAL();                       break;
	case 0x22: STCVBR(Rn);                 break;
	case 0x23: BRAF(Rn);                   break;
	case 0x24: MOVBS0(Rm, Rn);             break;
	case 0x25: MOVWS0(Rm, Rn);             break;
	case 0x26: MOVLS0(Rm, Rn);             break;
	case 0x27: MULL(Rm, Rn);               break;
	case 0x28: CLRMAC();                 break;
	case 0x29: MOVT(Rn);                   break;
	case 0x2a: STSPR(Rn);                  break;
	case 0x2b: RTE();                        break;
	case 0x2c: MOVBL0(Rm, Rn);             break;
	case 0x2d: MOVWL0(Rm, Rn);             break;
	case 0x2e: MOVLL0(Rm, Rn);             break;
	case 0x2f: MAC_L(Rm, Rn);              break;

	case 0x30: ILLEGAL();                       break;
	case 0x31: ILLEGAL();                       break;
	case 0x32: ILLEGAL();                       break;
	case 0x33: ILLEGAL();                       break;
	case 0x34: MOVBS0(Rm, Rn);             break;
	case 0x35: MOVWS0(Rm, Rn);             break;
	case 0x36: MOVLS0(Rm, Rn);             break;
	case 0x37: MULL(Rm, Rn);               break;
	case 0x38: ILLEGAL();                       break;
	case 0x39: ILLEGAL();                       break;
	case 0x3c: MOVBL0(Rm, Rn);             break;
	case 0x3d: MOVWL0(Rm, Rn);             break;
	case 0x3e: MOVLL0(Rm, Rn);             break;
	case 0x3f: MAC_L(Rm, Rn);              break;
	case 0x3a: ILLEGAL();                       break;
	case 0x3b: ILLEGAL();                       break;



	}
}

void sh2_device::op0001(UINT16 opcode)
{
	MOVLS4(Rm, opcode & 0x0f, Rn);
}

void sh2_device::op0010(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBS(Rm, Rn);                break;
	case  1: MOVWS(Rm, Rn);                break;
	case  2: MOVLS(Rm, Rn);                break;
	case  3: ILLEGAL();                         break;
	case  4: MOVBM(Rm, Rn);                break;
	case  5: MOVWM(Rm, Rn);                break;
	case  6: MOVLM(Rm, Rn);                break;
	case  7: DIV0S(Rm, Rn);                break;
	case  8: TST(Rm, Rn);                  break;
	case  9: AND(Rm, Rn);                  break;
	case 10: XOR(Rm, Rn);                  break;
	case 11: OR(Rm, Rn);                   break;
	case 12: CMPSTR(Rm, Rn);               break;
	case 13: XTRCT(Rm, Rn);                break;
	case 14: MULU(Rm, Rn);                 break;
	case 15: MULS(Rm, Rn);                 break;
	}
}

void sh2_device::op0011(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: CMPEQ(Rm, Rn);                break;
	case  1: ILLEGAL();                         break;
	case  2: CMPHS(Rm, Rn);                break;
	case  3: CMPGE(Rm, Rn);                break;
	case  4: DIV1(Rm, Rn);                 break;
	case  5: DMULU(Rm, Rn);                break;
	case  6: CMPHI(Rm, Rn);                break;
	case  7: CMPGT(Rm, Rn);                break;
	case  8: SUB(Rm, Rn);                  break;
	case  9: ILLEGAL();                         break;
	case 10: SUBC(Rm, Rn);                 break;
	case 11: SUBV(Rm, Rn);                 break;
	case 12: ADD(Rm, Rn);                  break;
	case 13: DMULS(Rm, Rn);                break;
	case 14: ADDC(Rm, Rn);                 break;
	case 15: ADDV(Rm, Rn);                 break;
	}
}

void sh2_device::op0100(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: SHLL(Rn);                   break;
	case 0x01: SHLR(Rn);                   break;
	case 0x02: STSMMACH(Rn);               break;
	case 0x03: STCMSR(Rn);                 break;
	case 0x04: ROTL(Rn);                   break;
	case 0x05: ROTR(Rn);                   break;
	case 0x06: LDSMMACH(Rn);               break;
	case 0x07: LDCMSR(Rn);                 break;
	case 0x08: SHLL2(Rn);                  break;
	case 0x09: SHLR2(Rn);                  break;
	case 0x0a: LDSMACH(Rn);                break;
	case 0x0b: JSR(Rn);                    break;
	case 0x0c: ILLEGAL();                       break;
	case 0x0d: ILLEGAL();                       break;
	case 0x0e: LDCSR(Rn);                  break;
	case 0x0f: MAC_W(Rm, Rn);              break;

	case 0x10: DT(Rn);                     break;
	case 0x11: CMPPZ(Rn);                  break;
	case 0x12: STSMMACL(Rn);               break;
	case 0x13: STCMGBR(Rn);                break;
	case 0x14: ILLEGAL();                       break;
	case 0x15: CMPPL(Rn);                  break;
	case 0x16: LDSMMACL(Rn);               break;
	case 0x17: LDCMGBR(Rn);                break;
	case 0x18: SHLL8(Rn);                  break;
	case 0x19: SHLR8(Rn);                  break;
	case 0x1a: LDSMACL(Rn);                break;
	case 0x1b: TAS(Rn);                    break;
	case 0x1c: ILLEGAL();                       break;
	case 0x1d: ILLEGAL();                       break;
	case 0x1e: LDCGBR(Rn);                 break;
	case 0x1f: MAC_W(Rm, Rn);              break;

	case 0x20: SHAL(Rn);                   break;
	case 0x21: SHAR(Rn);                   break;
	case 0x22: STSMPR(Rn);                 break;
	case 0x23: STCMVBR(Rn);                break;
	case 0x24: ROTCL(Rn);                  break;
	case 0x25: ROTCR(Rn);                  break;
	case 0x26: LDSMPR(Rn);                 break;
	case 0x27: LDCMVBR(Rn);                break;
	case 0x28: SHLL16(Rn);                 break;
	case 0x29: SHLR16(Rn);                 break;
	case 0x2a: LDSPR(Rn);                  break;
	case 0x2b: JMP(Rn);                    break;
	case 0x2c: ILLEGAL();                       break;
	case 0x2d: ILLEGAL();                       break;
	case 0x2e: LDCVBR(Rn);                 break;
	case 0x2f: MAC_W(Rm, Rn);              break;

	case 0x30: ILLEGAL();                       break;
	case 0x31: ILLEGAL();                       break;
	case 0x32: ILLEGAL();                       break;
	case 0x33: ILLEGAL();                       break;
	case 0x34: ILLEGAL();                       break;
	case 0x35: ILLEGAL();                       break;
	case 0x36: ILLEGAL();                       break;
	case 0x37: ILLEGAL();                       break;
	case 0x38: ILLEGAL();                       break;
	case 0x39: ILLEGAL();                       break;
	case 0x3a: ILLEGAL();                       break;
	case 0x3b: ILLEGAL();                       break;
	case 0x3c: ILLEGAL();                       break;
	case 0x3d: ILLEGAL();                       break;
	case 0x3e: ILLEGAL();                       break;
	case 0x3f: MAC_W(Rm, Rn);              break;

	}
}

void sh2_device::op0101(UINT16 opcode)
{
	MOVLL4(Rm, opcode & 0x0f, Rn);
}

void sh2_device::op0110(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBL(Rm, Rn);                break;
	case  1: MOVWL(Rm, Rn);                break;
	case  2: MOVLL(Rm, Rn);                break;
	case  3: MOV(Rm, Rn);                  break;
	case  4: MOVBP(Rm, Rn);                break;
	case  5: MOVWP(Rm, Rn);                break;
	case  6: MOVLP(Rm, Rn);                break;
	case  7: NOT(Rm, Rn);                  break;
	case  8: SWAPB(Rm, Rn);                break;
	case  9: SWAPW(Rm, Rn);                break;
	case 10: NEGC(Rm, Rn);                 break;
	case 11: NEG(Rm, Rn);                  break;
	case 12: EXTUB(Rm, Rn);                break;
	case 13: EXTUW(Rm, Rn);                break;
	case 14: EXTSB(Rm, Rn);                break;
	case 15: EXTSW(Rm, Rn);                break;
	}
}

void sh2_device::op0111(UINT16 opcode)
{
	ADDI(opcode & 0xff, Rn);
}

void sh2_device::op1000(UINT16 opcode)
{
	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: MOVBS4(opcode & 0x0f, Rm);   break;
	case  1 << 8: MOVWS4(opcode & 0x0f, Rm);   break;
	case  2<< 8: ILLEGAL();                 break;
	case  3<< 8: ILLEGAL();                 break;
	case  4<< 8: MOVBL4(Rm, opcode & 0x0f);    break;
	case  5<< 8: MOVWL4(Rm, opcode & 0x0f);    break;
	case  6<< 8: ILLEGAL();                 break;
	case  7<< 8: ILLEGAL();                 break;
	case  8<< 8: CMPIM(opcode & 0xff);     break;
	case  9<< 8: BT(opcode & 0xff);        break;
	case 10<< 8: ILLEGAL();                 break;
	case 11<< 8: BF(opcode & 0xff);        break;
	case 12<< 8: ILLEGAL();                 break;
	case 13<< 8: BTS(opcode & 0xff);       break;
	case 14<< 8: ILLEGAL();                 break;
	case 15<< 8: BFS(opcode & 0xff);       break;
	}
}


void sh2_device::op1001(UINT16 opcode)
{
	MOVWI(opcode & 0xff, Rn);
}

void sh2_device::op1010(UINT16 opcode)
{
	BRA(opcode & 0xfff);
}

void sh2_device::op1011(UINT16 opcode)
{
	BSR(opcode & 0xfff);
}

void sh2_device::op1100(UINT16 opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: MOVBSG(opcode & 0xff);     break;
	case  1<<8: MOVWSG(opcode & 0xff);     break;
	case  2<<8: MOVLSG(opcode & 0xff);     break;
	case  3<<8: TRAPA(opcode & 0xff);      break;
	case  4<<8: MOVBLG(opcode & 0xff);     break;
	case  5<<8: MOVWLG(opcode & 0xff);     break;
	case  6<<8: MOVLLG(opcode & 0xff);     break;
	case  7<<8: MOVA(opcode & 0xff);       break;
	case  8<<8: TSTI(opcode & 0xff);       break;
	case  9<<8: ANDI(opcode & 0xff);       break;
	case 10<<8: XORI(opcode & 0xff);       break;
	case 11<<8: ORI(opcode & 0xff);            break;
	case 12<<8: TSTM(opcode & 0xff);       break;
	case 13<<8: ANDM(opcode & 0xff);       break;
	case 14<<8: XORM(opcode & 0xff);       break;
	case 15<<8: ORM(opcode & 0xff);            break;
	}
}

void sh2_device::op1101(UINT16 opcode)
{
	MOVLI(opcode & 0xff, Rn);
}

void sh2_device::op1110(UINT16 opcode)
{
	MOVI(opcode & 0xff, Rn);
}

void sh2_device::op1111(UINT16 opcode)
{
	ILLEGAL();
}

/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

void sh2_device::device_reset()
{
	m_sh2_state->ppc = m_sh2_state->pc = m_sh2_state->pr = m_sh2_state->sr = m_sh2_state->gbr = m_sh2_state->vbr = m_sh2_state->mach = m_sh2_state->macl = 0;
	m_sh2_state->evec = m_sh2_state->irqsr = 0;
	memset(&m_sh2_state->r[0], 0, sizeof(m_sh2_state->r[0])*16);
	m_sh2_state->ea = m_delay = m_cpu_off = m_dvsr = m_dvdnth = m_dvdntl = m_dvcr = 0;
	m_sh2_state->pending_irq = m_test_irq = 0;
	memset(&m_irq_queue[0], 0, sizeof(m_irq_queue[0])*16);
	memset(&m_irq_line_state[0], 0, sizeof(m_irq_line_state[0])*17);
	m_frc = m_ocra = m_ocrb = m_icr = 0;
	m_frc_base = 0;
	m_frt_input = m_sh2_state->internal_irq_level = m_internal_irq_vector = 0;
	m_dma_timer_active[0] = m_dma_timer_active[1] = 0;
	m_dma_irq[0] = m_dma_irq[1] = 0;

	memset(m_m, 0, 0x200);

	m_sh2_state->pc = RL(0);
	m_sh2_state->r[15] = RL(4);
	m_sh2_state->sr = I;
	m_sh2_state->sleep_mode = 0;

	m_sh2_state->internal_irq_level = -1;

	m_cache_dirty = TRUE;
}


/* Execute cycles - returns number of cycles actually run */
void sh2_device::execute_run()
{
	if ( m_isdrc )
	{
		execute_run_drc();
		return;
	}

	if (m_cpu_off)
	{
		m_sh2_state->icount = 0;
		return;
	}

	// run any active DMAs now
#ifndef USE_TIMER_FOR_DMA
	for ( int i = 0; i < m_sh2_state->icount ; i++)
	{
		for( int dma=0;dma<1;dma++)
		{
			if (m_dma_timer_active[dma])
				sh2_do_dma(dma);
		}
	}
#endif

	do
	{
		UINT32 opcode;

		if (m_delay)
		{
			opcode = m_program->read_word(((UINT32)(m_delay & AM)));
			m_sh2_state->pc -= 2;
		}
		else
			opcode = m_program->read_word(((UINT32)(m_sh2_state->pc & AM)));

		debugger_instruction_hook(this, m_sh2_state->pc);

		m_delay = 0;
		m_sh2_state->pc += 2;
		m_sh2_state->ppc = m_sh2_state->pc;

		switch (opcode & ( 15 << 12))
		{
		case  0<<12: op0000(opcode); break;
		case  1<<12: op0001(opcode); break;
		case  2<<12: op0010(opcode); break;
		case  3<<12: op0011(opcode); break;
		case  4<<12: op0100(opcode); break;
		case  5<<12: op0101(opcode); break;
		case  6<<12: op0110(opcode); break;
		case  7<<12: op0111(opcode); break;
		case  8<<12: op1000(opcode); break;
		case  9<<12: op1001(opcode); break;
		case 10<<12: op1010(opcode); break;
		case 11<<12: op1011(opcode); break;
		case 12<<12: op1100(opcode); break;
		case 13<<12: op1101(opcode); break;
		case 14<<12: op1110(opcode); break;
		default: op1111(opcode); break;
		}

		if(m_test_irq && !m_delay)
		{
			CHECK_PENDING_IRQ("mame_sh2_execute");
			m_test_irq = 0;
		}
		m_sh2_state->icount--;
	} while( m_sh2_state->icount > 0 );
}

void sh2_device::device_start()
{
	/* allocate the implementation-specific state from the full cache */
	m_sh2_state = (internal_sh2_state *)m_cache.alloc_near(sizeof(internal_sh2_state));

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh2_device::sh2_timer_callback), this));
	m_timer->adjust(attotime::never);

	m_dma_current_active_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh2_device::sh2_dma_current_active_callback), this));
	m_dma_current_active_timer[0]->adjust(attotime::never);

	m_dma_current_active_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sh2_device::sh2_dma_current_active_callback), this));
	m_dma_current_active_timer[1]->adjust(attotime::never);

	/* resolve callbacks */
	m_dma_kludge_cb.bind_relative_to(*owner());
	m_dma_fifo_data_available_cb.bind_relative_to(*owner());
	m_ftcsr_read_cb.bind_relative_to(*owner());

	m_program = &space(AS_PROGRAM);
	m_decrypted_program = has_space(AS_DECRYPTED_OPCODES) ? &space(AS_DECRYPTED_OPCODES) : &space(AS_PROGRAM);
	m_direct = &m_decrypted_program->direct();
	m_internal = &space(AS_PROGRAM);

	save_item(NAME(m_sh2_state->pc));
	save_item(NAME(m_sh2_state->sr));
	save_item(NAME(m_sh2_state->pr));
	save_item(NAME(m_sh2_state->gbr));
	save_item(NAME(m_sh2_state->vbr));
	save_item(NAME(m_sh2_state->mach));
	save_item(NAME(m_sh2_state->macl));
	save_item(NAME(m_sh2_state->r));
	save_item(NAME(m_sh2_state->ea));
	save_item(NAME(m_delay));
	save_item(NAME(m_cpu_off));
	save_item(NAME(m_dvsr));
	save_item(NAME(m_dvdnth));
	save_item(NAME(m_dvdntl));
	save_item(NAME(m_dvcr));
	save_item(NAME(m_sh2_state->pending_irq));
	save_item(NAME(m_test_irq));
	save_item(NAME(m_sh2_state->pending_nmi));
	save_item(NAME(m_sh2_state->irqline));
	save_item(NAME(m_sh2_state->evec));
	save_item(NAME(m_sh2_state->irqsr));
	save_item(NAME(m_sh2_state->target));
	for (int i = 0; i < 16; ++i)
	{
		save_item(NAME(m_irq_queue[i].irq_vector), i);
		save_item(NAME(m_irq_queue[i].irq_priority), i);
	}
	save_item(NAME(m_pcfsel));
	save_item(NAME(m_maxpcfsel));
	save_item(NAME(m_pcflushes));
	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_m));
	save_item(NAME(m_nmi_line_state));
	save_item(NAME(m_frc));
	save_item(NAME(m_ocra));
	save_item(NAME(m_ocrb));
	save_item(NAME(m_icr));
	save_item(NAME(m_frc_base));
	save_item(NAME(m_frt_input));
	save_item(NAME(m_sh2_state->internal_irq_level));
	save_item(NAME(m_internal_irq_vector));
	save_item(NAME(m_dma_timer_active));
	save_item(NAME(m_dma_irq));
	save_item(NAME(m_wtcnt));
	save_item(NAME(m_wtcsr));
	save_item(NAME(m_sh2_state->sleep_mode));

	state_add( SH2_PC,   "PC",   m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add( SH2_SR,   "SR",   m_sh2_state->sr).callimport().formatstr("%08X");
	state_add( SH2_PR,   "PR",   m_sh2_state->pr).formatstr("%08X");
	state_add( SH2_GBR,  "GBR",  m_sh2_state->gbr).formatstr("%08X");
	state_add( SH2_VBR,  "VBR",  m_sh2_state->vbr).formatstr("%08X");
	state_add( SH2_MACH, "MACH", m_sh2_state->mach).formatstr("%08X");
	state_add( SH2_MACL, "MACL", m_sh2_state->macl).formatstr("%08X");
	state_add( SH2_R0,   "R0",   m_sh2_state->r[ 0]).formatstr("%08X");
	state_add( SH2_R1,   "R1",   m_sh2_state->r[ 1]).formatstr("%08X");
	state_add( SH2_R2,   "R2",   m_sh2_state->r[ 2]).formatstr("%08X");
	state_add( SH2_R3,   "R3",   m_sh2_state->r[ 3]).formatstr("%08X");
	state_add( SH2_R4,   "R4",   m_sh2_state->r[ 4]).formatstr("%08X");
	state_add( SH2_R5,   "R5",   m_sh2_state->r[ 5]).formatstr("%08X");
	state_add( SH2_R6,   "R6",   m_sh2_state->r[ 6]).formatstr("%08X");
	state_add( SH2_R7,   "R7",   m_sh2_state->r[ 7]).formatstr("%08X");
	state_add( SH2_R8,   "R8",   m_sh2_state->r[ 8]).formatstr("%08X");
	state_add( SH2_R9,   "R9",   m_sh2_state->r[ 9]).formatstr("%08X");
	state_add( SH2_R10,  "R10",  m_sh2_state->r[10]).formatstr("%08X");
	state_add( SH2_R11,  "R11",  m_sh2_state->r[11]).formatstr("%08X");
	state_add( SH2_R12,  "R12",  m_sh2_state->r[12]).formatstr("%08X");
	state_add( SH2_R13,  "R13",  m_sh2_state->r[13]).formatstr("%08X");
	state_add( SH2_R14,  "R14",  m_sh2_state->r[14]).formatstr("%08X");
	state_add( SH2_R15,  "R15",  m_sh2_state->r[15]).formatstr("%08X");
	state_add( SH2_EA,   "EA",   m_sh2_state->ea).formatstr("%08X");

	state_add( STATE_GENPC, "GENPC", m_sh2_state->pc ).noshow();
	state_add( STATE_GENSP, "GENSP", m_sh2_state->r[15] ).noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_sh2_state->ppc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_sh2_state->sr ).formatstr("%6s").noshow();

	m_icountptr = &m_sh2_state->icount;

	// Clear state
	m_sh2_state->ppc = 0;
	m_sh2_state->pc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	m_sh2_state->ea = 0;
	m_delay = 0;
	m_cpu_off = 0;
	m_dvsr = 0;
	m_dvdnth = 0;
	m_dvdntl = 0;
	m_dvcr = 0;
	m_sh2_state->pending_irq = 0;
	m_test_irq = 0;
	m_sh2_state->pending_nmi = 0;
	m_sh2_state->irqline = 0;
	m_sh2_state->evec = 0;
	m_sh2_state->irqsr = 0;
	m_sh2_state->target = 0;
	memset(m_irq_queue, 0, sizeof(m_irq_queue));
	m_maxpcfsel = 0;
	memset(m_pcflushes, 0, sizeof(m_pcflushes));
	memset(m_irq_line_state, 0, sizeof(m_irq_line_state));
	memset(m_m, 0, sizeof(m_m));
	m_nmi_line_state = 0;
	m_frc = 0;
	m_ocra = 0;
	m_ocrb = 0;
	m_icr = 0;
	m_frc_base = 0;
	m_frt_input = 0;
	m_sh2_state->internal_irq_level = 0;
	m_internal_irq_vector = 0;
	m_sh2_state->icount = 0;
	for ( int i = 0; i < 2; i++ )
	{
		m_dma_timer_active[i] = 0;
		m_dma_irq[i] = 0;
		m_active_dma_incs[i] = 0;
		m_active_dma_incd[i] = 0;
		m_active_dma_size[i] = 0;
		m_active_dma_steal[i] = 0;
		m_active_dma_src[i] = 0;
		m_active_dma_dst[i] = 0;
		m_active_dma_count[i] = 0;
	}
	m_wtcnt = 0;
	m_wtcsr = 0;
	m_sh2_state->sleep_mode = 0;
	m_numcycles = 0;
	m_sh2_state->arg0 = 0;
	m_arg1 = 0;
	m_irq = 0;
	m_fastram_select = 0;
	memset(m_fastram, 0, sizeof(m_fastram));

	/* reset per-driver pcflushes */
	m_pcfsel = 0;

	/* initialize the UML generator */
	UINT32 flags = 0;
	m_drcuml = auto_alloc(machine(), drcuml_state(*this, m_cache, flags, 1, 32, 1));

	/* add symbols for our stuff */
	m_drcuml->symbol_add(&m_sh2_state->pc, sizeof(m_sh2_state->pc), "pc");
	m_drcuml->symbol_add(&m_sh2_state->icount, sizeof(m_sh2_state->icount), "icount");
	for (int regnum = 0; regnum < 16; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		m_drcuml->symbol_add(&m_sh2_state->r[regnum], sizeof(m_sh2_state->r[regnum]), buf);
	}
	m_drcuml->symbol_add(&m_sh2_state->pr, sizeof(m_sh2_state->pr), "pr");
	m_drcuml->symbol_add(&m_sh2_state->sr, sizeof(m_sh2_state->sr), "sr");
	m_drcuml->symbol_add(&m_sh2_state->gbr, sizeof(m_sh2_state->gbr), "gbr");
	m_drcuml->symbol_add(&m_sh2_state->vbr, sizeof(m_sh2_state->vbr), "vbr");
	m_drcuml->symbol_add(&m_sh2_state->macl, sizeof(m_sh2_state->macl), "macl");
	m_drcuml->symbol_add(&m_sh2_state->mach, sizeof(m_sh2_state->macl), "mach");

	/* initialize the front-end helper */
	m_drcfe = auto_alloc(machine(), sh2_frontend(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* compute the register parameters */
	for (int regnum = 0; regnum < 16; regnum++)
	{
		m_regmap[regnum] = uml::mem(&m_sh2_state->r[regnum]);
	}

	/* if we have registers to spare, assign r0, r1, r2 to leftovers */
	/* WARNING: do not use synthetic registers that are mapped here! */
	if (!DISABLE_FAST_REGISTERS)
	{
		drcbe_info beinfo;
		m_drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 4)
		{
			m_regmap[0] = uml::I4;
		}
		if (beinfo.direct_iregs > 5)
		{
			m_regmap[1] = uml::I5;
		}
		if (beinfo.direct_iregs > 6)
		{
			m_regmap[2] = uml::I6;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = TRUE;
}


void sh2_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%d%c%c",
					m_sh2_state->sr & M ? 'M':'.',
					m_sh2_state->sr & Q ? 'Q':'.',
					(m_sh2_state->sr & I) >> 4,
					m_sh2_state->sr & S ? 'S':'.',
					m_sh2_state->sr & T ? 'T':'.');
			break;
	}
}


void sh2_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SH2_PC:
			m_sh2_state->pc = m_debugger_temp;
			m_delay = 0;
			break;

		case SH2_SR:
			CHECK_PENDING_IRQ("sh2_set_reg");
			break;
	}
}


void sh2_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SH2_PC:
			m_debugger_temp = (m_delay) ? (m_delay & AM) : (m_sh2_state->pc & AM);
			break;
	}
}


void sh2_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m_nmi_line_state == state)
			return;
		m_nmi_line_state = state;

		if( state == CLEAR_LINE )
		{
			LOG(("SH-2 '%s' cleared nmi\n", tag()));
		}
		else
		{
			LOG(("SH-2 '%s' assert nmi\n", tag()));

			sh2_exception("Set IRQ line", 16);

			if (m_isdrc)
				m_sh2_state->pending_nmi = 1;
		}
	}
	else
	{
		if (m_irq_line_state[irqline] == state)
			return;
		m_irq_line_state[irqline] = state;

		if( state == CLEAR_LINE )
		{
			LOG(("SH-2 '%s' cleared irq #%d\n", tag(), irqline));
			m_sh2_state->pending_irq &= ~(1 << irqline);
		}
		else
		{
			LOG(("SH-2 '%s' assert irq #%d\n", tag(), irqline));
			m_sh2_state->pending_irq |= 1 << irqline;
			if (m_isdrc)
			{
				m_test_irq = 1;
			} else {
				if(m_delay)
					m_test_irq = 1;
				else
					CHECK_PENDING_IRQ("sh2_set_irq_line");
			}
		}
	}
}

#include "sh2comn.c"
#include "sh2drc.c"
