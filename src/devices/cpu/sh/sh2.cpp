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
#include "sh_dasm.h"

//#define VERBOSE 1
#include "logmacro.h"




DEFINE_DEVICE_TYPE(SH1,  sh1_device,  "sh1",  "Hitachi SH-1")
DEFINE_DEVICE_TYPE(SH2,  sh2_device,  "sh2",  "Hitachi SH-2")
DEFINE_DEVICE_TYPE(SH2A, sh2a_device, "sh21", "Hitachi SH-2A")

/*-------------------------------------------------
    sh2_internal_a5 - read handler for
    SH2 internal map
-------------------------------------------------*/

uint32_t sh2_device::sh2_internal_a5()
{
	return 0xa5a5a5a5;
}


/*-------------------------------------------------
    sh2_internal_map - maps SH2 built-ins
-------------------------------------------------*/


void sh2_device::sh7604_map(address_map &map)
{
	map(0x40000000, 0xbfffffff).r(FUNC(sh2_device::sh2_internal_a5));

//  TODO: cps3boot breaks with this enabled. Needs callback
//  map(0xc0000000, 0xc0000fff).ram(); // cache data array

//  map(0xe0000000, 0xe00001ff).mirror(0x1ffffe00).rw(FUNC(sh2_device::sh7604_r), FUNC(sh2_device::sh7604_w));
	// TODO: internal map takes way too much resources if mirrored with 0x1ffffe00
	//       we eventually internalize again via trampoline & sh7604_device
	//       Also area 0xffff8000-0xffffbfff is for synchronous DRAM mode,
	//       so this isn't actually a full mirror
	// SCI
	map(0xfffffe00, 0xfffffe00).rw(FUNC(sh2_device::smr_r), FUNC(sh2_device::smr_w));
	map(0xfffffe01, 0xfffffe01).rw(FUNC(sh2_device::brr_r), FUNC(sh2_device::brr_w));
	map(0xfffffe02, 0xfffffe02).rw(FUNC(sh2_device::scr_r), FUNC(sh2_device::scr_w));
	map(0xfffffe03, 0xfffffe03).rw(FUNC(sh2_device::tdr_r), FUNC(sh2_device::tdr_w));
	map(0xfffffe04, 0xfffffe04).rw(FUNC(sh2_device::ssr_r), FUNC(sh2_device::ssr_w));
	map(0xfffffe05, 0xfffffe05).r(FUNC(sh2_device::rdr_r));

	// FRC
	map(0xfffffe10, 0xfffffe10).rw(FUNC(sh2_device::tier_r), FUNC(sh2_device::tier_w));
	map(0xfffffe11, 0xfffffe11).rw(FUNC(sh2_device::ftcsr_r), FUNC(sh2_device::ftcsr_w));
	map(0xfffffe12, 0xfffffe13).rw(FUNC(sh2_device::frc_r), FUNC(sh2_device::frc_w));
	map(0xfffffe14, 0xfffffe15).rw(FUNC(sh2_device::ocra_b_r), FUNC(sh2_device::ocra_b_w));
	map(0xfffffe16, 0xfffffe16).rw(FUNC(sh2_device::frc_tcr_r), FUNC(sh2_device::frc_tcr_w));
	map(0xfffffe17, 0xfffffe17).rw(FUNC(sh2_device::tocr_r), FUNC(sh2_device::tocr_w));
	map(0xfffffe18, 0xfffffe19).r(FUNC(sh2_device::frc_icr_r));

	// INTC
	map(0xfffffe60, 0xfffffe61).rw(FUNC(sh2_device::iprb_r), FUNC(sh2_device::iprb_w));
	map(0xfffffe62, 0xfffffe63).rw(FUNC(sh2_device::vcra_r), FUNC(sh2_device::vcra_w));
	map(0xfffffe64, 0xfffffe65).rw(FUNC(sh2_device::vcrb_r), FUNC(sh2_device::vcrb_w));
	map(0xfffffe66, 0xfffffe67).rw(FUNC(sh2_device::vcrc_r), FUNC(sh2_device::vcrc_w));
	map(0xfffffe68, 0xfffffe69).rw(FUNC(sh2_device::vcrd_r), FUNC(sh2_device::vcrd_w));

	map(0xfffffe71, 0xfffffe71).rw(FUNC(sh2_device::drcr_r<0>), FUNC(sh2_device::drcr_w<0>));
	map(0xfffffe72, 0xfffffe72).rw(FUNC(sh2_device::drcr_r<1>), FUNC(sh2_device::drcr_w<1>));

	// WTC
	map(0xfffffe80, 0xfffffe81).rw(FUNC(sh2_device::wtcnt_r), FUNC(sh2_device::wtcnt_w));
	map(0xfffffe82, 0xfffffe83).rw(FUNC(sh2_device::rstcsr_r), FUNC(sh2_device::rstcsr_w));

	// standby and cache control
	map(0xfffffe90, 0xfffffe91).rw(FUNC(sh2_device::fmr_sbycr_r), FUNC(sh2_device::fmr_sbycr_w));
	map(0xfffffe92, 0xfffffe92).rw(FUNC(sh2_device::ccr_r), FUNC(sh2_device::ccr_w));

	// INTC second section
	map(0xfffffee0, 0xfffffee1).rw(FUNC(sh2_device::intc_icr_r), FUNC(sh2_device::intc_icr_w));
	map(0xfffffee2, 0xfffffee3).rw(FUNC(sh2_device::ipra_r), FUNC(sh2_device::ipra_w));
	map(0xfffffee4, 0xfffffee5).rw(FUNC(sh2_device::vcrwdt_r), FUNC(sh2_device::vcrwdt_w));

	// DIVU
	map(0xffffff00, 0xffffff03).rw(FUNC(sh2_device::dvsr_r), FUNC(sh2_device::dvsr_w));
	map(0xffffff04, 0xffffff07).rw(FUNC(sh2_device::dvdnt_r), FUNC(sh2_device::dvdnt_w));
	map(0xffffff08, 0xffffff0b).rw(FUNC(sh2_device::dvcr_r), FUNC(sh2_device::dvcr_w));
	// INTC third section
	map(0xffffff0c, 0xffffff0f).rw(FUNC(sh2_device::vcrdiv_r), FUNC(sh2_device::vcrdiv_w));
	// DIVU continued (64-bit plus mirrors)
	map(0xffffff10, 0xffffff13).rw(FUNC(sh2_device::dvdnth_r), FUNC(sh2_device::dvdnth_w));
	map(0xffffff14, 0xffffff17).rw(FUNC(sh2_device::dvdntl_r), FUNC(sh2_device::dvdntl_w));
	map(0xffffff18, 0xffffff1b).r(FUNC(sh2_device::dvdnth_r));
	map(0xffffff1c, 0xffffff1f).r(FUNC(sh2_device::dvdntl_r));

	// DMAC
	map(0xffffff80, 0xffffff83).rw(FUNC(sh2_device::sar_r<0>), FUNC(sh2_device::sar_w<0>));
	map(0xffffff84, 0xffffff87).rw(FUNC(sh2_device::dar_r<0>), FUNC(sh2_device::dar_w<0>));
	map(0xffffff88, 0xffffff8b).rw(FUNC(sh2_device::dmac_tcr_r<0>), FUNC(sh2_device::dmac_tcr_w<0>));
	map(0xffffff8c, 0xffffff8f).rw(FUNC(sh2_device::chcr_r<0>), FUNC(sh2_device::chcr_w<0>));

	map(0xffffff90, 0xffffff93).rw(FUNC(sh2_device::sar_r<1>), FUNC(sh2_device::sar_w<1>));
	map(0xffffff94, 0xffffff97).rw(FUNC(sh2_device::dar_r<1>), FUNC(sh2_device::dar_w<1>));
	map(0xffffff98, 0xffffff9b).rw(FUNC(sh2_device::dmac_tcr_r<1>), FUNC(sh2_device::dmac_tcr_w<1>));
	map(0xffffff9c, 0xffffff9f).rw(FUNC(sh2_device::chcr_r<1>), FUNC(sh2_device::chcr_w<1>));

	map(0xffffffa0, 0xffffffa3).rw(FUNC(sh2_device::vcrdma_r<0>), FUNC(sh2_device::vcrdma_w<0>));
	map(0xffffffa8, 0xffffffab).rw(FUNC(sh2_device::vcrdma_r<1>), FUNC(sh2_device::vcrdma_w<1>));
	map(0xffffffb0, 0xffffffb3).rw(FUNC(sh2_device::dmaor_r), FUNC(sh2_device::dmaor_w));

	// BSC
	map(0xffffffe0, 0xffffffe3).rw(FUNC(sh2_device::bcr1_r), FUNC(sh2_device::bcr1_w));
	map(0xffffffe4, 0xffffffe7).rw(FUNC(sh2_device::bcr2_r), FUNC(sh2_device::bcr2_w));
	map(0xffffffe8, 0xffffffeb).rw(FUNC(sh2_device::wcr_r), FUNC(sh2_device::wcr_w));
	map(0xffffffec, 0xffffffef).rw(FUNC(sh2_device::mcr_r), FUNC(sh2_device::mcr_w));
	map(0xfffffff0, 0xfffffff3).rw(FUNC(sh2_device::rtcsr_r), FUNC(sh2_device::rtcsr_w));
	map(0xfffffff4, 0xfffffff7).rw(FUNC(sh2_device::rtcnt_r), FUNC(sh2_device::rtcnt_w));
	map(0xfffffff8, 0xfffffffb).rw(FUNC(sh2_device::rtcor_r), FUNC(sh2_device::rtcor_w));
}

