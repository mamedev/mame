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

    - Bug fix in void MOVBM(uint32_t m, uint32_t n) (see comment)
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
#include "sh2.h"
#include "sh2comn.h"

#include "debugger.h"

//#define VERBOSE 1
#include "logmacro.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DISABLE_FAST_REGISTERS              (0) // set to 1 to turn off usage of register caching
#define SINGLE_INSTRUCTION_MODE             (0)


/***************************************************************************
    CONSTANTS
***************************************************************************/



/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         64
#define COMPILE_FORWARDS_BYTES          256
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/2) + (COMPILE_FORWARDS_BYTES/2))
#define COMPILE_MAX_SEQUENCE            64


DEFINE_DEVICE_TYPE(SH1,  sh1_device,  "sh1",  "SH-1")
DEFINE_DEVICE_TYPE(SH2,  sh2_device,  "sh2",  "SH-2")
DEFINE_DEVICE_TYPE(SH2A, sh2a_device, "sh21", "SH-2A")

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
//  AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM // cache data array
//  AM_RANGE(0xffffff88, 0xffffff8b) AM_READWRITE(dma_dtcr0_r,dma_dtcr0_w)
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
//  AM_RANGE(0x07000000, 0x070003ff) AM_RAM AM_SHARE("oram")// on-chip RAM, actually at 0xf000000 (1 kb)
//  AM_RANGE(0x0f000000, 0x0f0003ff) AM_RAM AM_SHARE("oram")// on-chip RAM, actually at 0xf000000 (1 kb)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh7032_map, AS_PROGRAM, 32, sh1_device )
//  fall-back
	AM_RANGE(0x05fffe00, 0x05ffffff) AM_READWRITE16(sh7032_r,sh7032_w,0xffffffff) // SH-7032H internal i/o
ADDRESS_MAP_END

sh2_device::sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH2, tag, owner, clock, CPU_TYPE_SH2, ADDRESS_MAP_NAME(sh7604_map), 32)
{
}


void sh2_device::device_stop()
{
}




sh2_device::sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type, address_map_constructor internal_map, int addrlines)
	: cpu_device(mconfig, type, tag, owner, clock)
	, sh_common_execution()
	, m_program_config("program", ENDIANNESS_BIG, 32, addrlines, 0, internal_map)
	, m_decrypted_program_config("decrypted_opcodes", ENDIANNESS_BIG, 32, addrlines, 0)
	, m_is_slave(0)
	, m_cpu_type(cpu_type)
	, m_drcuml(nullptr)
//  , m_drcuml(*this, m_cache, 0, 1, 32, 1)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
	, m_entry(nullptr)
	, m_read8(nullptr)
	, m_write8(nullptr)
	, m_read16(nullptr)
	, m_write16(nullptr)
	, m_read32(nullptr)
	, m_write32(nullptr)
	, m_interrupt(nullptr)
	, m_nocode(nullptr)
	, m_out_of_cycles(nullptr)
	, m_debugger_temp(0)
{
	m_isdrc = allow_drc();
}

sh2a_device::sh2a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH2A, tag, owner, clock, CPU_TYPE_SH2, ADDRESS_MAP_NAME(sh7021_map), 28)
{
}

sh1_device::sh1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH1, tag, owner, clock, CPU_TYPE_SH1, ADDRESS_MAP_NAME(sh7032_map), 28)
{
}

device_memory_interface::space_config_vector sh2_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_decrypted_program_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}

offs_t sh2_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( sh2 );
	return CPU_DISASSEMBLE_NAME( sh2 )(this, stream, pc, oprom, opram, options);
}


/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS     1

uint8_t sh2_device::RB(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_byte(A & AM);

	return m_program->read_byte(A);
}

uint16_t sh2_device::RW(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_word(A & AM);

	return m_program->read_word(A);
}

uint32_t sh2_device::RL(offs_t A)
{
	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_dword(A & AM);

	return m_program->read_dword(A);
}

void sh2_device::WB(offs_t A, uint8_t V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_byte(A & AM,V);
		return;
	}

	m_program->write_byte(A,V);
}

