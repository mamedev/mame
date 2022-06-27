// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
/*
Namco Custom 117, used in System 1

CUS117 is a simple MMU that provides a 23 bit virtual address space for a pair
of M6809 CPUs. The chip outputs the various enable lines for RAM, ROM, video
customs, etc., and bits 12-21 of the virtual address (bit 22 of the virtual
address is handled internally: it selects between ROM and everything else)

Each CPU's address space is evenly divided into eight 8KB banks, and each of
these banks can be directed to any portion of the virtual address space
(however, the last bank for each CPU is effectively read only, since writes to
E000-FFFF control CUS117 itself)

The main and sub CPUs share the same address and data bus, but each CPU can
set up its own banks independently. The main CPU can also set the sub CPU's
last bank (E000-FFFF, where the 6809 reset and interrupt vectors reside)

As well as being an MMU, CUS117 serves as the interrupt controller for the
two 6809s and as the reset generator for the entire system.
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "c117.h"


DEFINE_DEVICE_TYPE(NAMCO_C117, namco_c117_device, "namco_c117", "Namco C117 MMU")


//-------------------------------------------------
//  namco_c117_device - constructor
//-------------------------------------------------

namco_c117_device::namco_c117_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_C117, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_subres_cb(*this),
	m_program_config("program", ENDIANNESS_BIG, 8, 23),
	m_cpuexec{ { *this, finder_base::DUMMY_TAG }, { *this, finder_base::DUMMY_TAG } },
	m_watchdog(*this, "watchdog")
{
}

device_memory_interface::space_config_vector namco_c117_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c117_device::device_start()
{
	m_subres_cb.resolve_safe();

	space(AS_PROGRAM).specific(m_program);

	m_cpuexec[0]->space(AS_PROGRAM).cache(m_cpucache[0]);
	m_cpuexec[1]->space(AS_PROGRAM).cache(m_cpucache[1]);

	memset(&m_offsets, 0, sizeof(m_offsets));
	m_subres = m_wdog = 0;
	save_item(NAME(m_offsets));
	save_item(NAME(m_subres));
	save_item(NAME(m_wdog));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c117_device::device_reset()
{
	// default MMU setup for main CPU
	m_offsets[0][0] = 0x0180 * 0x2000; // bank0 = 0x180(RAM) - evidence: wldcourt
	m_offsets[0][1] = 0x0180 * 0x2000; // bank1 = 0x180(RAM) - evidence: berabohm
	m_offsets[0][7] = 0x03ff * 0x2000; // bank7 = 0x3ff(PRG7)

	// default MMU setup for sub CPU
	m_offsets[1][0] = 0x0180 * 0x2000; // bank0 = 0x180(RAM) - evidence: wldcourt
	m_offsets[1][7] = 0x03ff * 0x2000; // bank7 = 0x3ff(PRG7)

	m_subres = m_wdog = 0;
	m_subres_cb(ASSERT_LINE);

	// reset the main CPU so it picks up the reset vector from the correct bank
	m_cpuexec[0]->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_c117_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog);
}


uint8_t namco_c117_device::main_r(offs_t offset)
{
	return m_program.read_byte(remap(0, offset));
}

uint8_t namco_c117_device::sub_r(offs_t offset)
{
	return m_program.read_byte(remap(1, offset));
}

void namco_c117_device::main_w(offs_t offset, uint8_t data)
{
	if (offset < 0xe000)
		m_program.write_byte(remap(0, offset), data);
	else
		register_w(0, offset, data);
}

void namco_c117_device::sub_w(offs_t offset, uint8_t data)
{
	if (offset < 0xe000)
		m_program.write_byte(remap(1, offset), data);
	else
		register_w(1, offset, data);
}

// FIXME: the sound CPU watchdog is probably in CUS121, and definitely isn't in CUS117
// however, until the watchdog is a device and it's possible to have two independent
// watchdogs in a machine, it's easiest to handle it here
void namco_c117_device::sound_watchdog_w(uint8_t data)
{
	kick_watchdog(2);
}


void namco_c117_device::register_w(int whichcpu, offs_t offset, uint8_t data)
{
	int reg = (offset >> 9) & 0xf;
	bool unknown_reg = false;

	switch (reg)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			bankswitch(whichcpu, reg, offset & 1, data);
			break;
		case 8:  // F000 - SUBRES (halt/reset everything but main CPU)
			if (whichcpu == 0)
			{
				m_subres = data & 1;
				m_subres_cb(m_subres ? CLEAR_LINE : ASSERT_LINE);
			}
			else
				unknown_reg = true;
			break;
		case 9:  // F200 - kick watchdog
			kick_watchdog(whichcpu);
			break;
//      case 10: // F400 - unknown but used
//          break;
		case 11: // F600 - IRQ ack
			m_cpuexec[whichcpu]->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
			break;
		case 12: // F800 - FIRQ ack
			m_cpuexec[whichcpu]->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
			break;
		case 13: // FA00 - assert FIRQ on sub CPU
			if (whichcpu == 0)
				m_cpuexec[1]->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
			else
				unknown_reg = true;
			break;
		case 14: // FC00 - set initial ROM bank for sub CPU
			if (whichcpu == 0)
				m_offsets[1][7] = 0x600000 | (data * 0x2000);
			else
				unknown_reg = true;
			break;
		default:
			unknown_reg = true;
	}
	if (unknown_reg)
		logerror("'%s' writing to unknown CUS117 register %04X = %02X\n", m_cpuexec[whichcpu].finder_tag(), offset, data);
}

void namco_c117_device::bankswitch(int whichcpu, int whichbank, int a0, uint8_t data)
{
	uint32_t &bank = m_offsets[whichcpu][whichbank];

	// even writes set a22-a21; odd writes set a20-a13
	if (a0 == 0)
		bank = (bank & 0x1fe000) | ((data & 0x03) * 0x200000);
	else
		bank = (bank & 0x600000) | (data * 0x2000);
}

void namco_c117_device::kick_watchdog(int whichcpu)
{
	// FIXME: change to 3 once sound CPU watchdog is separated from this device
	static const int ALL_CPU_MASK = 7;

	m_wdog |= (1 << whichcpu);

	if (m_wdog == ALL_CPU_MASK || !m_subres)
	{
		m_wdog = 0;
		m_watchdog->watchdog_reset();
	}
}