void sh2a_device::sh7021_map(address_map &map)
{
//  fall-back
	map(0x05fffe00, 0x05ffffff).rw(FUNC(sh2a_device::sh7021_r), FUNC(sh2a_device::sh7021_w)); // SH-7032H internal i/o
//  overrides
	map(0x05ffff40, 0x05ffff43).rw(FUNC(sh2a_device::dma_sar0_r), FUNC(sh2a_device::dma_sar0_w));
	map(0x05ffff44, 0x05ffff47).rw(FUNC(sh2a_device::dma_dar0_r), FUNC(sh2a_device::dma_dar0_w));
	map(0x05ffff48, 0x05ffff49).rw(FUNC(sh2a_device::dmaor_r), FUNC(sh2a_device::dmaor_w));
	map(0x05ffff4a, 0x05ffff4b).rw(FUNC(sh2a_device::dma_tcr0_r), FUNC(sh2a_device::dma_tcr0_w));
	map(0x05ffff4e, 0x05ffff4f).rw(FUNC(sh2a_device::dma_chcr0_r), FUNC(sh2a_device::dma_chcr0_w));
//  map(0x07000000, 0x070003ff).ram().share("oram"); // on-chip RAM, actually at 0xf000000 (1 kb)
//  map(0x0f000000, 0x0f0003ff).ram().share("oram"); // on-chip RAM, actually at 0xf000000 (1 kb)
}