void sh2_device::WW(offs_t A, uint16_t V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_word(A & AM,V);
		return;
	}

	m_program->write_word(A,V);
}

void sh2_device::WL(offs_t A, uint32_t V)
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

/*  LDC.L   @Rm+,SR */
inline void sh2_device::LDCMSR(const uint16_t opcode) // passes Rn
{
	uint32_t x = Rn;

	m_sh2_state->ea = m_sh2_state->r[x];
	m_sh2_state->sr = RL( m_sh2_state->ea ) & FLAGS;
	m_sh2_state->r[x] += 4;
	m_sh2_state->icount -= 2;
	m_test_irq = 1;
}

/*  LDC     Rm,SR */
inline void sh2_device::LDCSR(const uint16_t opcode) // passes Rn
{
	uint32_t x = Rn;

	m_sh2_state->sr = m_sh2_state->r[x] & FLAGS;
	m_test_irq = 1;
}

/*  RTE */
inline void sh2_device::RTE()
{
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->m_delay = RL( m_sh2_state->ea );
	m_sh2_state->r[15] += 4;
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->sr = RL( m_sh2_state->ea ) & FLAGS;
	m_sh2_state->r[15] += 4;
	m_sh2_state->icount -= 3;
	m_test_irq = 1;
}

/*  TRAPA   #imm */
inline void sh2_device::TRAPA(uint32_t i)
{
	uint32_t imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->vbr + imm * 4;

	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc );

	m_sh2_state->pc = RL( m_sh2_state->ea );

	m_sh2_state->icount -= 7;
}

/*  ILLEGAL */
inline void sh2_device::ILLEGAL()
{
	//logerror("Illegal opcode at %08x\n", m_sh2_state->pc - 2);
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc - 2 ); /* push PC onto stack */

	/* fetch PC */
	m_sh2_state->pc = RL( m_sh2_state->vbr + 4 * 4 );

	/* TODO: timing is a guess */
	m_sh2_state->icount -= 5;
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

void sh2_device::op0000(uint16_t opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: ILLEGAL();                       break;
	case 0x01: ILLEGAL();                       break;
	case 0x02: SH2STCSR(Rn);                  break;
	case 0x03: SH2BSRF(Rn);                   break;
	case 0x04: SH2MOVBS0(Rm, Rn);             break;
	case 0x05: SH2MOVWS0(Rm, Rn);             break;
	case 0x06: SH2MOVLS0(Rm, Rn);             break;
	case 0x07: SH2MULL(Rm, Rn);               break;
	case 0x08: SH2CLRT();                       break;
	case 0x09: SH2NOP();                           break;
	case 0x0a: SH2STSMACH(Rn);                break;
	case 0x0b: SH2RTS();                        break;
	case 0x0c: SH2MOVBL0(Rm, Rn);             break;
	case 0x0d: SH2MOVWL0(Rm, Rn);             break;
	case 0x0e: SH2MOVLL0(Rm, Rn);             break;
	case 0x0f: SH2MAC_L(Rm, Rn);              break;

	case 0x10: ILLEGAL();                       break;
	case 0x11: ILLEGAL();                       break;
	case 0x12: SH2STCGBR(Rn);                 break;
	case 0x13: ILLEGAL();                       break;
	case 0x14: SH2MOVBS0(Rm, Rn);             break;
	case 0x15: SH2MOVWS0(Rm, Rn);             break;
	case 0x16: SH2MOVLS0(Rm, Rn);             break;
	case 0x17: SH2MULL(Rm, Rn);               break;
	case 0x18: SH2SETT();                       break;
	case 0x19: SH2DIV0U();                  break;
	case 0x1a: SH2STSMACL(Rn);                break;
	case 0x1b: SH2SLEEP();                  break;
	case 0x1c: SH2MOVBL0(Rm, Rn);             break;
	case 0x1d: SH2MOVWL0(Rm, Rn);             break;
	case 0x1e: SH2MOVLL0(Rm, Rn);             break;
	case 0x1f: SH2MAC_L(Rm, Rn);              break;

	case 0x20: ILLEGAL();                       break;
	case 0x21: ILLEGAL();                       break;
	case 0x22: SH2STCVBR(Rn);                 break;
	case 0x23: SH2BRAF(Rn);                   break;
	case 0x24: SH2MOVBS0(Rm, Rn);             break;
	case 0x25: SH2MOVWS0(Rm, Rn);             break;
	case 0x26: SH2MOVLS0(Rm, Rn);             break;
	case 0x27: SH2MULL(Rm, Rn);               break;
	case 0x28: SH2CLRMAC();                 break;
	case 0x29: SH2MOVT(Rn);                   break;
	case 0x2a: SH2STSPR(Rn);                  break;
	case 0x2b: RTE();                        break;
	case 0x2c: SH2MOVBL0(Rm, Rn);             break;
	case 0x2d: SH2MOVWL0(Rm, Rn);             break;
	case 0x2e: SH2MOVLL0(Rm, Rn);             break;
	case 0x2f: SH2MAC_L(Rm, Rn);              break;

	case 0x30: ILLEGAL();                       break;
	case 0x31: ILLEGAL();                       break;
	case 0x32: ILLEGAL();                       break;
	case 0x33: ILLEGAL();                       break;
	case 0x34: SH2MOVBS0(Rm, Rn);             break;
	case 0x35: SH2MOVWS0(Rm, Rn);             break;
	case 0x36: SH2MOVLS0(Rm, Rn);             break;
	case 0x37: SH2MULL(Rm, Rn);               break;
	case 0x38: ILLEGAL();                       break;
	case 0x39: ILLEGAL();                       break;
	case 0x3c: SH2MOVBL0(Rm, Rn);             break;
	case 0x3d: SH2MOVWL0(Rm, Rn);             break;
	case 0x3e: SH2MOVLL0(Rm, Rn);             break;
	case 0x3f: SH2MAC_L(Rm, Rn);              break;
	case 0x3a: ILLEGAL();                       break;
	case 0x3b: ILLEGAL();                       break;



	}
}

