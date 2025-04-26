// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   sc61860.c
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner
 *
 * History of changes:
 * 29.7.2001 Several changes listed below taken by Mario Konegger
 *           (konegger@itp.tu-graz.ac.at)
 *           Added 0x7f to set_reg, to prevent p,q,r, overflow.
 *         Changed 512ms timerinterval from 256 to 128, thus the
 *         duration of one period is 512ms.
 *         Extended execute procudure with HLT-mode of CPU.
 *****************************************************************************/

#include "emu.h"
#include "sc61860.h"
#include "scdasm.h"


#define I 0
#define J 1
#define A 2
#define B 3
#define XL 4
#define XH 5
#define YL 6
#define YH 7
#define K 8
#define L 9
#define V 10 // some docus m
#define W 11 // some docus n
#define IA 92
#define IB 93
#define F0 94
#define C 95


//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SC61860, sc61860_device, "sc61860", "Sharp SC61860")


sc61860_device::sc61860_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SC61860, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_reset(*this, 0)
	, m_brk(*this, 0)
	, m_x(*this, 0)
	, m_ina(*this, 0)
	, m_outa(*this)
	, m_inb(*this, 0)
	, m_outb(*this)
	, m_outc(*this)
{
}

device_memory_interface::space_config_vector sc61860_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> sc61860_device::create_disassembler()
{
	return std::make_unique<sc61860_disassembler>();
}


uint8_t *sc61860_device::internal_ram()
{
	return m_ram;
}

TIMER_CALLBACK_MEMBER(sc61860_device::sc61860_2ms_tick)
{
	if (--m_timer.count == 0)
	{
		m_timer.count = 128;
		m_timer.t512ms = !m_timer.t512ms;
	}
	m_timer.t2ms = !m_timer.t2ms;
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "scops.hxx"
#include "sctable.hxx"

void sc61860_device::device_reset()
{
	m_timer.t2ms=0;
	m_timer.t512ms=0;
	m_timer.count=256;
	m_pc=0;
}

void sc61860_device::device_start()
{
	m_2ms_tick_timer = timer_alloc(FUNC(sc61860_device::sc61860_2ms_tick), this);
	m_2ms_tick_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	m_p = 0;
	m_q = 0;
	m_r = 0;
	m_c = 0;
	m_d = 0;
	m_h = 0;
	m_oldpc = 0;
	m_dp = 0;
	m_carry = 0;
	m_zero = 0;
	m_pc = 0;
	m_debugger_temp = 0;
	memset( m_ram, 0, sizeof(m_ram) );

	save_item(NAME(m_p));
	save_item(NAME(m_q));
	save_item(NAME(m_r));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_h));
	save_item(NAME(m_pc));
	save_item(NAME(m_dp));
	save_item(NAME(m_carry));
	save_item(NAME(m_zero));
	save_item(NAME(m_timer.t2ms));
	save_item(NAME(m_timer.t512ms));
	save_item(NAME(m_timer.count));
	save_item(NAME(m_ram));

	state_add( SC61860_PC,    "PC",    m_pc            ).formatstr("%04X");
	state_add( SC61860_DP,    "DP",    m_dp            ).formatstr("%04X");
	state_add( SC61860_P,     "P",     m_p             ).mask(0x7f).formatstr("%02X");
	state_add( SC61860_Q,     "Q",     m_q             ).mask(0x7f).formatstr("%02X");
	state_add( SC61860_R,     "R",     m_r             ).mask(0x7f).formatstr("%02X");
	state_add( SC61860_I,     "I",     m_ram[I]        ).formatstr("%02X");
	state_add( SC61860_J,     "J",     m_ram[J]        ).formatstr("%02X");
	state_add( SC61860_K,     "K",     m_ram[K]        ).formatstr("%02X");
	state_add( SC61860_L,     "L",     m_ram[L]        ).formatstr("%02X");
	state_add( SC61860_V,     "V",     m_ram[V]        ).formatstr("%02X");
	state_add( SC61860_W,     "Wx",    m_ram[W]        ).formatstr("%02X");
	state_add( SC61860_H,     "W",     m_h             ).formatstr("%02X");
	state_add( SC61860_BA,    "BA",    m_debugger_temp ).callimport().callexport().formatstr("%04X");
	state_add( SC61860_X,     "X",     m_debugger_temp ).callimport().callexport().formatstr("%04X");
	state_add( SC61860_Y,     "Y",     m_debugger_temp ).callimport().callexport().formatstr("%04X");
	state_add( SC61860_CARRY, "Carry", m_carry         ).mask(1).formatstr("%1u");
	state_add( SC61860_ZERO,  "Zero" , m_zero          ).mask(1).formatstr("%1u");

	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_oldpc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_debugger_temp).formatstr("%2s").noshow();

	set_icountptr(m_icount);
}


void sc61860_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c", m_zero ? 'Z' : '.', m_carry ? 'C' : '.');
			break;
	}
}


void sc61860_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SC61860_BA:
			m_ram[A] = m_debugger_temp & 0xff;
			m_ram[B] = ( m_debugger_temp >> 8 ) & 0xff;
			break;

		case SC61860_X:
			m_ram[XL] = m_debugger_temp & 0xff;
			m_ram[XH] = ( m_debugger_temp >> 8 ) & 0xff;
			break;

		case SC61860_Y:
			m_ram[YL] = m_debugger_temp & 0xff;
			m_ram[YH] = ( m_debugger_temp >> 8 ) & 0xff;
			break;
	}
}


void sc61860_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SC61860_BA:
			m_debugger_temp = ( m_ram[B] << 8 ) | m_ram[A];
			break;

		case SC61860_X:
			m_debugger_temp = ( m_ram[XH] << 8 ) | m_ram[XL];
			break;

		case SC61860_Y:
			m_debugger_temp = ( m_ram[YH] << 8 ) | m_ram[YL];
			break;
	}
}


void sc61860_device::execute_run()
{
	do
	{
		m_oldpc = m_pc;

		debugger_instruction_hook(m_pc);

		sc61860_instruction();

#if 0
		/* Are we in HLT-mode? */
		if (m_c & 4)
		{
			if (((m_ina()!=0)) || m_timer.t512ms)
			{
				m_c&=0xfb;
				m_outc(m_c);
			}
			m_icount-=4;
		}
		else if(m_c & 8) {}

		else sc61860_instruction();
#endif

	} while (m_icount > 0);
}