void sh1_device::sh7032_map(address_map &map)
{
//  fall-back
	map(0x05fffe00, 0x05ffffff).rw(FUNC(sh1_device::sh7032_r), FUNC(sh1_device::sh7032_w)); // SH-7032H internal i/o
}

sh2_device::sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH2, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh2_device::sh7604_map), this), 32)
{
}

sh2_device::~sh2_device()
{
}


void sh2_device::device_stop()
{
}




sh2_device::sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type, address_map_constructor internal_map, int addrlines)
	: sh_common_execution(mconfig, type, tag, owner, clock, ENDIANNESS_BIG, internal_map)
	, m_program_config("program", ENDIANNESS_BIG, 32, addrlines, 0, internal_map)
	, m_decrypted_program_config("decrypted_opcodes", ENDIANNESS_BIG, 32, addrlines, 0)
	, m_is_slave(0)
	, m_dma_kludge_cb(*this)
	, m_dma_fifo_data_available_cb(*this)
	, m_ftcsr_read_cb(*this)
	, m_drcfe(nullptr)
	, m_debugger_temp(0)
{
	m_cpu_type = cpu_type;
	m_am = SH12_AM;
	m_isdrc = allow_drc();
}

sh2a_device::sh2a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH2A, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh2a_device::sh7021_map), this), 28)
{
	std::fill(std::begin(m_sh7021_regs), std::end(m_sh7021_regs), 0);
}