void sh2_device::op0001(uint16_t opcode)
{
	SH2MOVLS4(Rm, opcode & 0x0f, Rn);
}

void sh2_device::op0010(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: SH2MOVBS(Rm, Rn);                break;
	case  1: SH2MOVWS(Rm, Rn);                break;
	case  2: SH2MOVLS(Rm, Rn);                break;
	case  3: ILLEGAL();                         break;
	case  4: SH2MOVBM(Rm, Rn);                break;
	case  5: SH2MOVWM(Rm, Rn);                break;
	case  6: SH2MOVLM(Rm, Rn);                break;
	case  7: SH2DIV0S(Rm, Rn);                break;
	case  8: SH2TST(Rm, Rn);                  break;
	case  9: SH2AND(Rm, Rn);                  break;
	case 10: SH2XOR(Rm, Rn);                  break;
	case 11: SH2OR(Rm, Rn);                   break;
	case 12: SH2CMPSTR(Rm, Rn);               break;
	case 13: SH2XTRCT(Rm, Rn);                break;
	case 14: SH2MULU(Rm, Rn);                 break;
	case 15: SH2MULS(Rm, Rn);                 break;
	}
}

void sh2_device::op0011(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: SH2CMPEQ(Rm, Rn);                break;
	case  1: ILLEGAL();                         break;
	case  2: SH2CMPHS(Rm, Rn);                break;
	case  3: SH2CMPGE(Rm, Rn);                break;
	case  4: SH2DIV1(Rm, Rn);                 break;
	case  5: SH2DMULU(Rm, Rn);                break;
	case  6: SH2CMPHI(Rm, Rn);                break;
	case  7: SH2CMPGT(Rm, Rn);                break;
	case  8: SH2SUB(Rm, Rn);                  break;
	case  9: ILLEGAL();                         break;
	case 10: SH2SUBC(Rm, Rn);                 break;
	case 11: SH2SUBV(Rm, Rn);                 break;
	case 12: SH2ADD(Rm, Rn);                  break;
	case 13: SH2DMULS(Rm, Rn);                break;
	case 14: SH2ADDC(Rm, Rn);                 break;
	case 15: SH2ADDV(Rm, Rn);                 break;
	}
}

void sh2_device::op0100(uint16_t opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: SH2SHLL(Rn);                   break;
	case 0x01: SH2SHLR(Rn);                   break;
	case 0x02: SH2STSMMACH(Rn);               break;
	case 0x03: SH2STCMSR(Rn);                 break;
	case 0x04: SH2ROTL(Rn);                   break;
	case 0x05: SH2ROTR(Rn);                   break;
	case 0x06: SH2LDSMMACH(Rn);               break;
	case 0x07: LDCMSR(opcode);                 break;
	case 0x08: SH2SHLL2(Rn);                  break;
	case 0x09: SH2SHLR2(Rn);                  break;
	case 0x0a: SH2LDSMACH(Rn);                break;
	case 0x0b: SH2JSR(Rn);                    break;
	case 0x0c: ILLEGAL();                       break;
	case 0x0d: ILLEGAL();                       break;
	case 0x0e: LDCSR(opcode);                  break;
	case 0x0f: SH2MAC_W(Rm, Rn);              break;

	case 0x10: SH2DT(Rn);                     break;
	case 0x11: SH2CMPPZ(Rn);                  break;
	case 0x12: SH2STSMMACL(Rn);               break;
	case 0x13: SH2STCMGBR(Rn);                break;
	case 0x14: ILLEGAL();                       break;
	case 0x15: SH2CMPPL(Rn);                  break;
	case 0x16: SH2LDSMMACL(Rn);               break;
	case 0x17: SH2LDCMGBR(Rn);                break;
	case 0x18: SH2SHLL8(Rn);                  break;
	case 0x19: SH2SHLR8(Rn);                  break;
	case 0x1a: SH2LDSMACL(Rn);                break;
	case 0x1b: SH2TAS(Rn);                    break;
	case 0x1c: ILLEGAL();                       break;
	case 0x1d: ILLEGAL();                       break;
	case 0x1e: SH2LDCGBR(Rn);                 break;
	case 0x1f: SH2MAC_W(Rm, Rn);              break;

	case 0x20: SH2SHAL(Rn);                   break;
	case 0x21: SH2SHAR(Rn);                   break;
	case 0x22: SH2STSMPR(Rn);                 break;
	case 0x23: SH2STCMVBR(Rn);                break;
	case 0x24: SH2ROTCL(Rn);                  break;
	case 0x25: SH2ROTCR(Rn);                  break;
	case 0x26: SH2LDSMPR(Rn);                 break;
	case 0x27: SH2LDCMVBR(Rn);                break;
	case 0x28: SH2SHLL16(Rn);                 break;
	case 0x29: SH2SHLR16(Rn);                 break;
	case 0x2a: SH2LDSPR(Rn);                  break;
	case 0x2b: SH2JMP(Rn);                    break;
	case 0x2c: ILLEGAL();                       break;
	case 0x2d: ILLEGAL();                       break;
	case 0x2e: SH2LDCVBR(Rn);                 break;
	case 0x2f: SH2MAC_W(Rm, Rn);              break;

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
	case 0x3f: SH2MAC_W(Rm, Rn);              break;

	}
}

void sh2_device::op0101(uint16_t opcode)
{
	SH2MOVLL4(Rm, opcode & 0x0f, Rn);
}

void sh2_device::op0110(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: SH2MOVBL(Rm, Rn);                break;
	case  1: SH2MOVWL(Rm, Rn);                break;
	case  2: SH2MOVLL(Rm, Rn);                break;
	case  3: SH2MOV(Rm, Rn);                  break;
	case  4: SH2MOVBP(Rm, Rn);                break;
	case  5: SH2MOVWP(Rm, Rn);                break;
	case  6: SH2MOVLP(Rm, Rn);                break;
	case  7: SH2NOT(Rm, Rn);                  break;
	case  8: SH2SWAPB(Rm, Rn);                break;
	case  9: SH2SWAPW(Rm, Rn);                break;
	case 10: SH2NEGC(Rm, Rn);                 break;
	case 11: SH2NEG(Rm, Rn);                  break;
	case 12: SH2EXTUB(Rm, Rn);                break;
	case 13: SH2EXTUW(Rm, Rn);                break;
	case 14: SH2EXTSB(Rm, Rn);                break;
	case 15: SH2EXTSW(Rm, Rn);                break;
	}
}