sh1_device::sh1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH1, tag, owner, clock, CPU_TYPE_SH1, address_map_constructor(FUNC(sh1_device::sh7032_map), this), 28)
{
	std::fill(std::begin(m_sh7032_regs), std::end(m_sh7032_regs), 0);
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

std::unique_ptr<util::disasm_interface> sh2_device::create_disassembler()
{
	return std::make_unique<sh_disassembler>(false);
}

uint8_t sh2_device::RB(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_byte(A & SH12_AM);

	return m_program->read_byte(A);
}

uint16_t sh2_device::RW(offs_t A)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_word(A & SH12_AM);

	return m_program->read_word(A);
}

uint32_t sh2_device::RL(offs_t A)
{
	/* 0x20000000 no Cache */
	/* 0x00000000 read thru Cache if CE bit is 1 */
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
		return m_program->read_dword(A & SH12_AM);

	return m_program->read_dword(A);
}

void sh2_device::WB(offs_t A, uint8_t V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_byte(A & SH12_AM,V);
		return;
	}

	m_program->write_byte(A,V);
}

void sh2_device::WW(offs_t A, uint16_t V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_word(A & SH12_AM,V);
		return;
	}

	m_program->write_word(A,V);
}

void sh2_device::WL(offs_t A, uint32_t V)
{
	if((A & 0xf0000000) == 0 || (A & 0xf0000000) == 0x20000000)
	{
		m_program->write_dword(A & SH12_AM,V);
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
	m_sh2_state->sr = RL( m_sh2_state->ea ) & SH_FLAGS;
	m_sh2_state->r[x] += 4;
	m_sh2_state->icount -= 2;
	m_test_irq = 1;
}

/*  LDC     Rm,SR */
inline void sh2_device::LDCSR(const uint16_t opcode) // passes Rn
{
	uint32_t x = Rn;

	m_sh2_state->sr = m_sh2_state->r[x] & SH_FLAGS;
	m_test_irq = 1;
}

/*  RTE */
inline void sh2_device::RTE()
{
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->m_delay = RL( m_sh2_state->ea );
	m_sh2_state->r[15] += 4;
	m_sh2_state->ea = m_sh2_state->r[15];
	m_sh2_state->sr = RL( m_sh2_state->ea ) & SH_FLAGS;
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


void sh2_device::execute_one_f000(uint16_t opcode)
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
	m_sh2_state->ea = m_sh2_state->m_delay = m_cpu_off = 0;
	//m_dvsr = m_dvdnth = m_dvdntl = m_dvcr = 0;
	m_sh2_state->pending_irq = m_test_irq = 0;
	//memset(&m_irq_queue[0], 0, sizeof(m_irq_queue[0])*16);
	memset(&m_irq_line_state[0], 0, sizeof(m_irq_line_state[0])*17);
	m_frc = m_ocra = m_ocrb = m_frc_icr = 0;
	m_frc_base = 0;
	m_frt_input = m_sh2_state->internal_irq_level = m_internal_irq_vector = 0;
	m_dma_timer_active[0] = m_dma_timer_active[1] = 0;
	m_dma_irq[0] = m_dma_irq[1] = 0;

	m_sh2_state->pc = RL(0);
	m_sh2_state->r[15] = RL(4);
	m_sh2_state->sr = SH_I;
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

	do
	{
		debugger_instruction_hook(m_sh2_state->pc);

		const uint16_t opcode = m_decrypted_program->read_word(m_sh2_state->pc >= 0x40000000 ? m_sh2_state->pc : m_sh2_state->pc & SH12_AM);

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if(m_test_irq && !m_sh2_state->m_delay)
		{
			CHECK_PENDING_IRQ("mame_sh2_execute");
			m_test_irq = 0;
		}
		m_sh2_state->icount--;
	} while( m_sh2_state->icount > 0 );
}


void sh2_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh2_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

void sh2_device::device_start()
{
	sh_common_execution::device_start();

	m_timer = timer_alloc(FUNC(sh2_device::sh2_timer_callback), this);
	m_timer->adjust(attotime::never);
	m_wdtimer = timer_alloc(FUNC(sh2_device::sh2_wdtimer_callback), this);
	m_wdtimer->adjust(attotime::never);

	m_dma_current_active_timer[0] = timer_alloc(FUNC(sh2_device::sh2_dma_current_active_callback), this);
	m_dma_current_active_timer[0]->adjust(attotime::never);

	m_dma_current_active_timer[1] = timer_alloc(FUNC(sh2_device::sh2_dma_current_active_callback), this);
	m_dma_current_active_timer[1]->adjust(attotime::never);

	/* resolve callbacks */
	m_dma_kludge_cb.resolve();
	m_dma_fifo_data_available_cb.resolve();
	m_ftcsr_read_cb.resolve();

	m_decrypted_program = has_space(AS_OPCODES) ? &space(AS_OPCODES) : &space(AS_PROGRAM);
	m_decrypted_program->cache(m_cache32);
	m_pr16 = [this](offs_t address) -> u16 { return m_cache32.read_word(address); };
	if (m_decrypted_program->endianness() != ENDIANNESS_NATIVE)
		m_prptr = [this](offs_t address) -> const void * {
			const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
			if(!(address & 2))
				ptr++;
			return ptr;
		};
	else
		m_prptr = [this](offs_t address) -> const void * {
			const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
			if(address & 2)
				ptr++;
			return ptr;
		};

	m_internal = &space(AS_PROGRAM);

	// SCI
	save_item(NAME(m_smr));
	save_item(NAME(m_brr));
	save_item(NAME(m_scr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_ssr));

	// FRT / FRC
	save_item(NAME(m_tier));
	save_item(NAME(m_ftcsr));
	save_item(NAME(m_frc_tcr));
	save_item(NAME(m_tocr));
	save_item(NAME(m_frc));
	save_item(NAME(m_ocra));
	save_item(NAME(m_ocrb));
	save_item(NAME(m_frc_icr));
	save_item(NAME(m_frc_base));
	save_item(NAME(m_frt_input));

	// INTC
	save_item(NAME(m_irq_level.frc));
	save_item(NAME(m_irq_level.sci));
	save_item(NAME(m_irq_level.divu));
	save_item(NAME(m_irq_level.dmac));
	save_item(NAME(m_irq_level.wdt));
	save_item(NAME(m_irq_vector.fic));
	save_item(NAME(m_irq_vector.foc));
	save_item(NAME(m_irq_vector.fov));
	save_item(NAME(m_irq_vector.divu));
	save_item(NAME(m_irq_vector.dmac[0]));
	save_item(NAME(m_irq_vector.dmac[1]));

	save_item(NAME(m_ipra));
	save_item(NAME(m_iprb));
	save_item(NAME(m_vcra));
	save_item(NAME(m_vcrb));
	save_item(NAME(m_vcrc));
	save_item(NAME(m_vcrd));
	save_item(NAME(m_vcrwdt));
	save_item(NAME(m_vcrdiv));
	save_item(NAME(m_intc_icr));
	save_item(NAME(m_vcrdma[0]));
	save_item(NAME(m_vcrdma[1]));

	save_item(NAME(m_vecmd));
	save_item(NAME(m_nmie));

	// DIVU
	save_item(NAME(m_divu_ovf));
	save_item(NAME(m_divu_ovfie));
	save_item(NAME(m_dvsr));
	save_item(NAME(m_dvdntl));
	save_item(NAME(m_dvdnth));

	// WTC
	save_item(NAME(m_wtcnt));
	save_item(NAME(m_wtcsr));
	save_item(NAME(m_rstcsr));
	save_item(NAME(m_wtcw[0]));
	save_item(NAME(m_wtcw[1]));

	// DMAC
	save_item(NAME(m_dmaor));
	save_item(NAME(m_dmac[0].drcr));
	save_item(NAME(m_dmac[1].drcr));
	save_item(NAME(m_dmac[0].sar));
	save_item(NAME(m_dmac[1].sar));
	save_item(NAME(m_dmac[0].dar));
	save_item(NAME(m_dmac[1].dar));
	save_item(NAME(m_dmac[0].tcr));
	save_item(NAME(m_dmac[1].tcr));
	save_item(NAME(m_dmac[0].chcr));
	save_item(NAME(m_dmac[1].chcr));

	// misc
	save_item(NAME(m_sbycr));
	save_item(NAME(m_ccr));

	// BSC
	save_item(NAME(m_bcr1));
	save_item(NAME(m_bcr2));
	save_item(NAME(m_wcr));
	save_item(NAME(m_mcr));
	save_item(NAME(m_rtcsr));
	save_item(NAME(m_rtcor));
	save_item(NAME(m_rtcnt));

	/*
	for (int i = 0; i < 16; ++i)
	{
	    save_item(NAME(m_irq_queue[i].irq_vector), i);
	    save_item(NAME(m_irq_queue[i].irq_priority), i);
	}
	*/

	// internals
	save_item(NAME(m_cpu_off));
	save_item(NAME(m_test_irq));
	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_nmi_line_state));
	save_item(NAME(m_internal_irq_vector));
	save_item(NAME(m_dma_timer_active));
	save_item(NAME(m_dma_irq));

	state_add( STATE_GENPC, "PC", m_sh2_state->pc).mask(SH12_AM).callimport();
	state_add( STATE_GENPCBASE, "CURPC", m_sh2_state->pc ).callimport().noshow();

	// Clear state
	m_cpu_off = 0;
	//m_dvsr = 0;
	//m_dvdnth = 0;
	//m_dvdntl = 0;
	//m_dvcr = 0;
	m_test_irq = 0;

	memset(m_irq_line_state, 0, sizeof(m_irq_line_state));

	m_nmi_line_state = 0;
	m_frc = 0;
	m_ocra = 0;
	m_ocrb = 0;
	m_frc_icr = 0;
	m_frc_base = 0;
	m_frt_input = 0;
	m_internal_irq_vector = 0;

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

	drc_start();
}