void sh2_device::op0111(uint16_t opcode)
{
	SH2ADDI(opcode & 0xff, Rn);
}

void sh2_device::op1000(uint16_t opcode)
{
	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: SH2MOVBS4(opcode & 0x0f, Rm);   break;
	case  1 << 8: SH2MOVWS4(opcode & 0x0f, Rm);   break;
	case  2<< 8: ILLEGAL();                 break;
	case  3<< 8: ILLEGAL();                 break;
	case  4<< 8: SH2MOVBL4(Rm, opcode & 0x0f);    break;
	case  5<< 8: SH2MOVWL4(Rm, opcode & 0x0f);    break;
	case  6<< 8: ILLEGAL();                 break;
	case  7<< 8: ILLEGAL();                 break;
	case  8<< 8: SH2CMPIM(opcode & 0xff);     break;
	case  9<< 8: SH2BT(opcode & 0xff);        break;
	case 10<< 8: ILLEGAL();                 break;
	case 11<< 8: SH2BF(opcode & 0xff);        break;
	case 12<< 8: ILLEGAL();                 break;
	case 13<< 8: SH2BTS(opcode & 0xff);       break;
	case 14<< 8: ILLEGAL();                 break;
	case 15<< 8: SH2BFS(opcode & 0xff);       break;
	}
}


void sh2_device::op1001(uint16_t opcode)
{
	SH2MOVWI(opcode & 0xff, Rn);
}

void sh2_device::op1010(uint16_t opcode)
{
	SH2BRA(opcode & 0xfff);
}

void sh2_device::op1011(uint16_t opcode)
{
	SH2BSR(opcode & 0xfff);
}

void sh2_device::op1100(uint16_t opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: SH2MOVBSG(opcode & 0xff);     break;
	case  1<<8: SH2MOVWSG(opcode & 0xff);     break;
	case  2<<8: SH2MOVLSG(opcode & 0xff);     break;
	case  3<<8: TRAPA(opcode & 0xff);      break;
	case  4<<8: SH2MOVBLG(opcode & 0xff);     break;
	case  5<<8: SH2MOVWLG(opcode & 0xff);     break;
	case  6<<8: SH2MOVLLG(opcode & 0xff);     break;
	case  7<<8: SH2MOVA(opcode & 0xff);       break;
	case  8<<8: SH2TSTI(opcode & 0xff);       break;
	case  9<<8: SH2ANDI(opcode & 0xff);       break;
	case 10<<8: SH2XORI(opcode & 0xff);       break;
	case 11<<8: SH2ORI(opcode & 0xff);            break;
	case 12<<8: SH2TSTM(opcode & 0xff);       break;
	case 13<<8: SH2ANDM(opcode & 0xff);       break;
	case 14<<8: SH2XORM(opcode & 0xff);       break;
	case 15<<8: SH2ORM(opcode & 0xff);            break;
	}
}

void sh2_device::op1101(uint16_t opcode)
{
	SH2MOVLI(opcode & 0xff, Rn);
}

void sh2_device::op1110(uint16_t opcode)
{
	SH2MOVI(opcode & 0xff, Rn);
}

void sh2_device::op1111(uint16_t opcode)
{
	ILLEGAL();
}

/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

void sh2_device::device_reset()
{
	m_sh2_state->pc = m_sh2_state->pr = m_sh2_state->sr = m_sh2_state->gbr = m_sh2_state->vbr = m_sh2_state->mach = m_sh2_state->macl = 0;
	m_sh2_state->evec = m_sh2_state->irqsr = 0;
	memset(&m_sh2_state->r[0], 0, sizeof(m_sh2_state->r[0])*16);
	m_sh2_state->ea = m_sh2_state->m_delay = m_cpu_off = m_dvsr = m_dvdnth = m_dvdntl = m_dvcr = 0;
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

	m_cache_dirty = true;
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
		uint32_t opcode;

		debugger_instruction_hook(this, m_sh2_state->pc);

		opcode = m_program->read_word(m_sh2_state->pc & AM);

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

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

		if(m_test_irq && !m_sh2_state->m_delay)
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
	m_decrypted_program = has_space(AS_OPCODES) ? &space(AS_OPCODES) : &space(AS_PROGRAM);
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
	save_item(NAME(m_sh2_state->m_delay));
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

	state_add( STATE_GENPC, "PC", m_sh2_state->pc).mask(AM).callimport();
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

	state_add( STATE_GENPCBASE, "CURPC", m_sh2_state->pc ).callimport().noshow();
	state_add( STATE_GENSP, "GENSP", m_sh2_state->r[15] ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_sh2_state->sr ).formatstr("%6s").noshow();

	m_icountptr = &m_sh2_state->icount;

	// Clear state
	m_sh2_state->pc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	m_sh2_state->ea = 0;
	m_sh2_state->m_delay = 0;
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
	uint32_t flags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, flags, 1, 32, 1);

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
	m_drcfe = std::make_unique<sh2_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

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
	m_cache_dirty = true;
}


void sh2_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%d%c%c",
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
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_sh2_state->m_delay = 0;
			break;

		case SH2_SR:
			CHECK_PENDING_IRQ("sh2_set_reg");
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

		if (state == CLEAR_LINE)
		{
			LOG("SH-2 cleared nmi\n");
		}
		else
		{
			LOG("SH-2 asserted nmi\n");

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

		if (state == CLEAR_LINE)
		{
			LOG("SH-2 cleared irq #%d\n", irqline);
			m_sh2_state->pending_irq &= ~(1 << irqline);
		}
		else
		{
			LOG("SH-2 asserted irq #%d\n", irqline);
			m_sh2_state->pending_irq |= 1 << irqline;
			if (m_isdrc)
			{
				m_test_irq = 1;
			} else {
				if(m_sh2_state->m_delay)
					m_test_irq = 1;
				else
					CHECK_PENDING_IRQ("sh2_set_irq_line");
			}
		}
	}
}

void sh2_device::sh2_exception(const char *message, int irqline)
{
	int vector;

	if (irqline != 16)
	{
		if (irqline <= ((m_sh2_state->sr >> 4) & 15)) /* If the cpu forbids this interrupt */
			return;

		// if this is an sh2 internal irq, use its vector
		if (m_sh2_state->internal_irq_level == irqline)
		{
			vector = m_internal_irq_vector;
			/* avoid spurious irqs with this (TODO: needs a better fix) */
			m_sh2_state->internal_irq_level = -1;
			LOG("SH-2 exception #%d (internal vector: $%x) after [%s]\n", irqline, vector, message);
		}
		else
		{
			if(m_m[0x38] & 0x00010000)
			{
				vector = standard_irq_callback(irqline);
				LOG("SH-2 exception #%d (external vector: $%x) after [%s]\n", irqline, vector, message);
			}
			else
			{
				standard_irq_callback(irqline);
				vector = 64 + irqline/2;
				LOG("SH-2 exception #%d (autovector: $%x) after [%s]\n", irqline, vector, message);
			}
		}
	}
	else
	{
		vector = 11;
		LOG("SH-2 nmi exception (autovector: $%x) after [%s]\n", vector, message);
	}

	if (m_isdrc)
	{
		m_sh2_state->evec = RL( m_sh2_state->vbr + vector * 4 );
		m_sh2_state->evec &= AM;
		m_sh2_state->irqsr = m_sh2_state->sr;

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~I) | (irqline << 4);

//  printf("sh2_exception [%s] irqline %x evec %x save SR %x new SR %x\n", message, irqline, m_sh2_state->evec, m_sh2_state->irqsr, m_sh2_state->sr);
	} else {
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->pc );     /* push PC onto stack */

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~I) | (irqline << 4);

		/* fetch PC */
		m_sh2_state->pc = RL( m_sh2_state->vbr + vector * 4 );
	}

	if(m_sh2_state->sleep_mode == 1) { m_sh2_state->sleep_mode = 2; }
}

#include "sh2drc.cpp"