void sh2_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%d%c%c",
					m_sh2_state->sr & SH_M ? 'M':'.',
					m_sh2_state->sr & SH_Q ? 'Q':'.',
					(m_sh2_state->sr & SH_I) >> 4,
					m_sh2_state->sr & SH_S ? 'S':'.',
					m_sh2_state->sr & SH_T ? 'T':'.');
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

		case SH_SR:
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
			if(m_vecmd == true)
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
		m_sh2_state->evec &= SH12_AM;
		m_sh2_state->irqsr = m_sh2_state->sr;

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | SH_I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~SH_I) | (irqline << 4);

//  printf("sh2_exception [%s] irqline %x evec %x save SR %x new SR %x\n", message, irqline, m_sh2_state->evec, m_sh2_state->irqsr, m_sh2_state->sr);
	} else {
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
		m_sh2_state->r[15] -= 4;
		WL( m_sh2_state->r[15], m_sh2_state->pc );     /* push PC onto stack */

		/* set I flags in SR */
		if (irqline > SH2_INT_15)
			m_sh2_state->sr = m_sh2_state->sr | SH_I;
		else
			m_sh2_state->sr = (m_sh2_state->sr & ~SH_I) | (irqline << 4);

		/* fetch PC */
		m_sh2_state->pc = RL( m_sh2_state->vbr + vector * 4 );
	}

	if(m_sh2_state->sleep_mode == 1) { m_sh2_state->sleep_mode = 2; }
}

// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh2drc.c
    Universal machine language-based SH-2 emulator.

***************************************************************************/

#include "emu.h"
#include "sh2.h"
#include "sh2comn.h"
#include "cpu/drcumlsh.h"


const opcode_desc* sh2_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}



/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void sh2_device::func_fastirq()
{
	sh2_exception("fastirq",m_sh2_state->irqline);
}
static void cfunc_fastirq(void *param) { ((sh2_device *)param)->func_fastirq(); };

void sh2_device::static_generate_entry_point()
{
	uml::code_label const skip = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(200));

	/* forward references */
	alloc_handle(m_nocode, "nocode");
	alloc_handle(m_write32, "write32");     // necessary?
	alloc_handle(m_entry, "entry");
	UML_HANDLE(block, *m_entry);                         // handle  entry

	/* load fast integer registers */
	load_fast_iregs(block);

	/* check for interrupts */
	UML_MOV(block, mem(&m_sh2_state->irqline), 0xffffffff);     // mov irqline, #-1
	UML_CMP(block, mem(&m_sh2_state->pending_nmi), 0);          // cmp pending_nmi, #0
	UML_JMPc(block, COND_Z, skip+2);                    // jz skip+2

	UML_MOV(block, mem(&m_sh2_state->pending_nmi), 0);          // zap pending_nmi
	UML_JMP(block, skip+1);                     // and then go take it (evec is already set)

	UML_LABEL(block, skip+2);                   // skip+2:
	UML_MOV(block, mem(&m_sh2_state->evec), 0xffffffff);        // mov evec, -1
	UML_MOV(block, I0, 0xffffffff);         // mov r0, -1 (r0 = irq)
	UML_AND(block, I1,  I0, 0xffff);                // and r1, 0xffff

	UML_LZCNT(block, I1, mem(&m_sh2_state->pending_irq));       // lzcnt r1, r1
	UML_CMP(block, I1, 32);             // cmp r1, #32
	UML_JMPc(block, COND_Z, skip+4);                    // jz skip+4

	UML_SUB(block, mem(&m_sh2_state->irqline), 31, I1);     // sub irqline, #31, r1

	UML_LABEL(block, skip+4);                   // skip+4:
	UML_CMP(block, mem(&m_sh2_state->internal_irq_level), 0xffffffff);  // cmp internal_irq_level, #-1
	UML_JMPc(block, COND_Z, skip+3);                    // jz skip+3
	UML_CMP(block, mem(&m_sh2_state->internal_irq_level), mem(&m_sh2_state->irqline));      // cmp internal_irq_level, irqline
	UML_JMPc(block, COND_LE, skip+3);                   // jle skip+3

	UML_MOV(block, mem(&m_sh2_state->irqline), mem(&m_sh2_state->internal_irq_level));      // mov r0, internal_irq_level

	UML_LABEL(block, skip+3);                   // skip+3:
	UML_CMP(block, mem(&m_sh2_state->irqline), 0xffffffff);     // cmp irqline, #-1
	UML_JMPc(block, COND_Z, skip+1);                    // jz skip+1
	UML_CALLC(block, cfunc_fastirq, this);               // callc fastirq

	UML_LABEL(block, skip+1);                   // skip+1:

	UML_CMP(block, mem(&m_sh2_state->evec), 0xffffffff);        // cmp evec, 0xffffffff
	UML_JMPc(block, COND_Z, skip);                  // jz skip

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, mem(&m_sh2_state->irqsr));           // mov r1, irqsr
	UML_CALLH(block, *m_write32);                    // call write32

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, mem(&m_sh2_state->pc));              // mov r1, pc
	UML_CALLH(block, *m_write32);                    // call write32

	UML_MOV(block, mem(&m_sh2_state->pc), mem(&m_sh2_state->evec));             // mov pc, evec

	UML_LABEL(block, skip);                         // skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode

	block.end();
}


/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void sh2_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception)
{
	/* check full interrupts if pending */
	if (compiler.checkints)
	{
		uml::code_label const skip = compiler.labelnum++;

		compiler.checkints = false;
		compiler.labelnum += 4;

		/* check for interrupts */
		UML_MOV(block, mem(&m_sh2_state->irqline), 0xffffffff);     // mov irqline, #-1
		UML_CMP(block, mem(&m_sh2_state->pending_nmi), 0);          // cmp pending_nmi, #0
		UML_JMPc(block, COND_Z, skip+2);                    // jz skip+2

		UML_MOV(block, mem(&m_sh2_state->pending_nmi), 0);          // zap pending_nmi
		UML_JMP(block, skip+1);                     // and then go take it (evec is already set)

		UML_LABEL(block, skip+2);                   // skip+2:
		UML_MOV(block, mem(&m_sh2_state->evec), 0xffffffff);        // mov evec, -1
		UML_MOV(block, I0, 0xffffffff);         // mov r0, -1 (r0 = irq)
		UML_AND(block, I1,  I0, 0xffff);                // and r1, r0, 0xffff

		UML_LZCNT(block, I1, mem(&m_sh2_state->pending_irq));       // lzcnt r1, pending_irq
		UML_CMP(block, I1, 32);             // cmp r1, #32
		UML_JMPc(block, COND_Z, skip+4);                    // jz skip+4

		UML_SUB(block, mem(&m_sh2_state->irqline), 31, I1);     // sub irqline, #31, r1

		UML_LABEL(block, skip+4);                   // skip+4:
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), 0xffffffff);  // cmp internal_irq_level, #-1
		UML_JMPc(block, COND_Z, skip+3);                    // jz skip+3
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), mem(&m_sh2_state->irqline));      // cmp internal_irq_level, irqline
		UML_JMPc(block, COND_LE, skip+3);                   // jle skip+3

		UML_MOV(block, mem(&m_sh2_state->irqline), mem(&m_sh2_state->internal_irq_level));      // mov r0, internal_irq_level

		UML_LABEL(block, skip+3);                   // skip+3:
		UML_CMP(block, mem(&m_sh2_state->irqline), 0xffffffff);     // cmp irqline, #-1
		UML_JMPc(block, COND_Z, skip+1);                    // jz skip+1
		UML_CALLC(block, cfunc_fastirq, this);               // callc fastirq

		UML_LABEL(block, skip+1);                   // skip+1:
		UML_CMP(block, mem(&m_sh2_state->evec), 0xffffffff);        // cmp evec, 0xffffffff
		UML_JMPc(block, COND_Z, skip);                  // jz skip

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, mem(&m_sh2_state->irqsr));           // mov r1, irqsr
		UML_CALLH(block, *m_write32);                    // call write32

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, param);              // mov r1, nextpc
		UML_CALLH(block, *m_write32);                    // call write32

		UML_HASHJMP(block, 0, mem(&m_sh2_state->evec), *m_nocode);       // hashjmp m_sh2_state->evec

		UML_LABEL(block, skip);                         // skip:
	}

	/* account for cycles */
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_sh2_state->icount), mem(&m_sh2_state->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler.cycles = 0;
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void sh2_device::static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	int label = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(handleptr, name);
	UML_HANDLE(block, *handleptr);                         // handle  *handleptr

	// with internal handlers this becomes easier.
	// if addr < 0x40000000 AND it with AM and do the read/write, else just do the read/write
	UML_TEST(block, I0, 0x80000000);        // test r0, #0x80000000
	UML_JMPc(block, COND_NZ, label);                // if high bit is set, don't mask

	UML_CMP(block, I0, 0x40000000);     // cmp #0x40000000, r0
	UML_JMPc(block, COND_AE, label);            // bae label

	UML_AND(block, I0, I0, SH12_AM);     // and r0, r0, #AM (0xc7ffffff)

	UML_LABEL(block, label++);              // label:

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		for (auto & elem : m_fastram)
		{
			if (elem.base != nullptr && (!iswrite || !elem.readonly))
			{
				void *fastbase = (uint8_t *)elem.base - elem.start;
				uint32_t skip = label++;
				if (elem.end != 0xffffffff)
				{
					UML_CMP(block, I0, elem.end);   // cmp     i0,end
					UML_JMPc(block, COND_A, skip);                                      // ja      skip
				}
				if (elem.start != 0x00000000)
				{
					UML_CMP(block, I0, elem.start);// cmp     i0,fastram_start
					UML_JMPc(block, COND_B, skip);                                      // jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, BYTE4_XOR_BE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);             // load    i0,fastbase,i0,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, WORD_XOR_BE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);         // load    i0,fastbase,i0,word_x1
					}
					else if (size == 4)
					{
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);            // load    i0,fastbase,i0,dword_x1
					}
					UML_RET(block);                                                     // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, BYTE4_XOR_BE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);// store   fastbase,i0,i1,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, WORD_XOR_BE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);// store   fastbase,i0,i1,word_x1
					}
					else if (size == 4)
					{
						UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);       // store   fastbase,i0,i1,dword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                             // skip:
			}
		}
	}

	if (iswrite)
	{
		switch (size)
		{
			case 1:
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM); // write r0, r1, program_byte
				break;

			case 2:
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM); // write r0, r1, program_word
				break;

			case 4:
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);    // write r0, r1, program_dword
				break;
		}
	}
	else
	{
		switch (size)
		{
			case 1:
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);  // read r0, program_byte
				break;

			case 2:
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);  // read r0, program_word
				break;

			case 4:
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM); // read r0, program_dword
				break;
		}
	}

	UML_RET(block);                         // ret

	block.end();
}


